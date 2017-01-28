// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

// Package engine provides methods allowing for the initialization and teardown
// of PHP engine bindings, off which execution contexts can be launched.
package php

// #cgo CFLAGS: -I/usr/include/php -I/usr/include/php/main -I/usr/include/php/TSRM
// #cgo CFLAGS: -I/usr/include/php/Zend -Iinclude
//
// #include <stdlib.h>
// #include <main/php.h>
// #include "receiver.h"
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
	contexts  map[*C.struct__engine_context]*Context
	receivers map[string]*Receiver
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
		engine:    ptr,
		contexts:  make(map[*C.struct__engine_context]*Context),
		receivers: make(map[string]*Receiver),
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
		Header:  make(http.Header),
		context: ptr,
		values:  make([]*Value, 0),
	}

	// Store reference to context, using pointer as key.
	e.contexts[ptr] = ctx

	return ctx, nil
}

// Define registers a PHP class for the name passed, using function fn as
// constructor for individual object instances as needed by the PHP context.
//
// The class name registered is assumed to be unique for the active engine.
//
// The constructor function accepts a slice of arguments, as passed by the PHP
// context, and should return a method receiver instance, or nil on error (in
// which case, an exception is thrown on the PHP object constructor).
func (e *Engine) Define(name string, fn func(args []interface{}) interface{}) error {
	if _, exists := e.receivers[name]; exists {
		return fmt.Errorf("Failed to define duplicate receiver '%s'", name)
	}

	rcvr := &Receiver{
		name:    name,
		create:  fn,
		objects: make(map[*C.struct__engine_receiver]*ReceiverObject),
	}

	n := C.CString(name)
	defer C.free(unsafe.Pointer(n))

	C.receiver_define(n)
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

//export engineReceiverNew
func engineReceiverNew(rcvr *C.struct__engine_receiver, args unsafe.Pointer) C.int {
	n := C.GoString(C._receiver_get_name(rcvr))
	if engine == nil || engine.receivers[n] == nil {
		return 1
	}

	va, err := NewValueFromPtr(args)
	if err != nil {
		return 1
	}

	defer va.Destroy()

	obj, err := engine.receivers[n].NewObject(va.Slice())
	if err != nil {
		return 1
	}

	engine.receivers[n].objects[rcvr] = obj

	return 0
}

//export engineReceiverGet
func engineReceiverGet(rcvr *C.struct__engine_receiver, name *C.char) unsafe.Pointer {
	n := C.GoString(C._receiver_get_name(rcvr))
	if engine == nil || engine.receivers[n].objects[rcvr] == nil {
		return nil
	}

	val, err := engine.receivers[n].objects[rcvr].Get(C.GoString(name))
	if err != nil {
		return nil
	}

	return val.Ptr()
}

//export engineReceiverSet
func engineReceiverSet(rcvr *C.struct__engine_receiver, name *C.char, val unsafe.Pointer) {
	n := C.GoString(C._receiver_get_name(rcvr))
	if engine == nil || engine.receivers[n].objects[rcvr] == nil {
		return
	}

	v, err := NewValueFromPtr(val)
	if err != nil {
		return
	}

	engine.receivers[n].objects[rcvr].Set(C.GoString(name), v.Interface())
}

//export engineReceiverExists
func engineReceiverExists(rcvr *C.struct__engine_receiver, name *C.char) C.int {
	n := C.GoString(C._receiver_get_name(rcvr))
	if engine == nil || engine.receivers[n].objects[rcvr] == nil {
		return 0
	}

	if engine.receivers[n].objects[rcvr].Exists(C.GoString(name)) {
		return 1
	}

	return 0
}

//export engineReceiverCall
func engineReceiverCall(rcvr *C.struct__engine_receiver, name *C.char, args unsafe.Pointer) unsafe.Pointer {
	n := C.GoString(C._receiver_get_name(rcvr))
	if engine == nil || engine.receivers[n].objects[rcvr] == nil {
		return nil
	}

	// Process input arguments.
	va, err := NewValueFromPtr(args)
	if err != nil {
		return nil
	}

	defer va.Destroy()

	val := engine.receivers[n].objects[rcvr].Call(C.GoString(name), va.Slice())
	if val == nil {
		return nil
	}

	return val.Ptr()
}
