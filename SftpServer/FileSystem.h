/*
 MySecureShell permit to add restriction to modified sftp-server
 when using MySecureShell as shell.
 Copyright (C) 2007-2014 MySecureShell Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation (version 2)

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "FileSystemAcl.h"

typedef struct sFSPath
{
	char *realPath;
	char *exposedPath;
	char *path;
} tFSPath;

void FSInit(char *realPath, char *exposedPath);
void FSShutdown();
tFSPath *FSResolvePath(const char *path1, const char *path2, int permitDotDirectory);
void FSResolvRelativePath(char *path, int permitDotDirectory);
char *FSBuildPath(const char *path1, const char *path2);
void FSDestroyPath(tFSPath *path);
int FSCheckSecurity(const char *fullPath, const char *path);
void FSChangeRights(struct stat *st);

tFSPath *FSCheckPath(const char *file);
tFSPath *FSRealPath(const char *file);
int FSOpenFile(const char *file, int *fileHandle, int flags, mode_t mode, struct stat *st);
int FSOpenDir(char *dir, DIR **dirHandle);
tFSPath *FSReadDir(const char *readDir, DIR *dirHandle, struct stat *st);
int FSStat(const char *file, int doLStat, struct stat *st);
int FSReadLink(const char *file, char *readLink, int sizeofReadLink);
int FSUnlink(const char *file);
int FSMkdir(const char *dir, mode_t mode);
int FSRmdir(const char *dir);
int FSRename(const char *oldFile, const char *newFile, int overwriteDestination);
int FSSymlink(const char *oldFile, const char *newFile);

#endif /* _FILESYSTEM_H_ */
