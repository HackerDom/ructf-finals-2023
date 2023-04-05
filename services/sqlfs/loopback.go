package main

import (
	"context"
	"database/sql"
	"os"
	"path/filepath"
	"syscall"

	"golang.org/x/sys/unix"

	"github.com/hanwen/go-fuse/v2/fs"
	"github.com/hanwen/go-fuse/v2/fuse"
)

// LoopbackRoot holds the parameters for creating a new loopback
// filesystem. Loopback filesystem delegate their operations to an
// underlying POSIX file system.
type LoopbackRoot struct {
	// The path to the root of the underlying file system.
	Path string

	// The device on which the Path resides. This must be set if
	// the underlying filesystem crosses file systems.
	Dev uint64

	// NewNode returns a new InodeEmbedder to be used to respond
	// to a LOOKUP/CREATE/MKDIR/MKNOD opcode. If not set, use a
	// LoopbackNode.
	NewNode func(rootData *LoopbackRoot, parent *fs.Inode, name string, st *syscall.Stat_t) fs.InodeEmbedder

	db *sql.DB
}

func (r *LoopbackRoot) newNode(parent *fs.Inode, name string, st *syscall.Stat_t) fs.InodeEmbedder {
	if r.NewNode != nil {
		return r.NewNode(r, parent, name, st)
	}
	return &LoopbackNode{
		RootData: r,
	}
}

func (r *LoopbackRoot) idFromStat(st *syscall.Stat_t) fs.StableAttr {
	// We compose an inode number by the underlying inode, and
	// mixing in the device number. In traditional filesystems,
	// the inode numbers are small. The device numbers are also
	// small (typically 16 bit). Finally, we mask out the root
	// device number of the root, so a loopback FS that does not
	// encompass multiple mounts will reflect the inode numbers of
	// the underlying filesystem
	swapped := (uint64(st.Dev) << 32) | (uint64(st.Dev) >> 32)
	swappedRootDev := (r.Dev << 32) | (r.Dev >> 32)
	return fs.StableAttr{
		Mode: uint32(st.Mode),
		Gen:  1,
		// This should work well for traditional backing FSes,
		// not so much for other go-fuse FS-es
		Ino: (swapped ^ swappedRootDev) ^ st.Ino,
	}
}

// LoopbackNode is a filesystem node in a loopback file system. It is
// public so it can be used as a basis for other loopback based
// filesystems. See NewLoopbackFile or LoopbackRoot for more
// information.
type LoopbackNode struct {
	fs.Inode

	// RootData points back to the root of the loopback filesystem.
	RootData *LoopbackRoot
}

var _ = (fs.NodeStatfser)((*LoopbackNode)(nil))
var _ = (fs.NodeStatfser)((*LoopbackNode)(nil))
var _ = (fs.NodeGetattrer)((*LoopbackNode)(nil))
var _ = (fs.NodeGetxattrer)((*LoopbackNode)(nil))
var _ = (fs.NodeSetxattrer)((*LoopbackNode)(nil))
var _ = (fs.NodeRemovexattrer)((*LoopbackNode)(nil))
var _ = (fs.NodeListxattrer)((*LoopbackNode)(nil))
var _ = (fs.NodeReadlinker)((*LoopbackNode)(nil))
var _ = (fs.NodeOpener)((*LoopbackNode)(nil))
var _ = (fs.NodeCopyFileRanger)((*LoopbackNode)(nil))
var _ = (fs.NodeLookuper)((*LoopbackNode)(nil))
var _ = (fs.NodeOpendirer)((*LoopbackNode)(nil))
var _ = (fs.NodeReaddirer)((*LoopbackNode)(nil))
var _ = (fs.NodeMkdirer)((*LoopbackNode)(nil))
var _ = (fs.NodeMknoder)((*LoopbackNode)(nil))
var _ = (fs.NodeLinker)((*LoopbackNode)(nil))
var _ = (fs.NodeSymlinker)((*LoopbackNode)(nil))
var _ = (fs.NodeUnlinker)((*LoopbackNode)(nil))
var _ = (fs.NodeRmdirer)((*LoopbackNode)(nil))
var _ = (fs.NodeRenamer)((*LoopbackNode)(nil))

func (n *LoopbackNode) Statfs(ctx context.Context, out *fuse.StatfsOut) syscall.Errno {
	s := syscall.Statfs_t{}
	err := syscall.Statfs(n.path(), &s)
	if err != nil {
		return fs.ToErrno(err)
	}
	out.FromStatfsT(&s)
	return fs.OK
}

// path returns the full path to the file in the underlying file
// system.
func (n *LoopbackNode) path() string {
	path := n.Path(n.Root())
	return filepath.Join(n.RootData.Path, path)
}

