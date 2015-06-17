#include "fuse2cbfs.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>

#ifndef ENODATA
#define ENODATA 61
#endif

#ifndef ENOATTR
#define ENOATTR ENODATA
#endif

//#define DEBUGW(...) do{fwprintf(stderr, __VA_ARGS__); fflush(stderr);} while (0)
#define DEBUGW(...) do {} while (0)

class unix_filename {
private:
  char *name;
  const char *stream;
public:
  unix_filename(const wchar_t *winfilename);
 
  ~unix_filename(){
    free(name);
  }
  
  bool is_stream(){
    return stream!=NULL;
  }
  
  const char *get_name(){
    return name;
  }
  
  const char *get_stream(){
    return stream;
  }
  
  const char *get_filename(){
    const char *fn;
    fn=strrchr(name, '/');
    if (fn)
      return fn+1;
    else
      return name;
  }
};

class unix_dir_path {
private:
  char *path;
  char *mask;
  size_t pathlen;
public:
  unix_dir_path(const wchar_t *winpath, const wchar_t *winmask);
  
  ~unix_dir_path(){
    free(path);
    free(mask);
  }
  
  const char *get_path(){
    return path;
  }
  
  const size_t get_pathlen(){
    return pathlen;
  }
  
  const char *get_mask(){
    return mask;
  }
};

class fs_handle {
public:
  unix_filename path;
  struct fuse_file_info fi;
  BOOL isfolder;
  fs_handle(const wchar_t *winpath) :
    path(winpath){
    memset(&fi, 0, sizeof(struct fuse_file_info));
  }
};

typedef struct _dir_list_element {
  struct _dir_list_element *next;
  struct FUSE_STAT st;
  size_t namelen;
  char name[1];
} dir_list_element;

typedef struct {
  dir_list_element *first;
  dir_list_element *current;
  unix_dir_path *path;
  struct fuse_file_info fi;
} dir_enum_handle;

typedef struct {
  unix_filename *path;
  char *names;
  int nameslen;
  int namesoff;
} stream_enum_handle;

static inline void throw_win_error(DWORD err){
  DEBUGW(L"throwing error %u\n", err);
  throw ECBFSError(err);
}

static int convert_to_win_err(int err){
  switch (err){
    case 0: err=0; break;
    case EINVAL: err=ERROR_BAD_ARGUMENTS; break;
    case ENOATTR:
    case ENOENT: err=ERROR_FILE_NOT_FOUND; break;
    case EISDIR:
    case EPERM:
    case EACCES: err=ERROR_ACCESS_DENIED; break;
    case EEXIST: err=ERROR_ALREADY_EXISTS; break;
    case ENOSPC: err=ERROR_DISK_FULL; break;
    case EIO: err=ERROR_IO_DEVICE; break;
    case ENOTEMPTY: err=ERROR_DIR_NOT_EMPTY; break;
    case ENOTCONN: err=ERROR_CONNECTION_UNAVAIL; break;
    case ENOSYS: err=ERROR_CALL_NOT_IMPLEMENTED; break;
    case ENOTDIR: err=ERROR_DIRECTORY; break;
    case ENOMEM: err=ERROR_NOT_ENOUGH_MEMORY; break;
    case EWOULDBLOCK: err=ERROR_LOCK_VIOLATION; break;
    case EROFS: err=ERROR_WRITE_PROTECT; break;
    case EXDEV: err=ERROR_NOT_SAME_DEVICE; break;
    
    default: err=ERROR_INVALID_FUNCTION; break;
  }
  return err;
}

static inline void throw_unix_error(int err){
  if (err<0)
    err=-err;
  throw_win_error(convert_to_win_err(err));
}

static inline void throw_unix_error_if_not_zero(int err){
  if (err)
    throw_unix_error(err);
}

unix_filename::unix_filename(const wchar_t *winfilename){
  int len, i;
  char *un;
  stream=NULL;
  len=WideCharToMultiByte(CP_UTF8, 0, winfilename, -1, NULL, 0, NULL, NULL);
  un=(char *)malloc(len);
  if (!un)
    throw_win_error(ERROR_NOT_ENOUGH_MEMORY);
  if (WideCharToMultiByte(CP_UTF8, 0, winfilename, -1, un, len, NULL, NULL)!=len){
    free(un);
    throw_win_error(ERROR_NOT_ENOUGH_MEMORY);
  }
  for (i=0; i<len; i++)
    if (un[i]=='\\')
      un[i]='/';
    else if (un[i]==':'){
      un[i]=0;
      stream=un+i+1;
    }
  name=un;
}

unix_dir_path::unix_dir_path(const wchar_t *winpath, const wchar_t *winmask){
  int len, i;
  char *un;
  len=WideCharToMultiByte(CP_UTF8, 0, winpath, -1, NULL, 0, NULL, NULL);
  un=(char *)malloc(len);
  if (!un)
    throw_win_error(ERROR_NOT_ENOUGH_MEMORY);
  if (WideCharToMultiByte(CP_UTF8, 0, winpath, -1, un, len, NULL, NULL)!=len){
    free(un);
    throw_win_error(ERROR_NOT_ENOUGH_MEMORY);
  }
  for (i=0; i<len; i++)
    if (un[i]=='\\')
      un[i]='/';
  path=un;
  len=WideCharToMultiByte(CP_UTF8, 0, winmask, -1, NULL, 0, NULL, NULL);
  un=(char *)malloc(len);
  if (!un){
    free(path);
    throw_win_error(ERROR_NOT_ENOUGH_MEMORY);
  }
  WideCharToMultiByte(CP_UTF8, 0, winmask, -1, un, len, NULL, NULL);
  mask=un;
}

