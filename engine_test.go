package php

import (
	"testing"
)

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
	}

	defer e.Destroy()

	ctx, err := e.NewContext(&MockWriter{})
	if err != nil {
		t.Errorf("NewContext(): %s", err)
	}

	defer ctx.Destroy()
}
