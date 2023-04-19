package db

import (
	"database/sql"
	_ "museumorphosis/pkg/fsql"
)

func New() (*sql.DB, error) {
	db, err := sql.Open("fsql", "")
	if err != nil {
		return nil, err
	}
	return db, nil
}
