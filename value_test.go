// Copyright 2017 Alexander Palaistras. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

package php

import (
	"reflect"
	"testing"
)

func TestValueStart(t *testing.T) {
	e, _ = New()
	t.SkipNow()
}

var valueNewTests = []struct {
	value    interface{}
	expected interface{}
}{
	{
		nil,
		nil,
	},
	{
		42,
		int64(42),
	},
	{
		3.14159,
		float64(3.14159),
	},
	{
		true,
		true,
	},
	{
		"Hello World",
		"Hello World",
	},
	{
		[]string{"Knick", "Knack"},
		[]interface{}{"Knick", "Knack"},
	},
	{
		[][]string{{"1", "2"}, {"3"}},
		[]interface{}{[]interface{}{"1", "2"}, []interface{}{"3"}},
	},
	{
		map[string]int{"biggs": 23, "wedge": 16},
		map[string]interface{}{"biggs": int64(23), "wedge": int64(16)},
	},
	{
		map[int]string{10: "this", 20: "that"},
		map[string]interface{}{"10": "this", "20": "that"},
	},
	{
		struct {
			I int
			S string
			B bool
			h string
		}{66, "wow", true, "hidden"},
		map[string]interface{}{"I": int64(66), "S": "wow", "B": true},
	},
}

func TestValueNew(t *testing.T) {
	c, _ := e.NewContext()

	for _, tt := range valueNewTests {
		val, err := NewValue(tt.value)
		if err != nil {
			t.Errorf("NewValue('%v'): %s", tt.value, err)
			continue
		}

		if val == nil {
			t.Errorf("NewValue('%v'): No error returned but value is `nil`", tt.value)
			continue
		}

		actual := val.Interface()

		if reflect.DeepEqual(actual, tt.expected) == false {
			t.Errorf("NewValue('%v'): expected '%#v', actual '%#v'", tt.value, tt.expected, actual)
		}

		val.Destroy()
	}

	c.Destroy()
}

var valueNewInvalidTests = []interface{}{
	uint(10),
	make(chan int),
	func() {},
	[]interface{}{uint(2)},
	map[string]interface{}{"t": make(chan bool)},
	map[bool]interface{}{false: true},
	struct {
		T interface{}
	}{func() {}},
}

func TestValueNewInvalid(t *testing.T) {
	c, _ := e.NewContext()

	for _, value := range valueNewInvalidTests {
		val, err := NewValue(value)
		if err == nil {
			val.Destroy()
			t.Errorf("NewValue('%v'): Value is invalid but no error occured", value)
		}
	}

	c.Destroy()
}

var valueKindTests = []struct {
	value    interface{}
	expected ValueKind
}{
	{
		42,
		Long,
	},
	{
		3.14159,
		Double,
	},
	{
		true,
		Bool,
	},
	{
		"Hello World",
		String,
	},
	{
		[]string{"Knick", "Knack"},
		Array,
	},
	{
		map[string]int{"t": 1, "c": 2},
		Map,
	},
	{
		struct {
			I int
			S string
		}{66, "wow"},
		Object,
	},
}

func TestValueKind(t *testing.T) {
	c, _ := e.NewContext()

	for _, tt := range valueKindTests {
		val, err := NewValue(tt.value)
		if err != nil {
			t.Errorf("NewValue('%v'): %s", tt.value, err)
			continue
		}

		actual := val.Kind()

		if actual != tt.expected {
			t.Errorf("Value.Kind('%v'): expected '%#v', actual '%#v'", tt.value, tt.expected, actual)
		}

		val.Destroy()
	}

	c.Destroy()
}

var valueIntTests = []struct {
	value    interface{}
	expected int64
}{
	{
		42,
		int64(42),
	},
	{
		3.14159,
		int64(3),
	},
	{
		true,
		int64(1),
	},
	{
		"Hello World",
		int64(0),
	},
	{
		[]string{"Knick", "Knack"},
		int64(1),
	},
	{
		map[string]int{"t": 1, "c": 2},
		int64(1),
	},
	{
		struct {
			I int
			S string
		}{66, "wow"},
		int64(1),
	},
}

func TestValueInt(t *testing.T) {
	c, _ := e.NewContext()

	for _, tt := range valueIntTests {
		val, err := NewValue(tt.value)
		if err != nil {
			t.Errorf("NewValue('%v'): %s", tt.value, err)
			continue
		}

		actual := val.Int()

		if reflect.DeepEqual(actual, tt.expected) == false {
			t.Errorf("Value.Int('%v'): expected '%#v', actual '%#v'", tt.value, tt.expected, actual)
		}

		val.Destroy()
	}

	c.Destroy()
}

var valueFloatTests = []struct {
	value    interface{}
	expected float64
}{
	{
		42,
		float64(42),
	},
	{
		3.14159,
		float64(3.14159),
	},
	{
		true,
		float64(1),
	},
	{
		"Hello World",
		float64(0),
	},
	{
		[]string{"Knick", "Knack"},
		float64(1),
	},
	{
		map[string]int{"t": 1, "c": 2},
		float64(1),
	},
	{
		struct {
			I int
			S string
		}{66, "wow"},
		float64(1),
	},
}

func TestValueFloat(t *testing.T) {
	c, _ := e.NewContext()

	for _, tt := range valueFloatTests {
		val, err := NewValue(tt.value)
		if err != nil {
			t.Errorf("NewValue('%v'): %s", tt.value, err)
			continue
		}

		actual := val.Float()

		if reflect.DeepEqual(actual, tt.expected) == false {
			t.Errorf("Value.Float('%v'): expected '%#v', actual '%#v'", tt.value, tt.expected, actual)
		}

		val.Destroy()
	}

	c.Destroy()
}

