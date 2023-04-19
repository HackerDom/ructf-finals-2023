package dto

import "github.com/google/uuid"

type MuseumCredentials struct {
	Name     string `json:"name" validate:"required"`
	Password string `json:"password" validate:"required"`
}

type MuseumLoginIn struct {
	Id       uuid.UUID `json:"id" validate:"required"`
	Password string    `json:"password" validate:"required"`
}

type ExhibitIn struct {
	Title       string `json:"title" validate:"required"`
	Description string `json:"description" validate:"required"`
	Metadata    string `json:"metadata" validate:"required"`
}
