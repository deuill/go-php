// Copyright 2025 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
//
//go:build !php74
// +build !php74

package php

// #cgo CFLAGS: -I/usr/include/php -I/usr/include/php/main -I/usr/include/php/TSRM -I/usr/include/php/Zend
// #cgo CFLAGS: -Iinclude -Iinclude/php8 -Isrc/php8
// #cgo LDFLAGS: -lphp
import "C"