var valueBoolTests = []struct {
	value    interface{}
	expected bool
}{
	{
		42,
		true,
	},
	{
		3.14159,
		true,
	},
	{
		true,
		true,
	},
	{
		"Hello World",
		true,
	},
	{
		[]string{"Knick", "Knack"},
		true,
	},
	{
		map[string]int{"t": 1, "c": 2},
		true,
	},
	{
		struct {
			I int
			S string
		}{66, "wow"},
		true,
	},
}

func TestValueBool(t *testing.T) {
	c, _ := e.NewContext()

	for _, tt := range valueBoolTests {
		val, err := NewValue(tt.value)
		if err != nil {
			t.Errorf("NewValue('%v'): %s", tt.value, err)
			continue
		}

		actual := val.Bool()

		if reflect.DeepEqual(actual, tt.expected) == false {
			t.Errorf("Value.Bool('%v'): expected '%#v', actual '%#v'", tt.value, tt.expected, actual)
		}

		val.Destroy()
	}

	c.Destroy()
}

var valueStringTests = []struct {
	value    interface{}
	expected string
}{
	{
		42,
		"42",
	},
	{
		3.14159,
		"3.14159",
	},
	{
		true,
		"1",
	},
	{
		"Hello World",
		"Hello World",
	},
	{
		[]string{"Knick", "Knack"},
		"Array",
	},
	{
		map[string]int{"t": 1, "c": 2},
		"Array",
	},
	{
		struct {
			I int
			S string
		}{66, "wow"},
		"",
	},
}

func TestValueString(t *testing.T) {
	c, _ := e.NewContext()

	for _, tt := range valueStringTests {
		val, err := NewValue(tt.value)
		if err != nil {
			t.Errorf("NewValue('%v'): %s", tt.value, err)
			continue
		}

		actual := val.String()

		if reflect.DeepEqual(actual, tt.expected) == false {
			t.Errorf("Value.String('%v'): expected '%#v', actual '%#v'", tt.value, tt.expected, actual)
		}

		val.Destroy()
	}

	c.Destroy()
}

var valueSliceTests = []struct {
	value    interface{}
	expected interface{}
}{
	{
		42,
		[]interface{}{int64(42)},
	},
	{
		3.14159,
		[]interface{}{float64(3.14159)},
	},
	{
		true,
		[]interface{}{true},
	},
	{
		"Hello World",
		[]interface{}{"Hello World"},
	},
	{
		[]string{"Knick", "Knack"},
		[]interface{}{"Knick", "Knack"},
	},
	{
		map[string]int{"t": 1, "c": 2},
		[]interface{}{int64(1), int64(2)},
	},
	{
		struct {
			I int
			S string
		}{66, "wow"},
		[]interface{}{int64(66), "wow"},
	},
}

func TestValueSlice(t *testing.T) {
	c, _ := e.NewContext()

	for _, tt := range valueSliceTests {
		val, err := NewValue(tt.value)
		if err != nil {
			t.Errorf("NewValue('%v'): %s", tt.value, err)
			continue
		}

		actual := val.Slice()

		if reflect.DeepEqual(actual, tt.expected) == false {
			t.Errorf("Value.Slice('%v'): expected '%#v', actual '%#v'", tt.value, tt.expected, actual)
		}

		val.Destroy()
	}

	c.Destroy()
}

var valueMapTests = []struct {
	value    interface{}
	expected map[string]interface{}
}{
	{
		42,
		map[string]interface{}{"0": int64(42)},
	},
	{
		3.14159,
		map[string]interface{}{"0": float64(3.14159)},
	},
	{
		true,
		map[string]interface{}{"0": true},
	},
	{
		"Hello World",
		map[string]interface{}{"0": "Hello World"},
	},
	{
		[]string{"Knick", "Knack"},
		map[string]interface{}{"0": "Knick", "1": "Knack"},
	},
	{
		map[string]int{"t": 1, "c": 2},
		map[string]interface{}{"t": int64(1), "c": int64(2)},
	},
	{
		struct {
			I int
			S string
		}{66, "wow"},
		map[string]interface{}{"I": int64(66), "S": "wow"},
	},
}

func TestValueMap(t *testing.T) {
	c, _ := e.NewContext()

	for _, tt := range valueMapTests {
		val, err := NewValue(tt.value)
		if err != nil {
			t.Errorf("NewValue('%v'): %s", tt.value, err)
			continue
		}

		actual := val.Map()

		if reflect.DeepEqual(actual, tt.expected) == false {
			t.Errorf("Value.Map('%v'): expected '%#v', actual '%#v'", tt.value, tt.expected, actual)
		}

		val.Destroy()
	}

	c.Destroy()
}

func TestValuePtr(t *testing.T) {
	c, _ := e.NewContext()
	defer c.Destroy()

	val, err := NewValue(42)
	if err != nil {
		t.Fatalf("NewValue('%v'): %s", 42, err)
	}

	if val.Ptr() == nil {
		t.Errorf("Value.Ptr('%v'): Unable to create pointer from value", 42)
	}
}

func TestValueDestroy(t *testing.T) {
	c, _ := e.NewContext()
	defer c.Destroy()

	val, err := NewValue(42)
	if err != nil {
		t.Fatalf("NewValue('%v'): %s", 42, err)
	}

	val.Destroy()

	if val.value != nil {
		t.Errorf("Value.Destroy(): Did not set internal fields to `nil`")
	}

	// Attempting to destroy a value twice should be a no-op.
	val.Destroy()
}

func TestValueEnd(t *testing.T) {
	e.Destroy()
	t.SkipNow()
}
