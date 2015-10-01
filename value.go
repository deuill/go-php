package php

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

type Value struct {
	value unsafe.Pointer
}

func (v *Value) Ptr() unsafe.Pointer {
	return v.value
}

func (v *Value) Destroy() {
	C.value_destroy(v.value)
	v = nil
}

func NewValue(val interface{}) (*Value, error) {
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
		defer C.free(unsafe.Pointer(str))

		ptr = C.value_create_string(str)
	// Bind slice to PHP indexed array type.
	case reflect.Slice:
		ptr = C.value_create_array(C.uint(v.Len()))

		for i := 0; i < v.Len(); i++ {
			vs, err := NewValue(v.Index(i).Interface())
			if err != nil {
				return nil, err
			}

			C.value_array_set_index(ptr, C.ulong(i), vs.Ptr())
		}
	// Bind map (with integer or string keys) to PHP associative array type.
	case reflect.Map:
		t := v.Type().Key().Kind()

		if t == reflect.Int || t == reflect.String {
			ptr = C.value_create_array(C.uint(v.Len()))

			for _, k := range v.MapKeys() {
				vm, err := NewValue(v.MapIndex(k).Interface())
				if err != nil {
					return nil, err
				}

				if t == reflect.Int {
					C.value_array_set_index(ptr, C.ulong(k.Int()), vm.Ptr())
				} else {
					str := C.CString(k.String())

					C.value_array_set_key(ptr, str, vm.Ptr())
					C.free(unsafe.Pointer(str))
				}
			}
		} else {
			return nil, errInvalidType
		}
	default:
		return nil, errInvalidType
	}

	return &Value{value: ptr}, nil
}
