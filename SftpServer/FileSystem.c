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

#include "../config.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../Core/FileSpec.h"
#include "Access.h"
#include "FileSystem.h"
#include "Global.h"
#include "Sftp.h"
#include "Util.h"
#include "Log.h"

static tFSPath *_home;

void FSInit(char *realPath, char *exposedPath)
{
	_home = malloc(sizeof(*_home));
	_home->realPath = realPath;
	_home->exposedPath = exposedPath;
	_home->path = NULL;
}

void FSShutdown()
{
	if (_home != NULL)
	{
		free(_home);
		_home = NULL;
	}
}

tFSPath *FSResolvePath(const char *path1, const char *path2, int permitDotDirectory)
{
	tFSPath *newPath;
	int idx, len;

	newPath = calloc(sizeof(*newPath), 1);
	if (_home->exposedPath == NULL)
	{
		if (path1[0] == '/')
			newPath->exposedPath = strdup(path1);
		else
			newPath->exposedPath = FSBuildPath(_home->realPath, path1);
		newPath->realPath = strdup(newPath->exposedPath);
		FSResolvRelativePath(newPath->exposedPath, permitDotDirectory);
	}
	else
	{
		if (path1[0] == '/')
			newPath->exposedPath = strdup(path1);
		else
			newPath->exposedPath = FSBuildPath(_home->exposedPath, path1);
		FSResolvRelativePath(newPath->exposedPath, permitDotDirectory);
		newPath->realPath = FSBuildPath(_home->realPath, newPath->exposedPath);
	}
	if (path2 != NULL)
	{
		char *oldPath;

		oldPath = newPath->exposedPath;
		newPath->exposedPath = FSBuildPath(oldPath, path2);
		free(oldPath);
		if (_home->exposedPath != NULL)
		{
			oldPath = newPath->realPath;
			newPath->realPath = FSBuildPath(oldPath, path2);
			free(oldPath);
		}
		else
		{
			free(newPath->realPath);
			newPath->realPath = strdup(newPath->exposedPath);
		}
	}
	FSResolvRelativePath(newPath->exposedPath, permitDotDirectory);
	FSResolvRelativePath(newPath->realPath, permitDotDirectory);

	//Strip directory and suffix from exposedPath
	len = strlen(newPath->exposedPath);
	for (idx = len - 2; idx >= 0; idx--)
		if (newPath->exposedPath[idx] == '/')
		{
			newPath->path = strdup(newPath->exposedPath + idx + 1);
			if (newPath->exposedPath[len - 1] == '/')
				newPath->path[len - idx - 2] = '\0';
			break;
		}
	if (newPath->path == NULL)
		newPath->path = strdup(newPath->exposedPath);

	DEBUG((MYLOG_DEBUG, "[FSResolvePath]realPath:'%s' exposedPath:'%s' path:'%s'", newPath->realPath, newPath->exposedPath, newPath->path));
	return newPath;
}

void FSResolvRelativePath(char *path, int permitDotDirectory)
{
	char *s = path;
	char *ptr;
	int len;

	len = strlen(path);
	while ((ptr = strstr(s, "..")) != NULL)
	{
		int	i, beg, end;

		beg = len - strlen(ptr);
		end = beg + 2;
		if ((beg == 0 || path[beg - 1] == '/') &&
				(path[end] == '\0' || path[end] == '/'))
		{
			if (path[end] == '\0' && permitDotDirectory == 1)
				break;

			//Ignore trailling '/'
			while (beg > 1 && path[beg - 1] == '/')
				beg--;

			//Search directory name before '/../'
			for (i = beg - 1; i >= 0; i--)
				if (path[i] == '/' && (i == 0 || path[i - 1] != '/'))
					break;
			beg = (i < 1 ? (i < 0 ? 0 : 1) : i);

			//Delete directory
			for (i = 0; (end + i) < len; i++)
				path[beg + i] = path[end + i];
			path[beg + i] = '\0';
			len -= end - beg;
		}
		else
			s = ptr + 2;
	}
	if (len >= 2 && path[len - 2] == '/' && path[len - 1] == '.')
	{
		if (permitDotDirectory == 0)
			path[len - 1] = '\0';
	}
}

