package app

import (
	"database/sql"
	"github.com/gorilla/mux"
	"museumorphosis/internal/db"
	"museumorphosis/internal/handlers"
	"museumorphosis/internal/repository"
	"museumorphosis/internal/service"
	"net/http"
)

type App struct {
	db       *sql.DB
	handlers *handlers.Handlers
}

func New() *App {
	return &App{}
}

func (a *App) Init() (*App, error) {
	dbClient, err := db.New()
	if err != nil {
		return nil, err
	}
	a.db = dbClient

	repo := repository.NewRepository(a.db)
	srv := service.NewService(repo)
	a.handlers = handlers.New(srv)
	return a, nil
}

func (a *App) GetApiRoutes() *mux.Router {
	r := mux.NewRouter()

	auth := handlers.AuthMiddleware

	r.HandleFunc("/register", a.handlers.Register).Methods("POST")
	r.HandleFunc("/login", a.handlers.Login).Methods("POST")

	r.HandleFunc("/museums", a.handlers.GetMuseumIdList).Methods("GET")
	r.HandleFunc("/museums/{id}", a.handlers.GetMuseumInfo).Methods("GET")

	r.Handle("/museum/exhibits", auth(http.HandlerFunc(a.handlers.GetExhibitListBySearch))).Methods("GET")
	r.Handle("/museum/exhibits", auth(http.HandlerFunc(a.handlers.CreateExhibit))).Methods("POST")
	r.Handle("/museum/exhibits/{id}", auth(http.HandlerFunc(a.handlers.GetExhibit))).Methods("GET")

	return r
}
