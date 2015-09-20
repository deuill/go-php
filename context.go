package php

// #include <stdio.h>
// #include <stdlib.h>
// #include "engine.h"
// #include "context.h"
// #include "value.h"
import "C"

import (
	"fmt"
	"io"
	"unsafe"
)

type Context struct {
	context *C.struct__engine_context
	writer  io.Writer
	zvals   map[string]unsafe.Pointer
}

func (c *Context) Bind(name string, value interface{}) error {
	var zval unsafe.Pointer
	var err error

	switch v := value.(type) {
	case int:
		zval, err = C.value_long(C.long(v))
	case float64:
		zval, err = C.value_double(C.double(v))
	case string:
		str := C.CString(v)
		defer C.free(unsafe.Pointer(str))

		zval, err = C.value_string(str)
	default:
		return fmt.Errorf("Cannot bind unknown type '%T'", v)
	}

	if err != nil {
		return fmt.Errorf("Binding value '%v' to context failed", value)
	}

	n := C.CString(name)
	defer C.free(unsafe.Pointer(n))

	if _, err = C.context_bind(c.context, n, zval); err != nil {
		C.value_destroy(zval)
		return fmt.Errorf("Binding value '%v' to context failed", value)
	}

	c.zvals[name] = zval

	return nil
}

func (c *Context) Exec(filename string) error {
	name := C.CString(filename)
	defer C.free(unsafe.Pointer(name))

	_, err := C.context_exec(c.context, name)
	if err != nil {
		return fmt.Errorf("Error executing script '%s' in context", filename)
	}

	return nil
}

func (c *Context) Destroy() {
	for _, zval := range c.zvals {
		C.value_destroy(zval)
	}

	C.context_destroy(c.context)
	c = nil
}

//export contextWrite
func contextWrite(ctxptr unsafe.Pointer, buffer unsafe.Pointer, length C.uint) C.int {
	context := (*Context)(ctxptr)

	written, err := context.writer.Write(C.GoBytes(buffer, C.int(length)))
	if err != nil {
		return C.int(-1)
	}

	return C.int(written)
}
