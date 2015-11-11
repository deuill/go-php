// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

// Package value implements value transformation between Go and PHP contexts.
package value

// #cgo CFLAGS: -I/usr/include/php -I/usr/include/php/main -I/usr/include/php/TSRM
// #cgo CFLAGS: -I/usr/include/php/Zend
// #cgo LDFLAGS: -lphp5
//
// #include <stdlib.h>
// #include <stdbool.h>
// #include <main/php.h>
// #include "value.h"
import "C"

import (
	"fmt"
	"reflect"
	"unsafe"
)

var errInvalidType = func(v interface{}) error {
	return fmt.Errorf("Cannot create value of unknown type '%T'", v)
}

// Kind represents the specific kind of type represented in Value.
type Kind int

const (
	Null   Kind = iota // PHP null value
	Long               // PHP long integer
	Double             // PHP double floating point number
	Bool               // PHP boolean
	Array              // PHP indexed array
	Object             // PHP object
	String             // PHP string
	Map                // PHP associative array
)

// Value represents a PHP value.
type Value struct {
	value *C.struct__engine_value
}

// New creates a PHP value representation of a Go value val. Available bindings
// for Go to PHP types are:
//
//	int             -> integer
//	float64         -> double
//	bool            -> boolean
//	string          -> string
//	slice           -> indexed array
//	map[int|string] -> associative array
//	struct          -> object
//
// It is only possible to bind maps with integer or string keys. Only exported
// struct fields are passed to the PHP context. Bindings for functions and method
// receivers to PHP functions and classes are only available in the engine scope,
// and must be predeclared before context execution.
func New(val interface{}) (*Value, error) {
	var ptr *C.struct__engine_value
	var err error

	// Determine value type and create PHP value from the concrete type.
	v := reflect.ValueOf(val)
	switch v.Kind() {
	// Bind integer to PHP int type.
	case reflect.Int:
		ptr, err = C.value_create_long(C.long(v.Int()))
	// Bind floating point number to PHP double type.
	case reflect.Float64:
		ptr, err = C.value_create_double(C.double(v.Float()))
	// Bind boolean to PHP bool type.
	case reflect.Bool:
		ptr, err = C.value_create_bool(C.bool(v.Bool()))
	// Bind string to PHP string type.
	case reflect.String:
		str := C.CString(v.String())

		ptr, err = C.value_create_string(str)
		C.free(unsafe.Pointer(str))
	// Bind slice to PHP indexed array type.
	case reflect.Slice:
		if ptr, err = C.value_create_array(C.uint(v.Len())); err != nil {
			break
		}

		for i := 0; i < v.Len(); i++ {
			vs, err := New(v.Index(i).Interface())
			if err != nil {
				C.value_destroy(ptr)
				return nil, err
			}

			C.value_array_set_next(ptr, vs.value)
		}
	// Bind map (with integer or string keys) to PHP associative array type.
	case reflect.Map:
		kt := v.Type().Key().Kind()

		if kt == reflect.Int || kt == reflect.String {
			if ptr, err = C.value_create_array(C.uint(v.Len())); err != nil {
				break
			}

			for _, key := range v.MapKeys() {
				kv, err := New(v.MapIndex(key).Interface())
				if err != nil {
					C.value_destroy(ptr)
					return nil, err
				}

				if kt == reflect.Int {
					C.value_array_set_index(ptr, C.ulong(key.Int()), kv.value)
				} else {
					str := C.CString(key.String())

					C.value_array_set_key(ptr, str, kv.value)
					C.free(unsafe.Pointer(str))
				}
			}
		} else {
			return nil, errInvalidType(val)
		}
	// Bind struct to PHP object (stdClass) type.
	case reflect.Struct:
		vt := v.Type()
		if ptr, err = C.value_create_object(); err != nil {
			break
		}

		for i := 0; i < v.NumField(); i++ {
			// Skip unexported fields.
			if vt.Field(i).PkgPath != "" {
				continue
			}

			fv, err := New(v.Field(i).Interface())
			if err != nil {
				C.value_destroy(ptr)
				return nil, err
			}

			str := C.CString(vt.Field(i).Name)

			C.value_object_add_property(ptr, str, fv.value)
			C.free(unsafe.Pointer(str))
		}
	default:
		return nil, errInvalidType(val)
	}

	if err != nil {
		return nil, fmt.Errorf("Unable to create PHP value from Go value '%v'", val)
	}

	return &Value{value: ptr}, nil
}

// NewFromPtr creates a Value type from an existing PHP value pointer.
func NewFromPtr(val unsafe.Pointer) (*Value, error) {
	if val == nil {
		return nil, fmt.Errorf("Cannot create value from 'nil' pointer")
	}

	v, err := C.value_new((*C.zval)(val))
	if err != nil {
		return nil, fmt.Errorf("Unable to create PHP value from pointer")
	}

	return &Value{value: v}, nil
}

// Kind returns the Value's concrete kind of type.
func (v *Value) Kind() Kind {
	return (Kind)(C.value_kind(v.value))
}

// Interface returns the internal PHP value as it lies, with no conversion step.
// Attempting to call this method on an object value will return an error, as
// conversion from objects to structs requires a struct definition to extract to.
func (v *Value) Interface() (interface{}, error) {
	switch v.Kind() {
	case Long:
		return v.Int(), nil
	case Double:
		return v.Float(), nil
	case Bool:
		return v.Bool(), nil
	case String:
		return v.String(), nil
	case Array:
		return v.Slice(), nil
	case Object:
		return nil, fmt.Errorf("Unable to return object value as interface")
	}

	return nil, nil
}

// Int returns the internal PHP value as an integer, converting if necessary.
func (v *Value) Int() int64 {
	return (int64)(C.value_get_long(v.value))
}

// Float returns the internal PHP value as a floating point number, converting
// if necessary.
func (v *Value) Float() float64 {
	return (float64)(C.value_get_double(v.value))
}

// Bool returns the internal PHP value as a boolean, converting if necessary.
func (v *Value) Bool() bool {
	return (bool)(C.value_get_bool(v.value))
}

// String returns the internal PHP value as a string, converting if necessary.
func (v *Value) String() string {
	return C.GoString(C.value_get_string(v.value))
}

// Slice returns the internal PHP value as a slice of Value types, converting if
// necessary.
func (v *Value) Slice() []*Value {
	size := (int)(C.value_array_size(v.value))
	slice := make([]*Value, size)

	for i := 0; i < size; i++ {
		slice[i] = &Value{value: C.value_array_get_index(v.value, C.ulong(i))}
	}

	return slice
}

// Ptr returns a pointer to the internal PHP value, and is mostly used for
// passing to C functions.
func (v *Value) Ptr() unsafe.Pointer {
	return unsafe.Pointer(v.value)
}

// Destroy removes all active references to the internal PHP value and frees
// any resources used.
func (v *Value) Destroy() {
	if v.value != nil {
		C.value_destroy(v.value)
		v.value = nil
	}
}
