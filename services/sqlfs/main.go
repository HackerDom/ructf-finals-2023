package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"os"
	"path"
	"runtime/pprof"
	"time"
	"database/sql"

	"github.com/hanwen/go-fuse/v2/fs"
	"github.com/hanwen/go-fuse/v2/fuse"
	_ "github.com/lib/pq"
)

func main() {
	db, err := connectDb()
	if err != nil {
		panic(err)
	}
	defer db.Close()
	run(db)
}

func run(db *sql.DB) {
	log.SetFlags(log.Lmicroseconds)
	// Scans the arg list and sets up flags
	debug := flag.Bool("debug", false, "print debugging messages.")
	other := flag.Bool("allow-other", false, "mount with -o allowother.")
	quiet := flag.Bool("q", false, "quiet")
	ro := flag.Bool("ro", false, "mount read-only")
	directmount := flag.Bool("directmount", false, "try to call the mount syscall instead of executing fusermount")
	cpuprofile := flag.String("cpuprofile", "", "write cpu profile to this file")
	flag.Parse()
	if flag.NArg() < 2 {
		fmt.Printf("usage: %s MOUNTPOINT ORIGINAL\n", path.Base(os.Args[0]))
		fmt.Printf("\noptions:\n")
		flag.PrintDefaults()
		os.Exit(2)
	}
	if *cpuprofile != "" {
		if !*quiet {
			fmt.Printf("Writing cpu profile to %s\n", *cpuprofile)
		}
		f, err := os.Create(*cpuprofile)
		if err != nil {
			fmt.Println(err)
			os.Exit(3)
		}
		pprof.StartCPUProfile(f)
		defer pprof.StopCPUProfile()
	}

	orig := flag.Arg(1)
	loopbackRoot, err := NewLoopbackRoot(orig, db)
	if err != nil {
		log.Fatalf("NewLoopbackRoot(%s): %v\n", orig, err)
	}

	sec := time.Second
	opts := &fs.Options{
		// The timeout options are to be compatible with libfuse defaults,
		// making benchmarking easier.
		AttrTimeout:  &sec,
		EntryTimeout: &sec,

		NullPermissions: true, // Leave file permissions on "000" files as-is

		MountOptions: fuse.MountOptions{
			AllowOther:  *other,
			Debug:       *debug,
			DirectMount: *directmount,
			FsName:      orig,       // First column in "df -T": original dir
			Name:        "loopback", // Second column in "df -T" will be shown as "fuse." + Name
		},
	}
	if opts.AllowOther {
		// Make the kernel check file permissions for us
		opts.MountOptions.Options = append(opts.MountOptions.Options, "default_permissions")
	}
	if *ro {
		opts.MountOptions.Options = append(opts.MountOptions.Options, "ro")
	}
	// Enable diagnostics logging
	if !*quiet {
		opts.Logger = log.New(os.Stderr, "", 0)
	}
	server, err := fs.Mount(flag.Arg(0), loopbackRoot, opts)
	if err != nil {
		log.Fatalf("Mount fail: %v\n", err)
	}
	if !*quiet {
		fmt.Println("Mounted!")
	}
	server.Wait()
}

func debug(v interface{}) {
	mv, err := json.Marshal(v)
	if err != nil {
		panic(err)
	}
	fmt.Println(string(mv))
}

func connectDb() (*sql.DB, error) {
	dsn := "postgresql://ructf:ructf@localhost:5432/ructf"
	db, err := sql.Open("postgres", dsn)
	if err != nil {
		return nil, fmt.Errorf("failed to connect to db: %w", err)
	}

	return db, nil
}
