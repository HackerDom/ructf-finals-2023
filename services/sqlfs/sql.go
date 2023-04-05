package main

import "database/sql"

// import "github.com/hanwen/go-fuse/v2/fuse"

type readResult struct {
	*sql.DB
} 

func NewReadResult() {
	
}