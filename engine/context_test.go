// Copyright 2016 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

package engine

import (
	"bytes"
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"reflect"
	"testing"
)

func writeTempScript(name, script string) (*os.File, error) {
	file, err := ioutil.TempFile("", name)
	if err != nil {
		return nil, err
	}

	if _, err := file.WriteString(script); err != nil {
		file.Close()
		os.Remove(file.Name())

		return nil, err
	}

	return file, nil
}

func TestContextNew(t *testing.T) {
	e, _ = New()
	c, err := NewContext()

	if err != nil {
		t.Fatalf("NewContext(): %s", err)
	}

	if c.context == nil || c.Header == nil || c.values == nil {
		t.Fatalf("NewContext(): Struct fields are `nil` but no error returned")
	}

	c.Destroy()
}

var execTests = []struct {
	name     string
	script   string
	expected string
}{
	{
		"helloworld.php",

		`<?php
		$a = 'Hello';
		$b = 'World';
		echo $a.' '.$b;`,

		"Hello World",
	},
}

func TestContextExec(t *testing.T) {
	var w bytes.Buffer

	c, _ := NewContext()
	c.Output = &w

	for _, tt := range execTests {
		file, err := writeTempScript(tt.name, tt.script)
		if err != nil {
			t.Errorf("Could not create temporary file for testing: %s", tt.name, err)
			continue
		}

		if err := c.Exec(file.Name()); err != nil {
			t.Errorf("Context.Exec('%s'): Execution failed: %s", tt.name, err)
			continue
		}

		actual := w.String()
		w.Reset()

		if actual != tt.expected {
			t.Errorf("Context.Exec('%s'): Expected `%s', actual `%s'", tt.name, tt.expected, actual)
		}

		file.Close()
		os.Remove(file.Name())
	}

	c.Destroy()
}

var evalTests = []struct {
	script string
	output string
	value  interface{}
}{
	{
		"echo 'Hello World';",
		"Hello World",
		nil,
	},
	{
		"$i = 10; $d = 20; return $i + $d;",
		"",
		int64(30),
	},
}

func TestContextEval(t *testing.T) {
	var w bytes.Buffer

	c, _ := NewContext()
	c.Output = &w

	for _, tt := range evalTests {
		val, err := c.Eval(tt.script)
		if err != nil {
			t.Errorf("Context.Eval('%s'): %s", tt.script, err)
			continue
		}

		output := w.String()
		w.Reset()

		if output != tt.output {
			t.Errorf("Context.Eval('%s'): Expected output '%s', actual '%s'", tt.script, tt.output, output)
		}

		result := val.Interface()

		if reflect.DeepEqual(result, tt.value) == false {
			t.Errorf("Context.Eval('%s'): Expected value '%#v', actual '%#v'", tt.script, tt.value, result)
		}

		val.Destroy()
	}

	c.Destroy()
}

var headerTests = []struct {
	script   string
	expected http.Header
}{
	{
		"header('X-Testing: Hello');",
		http.Header{"X-Testing": []string{"Hello"}},
	},
	{
		"header('X-Testing: World', false);",
		http.Header{"X-Testing": []string{"Hello", "World"}},
	},
	{
		"header_remove('X-Testing');",
		http.Header{},
	},
	{
		"header('X-Testing: Done', false);",
		http.Header{"X-Testing": []string{"Done"}},
	},
}

func TestContextHeader(t *testing.T) {
	c, _ := NewContext()

	for _, tt := range headerTests {
		if _, err := c.Eval(tt.script); err != nil {
			t.Errorf("Context.Eval('%s'): %s", tt.script, err)
			continue
		}

		if reflect.DeepEqual(c.Header, tt.expected) == false {
			t.Errorf("Context.Eval('%s'): expected '%#v', actual '%#v'", tt.script, tt.expected, c.Header)
		}
	}

	c.Destroy()
}

var logTests = []struct {
	script   string
	expected string
}{
	{
		"$a = 10; $a + $b;",
		"PHP Notice:  Undefined variable: b in gophp-engine on line 1",
	},
	{
		"strlen();",
		"PHP Warning:  strlen() expects exactly 1 parameter, 0 given in gophp-engine on line 1",
	},
	{
		"trigger_error('Test Error');",
		"PHP Notice:  Test Error in gophp-engine on line 1",
	},
}

func TestContextLog(t *testing.T) {
	var w bytes.Buffer

	c, _ := NewContext()
	c.Log = &w

	for _, tt := range logTests {
		if _, err := c.Eval(tt.script); err != nil {
			t.Errorf("Context.Eval('%s'): %s", tt.script, err)
			continue
		}

		actual := w.String()
		w.Reset()

		if actual != tt.expected {
			t.Errorf("Context.Eval('%s'): expected '%s', actual '%s'", tt.script, tt.expected, actual)
		}
	}

	c.Destroy()
}

var bindTests = []struct {
	value    interface{}
	expected string
}{
	{
		42,
		"i:42;",
	},
	{
		3.14159,
		"d:3.1415899999999999;",
	},
	{
		true,
		"b:1;",
	},
	{
		"Such bind",
		`s:9:"Such bind";`,
	},
	{
		[]string{"this", "that"},
		`a:2:{i:0;s:4:"this";i:1;s:4:"that";}`,
	},
	{
		[][]int{{1, 2}, {3}},
		`a:2:{i:0;a:2:{i:0;i:1;i:1;i:2;}i:1;a:1:{i:0;i:3;}}`,
	},
	{
		map[string]interface{}{"hello": []string{"hello", "!"}},
		`a:1:{s:5:"hello";a:2:{i:0;s:5:"hello";i:1;s:1:"!";}}`,
	},
	{
		struct {
			I int
			C string
			F struct {
				G bool
			}
			h bool
		}{3, "test", struct {
			G bool
		}{false}, true},
		`O:8:"stdClass":3:{s:1:"I";i:3;s:1:"C";s:4:"test";s:1:"F";O:8:"stdClass":1:{s:1:"G";b:0;}}`,
	},
}

func TestContextBind(t *testing.T) {
	var w bytes.Buffer

	c, _ := NewContext()
	c.Output = &w

	file, err := writeTempScript("evaltest.php", `<?php $i = (isset($i)) ? $i += 1 : 0; echo serialize($$i);`)
	if err != nil {
		t.Fatalf("Could not create temporary file for testing: %s", err)
	}

	for i, tt := range bindTests {
		if err := c.Bind(fmt.Sprintf("%d", i), tt.value); err != nil {
			t.Errorf("Context.Bind('%v'): %s", tt.value, err)
			continue
		}

		if err := c.Exec(file.Name()); err != nil {
			t.Errorf("Context.Exec(): %s", err)
			continue
		}

		actual := w.String()
		w.Reset()

		if actual != tt.expected {
			t.Errorf("Context.Bind('%v'): expected '%s', actual '%s'", tt.value, tt.expected, actual)
		}
	}

	file.Close()
	os.Remove(file.Name())

	c.Destroy()
}

func TestContextDestroy(t *testing.T) {
	c, _ := NewContext()
	c.Destroy()

	if c.context != nil || c.values != nil {
		t.Errorf("Destroy(): Did not set internal fields to `nil`")
	}

	e.Destroy()
}