char *FSBuildPath(const char *path1, const char *path2)
{
	char *path;
	char lastChar = '\0';
	int i, len, len1, len2;

	len1 = strlen(path1);
	len2 = strlen(path2);
	path = malloc(len1 + 1 + len2 + 1);
	for (len = 0, i = 0; i < len1; i++)
	{
		char currentChar = path1[i];

		if (!(lastChar == '/' && currentChar == '/'))
		{
			path[len++] = currentChar;
			lastChar = currentChar;
		}
	}
	if (lastChar != '/')
	{
		lastChar = '/';
		path[len++] = lastChar;
	}
	for (i = 0; i < len2; i++)
	{
		char currentChar = path2[i];

		if (!(lastChar == '/' && currentChar == '/'))
		{
			path[len++] = currentChar;
			lastChar = currentChar;
		}
	}
	path[len] = '\0';
	return path;
}

void FSDestroyPath(tFSPath *path)
{
	free(path->realPath);
	free(path->exposedPath);
	free(path->path);
	free(path);
}

static void FSCheckSecurityACL(void *data, int type, u_int32_t id, u_int32_t mode)
{
	int *result = (int *)data;

	switch (type)
	{
	case FS_ENUM_USER:
		if (id == getuid())
			*result = SSH2_FX_OK;
	case FS_ENUM_GROUP:
		if (id == getgid())
			*result = SSH2_FX_OK;
		break;
	case FS_ENUM_OTHER:
		if ((mode & (SSH5_ACE4_READ_DATA | SSH5_ACE4_WRITE_DATA | SSH5_ACE4_EXECUTE)) != 0)
			*result = SSH2_FX_OK;
		break;
	}
}

int FSCheckSecurity(const char *fullPath, const char *path)
{
	if (HAS_BIT(gl_var->flagsGlobals, SFTPWHO_STAY_AT_HOME)
			&& _home != NULL && _home->realPath != NULL
			&& strncmp(fullPath, _home->realPath, strlen(_home->realPath)) != 0)
		return SSH2_FX_PERMISSION_DENIED;

	if (HAS_BIT(gl_var->flagsGlobals, SFTPWHO_IGNORE_HIDDEN)
			&& path[0] == '.'
			&& path[1] != '.'
			&& path[1] != '\0')
		return SSH2_FX_NO_SUCH_FILE;
	if (HAS_BIT(gl_var->flagsGlobals, SFTPWHO_HIDE_NO_ACESS))
	{
		struct stat st;
		u_int32_t nbEntries;
		int result = SSH2_FX_NO_SUCH_FILE;

		FSEnumAcl(fullPath, 0, FSCheckSecurityACL, &result, &nbEntries);
		if (stat(fullPath, &st) == 0)
		{
			if ((st.st_uid == getuid() && HAS_BIT(st.st_mode, S_IRUSR))
					|| (UserIsInThisGroup(st.st_gid) == 1 && HAS_BIT(st.st_mode, S_IRGRP))
					|| HAS_BIT(st.st_mode, S_IROTH))
				result = SSH2_FX_OK;
		}
		if (errno == ENOENT)
			result = SSH2_FX_OK;
		if (result != SSH2_FX_OK)
			return result;
	}
	return FileSpecCheckRights(fullPath, path);
}

void FSChangeRights(struct stat *st)
{
	if (HAS_BIT(gl_var->flagsGlobals, SFTPWHO_FAKE_USER))
		st->st_uid = gl_var->current_user;
	if (HAS_BIT(gl_var->flagsGlobals, SFTPWHO_FAKE_GROUP))
		st->st_gid = gl_var->current_group;
	if (HAS_BIT(gl_var->flagsGlobals, SFTPWHO_FAKE_MODE))
	{
		st->st_mode = (st->st_mode & ~0x1fff) | gl_var->dir_mode;
		if (HAS_BIT(st->st_mode, S_IFDIR))
		{
			if (HAS_BIT(gl_var->dir_mode, S_IRUSR))
				st->st_mode |= S_IXUSR;
			if (HAS_BIT(gl_var->dir_mode, S_IRGRP))
				st->st_mode |= S_IXGRP;
			if (HAS_BIT(gl_var->dir_mode, S_IROTH))
				st->st_mode |= S_IXOTH;
		}
	}
}

