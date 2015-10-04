// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

// Package engine provides methods allowing for the initialization and teardown
// of PHP engine bindings, off which execution contexts can be launched.
package engine

// #cgo CFLAGS: -I/usr/include/php -I/usr/include/php/main -I/usr/include/php/TSRM
// #cgo CFLAGS: -I/usr/include/php/Zend -I../context
// #cgo LDFLAGS: -L${SRCDIR}/context -lphp5
//
// #include "context.h"
// #include "engine.h"
import "C"

import (
	"fmt"
	"unsafe"

	"github.com/deuill/go-php/context"
)

// Engine represents the core PHP engine bindings.
type Engine struct {
	engine *C.struct__php_engine
}

// Destroy shuts down and frees any resources related to the PHP engine
// bindings.
func (e *Engine) Destroy() {
	C.engine_shutdown(e.engine)
	e = nil
}

// New initializes a PHP engine instance on which contexts can be executed. It
// corresponds to PHP's MINIT (module init) phase.
func New() (*Engine, error) {
	ptr, err := C.engine_init()
	if err != nil {
		return nil, fmt.Errorf("PHP engine failed to initialize")
	}

	return &Engine{engine: ptr}, nil
}

//export ubwrite
func ubwrite(ctxptr unsafe.Pointer, buffer unsafe.Pointer, length C.uint) C.int {
	context := (*context.Context)(ctxptr)

	written, err := context.Write(C.GoBytes(buffer, C.int(length)))
	if err != nil {
		return C.int(-1)
	}

	return C.int(written)
}
