// Copyright 2015 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

package php

import (
	"bytes"
	"fmt"
	"os"
	"path"
	"reflect"
	"strconv"
	"testing"
)

var testDir string

func TestNewEngineContext(t *testing.T) {
	e, err := New()
	if err != nil {
		t.Errorf("New(): %s", err)
		return
	}

	_, err = e.NewContext()
	if err != nil {
		t.Errorf("NewContext(): %s", err)
	}

	e.Destroy()
}

var execTests = []struct {
	file     string // Filename to run
	expected string // Expected output
}{
	{"echo.php", "Hello World"},
}

func TestContextExec(t *testing.T) {
	var w bytes.Buffer

	e, _ := New()

	ctx, _ := e.NewContext()
	ctx.Output = &w

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

	e.Destroy()
}

var evalTests = []struct {
	script   string // Script to run
	expected string // Expected output
}{
	{"echo 'Hello World';", "Hello World"},
	{"$i = 10; $d = 20; echo $i + $d;", "30"},
}

func TestContextEval(t *testing.T) {
	var w bytes.Buffer

	e, _ := New()

	ctx, _ := e.NewContext()
	ctx.Output = &w

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

	e.Destroy()
}

type TestEngineReceiver struct {
	Var string
}

func (t *TestEngineReceiver) Test(p string) string {
	return "Hello " + p
}

var defineTests = []struct {
	script   string // Script to run
	expected string // Expected output
}{
	{"$t = new TestEngineReceiver; echo is_object($t);", "1"},
	{"$t = new TestEngineReceiver; echo $t->Var;", "hello"},
	{"$t = new TestEngineReceiver; $t->Var = 'world'; echo $t->Var;", "world"},
	{"$t = new TestEngineReceiver; echo $t->Test('World');", "Hello World"},
	{"$t = new TestEngineReceiver; echo ($t->Var) ? 1 : 0;", "1"},
	{"$t = new TestEngineReceiver; echo isset($t->Var) ? 1 : 0;", "1"},
	{"$t = new TestEngineReceiver; echo empty($t->Var) ? 1 : 0;", "0"},
}

