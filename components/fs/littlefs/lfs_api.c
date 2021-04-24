/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "lfs_api.h"
#include "iCunit.h"

lfs_t g_lfs;
FileDirInfo g_lfsDir[LFS_MAX_OPEN_DIRS];

FileOpInfo g_fsOp;
static LittleFsHandleStruct g_handle[LITTLE_FS_MAX_OPEN_FILES] = {0};
struct dirent g_nameValue;
struct fsmap_t g_fsmap[MAX_FILE_SYSTEM_LEN];
static pthread_mutex_t g_FslocalMutex = PTHREAD_MUTEX_INITIALIZER;

FileOpInfo GetFsOpInfo(void)
{
	return g_fsOp;
}

LittleFsHandleStruct *GetFreeFd(int *fd)
{
	for (int i = 0; i < LITTLE_FS_MAX_OPEN_FILES; i++) {
		if (g_handle[i].useFlag == 0) {
			*fd = i;
			g_handle[i].useFlag = 1;
			return &(g_handle[i]);
		}		
	}

	*fd = INVALID_FD;
	return NULL;
}

lfs_dir_t *GetFreeDir()
{
	for (int i = 0; i < LFS_MAX_OPEN_DIRS; i++) {
		if (g_lfsDir[i].useFlag == 0) {
			g_lfsDir[i].useFlag = 1;
			return &(g_lfsDir[i].dir);
		}
	}

	return NULL;
}

int InitMountInfo(const char *fileSystemType, const struct MountOps *fsMops)
{
	int len = strlen(fileSystemType) + 1;
	for (int i = 0; i < MAX_FILE_SYSTEM_LEN; i++) {
		if (g_fsmap[i].fileSystemtype == NULL) {
			g_fsmap[i].fileSystemtype = (char*)malloc(len);
			memcpy_s(g_fsmap[i].fileSystemtype, len, fileSystemType, len);
			g_fsmap[i].fs_mops = fsMops;
			return VFS_OK;
		}
	}

	return VFS_ERROR;
}

const struct fsmap_t *mount_findfs(const char*fileSystemtype)
{
	struct fsmap_t *m = NULL;

	for (int i = 0; i < MAX_FILE_SYSTEM_LEN; i++) {
		m = &(g_fsmap[i]);
		if (m->fileSystemtype && strcmp(fileSystemtype, m->fileSystemtype) == 0) {
			return m;
		}
	}

	return (const struct fsmap_t *)NULL;
}

const struct MountOps g_fsMnt = {
	.Mount = LfsMount,
	.Umount = LfsUmount,
};

const struct FileOps lfs_vops = {
	.Mkdir = LfsMkdir,
	.Unlink = LfsUnlink,
	.Rmdir = LfsRmdir,
	.Opendir = LfsOpendir,
	.Readdir = LfsReaddir,
	.Closedir = LfsClosedir,
	.Open = LfsOpen,
	.Close = LfsClose,
	.Write = LfsWrite,
	.Read = LfsRead,
	.Seek = LfsSeek,
	.Rename = LfsRename,
	.Getattr = LfsStat,
	.Fsync = LfsFsync,
};

int LfsMount(const char * source, const char * target, const char * fileSystemType, unsigned long mountflags,
	const void * data)
{
	int ret;

	pthread_mutex_lock(&g_FslocalMutex);
	g_fsOp.fsVops = &lfs_vops;
	ret = lfs_mount(&g_lfs, (struct lfs_config*)data);
	pthread_mutex_unlock(&g_FslocalMutex);

	return ret;
}

int LfsUmount(const char * target)
{
	int ret;

	pthread_mutex_lock(&g_FslocalMutex);
	ret = lfs_unmount(&g_lfs);
	pthread_mutex_unlock(&g_FslocalMutex);

	return ret;	
}

int LfsUnlink(const char * fileName)
{
	int ret;

	pthread_mutex_lock(&g_FslocalMutex);
	ret = lfs_remove(&g_lfs, fileName);
	pthread_mutex_unlock(&g_FslocalMutex);

	return ret;
}

int LfsMkdir(const char * dirName, mode_t mode)
{
	int ret;

	pthread_mutex_lock(&g_FslocalMutex);
	ret = lfs_mkdir(&g_lfs, dirName);
	pthread_mutex_unlock(&g_FslocalMutex);

	return ret;
}

int LfsRmdir(const char * dirName)
{
	int ret;

	pthread_mutex_lock(&g_FslocalMutex);
	ret = lfs_remove(&g_lfs, dirName);
	pthread_mutex_unlock(&g_FslocalMutex);

	return ret;
}

DIR *LfsOpendir(const char * dirName)
{
	int ret;

	lfs_dir_t *dir = GetFreeDir();
	if (dir == NULL) {
		return NULL;
	}

	pthread_mutex_lock(&g_FslocalMutex);
	ret = lfs_dir_open(&g_lfs, dir, dirName);
	pthread_mutex_unlock(&g_FslocalMutex);

	if (ret == 0) {
		return (DIR *)dir;
	} else {
		return NULL;
	}
}

