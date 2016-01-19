// +build !php7

package engine

// #cgo CFLAGS: -Iinclude/php5
// #cgo LDFLAGS: -lphp5
import "C"