tFSPath *FSCheckPath(const char *file)
{
	tFSPath *path;

	path = FSResolvePath(file, NULL, 0);
	if (FSCheckSecurity(path->realPath, path->path) != SSH2_FX_OK)
	{
		FSDestroyPath(path);
		return NULL;
	}
	return path;
}

tFSPath *FSRealPath(const char *file)
{
	tFSPath *path;
	int len;

	path = FSResolvePath(file, NULL, 0);
	len = strlen(path->exposedPath);
	if (len >= 1 && path->exposedPath[len - 1] != '/')
	{
		struct stat st;

		if (stat(path->exposedPath, &st) != -1 && (st.st_mode & S_IFMT) != S_IFREG)
		{
			path->exposedPath = realloc(path->exposedPath, len + 2);
			path->exposedPath[len] = '/';
			path->exposedPath[len + 1] = '\0';
		}
	}
	return path;
}

int FSOpenFile(const char *file, int *fileHandle, int flags, mode_t mode, struct stat *stfile)
{
	tFSPath *path;
	int returnValue;

	path = FSResolvePath(file, NULL, 0);
	if (FSCheckSecurity(path->realPath, path->path) != SSH2_FX_OK)
	{
		DEBUG((MYLOG_DEBUG, "[FSOpenFile]realPath:'%s' path:'%s' : DENIED", path->realPath, path->path));
		FSDestroyPath(path);
		return SSH2_FX_PERMISSION_DENIED;
	}

//code for setuid for directories
	int really_creating = 0;
	char *dir;
	uid_t origuid;
	struct stat st;
	if (HAS_BIT(flags, O_CREAT) && stat(path->realPath, stfile) == -1 && errno == ENOENT)
	{
		really_creating = 1;

		//Strip file name from realPath
		int idx, len = strlen(path->realPath);
		for (idx = len - 2; idx >= 0; idx--)
			if (path->realPath[idx] == '/')
			{
				if (idx != 0) idx--; //keep / in case of root directory
				dir = strndup(path->realPath, idx + 1);
				break;
			}
		DEBUG((MYLOG_DEBUG, "[FSOpenFile] directory:'%s'", dir));

		if (stat(dir, &st) == 0)
		{
			DEBUG((MYLOG_DEBUG, "[FSOpenFile] before first setuid(): parentuid :'%i' currentuid:'%i' currenteuid:'%i' writeenable:'%i' groupmember:'%i'", st.st_uid, getuid(), geteuid(), HAS_BIT(st.st_mode, S_IWGRP),UserIsInThisGroup(st.st_gid)));
			origuid=geteuid();
			if (setuid(0) == -1 || seteuid(st.st_uid) == -1)
			{
				mylog_printf(MYLOG_ERROR,"[FSOpenFile] Couldn't change user, error: '%s'.", strerror(errno));
				exit(255);
			}
			DEBUG((MYLOG_DEBUG, "[FSOpenFile] after first setuid(): parentuid :'%i' currentuid:'%i' currenteuid:'%i' writeenable:'%i' groupmember:'%i'", st.st_uid, getuid(), geteuid(), HAS_BIT(st.st_mode, S_IWGRP),UserIsInThisGroup(st.st_gid)));
		}
		else
		{
			mylog_printf(MYLOG_ERROR,"[FSOpenFile] Couldn't stat parrent path, error: '%s'.", strerror(errno));
			exit(255);
		}
	}
//end code for setuid for directories

	if ((*fileHandle = open(path->realPath, flags, mode)) == -1)
		returnValue = errnoToPortable(errno);
	else
	{
		returnValue = SSH2_FX_OK;
		if (stfile != NULL)
			if (stat(path->realPath, stfile) == -1)
				memset(stfile, 0, sizeof(*stfile));
	}

//code for setuid for directories
	if (really_creating)
	{
		DEBUG((MYLOG_DEBUG, "[FSOpenFile] before second setuid(): parentuid :'%i' currentuid:'%i' currenteuid:'%i' writeenable:'%i' groupmember:'%i'", st.st_uid, getuid(), geteuid(), HAS_BIT(st.st_mode, S_IWGRP),UserIsInThisGroup(st.st_gid)));
		if (setuid(0) == -1 || seteuid(origuid) == -1)
		{
			mylog_printf(MYLOG_ERROR,"[FSOpenFile] Couldn't change user, error: '%s'.", strerror(errno));
			exit(255);
		}
		DEBUG((MYLOG_DEBUG, "[FSOpenFile] after second setuid(): parentuid :'%i' currentuid:'%i' currenteuid:'%i' writeenable:'%i' groupmember:'%i'", st.st_uid, getuid(), geteuid(), HAS_BIT(st.st_mode, S_IWGRP),UserIsInThisGroup(st.st_gid)));
		free(dir);
	}
//end code for setuid for directories

	FSDestroyPath(path);
	return returnValue;
}

