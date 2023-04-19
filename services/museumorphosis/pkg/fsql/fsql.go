package fsql

import (
	"database/sql"
	"database/sql/driver"
)

func init() {
	sql.Register("fsql", &Driver{})
}

type Driver struct {
}

func (d *Driver) Open(_ string) (driver.Conn, error) {
	return &Conn{}, nil
}
