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

func (r *Repo) GetUser(ctx context.Context, username string) (map[string]string, error) {
	projection := options.FindOne().SetProjection(map[string]int{"username": 1, "password": 1, "permissions": 1})
	res := r.col.FindOne(ctx, map[string]string{"username": username}, projection)
	if err := res.Err(); err != nil {
		if err == mongo.ErrNoDocuments {
			return nil, common.ErrNotExists
		}
		return nil, fmt.Errorf("error while finding user: %w", err)
	}

	var u map[string]string

	if err := res.Decode(&u); err != nil {
		return nil, fmt.Errorf("error while decoding: %w", err)
	}

	return u, nil
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

func (r *Repo) GetUsersList(ctx context.Context) ([]string, error) {
	projection := options.Find().SetProjection(map[string]int{"_id": 0, "username": 1})
	res, err := r.col.Find(ctx, map[string]string{}, projection)
	if err != nil {
		return nil, fmt.Errorf("error while fetching users list: %w", err)
	}

	var a []map[string]string

	if err = res.All(ctx, &a); err != nil {
		return nil, fmt.Errorf("error while decoding: %w", err)
	}

	var users []string

	for _, u := range a {
		users = append(users, u["username"])
	}

	return users, nil
}

func (r *Repo) GetFriendsList(ctx context.Context, username string) ([]string, error) {
	projection := options.FindOne().SetProjection(map[string]int{"_id": 0, "friends": 1})
	res := r.col.FindOne(ctx, map[string]string{"username": username}, projection)
	if err := res.Err(); err != nil {
		return nil, fmt.Errorf("error while fetching user friends: %w", err)
	}

	var a map[string][]map[string]string

	if err := res.Decode(&a); err != nil {
		return nil, fmt.Errorf("error while decoding: %w", err)
	}

	friends := make([]string, 0)

	friendsList, ok := a["friends"]
	if ok {
		for _, f := range friendsList {
			for k := range f {
				friends = append(friends, k)
				break
			}
		}
	}

	return friends, nil
}

func (r *Repo) GetUserJokes(ctx context.Context, username string) ([]map[string]string, error) {
	projection := options.FindOne().SetProjection(map[string]int{"_id": 0, "jokes": 1})
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

	return jokes["jokes"], nil
}

func (r *Repo) AddJoke(ctx context.Context, username string, joke map[string]map[string]string) error {
	res, err := r.col.UpdateOne(ctx, map[string]string{"username": username}, map[string]map[string]map[string]string{"$push": joke})
	if err != nil {
		return fmt.Errorf("error while adding joke: %w", err)
	}
	if res.MatchedCount == 0 {
		return common.ErrNotExists
	}

	return nil
}

func (r *Repo) AddFriendRequest(ctx context.Context, from, to string) error {
	res, err := r.col.UpdateOne(ctx, map[string]string{"username": to}, map[string]map[string]string{"$push": {"friend_requests": from}})
	if err != nil {
		return fmt.Errorf("error while adding friend reqeust: %w", err)
	}
	if res.MatchedCount == 0 {
		return common.ErrNotExists
	}

	return nil
}

func (r *Repo) AcceptFriendRequest(ctx context.Context, username, from string) error {
	res, err := r.col.UpdateOne(ctx, map[string]string{"username": username}, map[string]map[string]string{"$pull": {"friend_requests": from}})
	if err != nil {
		return fmt.Errorf("error while deleting friend request: %w", err)
	}
	if res.MatchedCount == 0 {
		return common.ErrNotExists
	}

	return nil
}

func (r *Repo) MakeFriends(ctx context.Context, user1, user2 string) error {
	res, err := r.col.UpdateOne(ctx, map[string]string{"username": user1}, map[string]map[string]map[string]string{"$push": {"friends": {user2: "yes"}}})
	if err != nil {
		return fmt.Errorf("error while adding friend to user1: %w", err)
	}
	if res.MatchedCount == 0 {
		return common.ErrNotExists
	}

	res, err = r.col.UpdateOne(ctx, map[string]string{"username": user2}, map[string]map[string]map[string]string{"$push": {"friends": {user1: "yes"}}})
	if err != nil {
		return fmt.Errorf("error while adding friend to user2: %w", err)
	}
	if res.MatchedCount == 0 {
		return common.ErrNotExists
	}

	return nil
}

func (r *Repo) GetRequestsList(ctx context.Context, username string) ([]string, error) {
	projection := options.FindOne().SetProjection(map[string]int{"_id": 0, "friend_requests": 1})
	res := r.col.FindOne(ctx, map[string]string{"username": username}, projection)
	if err := res.Err(); err != nil {
		if err == mongo.ErrNoDocuments {
			return nil, common.ErrNotExists
		}
		return nil, fmt.Errorf("error while finding friend requests: %w", err)
	}

	var a map[string][]string

	if err := res.Decode(&a); err != nil {
		return nil, fmt.Errorf("error while decoding: %w", err)
	}

	requests := make([]string, 0)

	requestsList, ok := a["friend_requests"]
	if ok {
		for _, re := range requestsList {
			requests = append(requests, re)
		}
	}

	return requests, nil
}

func (r *Repo) Clean(ctx context.Context) error {
	_, err := r.col.UpdateMany(ctx, map[string]string{}, map[string]map[string]string{{"$lt" : new Date(Date.now() - 2*60*60 * 1000) }, "$unset": {
		"jokes": "", "friends": "", "friend_requests": ""}})
	if err != nil {
		return fmt.Errorf("error while cleaning: %w", err)
	}

	return nil
}
