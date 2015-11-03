// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

package php

import (
	"fmt"
	"os"
	"path"
	"strconv"
	"testing"
)

var testDir string

type MockWriter struct {
	buffer []byte
}

func (m *MockWriter) Write(p []byte) (int, error) {
	if m.buffer == nil {
		m.buffer = p
	} else {
		m.buffer = append(m.buffer, p...)
	}

	return len(p), nil
}

func (m *MockWriter) String() string {
	if m.buffer == nil {
		return ""
	}

	return string(m.buffer)
}

func (m *MockWriter) Reset() {
	if m.buffer != nil {
		m.buffer = m.buffer[:0]
	}
}

func TestNewEngineContext(t *testing.T) {
	e, err := New()
	if err != nil {
		t.Errorf("New(): %s", err)
		return
	}

	defer e.Destroy()

	_, err = e.NewContext(os.Stdout)
	if err != nil {
		t.Errorf("NewContext(): %s", err)
	}
}

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

	for _, tt := range execTests {
		file := path.Join(testDir, tt.file)
		if err := ctx.Exec(file); err != nil {
			t.Errorf("Context.Exec(%s): %s", tt.file, err)
			continue
		}

		actual := w.String()
		w.Reset()

		if actual != tt.expected {
			t.Errorf("Context.Exec(%s): expected '%s', actual '%s'", tt.file, tt.expected, actual)
		}
	}
}

var evalTests = []struct {
	script   string // Script to run
	expected string // Expected output
}{
	{"echo 'Hello World';", "Hello World"},
	{"$i = 10; $d = 20; echo $i + $d;", "30"},
}

func TestContextEval(t *testing.T) {
	var w MockWriter

	e, _ := New()
	ctx, _ := e.NewContext(&w)

	defer e.Destroy()

	for _, tt := range evalTests {
		if _, err := ctx.Eval(tt.script); err != nil {
			t.Errorf("Context.Eval(%s): %s", tt.script, err)
			continue
		}

		actual := w.String()
		w.Reset()

		if actual != tt.expected {
			t.Errorf("Context.Eval(%s): expected '%s', actual '%s'", tt.script, tt.expected, actual)
		}
	}
}

var bindTests = []struct {
	value    interface{} // Value to bind
	expected string      // Serialized form of value
}{
	// Integer to integer.
	{42, "i:42;"},
	// Float to double.
	{3.14159, "d:3.1415899999999999;"},
	// Boolean to boolean.
	{true, "b:1;"},
	// String to string.
	{"Such bind", `s:9:"Such bind";`},
	// Simple slice of strings to indexed array.
	{[]string{"this", "that"}, `a:2:{i:0;s:4:"this";i:1;s:4:"that";}`},
	// Nested slice of integers to indexed array.
	{[][]int{[]int{1, 2}, []int{3}}, `a:2:{i:0;a:2:{i:0;i:1;i:1;i:2;}i:1;a:1:{i:0;i:3;}}`},
	// Struct to object, with nested struct.
	{struct {
		I int
		C string
		F struct {
			G bool
		}
		h bool
	}{3, "test", struct {
		G bool
	}{false}, true}, `O:8:"stdClass":3:{s:1:"I";i:3;s:1:"C";s:4:"test";s:1:"F";O:8:"stdClass":1:{s:1:"G";b:0;}}`},
}

func TestContextBind(t *testing.T) {
	var w MockWriter

	e, _ := New()
	ctx, _ := e.NewContext(&w)

	defer e.Destroy()

	for i, tt := range bindTests {
		if err := ctx.Bind(strconv.FormatInt(int64(i), 10), tt.value); err != nil {
			t.Errorf("Context.Bind(%v): %s", tt.value, err)
			continue
		}

		ctx.Exec(path.Join(testDir, "bind.php"))

		actual := w.String()
		w.Reset()

		if actual != tt.expected {
			t.Errorf("Context.Bind(%v): expected '%s', actual '%s'", tt.value, tt.expected, actual)
		}
	}
}

var reverseBindTests = []struct {
	script   string // Script to run
	expected string // Expected value
}{
	{"return 'Hello World';", `"Hello World"`},
	{"$i = 10; $d = 20; return $i + $d;", "30"},
	{"$i = 1.2; $d = 2.4; return $i + $d;", "3.5999999999999996"},
	{"$what = true; return $what;", "true"},
	{"'This returns nothing';", "<nil>"},
}

func TestContextReverseBind(t *testing.T) {
	var w MockWriter

	e, _ := New()
	ctx, _ := e.NewContext(&w)

	defer e.Destroy()

	for _, tt := range reverseBindTests {
		val, err := ctx.Eval(tt.script)
		if err != nil {
			t.Errorf("Context.Eval(%s): %s", tt.script, err)
			continue
		}

		v, _ := val.Interface()
		actual := fmt.Sprintf("%#v", v)

		if actual != tt.expected {
			t.Errorf("Context.Eval(%s): expected '%s', actual '%s'", tt.script, tt.expected, actual)
		}
	}
}

func init() {
	wd, _ := os.Getwd()
	testDir = path.Join(wd, ".tests")
}
