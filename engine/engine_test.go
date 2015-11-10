// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

package engine

import (
	"os"
	"testing"
)

var e *Engine

func TestEngineNew(t *testing.T) {
	var err error

	if e, err = New(); err != nil {
		t.Fatalf("New(): %s", err)
	}

	if e.engine == nil || e.contexts == nil {
		t.Fatalf("New(): struct fields are 'nil' but no error returned")
	}
}

func TestEngineNewContext(t *testing.T) {
	_, err := e.NewContext(os.Stdout)
	if err != nil {
		t.Errorf("NewContext(): %s", err)
	}

	if len(e.contexts) != 1 {
		t.Errorf("NewContext(): context instance not added to engine")
	}
}

func TestEngineDestroy(t *testing.T) {
	e.Destroy()

	if e.engine != nil || e.contexts != nil {
		t.Errorf("Destroy(): did not release internal engine or context fields")
	}
}
