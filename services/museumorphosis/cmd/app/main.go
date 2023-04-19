package main

import (
	"context"
	"log"
	"museumorphosis/configs"
	"museumorphosis/internal/app"
	"museumorphosis/pkg/server"
	"os"
	"os/signal"
)

func main() {
	cfg, err := configs.Init()
	if err != nil {
		log.Fatal(err)
	}
	a, err := app.New().Init()
	if err != nil {
		log.Fatal(err)
	}
	srv := server.New(cfg.Port, a.GetApiRoutes())
	log.Println("Server started at", srv.Addr)
	go func() {
		if err := srv.Start(); err != nil {
			log.Fatal(err)
		}
	}()

	quit := make(chan os.Signal, 1)
	signal.Notify(quit, os.Interrupt, os.Kill)
	<-quit

	if err := srv.Shutdown(context.Background()); err != nil {
		log.Printf("%v", err)
	}
}
