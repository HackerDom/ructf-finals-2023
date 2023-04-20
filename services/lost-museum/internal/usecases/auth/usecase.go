package auth

import (
	"context"
	"errors"
	"fmt"
	"ructf-2023-finals/lost-museum/internal/common"
	"ructf-2023-finals/lost-museum/internal/infra"
)

type UseCase struct {
	repo *infra.Repo

	secret string
}

func New(r *infra.Repo, s string) *UseCase {
	return &UseCase{
		repo:   r,
		secret: s,
	}
}

func (u *UseCase) Register(ctx context.Context, username, password string) (string, error) {
	hash, err := common.HashPassword(password)
	if err != nil {
		return "", fmt.Errorf("error while hashing password: %w", err)
	}

	user := map[string]string{"username": username, "password": hash}

	exists, err := u.repo.CheckUserExists(ctx, username)
	if err != nil {
		return "", fmt.Errorf("error while checking user exists: %w", err)
	}

	if exists {
		return "", common.ErrAlreadyExists
	}

	if err = u.repo.AddUser(ctx, user); err != nil {
		return "", fmt.Errorf("error while adding user: %w", err)
	}

	token, err := infra.GenerateToken(user, []string{}, u.secret)
	if err != nil {
		return "", fmt.Errorf("error while generating token: %w", err)
	}

	return token, nil
}

func (u *UseCase) Login(ctx context.Context, username, password string) (string, error) {
	exists, err := u.repo.CheckUserExists(ctx, username)
	if err != nil {
		return "", fmt.Errorf("error while checking user exists: %w", err)
	}

	if !exists {
		return "", common.ErrNotExists
	}

	user, perms, err := u.repo.GetUser(ctx, username)
	if err != nil {
		return "", fmt.Errorf("error while fetching user: %w", err)
	}

	hash, ok := user["password"]
	if !ok {
		return "", errors.New("missing password field")
	}

	if err = common.CheckPasswordHash(password, hash); err != nil {
		return "", common.ErrInvalidPassword
	}

	token, err := infra.GenerateToken(user, perms, u.secret)
	if err != nil {
		return "", fmt.Errorf("error while generating token: %w", err)
	}

	return token, nil
}
