// Copyright 2016 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

// Package engine provides methods allowing for the initialization and teardown
// of PHP engine bindings, off which execution contexts can be launched.
package engine

// #cgo CFLAGS: -I/usr/include/php -I/usr/include/php/main -I/usr/include/php/TSRM
// #cgo CFLAGS: -I/usr/include/php/Zend -Iinclude
//
// #include <stdlib.h>
// #include <main/php.h>
// #include "context.h"
// #include "engine.h"
import "C"

import (
	"fmt"
	"io"
	"net/http"
	"strings"
	"unsafe"
)

// Engine represents the core PHP engine bindings.
type Engine struct {
	engine    *C.struct__php_engine
	contexts  []*Context
	receivers map[string]*Receiver
}

// New initializes a PHP engine instance on which contexts can be executed. It
// corresponds to PHP's MINIT (module init) phase.
func New() (*Engine, error) {
	ptr, err := C.engine_init()
	if err != nil {
		return nil, fmt.Errorf("PHP engine failed to initialize")
	}

	e := &Engine{
		engine:    ptr,
		contexts:  make([]*Context, 0),
		receivers: make(map[string]*Receiver),
	}

	return e, nil
}

// NewContext creates a new execution context for the active engine and returns
// an error if the execution context failed to initialize at any point. This
// corresponds to PHP's RINIT (request init) phase.
func (e *Engine) NewContext() (*Context, error) {
	ctx := &Context{
		Header: make(http.Header),
		values: make([]*Value, 0),
	}

	ptr, err := C.context_new(unsafe.Pointer(ctx))
	if err != nil {
		return nil, fmt.Errorf("Failed to initialize context for PHP engine")
	}

	ctx.context = ptr
	e.contexts = append(e.contexts, ctx)

	return ctx, nil
}

// Define registers a PHP class under the name passed, using fn as the class
// constructor.
func (e *Engine) Define(name string, fn func(args []interface{}) interface{}) error {
	if _, exists := e.receivers[name]; exists {
		return fmt.Errorf("Failed to define duplicate receiver '%s'", name)
	}

	rcvr, err := NewReceiver(name, fn)
	if err != nil {
		return err
	}

	e.receivers[name] = rcvr

	return nil
}

// Destroy shuts down and frees any resources related to the PHP engine bindings.
func (e *Engine) Destroy() {
	if e.engine == nil {
		return
	}

	for _, r := range e.receivers {
		r.Destroy()
	}

	e.receivers = nil

	for _, c := range e.contexts {
		c.Destroy()
	}

	e.contexts = nil

	C.engine_shutdown(e.engine)
	e.engine = nil
}

func write(w io.Writer, buffer unsafe.Pointer, length C.uint) C.int {
	// Do not return error if writer is unavailable.
	if w == nil {
		return C.int(length)
	}

	written, err := w.Write(C.GoBytes(buffer, C.int(length)))
	if err != nil {
		return -1
	}

	return C.int(written)
}

//export engineWriteOut
func engineWriteOut(ctxptr, buffer unsafe.Pointer, length C.uint) C.int {
	c := (*Context)(ctxptr)

	return write(c.Output, buffer, length)
}

//export engineWriteLog
func engineWriteLog(ctxptr unsafe.Pointer, buffer unsafe.Pointer, length C.uint) C.int {
	c := (*Context)(ctxptr)

	return write(c.Log, buffer, length)
}

//export engineSetHeader
func engineSetHeader(ctxptr unsafe.Pointer, operation C.uint, buffer unsafe.Pointer, length C.uint) {
	c := (*Context)(ctxptr)

	header := (string)(C.GoBytes(buffer, C.int(length)))
	split := strings.SplitN(header, ":", 2)

	for i := range split {
		split[i] = strings.TrimSpace(split[i])
	}

	switch operation {
	case 0: // Replace header.
		if len(split) == 2 && split[1] != "" {
			c.Header.Set(split[0], split[1])
		}
	case 1: // Append header.
		if len(split) == 2 && split[1] != "" {
			c.Header.Add(split[0], split[1])
		}
	case 2: // Delete header.
		if split[0] != "" {
			c.Header.Del(split[0])
		}
	}
}