static inline struct fuse *cbfs2fuse(CallbackFileSystem *cbfs){
  return (struct fuse *)cbfs->GetTag();
}

static void unixtime_to_filetime(time_t t, FILETIME *res){
  LONGLONG l;
  l=Int32x32To64(t, 10000000)+116444736000000000ULL;
  res->dwLowDateTime=(DWORD)(l&0xffffffffU);
  res->dwHighDateTime=(DWORD)(l>>32);
}

#define cbfs_require_function(ptr) do {if (!ptr) throw_win_error(ERROR_INVALID_FUNCTION);} while (0);

static void cbfs_mount(CallbackFileSystem *cbfs){
}

static void cbfs_unmount(CallbackFileSystem *cbfs){
  struct fuse *f;
  f=cbfs2fuse(cbfs);
  if (f->oper.destroy)
    f->oper.destroy(f->conn_info.user_data);
  SetEvent(f->unmountevent);
}

static int cbfs_is_filename_hidden(const char *filename){
  if (filename[0]=='.' || !_stricmp(filename, "Thumbs.db") || !_stricmp(filename, "System Volume Information"))
    return 1;
  else
    return 0;
}

static void cbfs_getfileinfo(CallbackFileSystem *cbfs, LPCTSTR file_name, LPBOOL exists, PFILETIME fctime, PFILETIME fatime,
                      PFILETIME fmtime, __int64 *endoffile, __int64 *allocsize, __int64 *fileid, PDWORD file_attributes,
                      LPTSTR short_filename OPTIONAL, PWORD short_filename_len OPTIONAL, LPTSTR real_filename OPTIONAL,
                      PWORD real_filename_len OPTIONAL){
  unix_filename ufn(file_name);
  struct fuse *f;
  struct FUSE_STAT st;
  int ret;
  DEBUGW(L"getfileinfo %s\n", file_name);
  f=cbfs2fuse(cbfs);
  *exists=FALSE;
  throw_unix_error_if_not_zero(f->oper.getattr(ufn.get_name(), &st));
  unixtime_to_filetime(st.st_ctime, fctime);
  unixtime_to_filetime(st.st_atime, fatime);
  unixtime_to_filetime(st.st_mtime, fmtime);
  *endoffile=st.st_size;
  *allocsize=st.st_size;
  *fileid=st.st_ino;
  *file_attributes=0;
  if (S_ISDIR(st.st_mode))
    *file_attributes=FILE_ATTRIBUTE_DIRECTORY;
  if (cbfs_is_filename_hidden(ufn.get_filename()))
    *file_attributes|=FILE_ATTRIBUTE_HIDDEN;
  if (*file_attributes==0)
    *file_attributes=FILE_ATTRIBUTE_NORMAL;
  if (ufn.is_stream()){
    cbfs_require_function(f->oper.getxattr);
    ret=f->oper.getxattr(ufn.get_name(), ufn.get_stream(), NULL, 0);
    if (ret<0)
      throw_unix_error(ret);
    *endoffile=ret;
    *allocsize=ret;
  }
  *exists=TRUE;
}

static void free_dir_enum_handle(dir_enum_handle *handle){
  dir_list_element *e, *n;
  delete handle->path;
  e=handle->first;
  while (e){
    n=e->next;
    free(e);
    e=n;
  }
  free(handle);
}

static int name_match_mask(const char *name, const char *mask){
  while (*mask){
    if (*mask=='?'){
      if (!*name)
        return 0;
    }
    else if (*mask=='*'){
      mask++;
      if (*mask==0)
        return 1;
      do {
        if (name_match_mask(name, mask))
          return 1;
      } while (*name++);
      return 0;
    }
    else if (tolower(*name)!=tolower(*mask))
      return 0;
    name++;
    mask++;
  }
  return *name==0;
}

static int fuse_filler(void *ptr, const char *name, const struct FUSE_STAT *stbuf, fuse_off_t off){
  dir_enum_handle *handle;
  dir_list_element *dle;
  size_t len;
  handle=(dir_enum_handle *)ptr;
  if (name[0]=='.' && (name[1]==0 || (name[1]=='.' && name[2]==0)))
    return 0;
  if (name_match_mask(name, handle->path->get_mask())){
    len=strlen(name);
    dle=(dir_list_element *)malloc(offsetof(dir_list_element, name)+len+1);
    if (!dle)
      return 1;
    if (stbuf)
      memcpy(&dle->st, stbuf, sizeof(struct FUSE_STAT));
    else
      memset(&dle->st, 0, sizeof(struct FUSE_STAT));
    dle->namelen=len;
    memcpy(dle->name, name, len+1);
    if (handle->current)
      handle->current->next=dle;
    else
      handle->first=dle;
    handle->current=dle;
  }
  return 0;
}

#define cbfs_require_function_destr(ptr, run) do {if (!ptr){run; throw_win_error(ERROR_INVALID_FUNCTION);}} while (0)