func TestEngineDefine(t *testing.T) {
	var w bytes.Buffer
	var ctor = func(args []interface{}) interface{} {
		return &TestEngineReceiver{Var: "hello"}
	}

	e, _ := New()
	if err := e.Define("TestEngineReceiver", ctor); err != nil {
		t.Errorf("Engine.Define(%s): %s", ctor, err)
	}

	ctx, _ := e.NewContext()
	ctx.Output = &w

	for _, tt := range defineTests {
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

	e.Destroy()
}

var headerTests = []struct {
	script   string // Script to run
	expected string // Expected output
}{
	{"header('X-Testing: Hello');", `http.Header{"X-Testing":[]string{"Hello"}}`},
	{"header('X-Testing: World', false);", `http.Header{"X-Testing":[]string{"Hello", "World"}}`},
	{"header_remove('X-Testing');", `http.Header{}`},
	{"header('X-Testing: Done', false);", `http.Header{"X-Testing":[]string{"Done"}}`},
}

func TestContextHeader(t *testing.T) {
	e, _ := New()
	ctx, _ := e.NewContext()

	for _, tt := range headerTests {
		if _, err := ctx.Eval(tt.script); err != nil {
			t.Errorf("Context.Header(%s): %s", tt.script, err)
			continue
		}

		actual := fmt.Sprintf("%#v", ctx.Header)

		if actual != tt.expected {
			t.Errorf("Context.Header(%s): expected '%s', actual '%s'", tt.script, tt.expected, actual)
		}
	}

	e.Destroy()
}

var logTests = []struct {
	script   string // Script to run
	expected string // Expected output
}{
	{"$a = 10; $a + $b;", "PHP Notice:  Undefined variable: b in gophp-engine on line 1"},
	{"strlen();", "PHP Warning:  strlen() expects exactly 1 parameter, 0 given in gophp-engine on line 1"},
	{"trigger_error('Test Error');", "PHP Notice:  Test Error in gophp-engine on line 1"},
}

func TestContextLog(t *testing.T) {
	var w bytes.Buffer

	e, _ := New()

	ctx, _ := e.NewContext()
	ctx.Log = &w

	for _, tt := range logTests {
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

	e.Destroy()
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
	{[][]int{{1, 2}, {3}}, `a:2:{i:0;a:2:{i:0;i:1;i:1;i:2;}i:1;a:1:{i:0;i:3;}}`},
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
	var w bytes.Buffer

	e, _ := New()

	ctx, _ := e.NewContext()
	ctx.Output = &w

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

	e.Destroy()
}

var reverseBindTests = []struct {
	script   string        // Script to run
	expected []interface{} // Expected value
}{
	{"return 'Hello World';", []interface{}{
		"Hello World",
		int64(0),
		float64(0),
		true,
		"Hello World",
		[]interface{}{"Hello World"},
		map[string]interface{}{"0": "Hello World"},
	}},
	{"$i = 10; $d = 20; return $i + $d;", []interface{}{
		int64(30),
		int64(30),
		float64(30),
		true,
		"30",
		[]interface{}{int64(30)},
		map[string]interface{}{"0": int64(30)},
	}},
	{"$i = 1.2; $d = 2.4; return $i + $d;", []interface{}{
		float64(3.5999999999999996),
		int64(3),
		float64(3.5999999999999996),
		true,
		"3.6",
		[]interface{}{float64(3.5999999999999996)},
		map[string]interface{}{"0": float64(3.5999999999999996)},
	}},
	{"$what = true; return $what;", []interface{}{
		true,
		int64(1),
		float64(1.0),
		true,
		"1",
		[]interface{}{true},
		map[string]interface{}{"0": true},
	}},
	{"return [];", []interface{}{
		[]interface{}{},
		int64(0),
		float64(0),
		false,
		"Array",
		[]interface{}{},
		map[string]interface{}{},
	}},
	{"return [1, 'w', 3.1, false];", []interface{}{
		[]interface{}{(int64)(1), "w", 3.1, false},
		int64(1),
		float64(1.0),
		true,
		"Array",
		[]interface{}{(int64)(1), "w", 3.1, false},
		map[string]interface{}{"0": (int64)(1), "1": "w", "2": 3.1, "3": false},
	}},
	{"return [0 => 'a', 2 => 'b', 1 => 'c'];", []interface{}{
		map[string]interface{}{"0": "a", "2": "b", "1": "c"},
		int64(1),
		float64(1),
		true,
		"Array",
		[]interface{}{"a", "b", "c"},
		map[string]interface{}{"0": "a", "2": "b", "1": "c"},
	}},
	{"return ['h' => 'hello', 'w' => 'world'];", []interface{}{
		map[string]interface{}{"h": "hello", "w": "world"},
		int64(1),
		float64(1.0),
		true,
		"Array",
		[]interface{}{"hello", "world"},
		map[string]interface{}{"h": "hello", "w": "world"},
	}},
	{"return (object) ['test' => 1, 2 => 'hello'];", []interface{}{
		map[string]interface{}{"test": int64(1), "2": "hello"},
		int64(1),
		float64(1.0),
		true,
		"",
		[]interface{}{int64(1), "hello"},
		map[string]interface{}{"test": int64(1), "2": "hello"},
	}},
	{"'This returns nothing';", []interface{}{
		nil,
		int64(0),
		float64(0.0),
		false,
		"",
		[]interface{}{},
		map[string]interface{}{},
	}},
}

func TestContextReverseBind(t *testing.T) {
	e, _ := New()
	ctx, _ := e.NewContext()

	for _, tt := range reverseBindTests {
		val, err := ctx.Eval(tt.script)
		if err != nil {
			t.Errorf(`Context.Eval("%s"): %s`, tt.script, err)
			continue
		}

		actual := []interface{}{val.Interface(), val.Int(), val.Float(), val.Bool(), val.String(), val.Slice(), val.Map()}

		for i, expected := range tt.expected {
			actual := actual[i]
			if reflect.DeepEqual(actual, expected) == false {
				t.Errorf(`Context.Eval("%s") to '%[3]T': expected  '%[2]v', actual '%[3]v'`, tt.script, expected, actual)
			}
		}
	}

	e.Destroy()
}

func init() {
	wd, _ := os.Getwd()
	testDir = path.Join(wd, ".tests")
}
