package jokes

import (
	"bytes"
	"context"
	"fmt"
	"gopkg.in/yaml.v2"
	"ructf-2023-finals/lost-museum/internal/common"
	"ructf-2023-finals/lost-museum/internal/infra"
	"text/template"
)

type UseCase struct {
	repo *infra.Repo
}

func New(r *infra.Repo) *UseCase {
	return &UseCase{repo: r}
}

func (u *UseCase) GetJoke(ctx context.Context, username, theme string) (map[string]string, error) {
	userJokes, err := u.repo.GetUserJokes(ctx, username)
	if err != nil {
		return nil, fmt.Errorf("error while fetching jokes: %w", err)
	}

	jokes := toList(userJokes)

	for _, joke := range jokes {
		if joke["theme"] == theme {
			return joke, nil
		}
	}

	return nil, common.ErrNotExists
}

func (u *UseCase) CreateJoke(ctx context.Context, username, theme, text string) error {
	exJoke, err := u.GetJoke(ctx, username, theme)
	if exJoke != nil {
		return common.ErrAlreadyExists
	}
	if err != nil && err != common.ErrNotExists {
		return fmt.Errorf("error while fetching jokes: %w", err)
	}

	joke, err := fillJokeTemplate("joke.tmpl", map[string]string{"Theme": theme, "Text": text})
	if err != nil {
		return fmt.Errorf("error while filling template: %w", err)
	}

	if err = u.repo.AddJoke(ctx, username, joke); err != nil {
		return fmt.Errorf("error while adding joke: %w", err)
	}

	return nil
}

func (u *UseCase) GetUserJokesList(ctx context.Context, username string) ([]map[string]string, error) {
	userJokes, err := u.repo.GetUserJokes(ctx, username)
	if err != nil {
		return nil, fmt.Errorf("error while fetching user jokes: %w", err)
	}

	return toList(userJokes), nil
}

func (u *UseCase) GetThemeJokes(ctx context.Context, theme string) ([]map[string]string, error) {
	jokes, err := u.repo.GetThemeJokes(ctx, theme)
	if err != nil {
		return nil, fmt.Errorf("error while fetching theme jokes: %w", err)
	}

	return jokes, nil
}

func toList(jokes map[string][]map[string]string) []map[string]string {
	res := make([]map[string]string, 0)

	for _, status := range []string{"hidden", "visible"} {
		data, ok := jokes[status]
		if ok {
			for _, joke := range data {
				theme, themeOK := joke["theme"]
				text, textOK := joke["text"]
				if themeOK && textOK {
					res = append(res, map[string]string{"theme": theme, "text": text, "status": status})
				}
			}
		}
	}

	return res
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