static dir_enum_handle *get_dir_enum_handle(CallbackFileSystem *cbfs, CbFsDirectoryEnumerationInfo *enum_info, CbFsFileInfo *dir_info, LPCTSTR mask){
  dir_enum_handle *handle;
  unix_dir_path *path;
  struct fuse *f;
  int ret;
  handle=(dir_enum_handle *)enum_info->get_UserContext();
  if (handle)
    return handle;
  DEBUGW(L"readdir=%s, mask=%s\n", dir_info->get_FileNameBuffer(), mask);
  f=cbfs2fuse(cbfs);
  cbfs_require_function(f->oper.readdir);
  path=new unix_dir_path(dir_info->get_FileNameBuffer(), mask);
  handle=(dir_enum_handle *)malloc(sizeof(dir_enum_handle));
  if (!handle){
    delete path;
    throw_win_error(ERROR_NOT_ENOUGH_MEMORY);
  }
  memset(handle, 0, sizeof(dir_enum_handle));
  handle->path=path;
  if (f->oper.opendir){
    ret=f->oper.opendir(path->get_path(), &handle->fi);
    if (ret){
      free_dir_enum_handle(handle);
      throw_unix_error(ret);
    }
  }
  ret=f->oper.readdir(path->get_path(), handle, fuse_filler, 0, &handle->fi);
  if (ret){
    if (f->oper.opendir && f->oper.releasedir)
      f->oper.releasedir(path->get_path(), &handle->fi);
    free_dir_enum_handle(handle);
    throw_unix_error(ret);
  }
  if (handle->current)
    handle->current->next=NULL;
  handle->current=handle->first;
  enum_info->set_UserContext(handle);
  return handle;
}

static int stat_entry(CallbackFileSystem *cbfs, dir_enum_handle *handle, dir_list_element *dle){
  char *path;
  int ret;
  size_t off;
  if (dle->st.st_ino || dle->st.st_mode || dle->st.st_size)
    return 0;
  off=handle->path->get_pathlen();
  path=(char *)malloc(off+dle->namelen+2);
  if (!path)
    return -ENOMEM;
  memcpy(path, handle->path->get_path(), off);
  path[off++]='/';
  memcpy(path+off, dle->name, dle->namelen+1);
  ret=cbfs2fuse(cbfs)->oper.getattr(path, &dle->st);
  free(path);
  return ret;
}

static void cbfs_enumeratedirectory(CallbackFileSystem* cbfs, CbFsFileInfo* dir_info, CbFsHandleInfo* handle_info, CbFsDirectoryEnumerationInfo* enum_info,
                             LPCTSTR mask, INT index, BOOL restart, LPBOOL found, LPTSTR filename, PDWORD filename_len, LPTSTR short_filename OPTIONAL,
                            PUCHAR short_filename_len OPTIONAL, PFILETIME fctime, PFILETIME fatime, PFILETIME fmtime, __int64* end_of_file, __int64* allocation_size,
                            __int64* fileid, PDWORD file_attributes){
  dir_enum_handle *handle;
  handle=get_dir_enum_handle(cbfs, enum_info, dir_info, mask);
  if (restart)
    handle->current=handle->first;
  while (handle->current && ((handle->current->st.st_ctime==0 && stat_entry(cbfs, handle, handle->current)) ||
    (*filename_len=MultiByteToWideChar(CP_UTF8, 0, handle->current->name, handle->current->namelen, filename, cbfs->GetMaxFileNameLength()))==0))
    handle->current=handle->current->next;
  if (!handle->current){
    *found=FALSE;
    return;
  }
  *found=TRUE;
  unixtime_to_filetime(handle->current->st.st_ctime, fctime);
  unixtime_to_filetime(handle->current->st.st_atime, fatime);
  unixtime_to_filetime(handle->current->st.st_mtime, fmtime);
  *end_of_file=handle->current->st.st_size;
  *allocation_size=handle->current->st.st_size;
  *fileid=0;//handle->current->st.st_ino;
  *file_attributes=0;
  if (S_ISDIR(handle->current->st.st_mode))
    *file_attributes=FILE_ATTRIBUTE_DIRECTORY;
  if (cbfs_is_filename_hidden(handle->current->name))
    *file_attributes|=FILE_ATTRIBUTE_HIDDEN;
  if (*file_attributes==0)
    *file_attributes=FILE_ATTRIBUTE_NORMAL;
  handle->current=handle->current->next;
}

static void cbfs_closedirectoryenumeration(CallbackFileSystem *cbfs, CbFsFileInfo *dir_info, CbFsDirectoryEnumerationInfo *enum_info){
  dir_enum_handle *handle;
  handle=(dir_enum_handle *)enum_info->get_UserContext();
  if (handle)
    free_dir_enum_handle(handle);
}


static stream_enum_handle *get_stream_enum_handle(struct fuse *f, CbFsNamedStreamsEnumerationInfo *enum_info, CbFsFileInfo *file_info){
  stream_enum_handle *handle;
  unix_filename *path;
  char *streamlist;
  int ret, tries;
  handle=(stream_enum_handle *)enum_info->get_UserContext();
  if (handle)
    return handle;
  path=new unix_filename(file_info->get_FileNameBuffer());
  tries=0;
retry:
  ret=f->oper.listxattr(path->get_name(), NULL, 0);
  if (ret<0){
    delete path;
    throw_unix_error(ret);
  }
  streamlist=(char *)malloc(ret+1);
  if (!streamlist){
    delete path;
    throw_win_error(ERROR_NOT_ENOUGH_MEMORY);
  }
  ret=f->oper.listxattr(path->get_name(), streamlist, ret);
  streamlist[ret]=0;
  if (ret<0){
    if (ret==-ERANGE && ++tries<=5){
      free(streamlist);
      goto retry;
    }
    delete path;
    throw_unix_error(ret);
  }
  handle=(stream_enum_handle *)malloc(sizeof(stream_enum_handle));
  if (!handle){
    delete path;
    free(streamlist);
    throw_win_error(ERROR_NOT_ENOUGH_MEMORY);
  }
  handle->path=path;
  handle->names=streamlist;
  handle->nameslen=ret;
  handle->namesoff=0;
  enum_info->set_UserContext(handle);
  return handle;
}