struct dirent *LfsReaddir(DIR * dir)
{
	int ret;
	struct lfs_info lfsInfo;

	pthread_mutex_lock(&g_FslocalMutex);
	ret = lfs_dir_read(&g_lfs, (lfs_dir_t *)dir, &lfsInfo);
	if (ret == 0) {
		(void)memcpy_s(g_nameValue.d_name, sizeof(g_nameValue.d_name), lfsInfo.name, strlen(lfsInfo.name) + 1);
		if (lfsInfo.type == LFS_TYPE_DIR) {
			g_nameValue.d_type = DT_DIR;
		} else if (lfsInfo.type == LFS_TYPE_REG) {
			g_nameValue.d_type = DT_REG;
		}

		g_nameValue.d_reclen = lfsInfo.size;
		pthread_mutex_unlock(&g_FslocalMutex);

		return &g_nameValue;
	}

	pthread_mutex_unlock(&g_FslocalMutex);
	return NULL;
}

int LfsClosedir(DIR * dir)
{
	int ret;

	pthread_mutex_lock(&g_FslocalMutex);
	ret = lfs_dir_close(&g_lfs, (lfs_dir_t *)dir);
	pthread_mutex_unlock(&g_FslocalMutex);

	return ret;
}

int LfsOpen(const char * path, int openFlag, int mode)
{
	int fd = INVALID_FD;

	pthread_mutex_lock(&g_FslocalMutex);
	LittleFsHandleStruct *fsHandle = GetFreeFd(&fd);
	if (fd == INVALID_FD) {
		goto errout;
	}

	int err = lfs_file_open(&g_lfs, &(fsHandle->file), path, openFlag);
	if (err != 0) {
		goto errout;
	}

	pthread_mutex_unlock(&g_FslocalMutex);
	return fd;
errout:
	pthread_mutex_unlock(&g_FslocalMutex);
	return INVALID_FD;	
}

int LfsRead(int fd, void * buf, unsigned int len)
{
	int ret = VFS_ERROR;

	if (fd >= LITTLE_FS_MAX_OPEN_FILES && fd < 0) {
		return ret;
	}

	pthread_mutex_lock(&g_FslocalMutex);
	ret = lfs_file_read(&g_lfs, &(g_handle[fd].file), buf, len);
	pthread_mutex_unlock(&g_FslocalMutex);

	return ret;
}

int LfsWrite(int fd, const void * buf, unsigned int len)
{
	int ret = VFS_ERROR;

	if (fd >= LITTLE_FS_MAX_OPEN_FILES && fd < 0) {
		return ret;
	}

	pthread_mutex_lock(&g_FslocalMutex);
	ret = lfs_file_write(&g_lfs, &(g_handle[fd].file), buf, len);
	pthread_mutex_unlock(&g_FslocalMutex);

	return ret;
}

int LfsSeek(int fd, off_t offset, int whence)
{
	int ret = VFS_ERROR;

	if (fd >= LITTLE_FS_MAX_OPEN_FILES && fd < 0) {
		return ret;
	}

	pthread_mutex_lock(&g_FslocalMutex);
	ret = lfs_file_seek(&g_lfs, &(g_handle[fd].file), offset, whence);
	pthread_mutex_unlock(&g_FslocalMutex);

	return ret;
}

int LfsClose(int fd)
{
	int ret = VFS_ERROR;

	if (fd >= LITTLE_FS_MAX_OPEN_FILES && fd < 0) {
		return ret;
	}

	pthread_mutex_lock(&g_FslocalMutex);
	ret = lfs_file_close(&g_lfs, &(g_handle[fd].file));
	g_handle[fd].useFlag = 0;
	pthread_mutex_unlock(&g_FslocalMutex);

	return ret;	
}

int LfsRename(const char * oldName, const char * newName)
{
	int ret;

	pthread_mutex_lock(&g_FslocalMutex);
	ret = lfs_rename(&g_lfs, oldName, newName);
	pthread_mutex_unlock(&g_FslocalMutex);

	return ret;
}

int LfsStat(const char * path, struct stat * buf)
{
	int ret;
	struct lfs_info info;

	pthread_mutex_lock(&g_FslocalMutex);
	ret = lfs_stat(&g_lfs, path, &info);
	if (ret == 0) {
		buf->st_size = info.size;
	}
	pthread_mutex_unlock(&g_FslocalMutex);

	return ret;	
}

int LfsFsync(int fd)
{
	int ret;

	pthread_mutex_lock(&g_FslocalMutex);
	ret = lfs_file_sync(&g_lfs, &(g_handle[fd].file));
	pthread_mutex_unlock(&g_FslocalMutex);

	return ret;
}

