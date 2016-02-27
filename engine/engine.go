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
	engine   *C.struct__php_engine
	contexts map[*C.struct__engine_context]*Context
}

// This contains a reference to the active engine, if any.
var engine *Engine

// New initializes a PHP engine instance on which contexts can be executed. It
// corresponds to PHP's MINIT (module init) phase.
func New() (*Engine, error) {
	if engine != nil {
		return nil, fmt.Errorf("Cannot activate multiple engine instances")
	}

	ptr, err := C.engine_init()
	if err != nil {
		return nil, fmt.Errorf("PHP engine failed to initialize")
	}

	engine = &Engine{
		engine:   ptr,
		contexts: make(map[*C.struct__engine_context]*Context),
	}

	return engine, nil
}

// NewContext creates a new execution context for the active engine and returns
// an error if the execution context failed to initialize at any point. This
// corresponds to PHP's RINIT (request init) phase.
func (e *Engine) NewContext() (*Context, error) {
	ptr, err := C.context_new()
	if err != nil {
		return nil, fmt.Errorf("Failed to initialize context for PHP engine")
	}

	ctx := &Context{
		Header:    make(http.Header),
		context:   ptr,
		values:    make([]*Value, 0),
		receivers: make(map[string]*Receiver),
	}

	// Store reference to context, using pointer as key.
	e.contexts[ptr] = ctx

	return ctx, nil
}

// Destroy shuts down and frees any resources related to the PHP engine bindings.
func (e *Engine) Destroy() {
	if e.engine == nil {
		return
	}

	for _, c := range e.contexts {
		c.Destroy()
	}

	e.contexts = nil

	C.engine_shutdown(e.engine)
	e.engine = nil

	engine = nil
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
func engineWriteOut(ctx *C.struct__engine_context, buffer unsafe.Pointer, length C.uint) C.int {
	if engine == nil || engine.contexts[ctx] == nil {
		return -1
	}

	return write(engine.contexts[ctx].Output, buffer, length)
}

//export engineWriteLog
func engineWriteLog(ctx *C.struct__engine_context, buffer unsafe.Pointer, length C.uint) C.int {
	if engine == nil || engine.contexts[ctx] == nil {
		return -1
	}

	return write(engine.contexts[ctx].Log, buffer, length)
}

//export engineSetHeader
func engineSetHeader(ctx *C.struct__engine_context, operation C.uint, buffer unsafe.Pointer, length C.uint) {
	if engine == nil || engine.contexts[ctx] == nil {
		return
	}

	header := (string)(C.GoBytes(buffer, C.int(length)))
	split := strings.SplitN(header, ":", 2)

	for i := range split {
		split[i] = strings.TrimSpace(split[i])
	}

	switch operation {
	case 0: // Replace header.
		if len(split) == 2 && split[1] != "" {
			engine.contexts[ctx].Header.Set(split[0], split[1])
		}
	case 1: // Append header.
		if len(split) == 2 && split[1] != "" {
			engine.contexts[ctx].Header.Add(split[0], split[1])
		}
	case 2: // Delete header.
		if split[0] != "" {
			engine.contexts[ctx].Header.Del(split[0])
		}
	}
}