static void cbfs_enumeratenamedstreams(CallbackFileSystem *cbfs, CbFsFileInfo *file_info, CbFsHandleInfo *handle_info,
                               CbFsNamedStreamsEnumerationInfo *enum_info, PWSTR stream_name, PDWORD stream_name_len,
                               __int64 *stream_size, __int64 *stream_allocation_size, LPBOOL found){
  stream_enum_handle *handle;
  struct fuse *f;
  char *name;
  size_t len;
  int ret;
  f=cbfs2fuse(cbfs);
  handle=get_stream_enum_handle(f, enum_info, file_info);
  while (1) {
    if (handle->namesoff>=handle->nameslen){
      *found=FALSE;
      return;
    }
    name=handle->names+handle->namesoff;
    len=strlen(name);
    handle->namesoff+=len+1;
    if (len>32768 || !len) // documentations says 32768 is the size of stream_name buffer, this is highly unlikely to ever happen
      continue;
    if (!strcmp(name, "Zone.Identifier"))
      continue;
    ret=f->oper.getxattr(handle->path->get_name(), name, NULL, 0);
    if (ret<0) // no, don't throw error, just skip, this might be a race condition
      continue;
    if ((*stream_name_len=MultiByteToWideChar(CP_UTF8, 0, name, len, stream_name, 32768))==0)
      continue;
    *stream_size=ret;
    *stream_allocation_size=ret;
    *found=TRUE;
    break;
  }
}

void cbfs_closenamedstreamsenumeration(CallbackFileSystem *cbfs, CbFsFileInfo *file_info, CbFsNamedStreamsEnumerationInfo *enum_info){
  stream_enum_handle *handle;
  handle=(stream_enum_handle *)enum_info->get_UserContext();
  if (handle){
    delete handle->path;
    free(handle->names);
    free(handle);
  }
}

static void cbfs_stat(struct fuse *f, unix_filename *fn, struct FUSE_STAT *st){
  throw_unix_error_if_not_zero(f->oper.getattr(fn->get_name(), st));
}

static int cbfs_is_dir(struct fuse *f, unix_filename *fn){
  struct FUSE_STAT st;
  cbfs_stat(f, fn, &st);
  return S_ISDIR(st.st_mode);
}

static void cbfs_canfilebedeleted(CallbackFileSystem *cbfs, CbFsFileInfo *file_info, CbFsHandleInfo *handle_info, BOOL *can_del){
  unix_filename ufn(file_info->get_FileNameBuffer());
  struct fuse *f;
  int ret;
  f=cbfs2fuse(cbfs);
  *can_del=FALSE;
  if (ufn.is_stream()){
    if (f->oper.removexattr){
      ret=f->oper.getxattr(ufn.get_name(), ufn.get_stream(), NULL, 0);
      if (ret<0)
        throw_unix_error(ret);
      else
        *can_del=TRUE;
    }
    return;
  }
  if (cbfs_is_dir(f, &ufn)){
    if (f->oper.can_rmdir)
      ret=f->oper.can_rmdir(ufn.get_name());
    else
      ret=0;
  }
  else{
    if (f->oper.can_unlink)
      ret=f->oper.can_unlink(ufn.get_name());
    else
      ret=0;
  }
  throw_unix_error_if_not_zero(ret);
  *can_del=TRUE;
}

void cfs_isdirectoryempty(CallbackFileSystem *cbfs, CbFsFileInfo *dir_info, LPCWSTR filename, LPBOOL isempty){
  *isempty=TRUE;
}

static void cbfs_deletefile(CallbackFileSystem *cbfs, CbFsFileInfo *file_info){
  unix_filename ufn(file_info->get_FileNameBuffer());
  struct fuse *f;
  int ret;
  f=cbfs2fuse(cbfs);
  if (ufn.is_stream()){
    cbfs_require_function(f->oper.removexattr);
    ret=f->oper.removexattr(ufn.get_name(), ufn.get_stream());
  }
  else{
    if (cbfs_is_dir(f, &ufn)){
      cbfs_require_function(f->oper.rmdir);
      ret=f->oper.rmdir(ufn.get_name());
    }
    else{
      cbfs_require_function(f->oper.unlink);
      ret=f->oper.unlink(ufn.get_name());
    }
  }
  throw_unix_error_if_not_zero(ret);
}

static void cbfs_renameormovefile(CallbackFileSystem *cbfs, CbFsFileInfo *file_info, LPCTSTR new_filename){
  unix_filename ufnsrc(file_info->get_FileNameBuffer());
  unix_filename ufndst(new_filename);
  struct fuse *f;
  f=cbfs2fuse(cbfs);
  cbfs_require_function(f->oper.rename);
  DEBUGW(L"rename %S to %S\n", ufnsrc.get_name(), ufndst.get_name());
  throw_unix_error_if_not_zero(f->oper.rename(ufnsrc.get_name(), ufndst.get_name()));
  DEBUGW(L"rename %S to %S out\n", ufnsrc.get_name(), ufndst.get_name());
}

