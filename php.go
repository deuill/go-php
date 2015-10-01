package php

import (
	"io"

	"github.com/deuill/go-php/context"
	"github.com/deuill/go-php/engine"
)

func New() (*engine.Engine, error) {
	return engine.New()
}

func NewContext(w io.Writer) (*context.Context, error) {
	return context.New(w)
}
