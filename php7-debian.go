// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
//
// Build tags specific to Debian (and Debian-derived, such as Ubuntu)
// distributions. Debian builds its PHP7 packages with non-standard naming
// conventions for include and library paths, so we need a specific build tag
// for building against those packages.
//
// +build debian,!php5

package php

// #cgo CFLAGS: -I/usr/include/php/20151012 -Iinclude/php7 -Isrc/php7
// #cgo CFLAGS: -I/usr/include/php/20151012/main -I/usr/include/php/20151012/Zend
// #cgo CFLAGS: -I/usr/include/php/20151012/TSRM
// #cgo LDFLAGS: -lphp7.0
import "C"
