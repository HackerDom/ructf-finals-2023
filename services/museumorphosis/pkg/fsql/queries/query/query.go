package query

import (
	"database/sql"
	"database/sql/driver"
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"regexp"
)

var Regex = regexp.MustCompile("QUERY \\((?P<query>.+?)\\) AT DATABASE \\((?P<id>.+?)\\)")

type result struct {
	rows    *sql.Rows
	columns []string
}

func (r *result) Columns() []string {
	return r.columns
}

func (r *result) Close() error {
	return r.rows.Close()
}

func (r *result) Next(dest []driver.Value) error {
	ok := r.rows.Next()
	if !ok {
		return io.EOF
	}
	pointers := make([]any, len(dest))
	for i, _ := range pointers {
		pointers[i] = &dest[i]
	}
	return r.rows.Scan(pointers...)
}

func QueryInDatabase(path, query, dbId string) (driver.Rows, error) {
	fmt.Println(query, dbId)
	dbPath := filepath.Join(path, dbId)
	if _, err := os.Stat(dbPath); errors.Is(err, os.ErrNotExist) {
		return nil, sql.ErrNoRows
	}
	db, err := sql.Open("sqlite3", dbPath)
	if err != nil {
		return nil, fmt.Errorf("error in query while open db conn: %w", err)
	}

	rows, err := db.Query(query)
	if err != nil {
		if err == sql.ErrNoRows {
			return nil, sql.ErrNoRows
		}
		return nil, fmt.Errorf("error in query while query in db: %w", err)
	}
	db.Close()
	columns, err := rows.Columns()
	if err != nil {
		return nil, fmt.Errorf("error in query while get columns: %w", err)
	}
	return &result{rows: rows, columns: columns}, nil
}