void cbfs_createfile(CallbackFileSystem *cbfs, LPCTSTR filename, ACCESS_MASK desired_access, DWORD file_attributes, DWORD share_mode,
                    CbFsFileInfo *file_info, CbFsHandleInfo *handle_info){
  fs_handle *fh;
  struct fuse *f;
  int ret;
  DEBUGW(L"createfile %s\n", filename);
  f=cbfs2fuse(cbfs);
  fh=new fs_handle(filename);
  if (fh->path.is_stream()){
    struct FUSE_STAT st;
    ret=f->oper.getattr(fh->path.get_name(), &st);
    if (ret){
      delete fh;
      throw_unix_error(ret); 
    }
  }
  else if (file_attributes&FILE_ATTRIBUTE_DIRECTORY){
    cbfs_require_function_destr(f->oper.mkdir, delete fh);
    ret=f->oper.mkdir(fh->path.get_name(), 0755);
    delete fh;
    if (ret)
      throw_unix_error(ret);
    else
      return;
  }
  else{
    cbfs_require_function_destr(f->oper.create, delete fh);
    if ((desired_access&(GENERIC_READ|GENERIC_WRITE))==(GENERIC_READ|GENERIC_WRITE))
      fh->fi.flags=O_RDWR|O_CREAT|O_EXCL;
    else if (desired_access&GENERIC_WRITE)
      fh->fi.flags=O_WRONLY|O_CREAT|O_EXCL;
    else
      fh->fi.flags=O_RDONLY|O_CREAT|O_EXCL;
    ret=f->oper.create(fh->path.get_name(), 0644, &fh->fi);
    if (ret){
      delete fh;
      throw_unix_error(ret);
    }
  }
  fh->isfolder=FALSE;
  file_info->set_UserContext(fh);
}

void cbfs_openfile(CallbackFileSystem *cbfs, LPCTSTR filename, ACCESS_MASK desired_access, DWORD file_attributes, DWORD share_mode,
                    CbFsFileInfo *file_info, CbFsHandleInfo *handle_info){
  fs_handle *fh;
  struct fuse *f;
  int ret;
  DEBUGW(L"openfile %s\n", filename);
  f=cbfs2fuse(cbfs);
  fh=new fs_handle(filename);
  if (fh->path.is_stream()){
    if (!strcmp(fh->path.get_stream(), "Zone.Identifier")){
      delete fh;
      throw_win_error(ERROR_FILE_NOT_FOUND);
    }
    ret=f->oper.getxattr(fh->path.get_name(), fh->path.get_stream(), NULL, 0);
    if (ret<0){
      delete fh;
      if (ret==-ENOATTR)
        ret=-ENOENT;
      throw_unix_error(ret); 
    }
  }
  else{
    struct FUSE_STAT st;
    ret=f->oper.getattr(fh->path.get_name(), &st);
    if (!ret){
      if ((desired_access&(GENERIC_READ|GENERIC_WRITE))==(GENERIC_READ|GENERIC_WRITE))
        fh->fi.flags=O_RDWR;
      else if (desired_access&GENERIC_WRITE)
        fh->fi.flags=O_WRONLY;
      else
        fh->fi.flags=O_RDONLY;
      if (S_ISDIR(st.st_mode)){
        fh->isfolder=TRUE;
        if (f->oper.opendir)
          ret=f->oper.opendir(fh->path.get_name(), &fh->fi);
        else
          ret=0;
      }
      else{
        fh->isfolder=FALSE;
        if (f->oper.open)
          ret=f->oper.open(fh->path.get_name(), &fh->fi);
        else
          ret=0;
      }
    }
    if (ret){
      delete fh;
      throw_unix_error(ret);
    }
  }
  file_info->set_UserContext(fh);
}

void cbfs_cleanupfile(CallbackFileSystem *cbfs, CbFsFileInfo *file_info, CbFsHandleInfo *handle_info){
  fs_handle *fh;
  struct fuse *f;
  fh=(fs_handle *)file_info->get_UserContext();
  if (!fh)
    return;
  f=cbfs2fuse(cbfs);
  if (!fh->isfolder && !fh->path.is_stream() && f->oper.flush)
    throw_unix_error_if_not_zero(f->oper.flush(fh->path.get_name(), &fh->fi));
}

void cbfs_closefile(CallbackFileSystem *cbfs, CbFsFileInfo *file_info, CbFsHandleInfo *handle_info){
  fs_handle *fh;
  struct fuse *f;
  int ret;
  fh=(fs_handle *)file_info->get_UserContext();
  if (!fh)
    return;
  f=cbfs2fuse(cbfs);
  if (fh->path.is_stream())
    ret=0;
  else if (fh->isfolder){
    if (f->oper.releasedir)
      ret=f->oper.releasedir(fh->path.get_name(), &fh->fi);
    else
      ret=0;
  }
  else{
    if (f->oper.release)
      ret=f->oper.release(fh->path.get_name(), &fh->fi);
    else
      ret=0;
  }
  delete fh;
  throw_unix_error_if_not_zero(ret);
}

void cbfs_flushfile(CallbackFileSystem *cbfs, CbFsFileInfo *file_info){
  struct fuse *f;
  fs_handle *fh;
  if (!file_info)
    return;
  f=cbfs2fuse(cbfs);
  fh=(fs_handle *)file_info->get_UserContext();
  if (!fh)
    return;
  if (fh->path.is_stream())
    return;
  if (fh->isfolder){
    if (f->oper.fsyncdir)
      throw_unix_error_if_not_zero(f->oper.fsyncdir(fh->path.get_name(), 0, &fh->fi));
  }
  else if (f->oper.fsync)
    throw_unix_error_if_not_zero(f->oper.fsync(fh->path.get_name(), 0, &fh->fi));
}

