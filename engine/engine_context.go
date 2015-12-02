// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

package engine

import "C"

import (
	"io"
	"strings"
	"unsafe"

	"github.com/deuill/go-php/context"
)

//export engine_context_write
func engine_context_write(ctxptr unsafe.Pointer, buffer unsafe.Pointer, length C.uint) C.int {
	c := (*context.Context)(ctxptr)

	return C.int(write(c.Output, C.GoBytes(buffer, C.int(length))))
}

//export engine_context_log
func engine_context_log(ctxptr unsafe.Pointer, buffer unsafe.Pointer, length C.uint) C.int {
	c := (*context.Context)(ctxptr)

	return C.int(write(c.Log, C.GoBytes(buffer, C.int(length))))
}

func write(w io.Writer, p []byte) int {
	// Do not return error if writer is unavailable.
	if w == nil {
		return len(p)
	}

	written, err := w.Write(p)
	if err != nil {
		return 0
	}

	return written
}

//export engine_context_header
func engine_context_header(ctxptr unsafe.Pointer, operation C.uint, buffer unsafe.Pointer, length C.uint) {
	c := (*context.Context)(ctxptr)

	header := (string)(C.GoBytes(buffer, C.int(length)))
	split := strings.SplitN(header, ":", 2)

	for i := range split {
		split[i] = strings.TrimSpace(split[i])
	}

	switch operation {
	case 0: // Replace header.
		if len(split) == 2 && split[1] != "" {
			c.Header.Set(split[0], split[1])
		}
	case 1: // Append header.
		if len(split) == 2 && split[1] != "" {
			c.Header.Add(split[0], split[1])
		}
	case 2: // Delete header.
		if split[0] != "" {
			c.Header.Del(split[0])
		}
	}
}
