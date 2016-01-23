// Copyright 2016 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

package engine

import (
	"testing"
)

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
	err := e.Define("TestDefine", func(args []interface{}) interface{} {
		return nil
	})

	if err != nil {
		t.Errorf("Define(): %s", err)
	}

	if len(e.receivers) != 1 {
		t.Errorf("Define(): `Engine.receivers` length is %d, should be 1", len(e.receivers))
	}
}

func TestEngineDestroy(t *testing.T) {
	e.Destroy()

	if e.engine != nil || e.contexts != nil || e.receivers != nil {
		t.Errorf("Destroy(): Did not set internal fields to `nil`")
	}
}
