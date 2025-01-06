// Copyright 2025 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
//
// Build tags for ArchLinux, which has PHP 7.4 available in AUR, and installs this in a dedicated
// directory to avoid conflicts with other, stable versions of PHP.
//
//go:build php74
// +build php74

package php

// #cgo CFLAGS: -I/usr/include/php74 -I/usr/include/php74/main
// #cgo CFLAGS: -I/usr/include/php74/Zend -I/usr/include/php74/TSRM
// #cgo CFLAGS: -Iinclude -Iinclude/php74 -Isrc/php74
// #cgo LDFLAGS: -lphp74
import "C"
