package php

// #cgo CFLAGS: -I/usr/include/php -I/usr/include/php/main -I/usr/include/php/TSRM
// #cgo CFLAGS: -I/usr/include/php/Zend
// #cgo LDFLAGS: -lphp5
//
// #include <stdio.h>
// #include "engine.h"
// #include "context.h"
import "C"

import (
	"fmt"
	"io"
	"unsafe"
)

type Engine struct {
	engine *C.struct__php_engine
}

func (e *Engine) NewContext(w io.Writer) (*Context, error) {
	ctx := &Context{writer: w, zvals: make(map[string]unsafe.Pointer)}

	ptr, err := C.context_new(e.engine, unsafe.Pointer(ctx))
	if err != nil {
		return nil, fmt.Errorf("Failed to initialize context for PHP engine")
	}

	ctx.context = ptr

	return ctx, nil
}

func (e *Engine) Destroy() {
	C.engine_shutdown(e.engine)
	e = nil
}

func New() (*Engine, error) {
	ptr, err := C.engine_init()
	if err != nil {
		return nil, fmt.Errorf("PHP engine failed to initialize")
	}

	return &Engine{engine: ptr}, nil
}
