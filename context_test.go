package php

import (
	"os"
	"path"
	"strconv"
	"testing"
)

var testDir string

var execTests = []struct {
	file     string // Filename to run
	expected string // Expected output
}{
	{"echo.php", "Hello World"},
}

func TestContextExec(t *testing.T) {
	var w MockWriter

	e, _ := New()
	ctx, _ := e.NewContext(&w)

	defer e.Destroy()
	defer ctx.Destroy()

	for _, tt := range execTests {
		file := path.Join(testDir, tt.file)
		if err := ctx.Exec(file); err != nil {
			t.Errorf("Context.Exec(%s): %s", tt.file, err)
		}

		actual := w.String()
		w.Reset()

		if actual != tt.expected {
			t.Errorf("Context.Exec(%s): expected '%s', actual '%s'", tt.file, tt.expected, actual)
		}
	}
}

var bindTests = []struct {
	value    interface{} // Value to bind
	expected string      // Serialized form of value
}{
	{42, "i:42;"},                      // Integer
	{3.14159, "d:3.1415899999999999;"}, // Floating point
	{"Such bind", `s:9:"Such bind";`},  // String
}

func TestContextBind(t *testing.T) {
	var w MockWriter

	e, _ := New()
	ctx, _ := e.NewContext(&w)

	defer e.Destroy()
	defer ctx.Destroy()

	for i, tt := range bindTests {
		if err := ctx.Bind(strconv.FormatInt(int64(i), 10), tt.value); err != nil {
			t.Errorf("Context.Bind(%v): %s", tt.value, err)
		}

		ctx.Exec(path.Join(testDir, "bind.php"))

		actual := w.String()
		w.Reset()

		if actual != tt.expected {
			t.Errorf("Context.Bind(%v): expected '%s', actual '%s'", tt.value, tt.expected, actual)
		}
	}
}

func init() {
	wd, _ := os.Getwd()
	testDir = path.Join(wd, "test")
}
