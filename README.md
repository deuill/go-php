# PHP bindings for Go

[![API Documentation][godoc-svg]][godoc-url] [![MIT License][license-svg]][license-url]

This package implements support for executing PHP scripts, exporting Go variables for use in PHP contexts, attaching Go method receivers as PHP classes and returning PHP variables for use in Go contexts.

Both PHP 5.x and PHP 7.x series are supported.

## Building

Building this package requires that you have PHP installed as a library. For most Linux systems, this can usually be found in the `php-embed` package, or variations thereof.

Once the PHP library is available, the bindings can be compiled with `go build` and are `go get`-able.

**Note**: Building against PHP 5.x requires that the `php5` tag is provided, i.e.:

```bash
go get -tags php5 github.com/deuill/go-php
```

This is due to the fact that PHP 7.x is the default build target.

## Status

Executing PHP [script files][Context.Exec] as well as [inline strings][Context.Eval] is supported and stable.

[Binding Go values][NewValue] as PHP variables is allowed for most base types, and PHP values returned from eval'd strings can be converted and used in Go contexts as `interface{}` values.

It is possible to [attach Go method receivers][NewReceiver] as PHP classes, with full support for calling expored methods, as well as getting and setting embedded fields (for `struct`-type method receivers).

### Caveats

Be aware that, by default, PHP is **not** designed to be used in multithreaded environments (which severely restricts the use of these bindings with Goroutines) if not built with [ZTS support](https://secure.php.net/manual/en/pthreads.requirements.php). However, ZTS support has seen major refactoring between PHP 5 and PHP 7, and as such is currently unsupported by this package.

Currently, it is recommended to either sync use of seperate Contexts between Goroutines, or share a single Context among all running Goroutines.

## Roadmap

Currently, the package lacks in several respects:

  * ZTS/multi-threading support. This basically means using Go-PHP in Goroutines is severely limited.
  * Documentation and examples, both package-level and external.
  * Performance. There's no reason to believe Go-PHP suffers from any serious performance issues in particular, but adding benchmarks, especially compared against vanilla PHP, might help.
  * Your feature request here?

These items will be tackled in order of significance (which may not be the order shown above).

## Usage

### Basic

Executing a script is simple:

```go
package main

import (
    php "github.com/deuill/go-php"
    "os"
)

func main() {
    engine, _ := php.New()

    context, _ := engine.NewContext()
    context.Output = os.Stdout

    context.Exec("index.php")
    engine.Destroy()
}
```

The above will execute script file `index.php` located in the current folder and will write any output to the `io.Writer` assigned to `Context.Output` (in this case, the standard output).

### Binding and returning variables

The following example demonstrates binding a Go variable to the running PHP context, and returning a PHP variable for use in Go:

```go
package main

import (
    "fmt"
    php "github.com/deuill/go-php"
)

func main() {
    engine, _ := php.New()
    context, _ := engine.NewContext()

    var str string = "Hello"
    context.Bind("var", str)

    val, _ := context.Eval("return $var.' World';")
    fmt.Printf("%s", val.Interface())
    // Prints 'Hello World' back to the user.

    engine.Destroy()
}
```

A string value "Hello" is attached using `Context.Bind` under a name `var` (available in PHP as `$var`). A script is executed inline using `Context.Eval`, combinding the attached value with a PHP string and returning it to the user.

Finally, the value is returned as an `interface{}` using `Value.Interface()` (one could also use `Value.String()`, though the both are equivalent in this case).

## License

All code in this repository is covered by the terms of the MIT License, the full text of which can be found in the LICENSE file.

[godoc-url]: https://godoc.org/github.com/deuill/go-php
[godoc-svg]: https://godoc.org/github.com/deuill/go-php?status.svg

[license-url]: https://github.com/deuill/go-php/blob/master/LICENSE
[license-svg]: https://img.shields.io/badge/license-MIT-blue.svg

[Context.Exec]: https://godoc.org/github.com/deuill/go-php/engine#Context.Exec
[Context.Eval]: https://godoc.org/github.com/deuill/go-php/engine#Context.Eval
[NewValue]:     https://godoc.org/github.com/deuill/go-php/engine#NewValue
[NewReceiver]:  https://godoc.org/github.com/deuill/go-php/engine#NewReceiver