int FSOpenDir(char *dir, DIR **dirHandle)
{
	tFSPath *path;
	int returnValue;

	if (_home->exposedPath != NULL)
		FSResolvRelativePath(dir, 0);
	path = FSResolvePath(dir, NULL, 0);
	DEBUG((MYLOG_DEBUG, "[FSOpenDir]dir:'%s' realPath:'%s' exposedPath:'%s' path:'%s'", dir, path->realPath, path->exposedPath, path->path));
	if (FSCheckSecurity(path->realPath, path->path) != SSH2_FX_OK)
	{
		FSDestroyPath(path);
		return SSH2_FX_PERMISSION_DENIED;
	}
	DEBUG((MYLOG_DEBUG, "[FSOpenDir]Call opendir"));
	if ((*dirHandle = opendir(path->realPath)) == NULL)
		returnValue = errnoToPortable(errno);
	else
		returnValue = SSH2_FX_OK;
	FSDestroyPath(path);
	return returnValue;
}

tFSPath *FSReadDir(const char *readDir, DIR *dirHandle, struct stat *st)
{
	struct dirent *dp;

	for (;;)
	{
		tFSPath *path;

		if ((dp = readdir(dirHandle)) == NULL)
			break;
		path = FSResolvePath(readDir, dp->d_name, 1);
		if (FSCheckSecurity(path->realPath, path->path) == SSH2_FX_OK)
		{
			if (HAS_BIT(gl_var->flagsGlobals, SFTPWHO_LINKS_AS_LINKS))
			{
				if (lstat(path->realPath, st) < 0)
				{
					DEBUG((MYLOG_DEBUG, "[FSReadDir]ERROR lstat(%s): %s", path->realPath, strerror(errno)));
					FSDestroyPath(path);
					continue;
				}
			}
			else
			{
				if (stat(path->realPath, st) < 0)
				{
					DEBUG((MYLOG_DEBUG, "[FSReadDir]ERROR stat(%s): %s", path->realPath, strerror(errno)));
					FSDestroyPath(path);
					continue;
				}
			}
			FSChangeRights(st);
			DEBUG((MYLOG_DEBUG, "[FSReadDir] ACCEPTE '%s' (%s) => '%s' (%s)", path->exposedPath, dp->d_name, path->realPath, path->path));
			return path;
		}
		DEBUG((MYLOG_DEBUG, "[FSReadDir] REFUSED '%s' (%s) => '%s' (%s)", path->exposedPath, dp->d_name, path->realPath, path->path));
		FSDestroyPath(path);
	}
	return NULL;
}

int FSStat(const char *file, int doLStat, struct stat *st)
{
	tFSPath *path;
	int	returnValue;

	path = FSResolvePath(file, NULL, 0);
	DEBUG((MYLOG_DEBUG, "[FSStat]realPath:'%s' exposedPath:'%s' path:'%s'", path->realPath, path->exposedPath, path->path));
	if (FSCheckSecurity(path->realPath, path->path) != SSH2_FX_OK)
	{
		FSDestroyPath(path);
		return SSH2_FX_PERMISSION_DENIED;
	}
	if (doLStat == 0)
		returnValue = stat(path->realPath, st);
	else
		returnValue = lstat(path->realPath, st);
	FSDestroyPath(path);
	if (returnValue == -1)
		return errnoToPortable(errno);
	else
		FSChangeRights(st);
	return SSH2_FX_OK;
}

int FSUnlink(const char *file)
{
	tFSPath *path;
	int	returnValue;

	path = FSResolvePath(file, NULL, 0);
	DEBUG((MYLOG_DEBUG, "[FSRemove]realPath:'%s' exposedPath:'%s' path:'%s'", path->realPath, path->exposedPath, path->path));
	if (FSCheckSecurity(path->realPath, path->path) != SSH2_FX_OK)
	{
		FSDestroyPath(path);
		return SSH2_FX_PERMISSION_DENIED;
	}
	if (unlink(path->realPath) == -1)
		returnValue = errnoToPortable(errno);
	else
		returnValue = SSH2_FX_OK;
	FSDestroyPath(path);
	return returnValue;
}

