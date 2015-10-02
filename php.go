package php

import (
	"io"

	"./context"
	"./engine"
)

func New() (*engine.Engine, error) {
	return engine.New()
}

func NewContext(w io.Writer) (*context.Context, error) {
	return context.New(w)
}
