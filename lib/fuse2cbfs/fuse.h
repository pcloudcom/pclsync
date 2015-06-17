/* Copyright (c) 2014 Anton Titov.
 * Copyright (c) 2014 pCloud Ltd.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of pCloud Ltd nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL pCloud Ltd BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _FUSE_H_
#define _FUSE_H_
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DEFAULT_FUSE_VOLUME_NAME
#define DEFAULT_FUSE_VOLUME_NAME "pCloud Drive"
#endif

#ifndef DEFAULT_FUSE_FILESYSTEM_NAME
#define DEFAULT_FUSE_FILESYSTEM_NAME "exFAT"
#endif

#ifndef DEFAULT_FUSE_VOLUME_SERIAL
#define DEFAULT_FUSE_VOLUME_SERIAL 0x5401f3f6
#endif

#define FUSE_ARGS_INIT(argc, argv) {argv, argc, 0}

#define FUSE_STAT   fuse_stat64

#define FUSE_HAS_CAN_UNLINK 1

#define FUSE_CAP_ASYNC_READ     (1 << 0)
#define FUSE_CAP_ATOMIC_O_TRUNC (1 << 1)
#define FUSE_CAP_BIG_WRITES     (1 << 2)

#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#define ST_NOSUID 0x02

struct fuse;
struct fuse_session;
struct fuse_chan;

#define HAS_FUSE_OFF_T 1
typedef int64_t fuse_off_t;
typedef int __mode_t;
typedef int uid_t;
typedef int gid_t;

typedef uint64_t fsfilcnt64_t;
typedef uint64_t fsblkcnt64_t;

struct statvfs{
  unsigned long int f_bsize;
  unsigned long int f_frsize;
  fsblkcnt64_t f_blocks;
  fsblkcnt64_t f_bfree;
  fsblkcnt64_t f_bavail;
  fsfilcnt64_t f_files;
  fsfilcnt64_t f_ffree;
  fsfilcnt64_t f_favail;
  unsigned long int f_fsid;
  unsigned long int f_flag;
  unsigned long int f_namemax;
};

struct fuse_stat64 {
  uint64_t st_size;
  uint64_t st_ino;
  time_t st_atime;
  time_t st_mtime;
  time_t st_ctime;
  uint32_t st_dev;
  uint32_t st_rdev;
  unsigned short st_mode;
  short st_nlink;
  short st_uid;
  short st_gid;
};

typedef int (*fuse_fill_dir_t)(void *buf, const char *name, const struct FUSE_STAT *stbuf, fuse_off_t off);

struct fuse_file_info {
  uint64_t fh;
  int flags;
};

struct fuse_args {
  char **argv;
  int argc;
  int allocated;
};

struct fuse_conn_info {
  void *user_data;
  unsigned proto_major;
  unsigned proto_minor;
  unsigned async_read;
  unsigned max_write;
  unsigned max_readahead;
  unsigned capable;
  unsigned want;
};

struct fuse_operations {
  int (*getattr)(const char *, struct FUSE_STAT *);
  int (*readlink)(const char *, char *, size_t);
  int (*mknod)(const char *, __mode_t, dev_t);
  int (*mkdir)(const char *, __mode_t);
  int (*can_unlink)(const char *);
  int (*unlink)(const char *);
  int (*can_rmdir)(const char *);
  int (*rmdir)(const char *);
  int (*symlink)(const char *, const char *);
  int (*rename)(const char *, const char *);
  int (*link)(const char *, const char *);
  int (*chmod)(const char *, __mode_t);
  int (*chown)(const char *, uid_t, gid_t);
  int (*truncate)(const char *, fuse_off_t);
  int (*utime)(const char *, struct utimbuf *);
  int (*open)(const char *, struct fuse_file_info *);
  int (*read)(const char *, char *, size_t, fuse_off_t, struct fuse_file_info *);
  int (*write)(const char *, const char *, size_t, fuse_off_t, struct fuse_file_info *);
  int (*statfs)(const char *, struct statvfs *);
  int (*flush)(const char *, struct fuse_file_info *);
  int (*release)(const char *, struct fuse_file_info *);
  int (*fsync)(const char *, int, struct fuse_file_info *);
  int (*setxattr)(const char *, const char *, const char *, size_t, int);
  int (*getxattr)(const char *, const char *, char *, size_t);
  int (*listxattr)(const char *, char *, size_t);
  int (*removexattr)(const char *, const char *);
  int (*opendir)(const char *, struct fuse_file_info *);
  int (*readdir)(const char *, void *, fuse_fill_dir_t, fuse_off_t, struct fuse_file_info *);
  int (*releasedir)(const char *, struct fuse_file_info *);
  int (*fsyncdir)(const char *, int, struct fuse_file_info *);
  void *(*init)(struct fuse_conn_info *conn);
  void (*destroy)(void *);
  int (*access)(const char *, int);
  int (*create)(const char *, __mode_t, struct fuse_file_info *);
  int (*ftruncate)(const char *, fuse_off_t, struct fuse_file_info *);
  int (*fgetattr)(const char *, struct FUSE_STAT *, struct fuse_file_info *);
  int (*lock)(const char *, struct fuse_file_info *, int cmd, struct flock *);
  int (*utimens)(const char *, const struct timespec tv[2]);
  int (*bmap)(const char *, size_t blocksize, uint64_t *idx);
};

int fuse_opt_add_arg(struct fuse_args *args, const char *arg);
void fuse_opt_free_args(struct fuse_args *args);

struct fuse_chan *fuse_mount(const char *mountpoint, struct fuse_args *args);
void fuse_unmount(const char *mountpoint, struct fuse_chan *ch);
void fuse_exit(struct fuse *f);

struct fuse *fuse_new(struct fuse_chan *ch, struct fuse_args *args,
          const struct fuse_operations *op, size_t op_size, void *user_data);

int fuse_loop_mt(struct fuse *f);
int fuse_loop(struct fuse *f);
void fuse_destroy(struct fuse *f);

#ifdef __cplusplus
}
#endif

#endif