void cbfs_readfile(CallbackFileSystem *cbfs, CbFsFileInfo *file_info, __int64 offset, PVOID buff, DWORD size, PDWORD bytesread){
  struct fuse *f;
  fs_handle *fh;
  int ret;
  f=cbfs2fuse(cbfs);
  fh=(fs_handle *)file_info->get_UserContext();
  if (fh->path.is_stream()){
    char *tbuff;
    ret=f->oper.getxattr(fh->path.get_name(), fh->path.get_stream(), NULL, 0);
    if (ret<0)
      throw_unix_error(ret);
    if (ret<=offset)
      throw_win_error(ERROR_HANDLE_EOF);
    tbuff=(char *)malloc(ret);
    if (!tbuff)
      throw_win_error(ERROR_NOT_ENOUGH_MEMORY);
    ret=f->oper.getxattr(fh->path.get_name(), fh->path.get_stream(), tbuff, ret);
    if (ret<0){
      free(tbuff);
      throw_unix_error(ret);
    }
    if (ret<=offset){
      free(tbuff);
      throw_win_error(ERROR_HANDLE_EOF);
    }
    if (offset+size>ret)
      size=ret-offset;
    memcpy(buff, tbuff+offset, size);
    free(tbuff);
    *bytesread=size;
  }
  else if (fh->isfolder)
    throw_win_error(ERROR_ACCESS_DENIED);
  else{
    ret=f->oper.read(fh->path.get_name(), (char *)buff, size, offset, &fh->fi);
    if (ret<0)
      throw_unix_error(ret);
    else if (ret)
      *bytesread=ret;
    else
      throw_win_error(ERROR_HANDLE_EOF);
  }
}

void cbfs_writefile(CallbackFileSystem *cbfs, CbFsFileInfo *file_info, __int64 offset, PVOID buff, DWORD size, PDWORD byteswritten){
  struct fuse *f;
  fs_handle *fh;
  int ret;
  f=cbfs2fuse(cbfs);
  fh=(fs_handle *)file_info->get_UserContext();
  if (fh->path.is_stream()){
    char *tbuff;
    int len;
    ret=f->oper.getxattr(fh->path.get_name(), fh->path.get_stream(), NULL, 0);
    if (ret<0)
      ret=0;
    if (ret>offset+size)
      len=ret;
    else
      len=offset+size;
    tbuff=(char *)malloc(len);
    if (!tbuff)
      throw_win_error(ERROR_NOT_ENOUGH_MEMORY);
    memset(tbuff, 0, len);
    if (ret){
      ret=f->oper.getxattr(fh->path.get_name(), fh->path.get_stream(), tbuff, len);
      if (ret<0){
        free(tbuff);
        throw_unix_error(ret);
      }
    }
    memcpy(tbuff+offset, buff, size);
    ret=f->oper.setxattr(fh->path.get_name(), fh->path.get_stream(), tbuff, len, 0);
    free(tbuff);
    throw_unix_error_if_not_zero(ret);
    *byteswritten=size;
  }
  else if (fh->isfolder)
    throw_win_error(ERROR_ACCESS_DENIED);
  else{
    ret=f->oper.write(fh->path.get_name(), (char *)buff, size, offset, &fh->fi);
    if (ret<0)
      throw_unix_error(ret);
    else 
      *byteswritten=ret;
  }
}

void cbfs_setendoffile(CallbackFileSystem *cbfs, CbFsFileInfo *file_info, __int64 eof){
  struct fuse *f;
  fs_handle *fh;
  int ret;
  f=cbfs2fuse(cbfs);
  fh=(fs_handle *)file_info->get_UserContext();
  if (fh->path.is_stream()){
    char *tbuff;
    ret=f->oper.getxattr(fh->path.get_name(), fh->path.get_stream(), NULL, 0);
    if (ret<0)
      ret=0;
    if (ret<eof)
      ret=eof;
    tbuff=(char *)malloc(ret);
    if (!tbuff)
      throw_win_error(ERROR_NOT_ENOUGH_MEMORY);
    ret=f->oper.getxattr(fh->path.get_name(), fh->path.get_stream(), tbuff, ret);
    if (ret<0)
      ret=0;
    if (ret<eof)
      memset(tbuff+ret, 0, eof-ret);
    ret=f->oper.setxattr(fh->path.get_name(), fh->path.get_stream(), tbuff, eof, 0);
    free(tbuff);
  }
  else if (fh->isfolder)
    throw_win_error(ERROR_ACCESS_DENIED);
  else{
    if (f->oper.ftruncate)
      ret=f->oper.ftruncate(fh->path.get_name(), eof, &fh->fi);
    else if (f->oper.truncate)
      ret=f->oper.truncate(fh->path.get_name(), eof);
    else
      throw_win_error(ERROR_INVALID_FUNCTION);
  }
  throw_unix_error_if_not_zero(ret);
}

void cbfs_setvaliddatalen(CallbackFileSystem *cbfs, CbFsFileInfo *file_info, __int64 eof){

}

void cbfs_setallocationsize(CallbackFileSystem *cbfs, CbFsFileInfo *file_info, __int64 eof){

}

void cbfs_getvolumeid(CallbackFileSystem *cbfs, PDWORD volid){
  *volid=DEFAULT_FUSE_VOLUME_SERIAL;
}

void utf8_to_wchar(const char *src, wchar_t *res, DWORD maxlen){
  int len;
  len=MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
  if (len>=maxlen){
    *res=L'\0';
    return;
  }
  MultiByteToWideChar(CP_UTF8, 0, src, -1, res, len);
}

