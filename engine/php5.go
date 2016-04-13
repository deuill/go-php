// +build !php7

package engine

// #cgo CFLAGS: -I/usr/include/php5 -I/usr/include/php5/main -I/usr/include/php5/TSRM
// #cgo CFLAGS: -I/usr/include/php5/Zend -Iinclude/php5
// #cgo LDFLAGS: -lphp5
import "C"
