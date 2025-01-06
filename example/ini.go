package main

import (
	"fmt"
	"os"

	"go.deuill.org/go-php"
)

var script = `
trigger_error("Cannot divide by zero", E_USER_WARNING);
error_log("Some Error\n");
echo "After Error" . PHP_EOL;
`

func main() {
	engine, err := php.New()
	if err != nil {
		fmt.Printf("could not initialize PHP engine: %s", err)
		os.Exit(1)
	}

	defer engine.Destroy()

	context, err := engine.NewContext()
	if err != nil {
		fmt.Printf("could not initialize PHP context: %s", err)
		os.Exit(1)
	}

	defer context.Destroy()

	context.Output = os.Stdout
	context.Log = os.Stderr

	context.Eval(script)
}
