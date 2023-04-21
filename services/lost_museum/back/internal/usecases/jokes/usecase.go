package jokes

import (
	"bytes"
	"context"
	"errors"
	"fmt"
	"golang.org/x/exp/slices"
	"gopkg.in/yaml.v2"
	"ructf-2023-finals/lost-museum/internal/common"
	"ructf-2023-finals/lost-museum/internal/infra"
	"text/template"
)

var (
	statuses = []string{"private", "public"}
)

type UseCase struct {
	repo *infra.Repo
}

func New(r *infra.Repo) *UseCase {
	return &UseCase{repo: r}
}

func (u *UseCase) GetJoke(ctx context.Context, requester, username, theme string) (map[string]string, error) {
	jokes, err := u.repo.GetUserJokes(ctx, username)
	if err != nil {
		return nil, fmt.Errorf("error while fetching jokes: %w", err)
	}

	friends, err := u.repo.GetFriendsList(ctx, requester)
	if err != nil {
		return nil, fmt.Errorf("error while fetching friends list: %w", err)
	}

	for _, joke := range jokes {
		t, themeOK := joke["theme"]
		s, statusOK := joke["status"]
		if themeOK && statusOK && t == theme {
			if s == "public" || s == "private" && slices.Contains(friends, username) || requester == username {
				return joke, nil
			}
		}
	}

	return nil, common.ErrNotExists
}

func (u *UseCase) CreateJoke(ctx context.Context, username, status, theme, text string) error {
	exJoke, err := u.GetJoke(ctx, username, username, theme)
	if exJoke != nil {
		return common.ErrAlreadyExists
	}
	if err != nil && !errors.Is(err, common.ErrNotExists) {
		return fmt.Errorf("error while fetching jokes: %w", err)
	}

	if !slices.Contains(statuses, status) {
		return common.ErrInvalidStatus
	}

	joke, err := fillJokeTemplate("joke.tmpl", map[string]string{"Status": status, "Theme": theme, "Text": text})
	if err != nil {
		return fmt.Errorf("error while filling template: %w", err)
	}

	if err = u.repo.AddJoke(ctx, username, joke); err != nil {
		return fmt.Errorf("error while adding joke: %w", err)
	}

	return nil
}

func (u *UseCase) GetUserJokesList(ctx context.Context, username string) ([]map[string]string, error) {
	jokes, err := u.repo.GetUserJokes(ctx, username)
	if err != nil {
		return nil, fmt.Errorf("error while fetching user jokes: %w", err)
	}

	return jokes, nil
}

func (u *UseCase) GetThemeJokes(ctx context.Context, username, theme string) ([]map[string]string, error) {
	users, err := u.repo.GetUsersList(ctx)
	if err != nil {
		return nil, fmt.Errorf("error while fetching users list: %w", err)
	}

	friends, err := u.repo.GetFriendsList(ctx, username)
	if err != nil {
		return nil, fmt.Errorf("error while fetching friends: %w", err)
	}

	var res []map[string]string

	for _, user := range users {
		jokes, err := u.repo.GetUserJokes(ctx, user)
		if err != nil {
			return nil, fmt.Errorf("error while fetching user jokes: %w", err)
		}

		for _, joke := range jokes {
			s, statusOK := joke["status"]
			t, themeOK := joke["theme"]
			_, textOK := joke["text"]
			if statusOK && themeOK && textOK && t == theme {
				if s == "public" || s == "private" && slices.Contains(friends, user) {
					res = append(res, joke)
				}
			}
		}
	}

	return res, nil
}

func fillJokeTemplate(file string, values map[string]string) (map[string]map[string]string, error) {
	tmpl, err := template.ParseFiles(file)
	if err != nil {
		return nil, fmt.Errorf("error while parsing template: %w", err)
	}

	buf := new(bytes.Buffer)

	if err = tmpl.Execute(buf, values); err != nil {
		return nil, fmt.Errorf("error while executing template: %w", err)
	}

	var v map[string]map[string]string

	if err = yaml.Unmarshal(buf.Bytes(), &v); err != nil {
		return nil, fmt.Errorf("error while unmarshalling: %w", err)
	}

	return v, nil
}
