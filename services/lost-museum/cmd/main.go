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
	"ructf-2023-finals/lost-museum/internal/usecases/jokes"
)

var ctx = context.Background()

func main() {
	client, err := mongo.Connect(ctx, options.Client().ApplyURI("mongodb://mongo:27017"))
	if err != nil {
		log.Fatal(err)
	}

	//col := client.Database("jokes").Collection("jokes")
	//
	//col.InsertOne(ctx, map[string]string{"username": "danilkaz", "password": "gagaga", "is_moderator": "false"})
	//
	//col.UpdateOne(ctx, map[string]string{"username": "danilkaz"}, map[string]map[string]map[string]string{"$push": {"hidden": {"theme": "theme1", "text": "text1"}}})
	//col.UpdateOne(ctx, map[string]string{"username": "danilkaz"}, map[string]map[string]map[string]string{"$push": {"hidden": {"theme": "theme3", "text": "text3"}}})
	//
	//var a map[string][]map[string]string
	//
	//b := col.FindOne(ctx, map[string]string{"username": "danilkaz"}, options.FindOne().SetProjection(map[string]int{"_id": 0, "hidden": 1, "visible": 1}))
	//fmt.Println(b.DecodeBytes())
	//
	//b.Decode(&a)
	//
	//fmt.Println(a)
	//
	//time.Sleep(time.Hour)

	secret := "HUI"

	r := infra.NewRepo(client.Database("jokes").Collection("jokes"))

	a := app.New(auth.New(r, secret), jokes.New(r))

	s := server.NewServer(":8000", secret, a)

	if err = s.Run(); err != nil {
		log.Fatal(err)
	}
}
