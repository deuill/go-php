// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

// Package php implements bindings from Go to PHP, and allows for executing
// scripts, binding variables and defining functions and classes which can then
// be called from within PHP scripts.
package php

import (
	"github.com/deuill/go-php/engine"
)

// New initializes a PHP engine instance on which contexts can be executed. It
// corresponds to PHP's MINIT (module init) phase.
func New() (*engine.Engine, error) {
	return engine.New()
}