func (n *LoopbackNode) Lookup(ctx context.Context, name string, out *fuse.EntryOut) (*fs.Inode, syscall.Errno) {
	p := filepath.Join(n.path(), name)

	st := syscall.Stat_t{}
	err := syscall.Lstat(p, &st)
	if err != nil {
		return nil, fs.ToErrno(err)
	}

	out.Attr.FromStat(&st)

	node := n.RootData.newNode(n.EmbeddedInode(), name, &st)
	ch := n.NewInode(ctx, node, n.RootData.idFromStat(&st))
	return ch, 0
}

// preserveOwner sets uid and gid of `path` according to the caller information
// in `ctx`.
func (n *LoopbackNode) preserveOwner(ctx context.Context, path string) error {
	if os.Getuid() != 0 {
		return nil
	}
	caller, ok := fuse.FromContext(ctx)
	if !ok {
		return nil
	}
	return syscall.Lchown(path, int(caller.Uid), int(caller.Gid))
}

func (n *LoopbackNode) Mknod(ctx context.Context, name string, mode, rdev uint32, out *fuse.EntryOut) (*fs.Inode, syscall.Errno) {
	p := filepath.Join(n.path(), name)
	err := syscall.Mknod(p, mode, int(rdev))
	if err != nil {
		return nil, fs.ToErrno(err)
	}
	n.preserveOwner(ctx, p)
	st := syscall.Stat_t{}
	if err := syscall.Lstat(p, &st); err != nil {
		syscall.Rmdir(p)
		return nil, fs.ToErrno(err)
	}

	out.Attr.FromStat(&st)

	node := n.RootData.newNode(n.EmbeddedInode(), name, &st)
	ch := n.NewInode(ctx, node, n.RootData.idFromStat(&st))

	return ch, 0
}

func (n *LoopbackNode) Mkdir(ctx context.Context, name string, mode uint32, out *fuse.EntryOut) (*fs.Inode, syscall.Errno) {
	p := filepath.Join(n.path(), name)
	err := os.Mkdir(p, os.FileMode(mode))
	if err != nil {
		return nil, fs.ToErrno(err)
	}
	n.preserveOwner(ctx, p)
	st := syscall.Stat_t{}
	if err := syscall.Lstat(p, &st); err != nil {
		syscall.Rmdir(p)
		return nil, fs.ToErrno(err)
	}

	out.Attr.FromStat(&st)

	node := n.RootData.newNode(n.EmbeddedInode(), name, &st)
	ch := n.NewInode(ctx, node, n.RootData.idFromStat(&st))

	return ch, 0
}

func (n *LoopbackNode) Rmdir(ctx context.Context, name string) syscall.Errno {
	p := filepath.Join(n.path(), name)
	err := syscall.Rmdir(p)
	return fs.ToErrno(err)
}

func (n *LoopbackNode) Unlink(ctx context.Context, name string) syscall.Errno {
	p := filepath.Join(n.path(), name)
	err := syscall.Unlink(p)
	return fs.ToErrno(err)
}

func (n *LoopbackNode) Rename(ctx context.Context, name string, newParent fs.InodeEmbedder, newName string, flags uint32) syscall.Errno {
	if flags&fs.RENAME_EXCHANGE != 0 {
		return n.renameExchange(name, newParent, newName)
	}

	p1 := filepath.Join(n.path(), name)
	p2 := filepath.Join(n.RootData.Path, newParent.EmbeddedInode().Path(nil), newName)

	err := syscall.Rename(p1, p2)
	return fs.ToErrno(err)
}

var _ = (fs.NodeCreater)((*LoopbackNode)(nil))

func (n *LoopbackNode) Create(ctx context.Context, name string, flags uint32, mode uint32, out *fuse.EntryOut) (inode *fs.Inode, fh fs.FileHandle, fuseFlags uint32, errno syscall.Errno) {
	p := filepath.Join(n.path(), name)
	flags = flags &^ syscall.O_APPEND
	fd, err := syscall.Open(p, int(flags)|os.O_CREATE, mode)
	if err != nil {
		return nil, nil, 0, fs.ToErrno(err)
	}
	n.preserveOwner(ctx, p)
	st := syscall.Stat_t{}
	if err := syscall.Fstat(fd, &st); err != nil {
		syscall.Close(fd)
		return nil, nil, 0, fs.ToErrno(err)
	}

	node := n.RootData.newNode(n.EmbeddedInode(), name, &st)
	ch := n.NewInode(ctx, node, n.RootData.idFromStat(&st))
	lf := NewLoopbackFile(fd, n.RootData.db)

	out.FromStat(&st)
	return ch, lf, 0, 0
}

