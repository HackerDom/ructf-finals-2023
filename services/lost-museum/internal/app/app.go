package app

import (
	"context"
)

type AuthUseCase interface {
	Register(ctx context.Context, username, password string) (string, error)
	Login(ctx context.Context, username, password string) (string, error)
}

type JokesUseCase interface {
	GetJoke(ctx context.Context, username, theme string) (map[string]string, error)
	CreateJoke(ctx context.Context, username, theme, text string) error
	GetUserJokesList(ctx context.Context, username string) ([]map[string]string, error)
	GetThemeJokes(ctx context.Context, theme string) ([]map[string]string, error)
}

type App struct {
	Auth  AuthUseCase
	Jokes JokesUseCase
}

func New(auth AuthUseCase, jokes JokesUseCase) App {
	return App{
		Auth:  auth,
		Jokes: jokes,
	}
}
