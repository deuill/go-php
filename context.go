package php

// #include <stdio.h>
// #include <stdlib.h>
// #include "engine.h"
// #include "context.h"
import "C"

import (
	"fmt"
	"io"
	"unsafe"
)

type Context struct {
	context *C.struct__engine_context
	writer  io.Writer
	values  map[string]*Value
}

func (c *Context) Bind(name string, value interface{}) error {
	v, err := NewValue(value)
	if err != nil {
		return err
	}

	n := C.CString(name)
	defer C.free(unsafe.Pointer(n))

	if _, err = C.context_bind(c.context, n, v.Ptr()); err != nil {
		v.Destroy()
		return fmt.Errorf("Binding value '%v' to context failed", value)
	}

	c.values[name] = v

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
	for _, v := range c.values {
		v.Destroy()
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
