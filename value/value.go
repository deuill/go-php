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
// #include "value.h"
import "C"

import (
	"errors"
	"reflect"
	"unsafe"
)

var errInvalidType = errors.New("Cannot create value of unknown type")

// Value represents a PHP value.
type Value struct {
	value unsafe.Pointer
}

// Ptr returns a pointer to the internal PHP value, and is mostly used for
// passing to C functions.
func (v *Value) Ptr() unsafe.Pointer {
	return v.value
}

// Destroy removes all active references to the internal PHP value and frees
// any resources used.
func (v *Value) Destroy() {
	if v.value != nil {
		C.value_destroy(v.value)
		v.value = nil
	}
}

// New creates a PHP value representtion of a Go value val. Available bindings
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
	var ptr unsafe.Pointer

	// Determine value type and create PHP value from the concrete type.
	v := reflect.ValueOf(val)
	switch v.Kind() {
	// Bind integer to PHP int type.
	case reflect.Int:
		ptr = C.value_create_long(C.long(v.Int()))
	// Bind floating point number to PHP double type.
	case reflect.Float64:
		ptr = C.value_create_double(C.double(v.Float()))
	// Bind boolean to PHP bool type.
	case reflect.Bool:
		ptr = C.value_create_bool(C.bool(v.Bool()))
	// Bind string to PHP string type.
	case reflect.String:
		str := C.CString(v.String())

		ptr = C.value_create_string(str)
		C.free(unsafe.Pointer(str))
	// Bind slice to PHP indexed array type.
	case reflect.Slice:
		ptr = C.value_create_array(C.uint(v.Len()))

		for i := 0; i < v.Len(); i++ {
			vs, err := New(v.Index(i).Interface())
			if err != nil {
				C.value_destroy(ptr)
				return nil, err
			}

			C.value_array_set_index(ptr, C.ulong(i), vs.Ptr())
		}
	// Bind map (with integer or string keys) to PHP associative array type.
	case reflect.Map:
		kt := v.Type().Key().Kind()

		if kt == reflect.Int || kt == reflect.String {
			ptr = C.value_create_array(C.uint(v.Len()))

			for _, key := range v.MapKeys() {
				kv, err := New(v.MapIndex(key).Interface())
				if err != nil {
					C.value_destroy(ptr)
					return nil, err
				}

				if kt == reflect.Int {
					C.value_array_set_index(ptr, C.ulong(key.Int()), kv.Ptr())
				} else {
					str := C.CString(key.String())

					C.value_array_set_key(ptr, str, kv.Ptr())
					C.free(unsafe.Pointer(str))
				}
			}
		} else {
			return nil, errInvalidType
		}
	// Bind struct to PHP object (stdClass) type.
	case reflect.Struct:
		vt := v.Type()
		ptr = C.value_create_object()

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

			C.value_object_add_property(ptr, str, fv.Ptr())
			C.free(unsafe.Pointer(str))
		}
	default:
		return nil, errInvalidType
	}

	return &Value{value: ptr}, nil
}
