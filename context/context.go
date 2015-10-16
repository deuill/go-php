// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

// Package context contains methods related to PHP engine execution contexts. It
// allows for binding Go variables and executing PHP scripts as a single request.
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

	"github.com/deuill/go-php/value"
)

// Context represents an individual execution context.
type Context struct {
	context *C.struct__engine_context
	writer  io.Writer
	values  map[string]*value.Value
}

// Bind allows for binding Go values into the current execution context under
// a certain name. Bind returns an error if attempting to bind an invalid value
// (check the documentation for value.New for what is considered to be a "valid"
// value).
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

// Exec executes a PHP script pointed to by filename in the current execution
// context, and returns an error, if any. Output produced by the script is
// written to the context's pre-defined io.Writer instance.
func (c *Context) Exec(filename string) error {
	f := C.CString(filename)
	defer C.free(unsafe.Pointer(f))

	_, err := C.context_exec(c.context, f)
	if err != nil {
		return fmt.Errorf("Error executing script '%s' in context", filename)
	}

	return nil
}

// Eval executes the PHP script contained in script, and follows the same
// semantics as the Exec function.
func (c *Context) Eval(script string) error {
	s := C.CString(script)
	defer C.free(unsafe.Pointer(s))

	_, err := C.context_eval(c.context, s)
	if err != nil {
		return fmt.Errorf("Error executing script '%s' in context", script)
	}

	return nil
}

// Destroy tears down the current execution context along with any active value
// bindings for that context.
func (c *Context) Destroy() {
	for _, v := range c.values {
		v.Destroy()
	}

	if c.context != nil {
		C.context_destroy(c.context)
		c.context = nil
	}
}

// New creates a new execution context, passing all script output into w. It
// returns an error if the execution context failed to initialize at any point.
func New(w io.Writer) (*Context, error) {
	ctx := &Context{writer: w, values: make(map[string]*value.Value)}

	ptr, err := C.context_new(unsafe.Pointer(ctx))
	if err != nil {
		return nil, fmt.Errorf("Failed to initialize context for PHP engine")
	}

	ctx.context = ptr

	return ctx, nil
}

//export contextWrite
func contextWrite(ctxptr unsafe.Pointer, buffer unsafe.Pointer, length C.uint) C.int {
	c := (*Context)(ctxptr)

	written, err := c.writer.Write(C.GoBytes(buffer, C.int(length)))
	if err != nil {
		return C.int(-1)
	}

	return C.int(written)
}
