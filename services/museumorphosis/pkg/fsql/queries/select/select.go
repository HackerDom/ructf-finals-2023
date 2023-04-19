package select_query

import (
	"database/sql/driver"
	"fmt"
	"io"
	"os"
	"regexp"
)

var Regex = regexp.MustCompile("SELECT id FROM DATABASE \\(ALL\\)")

type result struct {
	fileNames []string
	current   int
}

func (r *result) Columns() []string {
	return []string{"id"}
}

func (r *result) Close() error {
	return nil
}

func (r *result) Next(dest []driver.Value) error {
	if r.current == len(r.fileNames) {
		return io.EOF
	}
	dest[0] = r.fileNames[r.current]
	r.current += 1
	return nil
}

func GetDatabasesIds(path string) (driver.Rows, error) {
	files, err := os.ReadDir(path)
	if err != nil {
		return nil, fmt.Errorf("error in get databases ids while read dir: %w", err)
	}
	fileNames := make([]string, 0)
	for _, f := range files {
		if !f.IsDir() {
			fileNames = append(fileNames, f.Name())
		}
	}
	return &result{fileNames: fileNames, current: 0}, nil
}
