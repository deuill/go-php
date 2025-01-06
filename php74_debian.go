// Copyright 2025 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
//
// Build tags specific to Debian (and Debian-derived, such as Ubuntu)
// distributions. Debian builds its PHP packages with non-standard naming
// conventions for include and library paths, so we need a specific build tag
// for building against those packages.
//
//go:build php74
// +build php74

package php

// #cgo CFLAGS: -I/usr/include/php/20151012 -I/usr/include/php/20151012/main
// #cgo CFLAGS: -I/usr/include/php/20151012/Zend -I/usr/include/php/20151012/TSRM
// #cgo CFLAGS: -Iinclude -Iinclude/php74 -Isrc/php74
// #cgo LDFLAGS: -lphp7.4
import "C"
