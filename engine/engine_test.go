package engine_test

import (
	"os"
	"testing"

	php "github.com/deuill/go-php"
)

func TestNewEngineContext(t *testing.T) {
	e, err := php.New()
	if err != nil {
		t.Errorf("New(): %s", err)
	}

	defer e.Destroy()

	ctx, err := php.NewContext(os.Stdout)
	if err != nil {
		t.Errorf("NewContext(): %s", err)
	}

	defer ctx.Destroy()
}