void cbfs_getvolumelabel(CallbackFileSystem *cbfs, LPTSTR vol_label){
  utf8_to_wchar(DEFAULT_FUSE_VOLUME_NAME, vol_label, 32);
}

void cbfs_setvolumelabel(CallbackFileSystem *cbfs, LPCTSTR vol_label){
}

void cbfs_getvolumesize(CallbackFileSystem *cbfs,  __int64 *total_sectors, __int64 *free_sectors){
  struct fuse *f;
  struct statvfs svfs;
  f=cbfs2fuse(cbfs);
  cbfs_require_function(f->oper.statfs);
  throw_unix_error_if_not_zero(f->oper.statfs("/", &svfs));
  *total_sectors=svfs.f_blocks*svfs.f_frsize/cbfs->GetSectorSize();
  *free_sectors=svfs.f_bavail*svfs.f_frsize/cbfs->GetSectorSize();
}

void cbfs_setfileattributes(CallbackFileSystem *cbfs, CbFsFileInfo *file_info, CbFsHandleInfo *handle_info, PFILETIME ctime,
                            PFILETIME atime, PFILETIME mtime, DWORD file_attributes){
}

struct fuse_chan *fuse_mount(const char *mountpoint, struct fuse_args *args){
  struct fuse_chan *ch;
  char mp[64];
  if (!mountpoint || ((mountpoint[0]<'a' || mountpoint[0]>'z') && (mountpoint[0]<'A' || mountpoint[0]>'Z')))
    goto err0;
  _snprintf(mp, sizeof(mp)-1, "%c:;pCloud;pCloud Drive", mountpoint[0]);
//  _snprintf(mp, sizeof(mp)-1, "%c:", mountpoint[0]);
  mp[sizeof(mp)-1]=0;
  ch=(struct fuse_chan *)malloc(sizeof(struct fuse_chan));
  if (!ch)
    goto err0;
  mbstowcs(ch->mountpoint, mp, sizeof(ch->mountpoint)/sizeof(wchar_t));
  ch->fs=NULL;
  ch->flags=CBFS_SYMLINK_NETWORK|CBFS_SYMLINK_NETWORK_HIDDEN_SHARE;
 // ch->flags=CBFS_SYMLINK_SIMPLE|CBFS_SYMLINK_LOCAL;
  return ch;
err0:
  return NULL;
}

static void fuse_free_chan(struct fuse_chan *ch){
  free(ch);
}

void fuse_unmount(const char *mountpoint, struct fuse_chan *ch){
  if (ch->fs)
    SetEvent(ch->fs->unmountevent);
  else
    fuse_free_chan(ch);
}

struct fuse *fuse_new(struct fuse_chan *ch, struct fuse_args *args,
          const struct fuse_operations *op, size_t op_size, void *user_data){
  struct fuse *f;
  f=(struct fuse *)malloc(sizeof(struct fuse));
  if (!f)
    goto err0;
  memset(f, 0, sizeof(struct fuse));
  if (pthread_mutex_init(&f->mutex, NULL))
    goto err1;
  if (pthread_cond_init(&f->cond, NULL))
    goto err2;
  f->session.channel=ch;
  memcpy(&f->oper, op, op_size<sizeof(struct fuse_operations)?op_size:sizeof(struct fuse_operations));
  f->conn_info.user_data=user_data;
  f->unmountevent=CreateEvent(NULL, FALSE, FALSE, NULL);
  if (!f->unmountevent)
    goto err3;
  ch->fs=f;
  return f;

err3:
  pthread_cond_destroy(&f->cond);
err2:
  pthread_mutex_destroy(&f->mutex);
err1:
  free(f);
err0:
  return NULL;
}

static void fuse_unset_mounted(struct fuse *f){
  pthread_mutex_lock(&f->mutex);
  f->mounted=0;
  pthread_cond_broadcast(&f->cond);
  pthread_mutex_unlock(&f->mutex);
}