func (n *LoopbackNode) Symlink(ctx context.Context, target, name string, out *fuse.EntryOut) (*fs.Inode, syscall.Errno) {
	p := filepath.Join(n.path(), name)
	err := syscall.Symlink(target, p)
	if err != nil {
		return nil, fs.ToErrno(err)
	}
	n.preserveOwner(ctx, p)
	st := syscall.Stat_t{}
	if err := syscall.Lstat(p, &st); err != nil {
		syscall.Unlink(p)
		return nil, fs.ToErrno(err)
	}
	node := n.RootData.newNode(n.EmbeddedInode(), name, &st)
	ch := n.NewInode(ctx, node, n.RootData.idFromStat(&st))

	out.Attr.FromStat(&st)
	return ch, 0
}

func (n *LoopbackNode) Link(ctx context.Context, target fs.InodeEmbedder, name string, out *fuse.EntryOut) (*fs.Inode, syscall.Errno) {

	p := filepath.Join(n.path(), name)
	err := syscall.Link(filepath.Join(n.RootData.Path, target.EmbeddedInode().Path(nil)), p)
	if err != nil {
		return nil, fs.ToErrno(err)
	}
	st := syscall.Stat_t{}
	if err := syscall.Lstat(p, &st); err != nil {
		syscall.Unlink(p)
		return nil, fs.ToErrno(err)
	}
	node := n.RootData.newNode(n.EmbeddedInode(), name, &st)
	ch := n.NewInode(ctx, node, n.RootData.idFromStat(&st))

	out.Attr.FromStat(&st)
	return ch, 0
}

func (n *LoopbackNode) Readlink(ctx context.Context) ([]byte, syscall.Errno) {
	p := n.path()

	for l := 256; ; l *= 2 {
		buf := make([]byte, l)
		sz, err := syscall.Readlink(p, buf)
		if err != nil {
			return nil, fs.ToErrno(err)
		}

		if sz < len(buf) {
			return buf[:sz], 0
		}
	}
}

func (n *LoopbackNode) Open(ctx context.Context, flags uint32) (fh fs.FileHandle, fuseFlags uint32, errno syscall.Errno) {
	flags = flags &^ syscall.O_APPEND
	p := n.path()
	f, err := syscall.Open(p, int(flags), 0)
	if err != nil {
		return nil, 0, fs.ToErrno(err)
	}
	lf := NewLoopbackFile(f, n.RootData.db)
	return lf, 0, 0
}

func (n *LoopbackNode) Opendir(ctx context.Context) syscall.Errno {
	fd, err := syscall.Open(n.path(), syscall.O_DIRECTORY, 0755)
	if err != nil {
		return fs.ToErrno(err)
	}
	syscall.Close(fd)
	return fs.OK
}

func (n *LoopbackNode) Readdir(ctx context.Context) (fs.DirStream, syscall.Errno) {
	return fs.NewLoopbackDirStream(n.path())
}

func (n *LoopbackNode) Getattr(ctx context.Context, f fs.FileHandle, out *fuse.AttrOut) syscall.Errno {
	if f != nil {
		return f.(fs.FileGetattrer).Getattr(ctx, out)
	}

	p := n.path()

	var err error
	st := syscall.Stat_t{}
	if &n.Inode == n.Root() {
		err = syscall.Stat(p, &st)
	} else {
		err = syscall.Lstat(p, &st)
	}

	if err != nil {
		return fs.ToErrno(err)
	}
	out.FromStat(&st)
	return fs.OK
}

var _ = (fs.NodeSetattrer)((*LoopbackNode)(nil))

func (n *LoopbackNode) Setattr(ctx context.Context, f fs.FileHandle, in *fuse.SetAttrIn, out *fuse.AttrOut) syscall.Errno {
	p := n.path()
	fsa, ok := f.(fs.FileSetattrer)
	if ok && fsa != nil {
		fsa.Setattr(ctx, in, out)
	} else {
		if m, ok := in.GetMode(); ok {
			if err := syscall.Chmod(p, m); err != nil {
				return fs.ToErrno(err)
			}
		}

		uid, uok := in.GetUID()
		gid, gok := in.GetGID()
		if uok || gok {
			suid := -1
			sgid := -1
			if uok {
				suid = int(uid)
			}
			if gok {
				sgid = int(gid)
			}
			if err := syscall.Chown(p, suid, sgid); err != nil {
				return fs.ToErrno(err)
			}
		}

		mtime, mok := in.GetMTime()
		atime, aok := in.GetATime()

		if mok || aok {

			ap := &atime
			mp := &mtime
			if !aok {
				ap = nil
			}
			if !mok {
				mp = nil
			}
			var ts [2]syscall.Timespec
			ts[0] = fuse.UtimeToTimespec(ap)
			ts[1] = fuse.UtimeToTimespec(mp)

			if err := syscall.UtimesNano(p, ts[:]); err != nil {
				return fs.ToErrno(err)
			}
		}

		if sz, ok := in.GetSize(); ok {
			if err := syscall.Truncate(p, int64(sz)); err != nil {
				return fs.ToErrno(err)
			}
		}
	}

	fga, ok := f.(fs.FileGetattrer)
	if ok && fga != nil {
		fga.Getattr(ctx, out)
	} else {
		st := syscall.Stat_t{}
		err := syscall.Lstat(p, &st)
		if err != nil {
			return fs.ToErrno(err)
		}
		out.FromStat(&st)
	}
	return fs.OK
}

