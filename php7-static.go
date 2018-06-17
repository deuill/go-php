// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
//
// +build static

package php

// #cgo LDFLAGS: -ldl -lm -lcurl -lpcre -lssl -lcrypto -lresolv -ledit -lz -lxml2
import "C"
