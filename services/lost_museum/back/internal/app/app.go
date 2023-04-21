package app

import (
	"context"
)

type AuthUseCase interface {
	Register(ctx context.Context, username, password string) (string, error)
	Login(ctx context.Context, username, password string) (string, error)
}

type JokesUseCase interface {
	GetJoke(ctx context.Context, requester, username, theme string) (map[string]string, error)
	CreateJoke(ctx context.Context, username, status, theme, text string) error
	GetUserJokesList(ctx context.Context, username string) ([]map[string]string, error)
	GetThemeJokes(ctx context.Context, username, theme string) ([]map[string]string, error)
}

type FriendsUseCase interface {
	CreateFriendRequest(ctx context.Context, from, to string) error
	AcceptFriendRequest(ctx context.Context, username, from string) error
	GetRequestsList(ctx context.Context, username string) ([]string, error)
	GetFriendsList(ctx context.Context, username string) ([]string, error)
}

type App struct {
	Auth    AuthUseCase
	Jokes   JokesUseCase
	Friends FriendsUseCase
}

func New(auth AuthUseCase, jokes JokesUseCase, friends FriendsUseCase) App {
	return App{
		Auth:    auth,
		Jokes:   jokes,
		Friends: friends,
	}
}
