// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

package engine

import "C"

import (
	"io"
	"reflect"
	"strings"
	"unsafe"

	"github.com/deuill/go-php/context"
	"github.com/deuill/go-php/value"
)

//export engine_receiver_get
func engine_receiver_get(rcvr unsafe.Pointer, name *C.char) unsafe.Pointer {
	r := (*Receiver)(rcvr)
	n := C.GoString(name)

	if _, exists := r.values[n]; !exists {
		return nil
	}

	result, err := value.New(r.values[n].Interface())
	if err != nil {
		return nil
	}

	return result.Ptr()
}

//export engine_receiver_set
func engine_receiver_set(rcvr unsafe.Pointer, name *C.char, val unsafe.Pointer) {
	r := (*Receiver)(rcvr)
	n := C.GoString(name)

	// Do not attempt to set non-existing or unset-able field.
	if _, exists := r.values[n]; !exists || !r.values[n].CanSet() {
		return
	}

	v, err := value.NewFromPtr(val)
	if err != nil {
		return
	}

	r.values[n].Set(reflect.ValueOf(v.Interface()))
	v.Destroy()
}

//export engine_receiver_exists
func engine_receiver_exists(rcvr unsafe.Pointer, name *C.char) C.int {
	r := (*Receiver)(rcvr)
	n := C.GoString(name)

	if _, exists := r.values[n]; !exists {
		return 0
	}

	return 1
}

//export engine_receiver_call
func engine_receiver_call(rcvr unsafe.Pointer, name *C.char, args unsafe.Pointer) unsafe.Pointer {
	r := (*Receiver)(rcvr)
	n := C.GoString(name)

	if _, exists := r.methods[n]; !exists {
		return nil
	}

	// Process input arguments.
	va, err := value.NewFromPtr(args)
	if err != nil {
		return nil
	}

	in := make([]reflect.Value, 0)
	for _, v := range va.Slice() {
		in = append(in, reflect.ValueOf(v))
	}

	va.Destroy()

	// Call receiver method.
	var result interface{}
	ret := r.methods[n].Call(in)

	// Process results, returning a single value if result slice contains a single
	// element, otherwise returns a slice of values.
	if len(ret) > 1 {
		t := make([]interface{}, len(ret))
		for _, v := range ret {
			t = append(t, v.Interface())
		}

		result = t
	} else if len(ret) == 1 {
		result = ret[0].Interface()
	} else {
		return nil
	}

	v, err := value.New(result)
	if err != nil {
		return nil
	}

	return v.Ptr()
}

//export engine_context_write
func engine_context_write(ctxptr, buffer unsafe.Pointer, length C.uint) C.int {
	c := (*context.Context)(ctxptr)

	return write(c.Output, buffer, length)
}

//export engine_context_log
func engine_context_log(ctxptr unsafe.Pointer, buffer unsafe.Pointer, length C.uint) C.int {
	c := (*context.Context)(ctxptr)

	return write(c.Log, buffer, length)
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

//export engine_context_header
func engine_context_header(ctxptr unsafe.Pointer, operation C.uint, buffer unsafe.Pointer, length C.uint) {
	c := (*context.Context)(ctxptr)

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
