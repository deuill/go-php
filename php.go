// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

// Package php implements bindings from Go to PHP, and allows for executing
// scripts, binding variables and defining functions and classes which can then
// be called from within PHP scripts.
package php

import (
	"io"

	"github.com/deuill/go-php/context"
	"github.com/deuill/go-php/engine"
)

// New initializes a PHP engine instance on which contexts can be executed. It
// corresponds to PHP's MINIT (module init) phase.
func New() (*engine.Engine, error) {
	return engine.New()
}

// NewContext creates a new execution context on which scripts can be executed
// and variables can be binded. It corresponds to PHP's RINIT (request init
// phase and *must* be preceeded by a call to `php.New()`.
func NewContext(w io.Writer) (*context.Context, error) {
	return context.New(w)
}
