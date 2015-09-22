package php

import (
	"os"
	"testing"
)

func TestNewEngineContext(t *testing.T) {
	e, err := New()
	if err != nil {
		t.Errorf("New(): %s", err)
	}

	defer e.Destroy()

	ctx, err := e.NewContext(os.Stdout)
	if err != nil {
		t.Errorf("NewContext(): %s", err)
	}

	defer ctx.Destroy()
}
