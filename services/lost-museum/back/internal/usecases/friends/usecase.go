package friends

import (
	"context"
	"fmt"
	"ructf-2023-finals/lost-museum/internal/infra"
)

type UseCase struct {
	repo *infra.Repo
}

func New(r *infra.Repo) *UseCase {
	return &UseCase{repo: r}
}

func (u *UseCase) CreateFriendRequest(ctx context.Context, from, to string) error {
	if err := u.repo.AddFriendRequest(ctx, from, to); err != nil {
		return fmt.Errorf("error while adding friend requets: %w", err)
	}

	return nil
}

func (u *UseCase) AcceptFriendRequest(ctx context.Context, username, from string) error {
	if err := u.repo.AcceptFriendRequest(ctx, username, from); err != nil {
		return fmt.Errorf("error while accepting friend request: %w", err)
	}

	if err := u.repo.MakeFriends(ctx, username, from); err != nil {
		return fmt.Errorf("error while making friends: %w", err)
	}

	return nil
}

func (u *UseCase) GetRequestsList(ctx context.Context, username string) ([]string, error) {
	return u.repo.GetRequestsList(ctx, username)
}

func (u *UseCase) GetFriendsList(ctx context.Context, username string) ([]string, error) {
	return u.repo.GetFriendsList(ctx, username)
}
