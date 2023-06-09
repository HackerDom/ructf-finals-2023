package dto

import (
	"github.com/google/uuid"
	"museumorphosis/internal"
)

type MuseumOut struct {
	Id   string `json:"id"`
	Name string `json:"name"`
}

func SerializeMuseum(museum *internal.Museum) *MuseumOut {
	return &MuseumOut{Id: museum.Id, Name: museum.Name}
}

type ExhibitOut struct {
	Id          string `json:"id"`
	Title       string `json:"title"`
	Description string `json:"description"`
	Metadata    string `json:"metadata"`
}

func SerializeExhibit(exhibit *internal.Exhibit, isAuth bool) *ExhibitOut {
	if isAuth {
		return &ExhibitOut{
			Id:          exhibit.Id,
			Title:       exhibit.Title,
			Description: exhibit.Description,
			Metadata:    exhibit.Metadata,
		}
	}
	return &ExhibitOut{
		Id:          exhibit.Id,
		Title:       exhibit.Title,
		Description: exhibit.Description,
	}
}

type ExhibitListOut struct {
	Exhibits []*ExhibitOut `json:"exhibits"`
}

func SerializeExhibitList(exhibits []*internal.Exhibit, isAuth bool) *ExhibitListOut {
	res := make([]*ExhibitOut, 0)
	for _, exhibit := range exhibits {
		res = append(res, SerializeExhibit(exhibit, isAuth))
	}
	return &ExhibitListOut{Exhibits: res}
}

type MuseumIdList struct {
	MuseumIdList []string `json:"museum_id_list"`
}

func SerializeMuseumIdList(museumIds []uuid.UUID) *MuseumIdList {
	res := make([]string, 0)
	for _, id := range museumIds {
		res = append(res, id.String())
	}
	return &MuseumIdList{MuseumIdList: res}
}
