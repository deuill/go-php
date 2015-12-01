// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

// Package context contains methods related to PHP engine execution contexts. It
// allows for binding Go variables and executing PHP scripts as a single request.
package context

// #cgo CFLAGS: -I/usr/include/php -I/usr/include/php/main -I/usr/include/php/TSRM
// #cgo CFLAGS: -I/usr/include/php/Zend -I../value
// #cgo LDFLAGS: -lphp5
//
// #include <stdlib.h>
// #include "context.h"
import "C"

import (
	"fmt"
	"io"
	"net/http"
	"strings"
	"unsafe"

	"github.com/deuill/go-php/value"
)

// Context represents an individual execution context.
type Context struct {
	// Output is the unbuffered writer used for regular script output. If left
	// unset, any data written by the calling context will be lost.
	Output io.Writer

	context *C.struct__engine_context
	header  http.Header
	values  map[string]*value.Value
}

// New creates a new execution context, passing all script output into w. It
// returns an error if the execution context failed to initialize at any point.
func New() (*Context, error) {
	ctx := &Context{
		header: make(http.Header),
		values: make(map[string]*value.Value),
	}

	ptr, err := C.context_new(unsafe.Pointer(ctx))
	if err != nil {
		return nil, fmt.Errorf("Failed to initialize context for PHP engine")
	}

	ctx.context = ptr

	return ctx, nil
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

// Eval executes the PHP expression contained in script, and returns a Value
// containing the PHP value returned by the expression, if any. Any output
// produced is written context's pre-defined io.Writer instance.
func (c *Context) Eval(script string) (*value.Value, error) {
	// When PHP compiles code with a non-NULL return value expected, it simply
	// prepends a `return` call to the code, thus breaking simple scripts that
	// would otherwise work. Thus, we need to wrap the code in a closure, and
	// call it immediately.
	s := C.CString("call_user_func(function(){" + script + "});")
	defer C.free(unsafe.Pointer(s))

	vptr, err := C.context_eval(c.context, s)
	if err != nil {
		return nil, fmt.Errorf("Error executing script '%s' in context", script)
	}

	val, err := value.NewFromPtr(vptr)
	if err != nil {
		return nil, err
	}

	return val, nil
}

// Header returns the HTTP headers set by current PHP context.
func (c *Context) Header() http.Header {
	return c.header
}

// Destroy tears down the current execution context along with any active value
// bindings for that context.
func (c *Context) Destroy() {
	for _, v := range c.values {
		v.Destroy()
	}

	c.values = nil

	if c.context != nil {
		C.context_destroy(c.context)
		c.context = nil
	}
}

//export contextWrite
func contextWrite(ctxptr unsafe.Pointer, buffer unsafe.Pointer, length C.uint) C.int {
	c := (*Context)(ctxptr)

	// Abort with no error if output io.Writer is unset.
	if c.Output == nil {
		return C.int(length)
	}

	written, err := c.Output.Write(C.GoBytes(buffer, C.int(length)))
	if err != nil {
		return C.int(-1)
	}

	return C.int(written)
}

//export contextHeader
func contextHeader(ctxptr unsafe.Pointer, operation C.uint, buffer unsafe.Pointer, length C.uint) {
	c := (*Context)(ctxptr)

	header := (string)(C.GoBytes(buffer, C.int(length)))
	split := strings.SplitN(header, ":", 2)

	for i := range split {
		split[i] = strings.TrimSpace(split[i])
	}

	switch operation {
	case 0: // Replace header.
		if len(split) == 2 && split[1] != "" {
			c.header.Set(split[0], split[1])
		}
	case 1: // Append header.
		if len(split) == 2 && split[1] != "" {
			c.header.Add(split[0], split[1])
		}
	case 2: // Delete header.
		if split[0] != "" {
			c.header.Del(split[0])
		}
	}
}
