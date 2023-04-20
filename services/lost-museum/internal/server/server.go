package server

import (
	"github.com/gofiber/fiber/v2"
	"ructf-2023-finals/lost-museum/internal/app"
)

type Server struct {
	port string
	app  *fiber.App
}

func NewServer(port, secret string, a app.App) *Server {
	s := &Server{port: port, app: fiber.New()}
	api := s.app.Group("/api")

	api.Post("/register", Register(a.Auth))
	api.Post("/login", Login(a.Auth))

	auth := api.Group("/")
	auth.Use(AuthMiddleware(secret))

	jokes := auth.Group("/jokes")
	{
		jokes.Get("/", GetJoke(a.Jokes))
		jokes.Post("/", CreateJoke(a.Jokes))
		jokes.Get("/list", GetUserJokesList(a.Jokes))
		jokes.Get("/list/:theme", GetThemeJokes(a.Jokes))
	}

	return s
}

func (s *Server) Run() error {
	return s.app.Listen(s.port)
}
