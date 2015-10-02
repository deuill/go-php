package context

// #cgo CFLAGS: -I/usr/include/php -I/usr/include/php/main -I/usr/include/php/TSRM
// #cgo CFLAGS: -I/usr/include/php/Zend
// #cgo LDFLAGS: -lphp5
//
// #include <stdlib.h>
// #include "context.h"
import "C"

import (
	"fmt"
	"io"
	"unsafe"

	"../value"
)

type Context struct {
	context *C.struct__engine_context
	writer  io.Writer
	values  map[string]*value.Value
}

func (c *Context) Bind(name string, val interface{}) error {
	v, err := value.New(val)
	if err != nil {
		return err
	}

	n := C.CString(name)
	defer C.free(unsafe.Pointer(n))

	if _, err = C.context_bind(c.context, n, v.Ptr()); err != nil {
		v.Destroy()
		return fmt.Errorf("Binding value '%v' to context failed", val)
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

func (c *Context) Write(p []byte) (int, error) {
	return c.writer.Write(p)
}

func (c *Context) Destroy() {
	for _, v := range c.values {
		v.Destroy()
	}

	C.context_destroy(c.context)
	c = nil
}

func New(w io.Writer) (*Context, error) {
	ctx := &Context{writer: w, values: make(map[string]*value.Value)}

	ptr, err := C.context_new(unsafe.Pointer(ctx))
	if err != nil {
		return nil, fmt.Errorf("Failed to initialize context for PHP engine")
	}

	ctx.context = ptr

	return ctx, nil
}
