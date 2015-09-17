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
	C.context_destroy(c.context)
	c = nil
}

//export contextWrite
func contextWrite(ctxptr unsafe.Pointer, buffer unsafe.Pointer, length C.int) C.int {
	context := (*Context)(ctxptr)

	written, err := context.writer.Write(C.GoBytes(buffer, length))
	if err != nil {
		return C.int(0)
	}

	return C.int(written)
}
