// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

package php

// #cgo CFLAGS: -I/usr/include/php -I/usr/include/php/main -I/usr/include/php/TSRM
// #cgo CFLAGS: -I/usr/include/php/Zend -Iinclude
//
// #include <stdlib.h>
// #include <main/php.h>
// #include "receiver.h"
import "C"

import (
	"fmt"
	"reflect"
	"unsafe"
)

// Receiver represents a method receiver.
type Receiver struct {
	name    string
	create  func(args []interface{}) interface{}
	objects map[*C.struct__engine_receiver]*ReceiverObject
}

// NewObject instantiates a new method receiver object, using the Receiver's
// create function and passing in a slice of values as a parameter.
func (r *Receiver) NewObject(args []interface{}) (*ReceiverObject, error) {
	obj := &ReceiverObject{
		instance: r.create(args),
		values:   make(map[string]reflect.Value),
		methods:  make(map[string]reflect.Value),
	}

	if obj.instance == nil {
		return nil, fmt.Errorf("Failed to instantiate method receiver")
	}

	v := reflect.ValueOf(obj.instance)
	vi := reflect.Indirect(v)

	for i := 0; i < v.NumMethod(); i++ {
		// Skip unexported methods.
		if v.Type().Method(i).PkgPath != "" {
			continue
		}

		obj.methods[v.Type().Method(i).Name] = v.Method(i)
	}

	if vi.Kind() == reflect.Struct {
		for i := 0; i < vi.NumField(); i++ {
			// Skip unexported fields.
			if vi.Type().Field(i).PkgPath != "" {
				continue
			}

			obj.values[vi.Type().Field(i).Name] = vi.Field(i)
		}
	}

	return obj, nil
}

// Destroy removes references to the generated PHP class for this receiver and
// frees any memory used by object instances.
func (r *Receiver) Destroy() {
	if r.create == nil {
		return
	}

	n := C.CString(r.name)
	defer C.free(unsafe.Pointer(n))

	C.receiver_destroy(n)
	r.create = nil
	r.objects = nil
}

// ReceiverObject represents an object instance of a pre-defined method receiver.
type ReceiverObject struct {
	instance interface{}
	values   map[string]reflect.Value
	methods  map[string]reflect.Value
}

// Get returns a named internal property of the receiver object instance, or an
// error if the property does not exist or is not addressable.
func (o *ReceiverObject) Get(name string) (*Value, error) {
	if _, exists := o.values[name]; !exists || !o.values[name].CanInterface() {
		return nil, fmt.Errorf("Value '%s' does not exist or is not addressable", name)
	}

	val, err := NewValue(o.values[name].Interface())
	if err != nil {
		return nil, err
	}

	return val, nil
}

// Set assigns value to named internal property. If the named property does not
// exist or cannot be set, the method does nothing.
func (o *ReceiverObject) Set(name string, val interface{}) {
	// Do not attempt to set non-existing or unset-able field.
	if _, exists := o.values[name]; !exists || !o.values[name].CanSet() {
		return
	}

	o.values[name].Set(reflect.ValueOf(val))
}

// Exists checks if named internal property exists and returns true, or false if
// property does not exist.
func (o *ReceiverObject) Exists(name string) bool {
	if _, exists := o.values[name]; !exists {
		return false
	}

	return true
}

// Call executes a method receiver's named internal method, passing a slice of
// values as arguments to the method. If the method fails to execute or returns
// no value, nil is returned, otherwise a Value instance is returned.
func (o *ReceiverObject) Call(name string, args []interface{}) *Value {
	if _, exists := o.methods[name]; !exists {
		return nil
	}

	var in []reflect.Value
	for _, v := range args {
		in = append(in, reflect.ValueOf(v))
	}

	// Call receiver method.
	var result interface{}
	val := o.methods[name].Call(in)

	// Process results, returning a single value if result slice contains a single
	// element, otherwise returns a slice of values.
	if len(val) > 1 {
		t := make([]interface{}, len(val))
		for i, v := range val {
			t[i] = v.Interface()
		}

		result = t
	} else if len(val) == 1 {
		result = val[0].Interface()
	} else {
		return nil
	}

	v, err := NewValue(result)
	if err != nil {
		return nil
	}

	return v
}
