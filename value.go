package php

// #include <stdlib.h>
// #include "value.h"
import "C"

import (
	"fmt"
	"unsafe"
)

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

func NewValue(v interface{}) (*Value, error) {
	var ptr unsafe.Pointer
	var err error

	switch v := v.(type) {
	case int:
		ptr, err = C.value_long(C.long(v))
	case float64:
		ptr, err = C.value_double(C.double(v))
	case string:
		str := C.CString(v)
		defer C.free(unsafe.Pointer(str))

		ptr, err = C.value_string(str)
	default:
		return nil, fmt.Errorf("Cannot create value of unknown type '%T'", v)
	}

	if err != nil {
		return nil, fmt.Errorf("Creating value from '%v' failed", v)
	}

	return &Value{value: ptr}, nil
}