// NewLoopbackRoot returns a root node for a loopback file system whose
// root is at the given root. This node implements all NodeXxxxer
// operations available.
func NewLoopbackRoot(rootPath string, db *sql.DB) (fs.InodeEmbedder, error) {
	var st syscall.Stat_t
	err := syscall.Stat(rootPath, &st)
	if err != nil {
		return nil, err
	}

	root := &LoopbackRoot{
		Path: rootPath,
		Dev:  uint64(st.Dev),
		db: db,
	}

	return root.newNode(nil, "", &st), nil
}

func (n *LoopbackNode) Getxattr(ctx context.Context, attr string, dest []byte) (uint32, syscall.Errno) {
	sz, err := unix.Lgetxattr(n.path(), attr, dest)
	return uint32(sz), fs.ToErrno(err)
}

func (n *LoopbackNode) Setxattr(ctx context.Context, attr string, data []byte, flags uint32) syscall.Errno {
	err := unix.Lsetxattr(n.path(), attr, data, int(flags))
	return fs.ToErrno(err)
}

func (n *LoopbackNode) Removexattr(ctx context.Context, attr string) syscall.Errno {
	err := unix.Lremovexattr(n.path(), attr)
	return fs.ToErrno(err)
}

func (n *LoopbackNode) Listxattr(ctx context.Context, dest []byte) (uint32, syscall.Errno) {
	sz, err := unix.Llistxattr(n.path(), dest)
	return uint32(sz), fs.ToErrno(err)
}

func (n *LoopbackNode) renameExchange(name string, newparent fs.InodeEmbedder, newName string) syscall.Errno {
	fd1, err := syscall.Open(n.path(), syscall.O_DIRECTORY, 0)
	if err != nil {
		return fs.ToErrno(err)
	}
	defer syscall.Close(fd1)
	p2 := filepath.Join(n.RootData.Path, newparent.EmbeddedInode().Path(nil))
	fd2, err := syscall.Open(p2, syscall.O_DIRECTORY, 0)
	defer syscall.Close(fd2)
	if err != nil {
		return fs.ToErrno(err)
	}

	var st syscall.Stat_t
	if err := syscall.Fstat(fd1, &st); err != nil {
		return fs.ToErrno(err)
	}

	// Double check that nodes didn't change from under us.
	inode := &n.Inode
	if inode.Root() != inode && inode.StableAttr().Ino != n.RootData.idFromStat(&st).Ino {
		return syscall.EBUSY
	}
	if err := syscall.Fstat(fd2, &st); err != nil {
		return fs.ToErrno(err)
	}

	newinode := newparent.EmbeddedInode()
	if newinode.Root() != newinode && newinode.StableAttr().Ino != n.RootData.idFromStat(&st).Ino {
		return syscall.EBUSY
	}

	return fs.ToErrno(unix.Renameat2(fd1, name, fd2, newName, unix.RENAME_EXCHANGE))
}

func (n *LoopbackNode) CopyFileRange(ctx context.Context, fhIn fs.FileHandle,
	offIn uint64, out *fs.Inode, fhOut fs.FileHandle, offOut uint64,
	len uint64, flags uint64) (uint32, syscall.Errno) {
	lfIn, ok := fhIn.(*loopbackFile)
	if !ok {
		return 0, syscall.ENOTSUP
	}
	lfOut, ok := fhOut.(*loopbackFile)
	if !ok {
		return 0, syscall.ENOTSUP
	}

	signedOffIn := int64(offIn)
	signedOffOut := int64(offOut)
	count, err := unix.CopyFileRange(lfIn.fd, &signedOffIn, lfOut.fd, &signedOffOut, int(len), int(flags))
	return uint32(count), fs.ToErrno(err)
}
