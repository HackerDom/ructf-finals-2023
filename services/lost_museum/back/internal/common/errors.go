package common

import "errors"

var (
	ErrAlreadyExists   = errors.New("already exists")
	ErrNotExists       = errors.New("not exists")
	ErrInvalidPassword = errors.New("invalid password")
	ErrInvalidStatus   = errors.New("invalid status")
)
