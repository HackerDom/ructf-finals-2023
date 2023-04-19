package create

import (
	"database/sql"
	"database/sql/driver"
	"fmt"
	"github.com/google/uuid"
	_ "github.com/mattn/go-sqlite3"
	"os"
	"path/filepath"
	"regexp"
)

var Regex = regexp.MustCompile("CREATE DATABASE WITH \\((?P<schema>.+?)\\) SCHEMA")

type result struct {
	dbId string
}

func (r *result) Columns() []string {
	return []string{"id"}
}

func (r *result) Close() error {
	return nil
}

func (r *result) Next(dest []driver.Value) error {
	if r.dbId != "" {
		dest[0] = r.dbId
	}
	return nil
}

func CreateDatabase(path, schema string) (driver.Rows, error) {
	dbId, err := uuid.NewRandom()
	if err != nil {
		return nil, fmt.Errorf("error in create db while generate db id: %w", err)
	}

	dbPath := filepath.Join(path, dbId.String())
	file, err := os.Create(dbPath)
	if err != nil {
		return nil, fmt.Errorf("error in create db while create db file: %w", err)
	}
	file.Close()

	db, err := sql.Open("sqlite3", dbPath)
	if err != nil {
		return nil, fmt.Errorf("error in create db while open db conn: %w", err)
	}
	if _, err := db.Exec(schema); err != nil {
		return nil, fmt.Errorf("error in create db while exec schema: %w", err)
	}
	db.Close()
	return &result{dbId: dbId.String()}, nil
}