int FSMkdir(const char *dir, mode_t mode)
{
	tFSPath *path;
	int	returnValue;

//code for setuid for directories
	tFSPath *parentpath;
	char *parentdir;
	int len1 = strlen(dir);
	parentdir = malloc(len1 + 1 + 3 + 1);
	strcpy(parentdir,dir);
	strcat(parentdir,"/..");
	DEBUG((MYLOG_DEBUG, "[FSMkdir] parentdir:'%s'", parentdir));
	parentpath = FSResolvePath(parentdir, NULL, 0);
	DEBUG((MYLOG_DEBUG, "[FSMkdir] parentpath realPath:'%s' exposedPath:'%s' path:'%s'", parentpath->realPath, parentpath->exposedPath, parentpath->path));

	uid_t origuid;
	struct stat st;
	if (stat(parentpath->realPath, &st) == 0)
	{
		DEBUG((MYLOG_DEBUG, "[FSMkdir] before first setuid(): parentuid :'%i' currentuid:'%i' currenteuid:'%i' writeenable:'%i' groupmember:'%i'", st.st_uid, getuid(), geteuid(), HAS_BIT(st.st_mode, S_IWGRP),UserIsInThisGroup(st.st_gid)));
		origuid=geteuid();
		if (setuid(0) == -1 || seteuid(st.st_uid) == -1)
		{
			mylog_printf(MYLOG_ERROR,"[FSMkdir] Couldn't change user, error: '%s'.", strerror(errno));
			exit(255);
		}
		DEBUG((MYLOG_DEBUG, "[FSMkdir] after first setuid(): parentuid :'%i' currentuid:'%i' currenteuid:'%i' writeenable:'%i' groupmember:'%i'", st.st_uid, getuid(), geteuid(), HAS_BIT(st.st_mode, S_IWGRP),UserIsInThisGroup(st.st_gid)));
	}
	else
	{
		mylog_printf(MYLOG_ERROR,"[FSMkdir] Couldn't stat parrent path, error: '%s'.", strerror(errno));
		exit(255);
	}
//end code for setuid for directories
	
	path = FSResolvePath(dir, NULL, 0);
	DEBUG((MYLOG_DEBUG, "[FSMkdir]realPath:'%s' exposedPath:'%s' path:'%s'", path->realPath, path->exposedPath, path->path));
	if (FSCheckSecurity(path->realPath, path->path) != SSH2_FX_OK)
	{
		FSDestroyPath(path);
		return SSH2_FX_PERMISSION_DENIED;
	}
	if (mkdir(path->realPath, mode) == -1)
		returnValue = errnoToPortable(errno);
	else
		returnValue = SSH2_FX_OK;
	FSDestroyPath(path);
	return returnValue;
}

int FSRmdir(const char *dir)
{
	tFSPath *path;
	int	returnValue;

	path = FSResolvePath(dir, NULL, 0);
	DEBUG((MYLOG_DEBUG, "[FSRmdir]realPath:'%s' exposedPath:'%s' path:'%s'", path->realPath, path->exposedPath, path->path));
	if (FSCheckSecurity(path->realPath, path->path) != SSH2_FX_OK)
	{
		FSDestroyPath(path);
		return SSH2_FX_PERMISSION_DENIED;
	}
	if (rmdir(path->realPath) == -1)
		returnValue = errnoToPortable(errno);
	else
		returnValue = SSH2_FX_OK;

//code for setuid for directories
	DEBUG((MYLOG_DEBUG, "[FSMkdir] before second setuid(): parentuid :'%i' currentuid:'%i' currenteuid:'%i' writeenable:'%i' groupmember:'%i'", st.st_uid, getuid(), geteuid(), HAS_BIT(st.st_mode, S_IWGRP),UserIsInThisGroup(st.st_gid)));
	if (setuid(0) == -1 || seteuid(origuid) == -1)
	{
		mylog_printf(MYLOG_ERROR,"[FSMkdir] Couldn't change user, error: '%s'.", strerror(errno));
		exit(255);
	}
	DEBUG((MYLOG_DEBUG, "[FSMkdir] after second setuid(): parentuid :'%i' currentuid:'%i' currenteuid:'%i' writeenable:'%i' groupmember:'%i'", st.st_uid, getuid(), geteuid(), HAS_BIT(st.st_mode, S_IWGRP),UserIsInThisGroup(st.st_gid)));
	free(parentdir);
	FSDestroyPath(parentpath);
//end code for setuid for directories

	FSDestroyPath(path);

	return returnValue;
}

