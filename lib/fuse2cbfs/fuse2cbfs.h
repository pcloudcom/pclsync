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

#ifndef _FUSE2CBFS_H_
#define _FUSE2CBFS_H_

#include <pthread.h>
#include <windows.h>

#include "fuse.h"
#include "CbFS.h"

#define CACHE_LINE_SIZE 64

#define FUSE_THREAD_CNT 12

struct fuse_chan {
  wchar_t mountpoint[64];
  struct fuse *fs;
  DWORD flags;
};

struct fuse_session {
  struct fuse_chan *channel;  
};

struct fuse_config {
  const char *fsname;
  const char *volname;
  uint32_t serialnumber;
  mode_t defaultfilemode;
  mode_t defaultdirmode;
  char debug;
};

struct fuse {
  struct fuse_session session;
  struct fuse_config config;
  struct fuse_operations oper;
  struct fuse_conn_info conn_info;
  CallbackFileSystem *cbfs;
  HANDLE unmountevent;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  char mounted;
};

#endif