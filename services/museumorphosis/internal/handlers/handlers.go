package handlers

import (
	"errors"
	"github.com/google/uuid"
	"github.com/gorilla/mux"
	"museumorphosis/internal"
	"museumorphosis/internal/handlers/dto"
	"museumorphosis/internal/service"
	"museumorphosis/pkg/utils"
	"net/http"
)

type Handlers struct {
	service *service.Service
}

func New(s *service.Service) *Handlers {
	return &Handlers{service: s}
}

func (h *Handlers) Register(w http.ResponseWriter, r *http.Request) {
	cred, err := utils.DecodeValidateRequest[dto.MuseumCredentials](r.Body)
	if err != nil {
		utils.ResponseError(w, http.StatusUnprocessableEntity, err)
		return
	}
	token, err := h.service.Register(cred)
	if err != nil {
		utils.ResponseServerError(w, err)
		return
	}
	utils.ResponseJSON(w, http.StatusOK, map[string]string{"token": token})
}

func (h *Handlers) Login(w http.ResponseWriter, r *http.Request) {
	cred, err := utils.DecodeValidateRequest[dto.MuseumLoginIn](r.Body)
	if err != nil {
		utils.ResponseError(w, http.StatusUnprocessableEntity, err)
		return
	}
	token, err := h.service.Login(cred)
	if err != nil {
		switch err {
		case internal.NotFoundError:
			utils.ResponseError(w, http.StatusNotFound, err)
		case internal.UnauthorizedError:
			utils.ResponseError(w, http.StatusUnauthorized, err)
		default:
			utils.ResponseServerError(w, err)
		}
		return
	}
	utils.ResponseJSON(w, http.StatusOK, map[string]string{"token": token})
}

func (h *Handlers) GetMuseumIdList(w http.ResponseWriter, _ *http.Request) {
	museumIdList, err := h.service.GetMuseumIdList()
	if err != nil {
		switch err {
		case internal.NotFoundError:
			utils.ResponseError(w, http.StatusNotFound, err)
		default:
			utils.ResponseServerError(w, err)
		}
		return
	}
	utils.ResponseJSON(w, http.StatusOK, dto.SerializeMuseumIdList(museumIdList))
}

func (h *Handlers) GetMuseumInfo(w http.ResponseWriter, r *http.Request) {
	id, err := uuid.Parse(mux.Vars(r)["id"])
	if err != nil {
		utils.ResponseError(w, http.StatusUnprocessableEntity, errors.New("id must be uuid"))
		return
	}
	museum, err := h.service.GetMuseum(id)
	if err != nil {
		switch err {
		case internal.NotFoundError:
			utils.ResponseError(w, http.StatusNotFound, err)
		default:
			utils.ResponseServerError(w, err)
		}
		return
	}
	utils.ResponseJSON(w, http.StatusOK, dto.SerializeMuseum(museum))
}

func (h *Handlers) GetExhibitListBySearch(w http.ResponseWriter, r *http.Request) {
	museumId, err := uuid.Parse(r.Header.Get("X-MuseumId"))
	if err != nil {
		utils.ResponseError(w, http.StatusUnprocessableEntity, errors.New("id must be uuid"))
	}
	search := r.URL.Query().Get("search")
	exhibits, err := h.service.GetExhibitList(museumId, search)
	if err != nil {
		switch err {
		case internal.NotFoundError:
			utils.ResponseError(w, http.StatusNotFound, err)
		default:
			utils.ResponseServerError(w, err)
		}
		return
	}
	utils.ResponseJSON(w, http.StatusOK, dto.SerializeExhibitList(exhibits))
}

func (h *Handlers) CreateExhibit(w http.ResponseWriter, r *http.Request) {
	museumId, err := uuid.Parse(r.Header.Get("X-MuseumId"))
	if err != nil {
		utils.ResponseError(w, http.StatusUnprocessableEntity, errors.New("id must be uuid"))
		return
	}
	data, err := utils.DecodeValidateRequest[dto.ExhibitIn](r.Body)
	if err != nil {
		utils.ResponseError(w, http.StatusUnprocessableEntity, err)
		return
	}
	exhibit, err := h.service.CreateExhibit(museumId, data)
	if err != nil {
		utils.ResponseServerError(w, err)
		return
	}
	utils.ResponseJSON(w, http.StatusOK, dto.SerializeExhibit(exhibit))
}

func (h *Handlers) GetExhibit(w http.ResponseWriter, r *http.Request) {
	museumId, err := uuid.Parse(r.Header.Get("X-MuseumId"))
	if err != nil {
		utils.ResponseError(w, http.StatusUnprocessableEntity, errors.New("id must be uuid"))
		return
	}
	exhibitId, err := uuid.Parse(mux.Vars(r)["id"])
	if err != nil {
		utils.ResponseError(w, http.StatusUnprocessableEntity, errors.New("id must be uuid"))
		return
	}
	exhibit, err := h.service.GetExhibit(museumId, exhibitId)
	if err != nil {
		switch err {
		case internal.NotFoundError:
			utils.ResponseError(w, http.StatusNotFound, err)
		default:
			utils.ResponseServerError(w, err)
		}
	}
	utils.ResponseJSON(w, http.StatusOK, dto.SerializeExhibit(exhibit))
}