static int fuse_loop_threadno(struct fuse *f, DWORD threadcnt){
  CallbackFileSystem cbfs;
  pthread_mutex_lock(&f->mutex);
  if (f->mounted){
    pthread_mutex_unlock(&f->mutex);
    return -1;
  }
  else
    f->mounted=1;
  pthread_mutex_unlock(&f->mutex);
  try{
/*    DWORD reb;
    cbfs.Install(L"C:\\Program Files (x86)\\EldoS\\Callback File System\\Drivers\\cbfs.cab", DEFAULT_FUSE_VOLUME_NAME, 
                 L"C:\\Users\\virco\\AppData\\Local\\Documents\\Visual Studio 2013\\Projects\\Console Drive\\Debug", 1, 
                 CBFS_MODULE_NET_REDIRECTOR_DLL|CBFS_MODULE_MOUNT_NOTIFIER_DLL, &reb);
    if (reb)
      printf("reboot needed %u\n", reb);
    else
      printf("no reboot needed %u\n", reb);*/
    cbfs.SetRegistrationKey("7CFF5E672FA0585D06CB48BD6A8F6C41CE96A5A7C42CCFAB3370A51287E768B2E223C039D2F3106DF9299BE83CFF25B2E18B664F43727E818E135099E6AB28A956BF1C49365FBC2D7A3B386DBA7B7869167FDC3DCACB48ADFABBB8ADDA7FDC7D55");
    cbfs.Initialize(f->config.fsname?f->config.fsname:DEFAULT_FUSE_VOLUME_NAME);
    f->conn_info.capable=FUSE_CAP_ASYNC_READ|FUSE_CAP_ATOMIC_O_TRUNC|FUSE_CAP_BIG_WRITES;
    if (f->oper.init)
      f->conn_info.user_data=f->oper.init(&f->conn_info);
    if (threadcnt==1)
      cbfs.SetSerializeCallbacks(TRUE);
    else{
      cbfs.SetSerializeCallbacks(FALSE);
      cbfs.SetParalleledProcessingAllowed((f->conn_info.want&FUSE_CAP_ASYNC_READ)!=0);
      cbfs.SetMaxWorkerThreadCount(threadcnt);
    }
    if ((f->conn_info.want&FUSE_CAP_BIG_WRITES)==0)
      cbfs.SetMaxReadWriteBlockSize(4096);
    cbfs.SetShortFileNameSupport(FALSE);
    cbfs.SetFileCacheEnabled(FALSE);
    cbfs.SetMetaDataCacheEnabled(FALSE);
    cbfs.SetNonexistentFilesCacheEnabled(FALSE);
    cbfs.SetClusterSize(4096);
    cbfs.SetCaseSensitiveFileNames(FALSE);
    cbfs.SetCallAllOpenCloseCallbacks(FALSE);
    cbfs.SetTag(f);
  
    cbfs.SetOnMount(cbfs_mount);
    cbfs.SetOnUnmount(cbfs_unmount);
    cbfs.SetOnGetFileInfo(cbfs_getfileinfo);
    cbfs.SetOnEnumerateDirectory(cbfs_enumeratedirectory);
    cbfs.SetOnCloseDirectoryEnumeration(cbfs_closedirectoryenumeration);
    if (f->oper.listxattr && f->oper.getxattr && f->oper.setxattr){
      cbfs.SetOnEnumerateNamedStreams(cbfs_enumeratenamedstreams);
      cbfs.SetOnCloseNamedStreamsEnumeration(cbfs_closenamedstreamsenumeration);
    }
    cbfs.SetOnCanFileBeDeleted(cbfs_canfilebedeleted);
    cbfs.SetOnIsDirectoryEmpty(cfs_isdirectoryempty);
    cbfs.SetOnDeleteFile(cbfs_deletefile);
    cbfs.SetOnRenameOrMoveFile(cbfs_renameormovefile);
    cbfs.SetOnCreateFile(cbfs_createfile);
    cbfs.SetOnOpenFile(cbfs_openfile);
    cbfs.SetOnCleanupFile(cbfs_cleanupfile);
    cbfs.SetOnCloseFile(cbfs_closefile);
    cbfs.SetOnFlushFile(cbfs_flushfile);
    cbfs.SetOnReadFile(cbfs_readfile);
    cbfs.SetOnWriteFile(cbfs_writefile);
    cbfs.SetOnSetEndOfFile(cbfs_setendoffile);
    cbfs.SetOnSetValidDataLength(cbfs_setvaliddatalen);
    cbfs.SetOnSetAllocationSize(cbfs_setallocationsize);
    cbfs.SetOnGetVolumeId(cbfs_getvolumeid);
    cbfs.SetOnGetVolumeLabel(cbfs_getvolumelabel);
    cbfs.SetOnSetVolumeLabel(cbfs_setvolumelabel);
    cbfs.SetOnGetVolumeSize(cbfs_getvolumesize);
    cbfs.SetOnSetFileAttributes(cbfs_setfileattributes);
  
    f->cbfs=&cbfs;

    cbfs.SetFileSystemName(L"exFAT");
    cbfs.CreateStorage();
    cbfs.MountMedia(0);
    cbfs.AddMountingPoint(f->session.channel->mountpoint, f->session.channel->flags, NULL);
    if (cbfs.IconInstalled(L"pcloud")){
      printf("setting icon\n");
      cbfs.SetIcon(L"pcloud");
    }
  }
  catch (ECBFSError e){
    fuse_unset_mounted(f);
    return e.ErrorCode();
  }

  WaitForSingleObject(f->unmountevent, INFINITE);
  
  try{
    cbfs.DeleteMountingPoint(f->session.channel->mountpoint, f->session.channel->flags, NULL);
    cbfs.UnmountMedia();
    cbfs.DeleteStorage(TRUE);
  }
  catch (...){
  }
  
  fuse_unset_mounted(f);
  f->cbfs=NULL;
  return 0;
}

int fuse_loop_mt(struct fuse *f){
  return fuse_loop_threadno(f, FUSE_THREAD_CNT);
}

int fuse_loop(struct fuse *f){
  return fuse_loop_threadno(f, 1);
}

void fuse_destroy(struct fuse *f){
  pthread_mutex_lock(&f->mutex);
  if (f->mounted){
    SetEvent(f->unmountevent);
    do {
      pthread_cond_wait(&f->cond, &f->mutex);
    } while (f->mounted);
  }
  pthread_mutex_unlock(&f->mutex);
  fuse_free_chan(f->session.channel);
  CloseHandle(f->unmountevent);
  pthread_cond_destroy(&f->cond);
  pthread_mutex_destroy(&f->mutex);
  free(f);
}

void fuse_exit(struct fuse *f){
  pthread_mutex_lock(&f->mutex);
  if (f->mounted){
    SetEvent(f->unmountevent);
    do {
      pthread_cond_wait(&f->cond, &f->mutex);
    } while (f->mounted);
  }
  pthread_mutex_unlock(&f->mutex);
}

int fuse_opt_add_arg(struct fuse_args *args, const char *arg){
  return 0;
}

void fuse_opt_free_args(struct fuse_args *args){
}
