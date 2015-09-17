package php

// #include "value.h"
import "C"

import (
	"fmt"
	"reflect"
	"runtime"
	"unsafe"
)

type Value struct {
	val  unsafe.Pointer
	kind reflect.Kind
}

func (v *Value) Int() int64 {
	return 0
}

func (v *Value) Float() float64 {
	return 0.0
}

func (v *Value) Bool() bool {
	return false
}

func (v *Value) String() string {
	return ""
}

func (v *Value) Kind() reflect.Kind {
	var k reflect.Kind = reflect.String
	return k
}

func NewValue(ptr unsafe.Pointer) (*Value, error) {
	if ptr == nil {
		return nil, fmt.Errorf("Attempted to initialize value with nil pointer")
	}

	val := &Value{
		val: ptr,
	}

	runtime.SetFinalizer(val, func(v *Value) {
		C.value_destroy(v.val)
	})

	return val, nil
}
