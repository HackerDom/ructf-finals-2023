package server

import (
	"context"
	"net/http"
	"strconv"

	"github.com/gorilla/mux"
)

type Server struct {
	*http.Server
}

func New(port int, h *mux.Router) *Server {
	return &Server{
		&http.Server{
			Addr:           ":" + strconv.Itoa(port),
			Handler:        h,
			MaxHeaderBytes: 1 << 20,
		},
	}
}

func (s *Server) Start() error {
	return s.ListenAndServe()
}

func (s *Server) Shutdown(ctx context.Context) error {
	return s.Server.Shutdown(ctx)
}
