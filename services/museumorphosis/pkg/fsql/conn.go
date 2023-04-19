package fsql

import (
	"database/sql/driver"
	"errors"
	"fmt"
	"museumorphosis/pkg/fsql/queries/create"
	"museumorphosis/pkg/fsql/queries/query"
	"museumorphosis/pkg/fsql/queries/select"
	"strings"
)

var path = "/database"

type Conn struct {
}

func (c *Conn) Prepare(query string) (driver.Stmt, error) {
	return nil, nil
}

func (c *Conn) Begin() (driver.Tx, error) {
	return nil, nil
}

func (c *Conn) Close() error {
	return nil
}

func (c *Conn) Query(q string, args []driver.Value) (driver.Rows, error) {
	filledQuery := c.fillQuery(q, args)
	filledQuery = strings.ReplaceAll(filledQuery, "\n", "")
	if match := create.Regex.FindStringSubmatch(filledQuery); len(match) > 1 {
		return create.CreateDatabase(path, match[1])
	}
	if match := query.Regex.FindStringSubmatch(filledQuery); len(match) > 1 {
		return query.QueryInDatabase(path, match[1], match[2])
	}
	if select_query.Regex.MatchString(filledQuery) {
		return select_query.GetDatabasesIds(path)
	}
	return nil, errors.New("cant understand query")
}

func (c *Conn) fillQuery(q string, args []driver.Value) string {
	for _, v := range args {
		q = strings.Replace(q, "?", fmt.Sprintf("%v", v), 1)
	}
	return q
}
