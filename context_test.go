package php

import (
	"os"
	"path"
	"testing"
)

var testDir string

var execTests = []struct {
	file     string // Filename to run
	expected string // Expected output
}{
	{"hello.php", "Hello World"},
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
			t.Errorf("ContextExec(%s): %s", tt.file, err)
		}

		actual := w.String()
		if actual != tt.expected {
			t.Errorf("ContextExec(%s): expected '%s', actual '%s'", tt.file, tt.expected, actual)
		}
	}
}

func init() {
	wd, _ := os.Getwd()
	testDir = path.Join(wd, "tests")
}
