// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

// Package engine provides methods allowing for the initialization and teardown
// of PHP engine bindings, off which execution contexts can be launched.
package engine

// #cgo CFLAGS: -I/usr/include/php -I/usr/include/php/main -I/usr/include/php/TSRM
// #cgo CFLAGS: -I/usr/include/php/Zend -I../value -I../context
// #cgo LDFLAGS: -L${SRCDIR}/value -L${SRCDIR}/context -lphp5
//
// #include <stdlib.h>
// #include <stdbool.h>
// #include <main/php.h>
//
// #include "context.h"
// #include "engine.h"
// #include "class.h"
import "C"

import (
	"fmt"
	"reflect"
	"unsafe"

	"github.com/deuill/go-php/context"
)

// Receiver represents a method receiver.
type Receiver struct {
	values  map[string]reflect.Value
	methods map[string]reflect.Value
}

// Engine represents the core PHP engine bindings.
type Engine struct {
	engine   *C.struct__php_engine
	contexts []*context.Context
}

// New initializes a PHP engine instance on which contexts can be executed. It
// corresponds to PHP's MINIT (module init) phase.
func New() (*Engine, error) {
	ptr, err := C.engine_init()
	if err != nil {
		return nil, fmt.Errorf("PHP engine failed to initialize")
	}

	return &Engine{engine: ptr, contexts: make([]*context.Context, 0)}, nil
}

// NewContext creates a new execution context on which scripts can be executed
// and variables can be binded. It corresponds to PHP's RINIT (request init)
// phase.
func (e *Engine) NewContext() (*context.Context, error) {
	c, err := context.New()
	if err != nil {
		return nil, err
	}

	e.contexts = append(e.contexts, c)

	return c, nil
}

func (e *Engine) Define(rcvr interface{}) error {
	v := reflect.ValueOf(rcvr)
	name := reflect.Indirect(v).Type().Name()

	if name == "" {
		return fmt.Errorf("Cannot define anonymous method receiver")
	} else if v.Type().NumMethod() == 0 {
		return fmt.Errorf("Cannot define receiver with no embedded methods")
	}

	n := C.CString(name)
	defer C.free(unsafe.Pointer(n))

	r := unsafe.Pointer(newReceiver(rcvr))

	if ok := (bool)(C.engine_class_define(r, n)); !ok {
		return fmt.Errorf("Failed to define method receiver")
	}

	return nil
}

// Destroy shuts down and frees any resources related to the PHP engine bindings.
func (e *Engine) Destroy() {
	for _, c := range e.contexts {
		c.Destroy()
	}

	e.contexts = nil

	if e.engine != nil {
		C.engine_shutdown(e.engine)
		e.engine = nil
	}
}

func newReceiver(rcvr interface{}) *Receiver {
	v := reflect.ValueOf(rcvr)
	vi := reflect.Indirect(v)

	methods := make(map[string]reflect.Value)
	values := make(map[string]reflect.Value)

	for i := 0; i < v.NumMethod(); i++ {
		methods[v.Type().Method(i).Name] = v.Method(i)
	}

	if vi.Kind() == reflect.Struct {
		for i := 0; i < vi.NumField(); i++ {
			values[vi.Type().Field(i).Name] = vi.Field(i)
		}
	}

	return &Receiver{values: values, methods: methods}
}
