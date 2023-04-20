package infra

import (
	"context"
	"fmt"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
	"ructf-2023-finals/lost-museum/internal/common"
)

type Repo struct {
	col *mongo.Collection
}

func NewRepo(col *mongo.Collection) *Repo {
	return &Repo{col: col}
}

func (r *Repo) GetUser(ctx context.Context, username string) (map[string]string, []string, error) {
	projection := options.FindOne().SetProjection(map[string]int{"username": 1, "password": 1, "permissions": 1})
	res := r.col.FindOne(ctx, map[string]string{"username": username}, projection)
	if err := res.Err(); err != nil {
		if err == mongo.ErrNoDocuments {
			return nil, nil, common.ErrNotExists
		}
		return nil, nil, fmt.Errorf("error while finding user: %w", err)
	}

	u := struct {
		Username    string
		Password    string
		Permissions []map[string]string
	}{}

	if err := res.Decode(&u); err != nil {
		return nil, nil, fmt.Errorf("error while decoding: %w", err)
	}

	perms := make([]string, 0, len(u.Permissions))
	for _, perm := range u.Permissions {
		for p := range perm {
			perms = append(perms, p)
		}
	}

	return map[string]string{"username": u.Username, "password": u.Password}, perms, nil
}

func (r *Repo) AddUser(ctx context.Context, u map[string]string) error {
	if _, err := r.col.InsertOne(ctx, u); err != nil {
		return fmt.Errorf("error while adding user: %w", err)
	}

	return nil
}

func (r *Repo) CheckUserExists(ctx context.Context, username string) (bool, error) {
	err := r.col.FindOne(ctx, map[string]string{"username": username}).Err()
	if err != nil {
		if err == mongo.ErrNoDocuments {
			return false, nil
		}
		return false, fmt.Errorf("error while findind user: %w", err)
	}

	return true, nil
}

func (r *Repo) GetUserJokes(ctx context.Context, username string) (map[string][]map[string]string, error) {
	projection := options.FindOne().SetProjection(map[string]int{"_id": 0, "hidden": 1, "visible": 1})
	res := r.col.FindOne(ctx, map[string]string{"username": username}, projection)
	if err := res.Err(); err != nil {
		if err == mongo.ErrNoDocuments {
			return nil, common.ErrNotExists
		}
		return nil, fmt.Errorf("error while finding joke: %w", err)
	}

	jokes := make(map[string][]map[string]string)

	if err := res.Decode(jokes); err != nil {
		return nil, fmt.Errorf("error while decoding: %w", err)
	}

	return jokes, nil
}

func (r *Repo) AddJoke(ctx context.Context, username string, joke map[string]map[string]string) error {
	res, err := r.col.UpdateOne(ctx, map[string]string{"username": username}, map[string]map[string]map[string]string{"$push": joke})
	if err != nil {
		return fmt.Errorf("error while updating: %w", err)
	}
	if res.MatchedCount == 0 {
		return common.ErrNotExists
	}

	return nil
}

func (r *Repo) GetThemeJokes(ctx context.Context, theme string) ([]map[string]string, error) {
	res, err := r.col.Find(ctx, map[string]string{"visible.theme": theme})
	if err != nil {
		return nil, fmt.Errorf("error while finding theme jokes: %w", err)
	}

	var a []any

	fmt.Println(res.All(ctx, &a))

	return nil, nil
}
