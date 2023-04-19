package service

import (
	"github.com/google/uuid"
	"museumorphosis/configs"
	"museumorphosis/internal"
	"museumorphosis/internal/handlers/dto"
	"museumorphosis/internal/repository"
	"museumorphosis/pkg/jwt"
)

type Service struct {
	repo *repository.Repository
}

func NewService(repo *repository.Repository) *Service {
	return &Service{repo: repo}
}

func (s *Service) Register(cred dto.MuseumCredentials) (token string, err error) {
	id, err := s.repo.CreateMuseum(cred.Name, cred.Password)
	if err != nil {
		return "", err
	}
	return jwt.GenToken(id.String(), configs.Cfg.JWTSecret)
}

func (s *Service) Login(cred dto.MuseumLoginIn) (token string, err error) {
	museum, err := s.repo.GetMuseumById(cred.Id)
	if err != nil {
		return "", err
	}
	if museum.Password != cred.Password {
		return "", internal.UnauthorizedError
	}
	return jwt.GenToken(museum.Id, configs.Cfg.JWTSecret)
}

func (s *Service) GetMuseum(id uuid.UUID) (*internal.Museum, error) {
	return s.repo.GetMuseumById(id)
}

func (s *Service) GetMuseumIdList() ([]uuid.UUID, error) {
	return s.repo.GetMuseumIdList()
}

func (s *Service) CreateExhibit(museumId uuid.UUID, data dto.ExhibitIn) (*internal.Exhibit, error) {
	exhibitId, err := s.repo.CreateExhibit(museumId, data)
	if err != nil {
		return nil, err
	}
	return s.GetExhibit(museumId, exhibitId)
}

func (s *Service) GetExhibit(museumId, exhibitId uuid.UUID) (*internal.Exhibit, error) {
	exists, err := s.repo.CheckExhibitInMuseum(museumId, exhibitId)
	if err != nil {
		return nil, err
	}
	if !exists {
		return nil, internal.NotFoundError
	}
	return s.repo.GetExhibit(museumId, exhibitId)
}

func (s *Service) GetExhibitList(museumId uuid.UUID, search string) ([]*internal.Exhibit, error) {
	return s.repo.GetExhibitList(museumId, search)
}
