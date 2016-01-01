// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

// Package receiver implements one-way bindings for using Go method receivers as
// PHP classes, with full support for calling embedded methods as well as getting
// and setting embedded fields for struct method receivers.
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

type object struct {
	instance interface{}
	values   map[string]reflect.Value
	methods  map[string]reflect.Value
}

// Receiver represents a method receiver.
type Receiver struct {
	create  func(args []interface{}) interface{}
	objects []*object
}

// New registers a PHP class for the name passed, using function fn as constructor
// for individual object instances as needed by the PHP context.
//
// The class name registered is assumed to be unique for the active engine.
//
// The constructor function accepts a slice of arguments, as passed by the PHP
// context, and should return a method receiver instance, or nil on error (in
// which case, an exception is thrown on the PHP object constructor).
func New(name string, fn func(args []interface{}) interface{}) (*Receiver, error) {
	rcvr := &Receiver{
		create:  fn,
		objects: make([]*object, 0),
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

	defer va.Destroy()

	obj := &object{
		instance: r.create(va.Slice()),
		values:   make(map[string]reflect.Value),
		methods:  make(map[string]reflect.Value),
	}

	if obj.instance == nil {
		return nil
	}

	r.objects = append(r.objects, obj)

	v := reflect.ValueOf(obj.instance)
	vi := reflect.Indirect(v)

	for i := 0; i < v.NumMethod(); i++ {
		obj.methods[v.Type().Method(i).Name] = v.Method(i)
	}

	if vi.Kind() == reflect.Struct {
		for i := 0; i < vi.NumField(); i++ {
			obj.values[vi.Type().Field(i).Name] = vi.Field(i)
		}
	}

	return unsafe.Pointer(obj)
}

//export receiverGet
func receiverGet(obj unsafe.Pointer, name *C.char) unsafe.Pointer {
	o := (*object)(obj)
	n := C.GoString(name)

	if _, exists := o.values[n]; !exists {
		return nil
	}

	result, err := value.New(o.values[n].Interface())
	if err != nil {
		return nil
	}

	return result.Ptr()
}

//export receiverSet
func receiverSet(obj unsafe.Pointer, name *C.char, val unsafe.Pointer) {
	o := (*object)(obj)
	n := C.GoString(name)

	// Do not attempt to set non-existing or unset-able field.
	if _, exists := o.values[n]; !exists || !o.values[n].CanSet() {
		return
	}

	v, err := value.NewFromPtr(val)
	if err != nil {
		return
	}

	o.values[n].Set(reflect.ValueOf(v.Interface()))
}

//export receiverExists
func receiverExists(obj unsafe.Pointer, name *C.char) C.int {
	o := (*object)(obj)
	n := C.GoString(name)

	if _, exists := o.values[n]; !exists {
		return 0
	}

	return 1
}

//export receiverCall
func receiverCall(obj unsafe.Pointer, name *C.char, args unsafe.Pointer) unsafe.Pointer {
	o := (*object)(obj)
	n := C.GoString(name)

	if _, exists := o.methods[n]; !exists {
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
	ret := o.methods[n].Call(in)

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
