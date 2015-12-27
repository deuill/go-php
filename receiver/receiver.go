// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

package receiver

// #cgo CFLAGS: -I/usr/include/php -I/usr/include/php/main -I/usr/include/php/TSRM
// #cgo CFLAGS: -I/usr/include/php/Zend -I../value
// #cgo LDFLAGS: -L${SRCDIR}/value -lphp5
//
// #include <stdlib.h>
// #include <main/php.h>
// #include "receiver.h"
import "C"

import (
	"reflect"
	"unsafe"

	"github.com/deuill/go-php/value"
)

type reference struct {
	instance interface{}
	values   map[string]reflect.Value
	methods  map[string]reflect.Value
}

type Receiver struct {
	ctor func(args ...interface{}) interface{}
	refs []*reference
}

func New(name string, fn func(args ...interface{}) interface{}) (*Receiver, error) {
	rcvr := &Receiver{
		ctor: fn,
		refs: make([]*reference, 0),
	}

	n := C.CString(name)
	defer C.free(unsafe.Pointer(n))

	C.receiver_define(n, unsafe.Pointer(rcvr))

	return rcvr, nil
}

//export receiverNew
func receiverNew(rcvr unsafe.Pointer, args unsafe.Pointer) unsafe.Pointer {
	r := (*Receiver)(rcvr)

	va, err := value.NewFromPtr(args)
	if err != nil {
		return nil
	}

	instance := r.ctor(va.Slice()...)
	va.Destroy()

	ref := &reference{
		instance: instance,
		values:   make(map[string]reflect.Value),
		methods:  make(map[string]reflect.Value),
	}

	r.refs = append(r.refs, ref)

	v := reflect.ValueOf(instance)
	for i := 0; i < v.NumMethod(); i++ {
		ref.methods[v.Type().Method(i).Name] = v.Method(i)
	}

	vi := reflect.Indirect(v)
	if vi.Kind() == reflect.Struct {
		for i := 0; i < vi.NumField(); i++ {
			ref.values[vi.Type().Field(i).Name] = vi.Field(i)
		}
	}

	return unsafe.Pointer(ref)
}

//export receiverGet
func receiverGet(ref unsafe.Pointer, name *C.char) unsafe.Pointer {
	r := (*reference)(ref)
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

//export receiverSet
func receiverSet(ref unsafe.Pointer, name *C.char, val unsafe.Pointer) {
	r := (*reference)(ref)
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

//export receiverExists
func receiverExists(ref unsafe.Pointer, name *C.char) C.int {
	r := (*reference)(ref)
	n := C.GoString(name)

	if _, exists := r.values[n]; !exists {
		return 0
	}

	return 1
}

//export receiverCall
func receiverCall(ref unsafe.Pointer, name *C.char, args unsafe.Pointer) unsafe.Pointer {
	r := (*reference)(ref)
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
