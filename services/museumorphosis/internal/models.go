package internal

import "errors"

var UnauthorizedError = errors.New("unauthorized, please check your id or password")

var NotFoundError = errors.New("object not found")

type Exhibit struct {
	Id          string
	Title       string
	Description string
	Metadata    string
}

type Museum struct {
	Id       string
	Name     string
	Password string
}
