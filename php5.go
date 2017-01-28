// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
//
// +build php5

package php

// #cgo CFLAGS: -I/usr/include/php5 -I/usr/include/php5/main -I/usr/include/php5/TSRM
// #cgo CFLAGS: -I/usr/include/php5/Zend -Iinclude/php5 -Isrc/php5
// #cgo LDFLAGS: -lphp5
import "C"
