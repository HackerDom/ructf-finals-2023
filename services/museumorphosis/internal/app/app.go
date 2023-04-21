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
	api := r.PathPrefix("/api").Subrouter()

	auth := handlers.AuthMiddleware

	api.HandleFunc("/register", a.handlers.Register).Methods("POST")
	api.HandleFunc("/login", a.handlers.Login).Methods("POST")

	api.HandleFunc("/museums", a.handlers.GetMuseumIdList).Methods("GET")
	api.HandleFunc("/museums/{id}", a.handlers.GetMuseumInfo).Methods("GET")
	api.HandleFunc("/museums/{id}/exhibits", a.handlers.GetExhibitsByMuseum).Methods("GET")

	api.Handle("/museum/exhibits", auth(http.HandlerFunc(a.handlers.GetMyExhibitListBySearch))).Methods("GET")
	api.Handle("/museum/exhibits", auth(http.HandlerFunc(a.handlers.CreateExhibit))).Methods("POST")
	api.Handle("/museum/exhibits/{id}", auth(http.HandlerFunc(a.handlers.GetMyExhibit))).Methods("GET")

	return r
}
