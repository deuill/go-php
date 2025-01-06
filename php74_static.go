// Copyright 2025 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
//
//go:build php74
// +build php74

package php

// #cgo LDFLAGS: -ldl -lm -lcurl -lpcre -lssl -lcrypto -lresolv -ledit -lz -lxml2
import "C"
