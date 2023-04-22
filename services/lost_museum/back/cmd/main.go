package main

import (
	"context"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
	"log"
	"ructf-2023-finals/lost-museum/internal/app"
	"ructf-2023-finals/lost-museum/internal/infra"
	"ructf-2023-finals/lost-museum/internal/server"
	"ructf-2023-finals/lost-museum/internal/usecases/auth"
	"ructf-2023-finals/lost-museum/internal/usecases/friends"
	"ructf-2023-finals/lost-museum/internal/usecases/jokes"
)

var ctx = context.Background()

func main() {
	client, err := mongo.Connect(ctx, options.Client().ApplyURI("mongodb://mongo:27017"))
	if err != nil {
		log.Fatal(err)
	}

	secret := "HUI"

	r := infra.NewRepo(client.Database("jokes").Collection("jokes"))

	a := app.New(auth.New(r, secret), jokes.New(r), friends.New(r))

	s := server.NewServer(":8000", secret, a)

	if err = s.Run(); err != nil {
		log.Fatal(err)
	}
}
