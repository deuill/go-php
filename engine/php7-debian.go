// +build php7.debian
//
// Build tags specific to Debian (and Debian-derived, such as Ubuntu) distributions.
// Debian builds its PHP7 packages with non-standard naming conventions for include
// and library paths, so we need a specific build tag for building against those
// packages.

package engine

// #cgo CFLAGS: -I/usr/include/php/20151012 -I/usr/include/php/20151012/main -I/usr/include/php/20151012/Zend -I/usr/include/php/20151012/TSRM -Iinclude/php7 -Isrc/php7
// #cgo LDFLAGS: -lphp7.0
import "C"