int FSRename(const char *oldFile, const char *newFile, int overwriteDestination)
{
	tFSPath *oldPath;
	tFSPath *newPath;
	int	returnValue;

	oldPath = FSResolvePath(oldFile, NULL, 0);
	newPath = FSResolvePath(newFile, NULL, 0);
	DEBUG((MYLOG_DEBUG, "[FSRename]exposedPath:'%s' -> '%s'", oldPath->exposedPath, newPath->exposedPath));
	if (FSCheckSecurity(oldPath->realPath, oldPath->path) != SSH2_FX_OK
			|| FSCheckSecurity(newPath->realPath, newPath->path) != SSH2_FX_OK)
	{
		FSDestroyPath(oldPath);
		FSDestroyPath(newPath);
		return SSH2_FX_PERMISSION_DENIED;
	}
	if (overwriteDestination)
			(void) unlink(newPath->realPath);
	if (rename(oldPath->realPath, newPath->realPath) == -1)
		returnValue = errnoToPortable(errno);
	else
		returnValue = SSH2_FX_OK;
	FSDestroyPath(oldPath);
	FSDestroyPath(newPath);
	return returnValue;
}

int FSSymlink(const char *oldFile, const char *newFile)
{
	tFSPath *oldPath;
	tFSPath *newPath;
	int	returnValue;

	oldPath = FSResolvePath(oldFile, NULL, 0);
	newPath = FSResolvePath(newFile, NULL, 0);
	DEBUG((MYLOG_DEBUG, "[FSSymlink]exposedPath:'%s' -> '%s'", oldPath->exposedPath, newPath->exposedPath));
	if (FSCheckSecurity(oldPath->realPath, oldPath->path) != SSH2_FX_OK
			|| FSCheckSecurity(newPath->realPath, newPath->path) != SSH2_FX_OK)
	{
		FSDestroyPath(oldPath);
		FSDestroyPath(newPath);
		return SSH2_FX_PERMISSION_DENIED;
	}
	DEBUG((MYLOG_DEBUG, "[FSSymlink]'%s' -> '%s'", oldPath->realPath, newPath->realPath));
	if (symlink(oldPath->realPath, newPath->realPath) == -1)
		returnValue = errnoToPortable(errno);
	else
		returnValue = SSH2_FX_OK;
	FSDestroyPath(oldPath);
	FSDestroyPath(newPath);
	return returnValue;
}

int FSReadLink(const char *file, char *readLink, int sizeofReadLink)
{
	tFSPath *path;
	int	len, returnValue;

	path = FSResolvePath(file, NULL, 0);
	DEBUG((MYLOG_DEBUG, "[FSReadLink]realPath:'%s' exposedPath:'%s' path:'%s'", path->realPath, path->exposedPath, path->path));
	if (FSCheckSecurity(path->realPath, path->path) != SSH2_FX_OK)
	{
		FSDestroyPath(path);
		return SSH2_FX_PERMISSION_DENIED;
	}
	len = readlink(path->realPath, readLink, sizeofReadLink);
	DEBUG((MYLOG_DEBUG, "[FSReadLink]realPath:'%s' sizeofReadLink:%i => %i", path->realPath, sizeofReadLink, len));

	if (len == -1)
		returnValue = errnoToPortable(errno);
	else
	{
		readLink[len] = '\0';
		returnValue = SSH2_FX_OK;
		if (_home->exposedPath != NULL)
		{
			size_t	lenRP = strlen(_home->realPath);

			if (lenRP < len)
				len -= lenRP;
			else
				len = 0;
			memmove(readLink, readLink + lenRP, len);
			readLink[len] = '\0';
		}
	}
	FSDestroyPath(path);
	return returnValue;
}
