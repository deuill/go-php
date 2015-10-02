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

	"../context"
)

type Engine struct {
	engine *C.struct__php_engine
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

//export uwrite
func uwrite(ctxptr unsafe.Pointer, buffer unsafe.Pointer, length C.uint) C.int {
	context := (*context.Context)(ctxptr)

	written, err := context.Write(C.GoBytes(buffer, C.int(length)))
	if err != nil {
		return C.int(-1)
	}

	return C.int(written)
}
