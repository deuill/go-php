// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

package php

import (
	"io/ioutil"
	"os"
	"testing"
)

type Script struct {
	*os.File
}

func NewScript(name, contents string) (*Script, error) {
	file, err := ioutil.TempFile("", name)
	if err != nil {
		return nil, err
	}

	if _, err := file.WriteString(contents); err != nil {
		file.Close()
		os.Remove(file.Name())

		return nil, err
	}

	return &Script{file}, nil
}

func (s *Script) Remove() {
	s.Close()
	os.Remove(s.Name())
}

var e *Engine

func TestEngineNew(t *testing.T) {
	var err error

	if e, err = New(); err != nil {
		t.Fatalf("New(): %s", err)
	}

	if e.engine == nil || e.contexts == nil || e.receivers == nil {
		t.Fatalf("New(): Struct fields are `nil` but no error returned")
	}
}

func TestEngineNewContext(t *testing.T) {
	_, err := e.NewContext()
	if err != nil {
		t.Errorf("NewContext(): %s", err)
	}

	if len(e.contexts) != 1 {
		t.Errorf("NewContext(): `Engine.contexts` length is %d, should be 1", len(e.contexts))
	}
}

func TestEngineDefine(t *testing.T) {
	ctor := func(args []interface{}) interface{} {
		return nil
	}

	if err := e.Define("TestDefine", ctor); err != nil {
		t.Errorf("Engine.Define(): %s", err)
	}

	if len(e.receivers) != 1 {
		t.Errorf("Engine.Define(): `Engine.receivers` length is %d, should be 1", len(e.receivers))
	}

	if err := e.Define("TestDefine", ctor); err == nil {
		t.Errorf("Engine.Define(): Incorrectly defined duplicate receiver")
	}
}

func TestEngineDestroy(t *testing.T) {
	e.Destroy()

	if e.engine != nil || e.contexts != nil || e.receivers != nil {
		t.Errorf("Engine.Destroy(): Did not set internal fields to `nil`")
	}

	// Attempting to destroy an engine instance twice should be a no-op.
	e.Destroy()
}
