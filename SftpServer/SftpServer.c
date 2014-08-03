/*
 MySecureShell permit to add restriction to modified sftp-server
 when using MySecureShell as shell.
 Copyright (C) 2007 Sebastien Tardif

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
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../Core/FileSpec.h"
#include "Access.h"
#include "Defines.h"
#include "Encoding.h"
#include "FileSystem.h"
#include "Global.h"
#include "Handle.h"
#include "Log.h"
#include "Sftp.h"
#include "GetUsersInfos.h"
#include "SftpServer.h"

tGlobal *gl_var = NULL;


static void end_sftp()
{
	if (gl_var != NULL)
	{
		if (cVersion != SSH2_ADMIN_VERSION)
		{
			CloseInfoForOpenFiles();
			mylog_printf(MYLOG_CONNECTION, "[%s][%s]Quit.", gl_var->user, gl_var->ip);
		}
		mylog_close_and_free();
		SftpWhoReleaseStruct(gl_var->who);
		if (gl_var->force_user != NULL)
		{
			free(gl_var->force_user);
			gl_var->force_user = NULL;
		}
		if (gl_var->force_group != NULL)
		{
			free(gl_var->force_group);
			gl_var->force_group = NULL;
		}
		free(gl_var->user);
		free(gl_var->ip);
		free(gl_var->home);
		free(gl_var);
		gl_var = NULL;
		setCharset(NULL);
		BufferDelete(bIn);
		BufferDelete(bOut);
		free_usersinfos();
		HandleCloseAll();
		FreeAccess();
		FileSpecDestroy();
		FSShutdown();
	}
	_exit(0);
}

static void end_sftp_by_signal(int signal)
{
	gl_var->must_shutdown = 1;
}

static void reopen_log_file(int signal)
{
	mylog_reopen();
}

void ParseConf(tGlobal *params, int sftpProtocol)
{
	gl_var = params;
	(void) atexit(end_sftp);
	(void) signal(SIGHUP, end_sftp_by_signal);
	(void) signal(SIGINT, end_sftp_by_signal);
	(void) signal(SIGTERM, end_sftp_by_signal);
	(void) signal(SIGUSR1, reopen_log_file);
	(void) signal(SIGUSR2, reopen_log_file);
	if (sftpProtocol > 0)
		cVersion = sftpProtocol;
}

void DoInitUser()
{
	t_info *pw;
	int uid, gid;

	mylog_printf(MYLOG_CONNECTION, "New client [%s] from [%s]", gl_var->user, gl_var->ip);
	umask(000);
	uid = getuid();
	if (gl_var->force_user != NULL)
	{
		if ((pw = mygetpwnam(gl_var->force_user)) != NULL)
			uid = pw->id;
		else
			mylog_printf(MYLOG_WARNING,
					"[%s][%s]Unable to force user: %s (user unknown)",
					gl_var->user, gl_var->ip, gl_var->force_user);
	}
	gid = getgid();
	if (gl_var->force_group != NULL)
	{
		if ((pw = mygetgrnam(gl_var->force_group)) != NULL)
			gid = pw->id;
		else
			mylog_printf(MYLOG_WARNING,
					"[%s][%s]Unable to force group: %s (group unknown)",
					gl_var->user, gl_var->ip, gl_var->force_group);
	}
	if (HAS_BIT(gl_var->flagsGlobals, SFTPWHO_CREATE_HOME)
			&& chdir(gl_var->home) == -1 && errno == ENOENT)
	{
		int mode = 0755;

		mode |= gl_var->minimum_rights_directory;
		mode &= gl_var->maximum_rights_directory;
		if (mkdir(gl_var->home, mode) == -1)
		{
			mylog_printf(MYLOG_ERROR,
					"[%s][%s]Couldn't create to home '%s' : %s",
					gl_var->user, gl_var->ip, gl_var->home,
					strerror(errno));
		}
		else if (chown(gl_var->home, uid, gid) == -1)
			mylog_printf(MYLOG_ERROR,
					"[%s][%s]Couldn't chown the home '%s' : %s",
					gl_var->user, gl_var->ip, gl_var->home,
					strerror(errno));
	}
	if (chdir(gl_var->home) == -1)
		mylog_printf(MYLOG_ERROR, "[%s][%s]Couldn't go to home '%s' : %s",
				gl_var->user, gl_var->ip, gl_var->home,
				strerror(errno));
	if (HAS_BIT(gl_var->flagsGlobals, SFTPWHO_VIRTUAL_CHROOT))
	{
		gl_var->flagsGlobals &= ~SFTPWHO_STAY_AT_HOME;
		FSInit(gl_var->home, "/");
	}
	else if (HAS_BIT(gl_var->flagsGlobals, SFTPWHO_STAY_AT_HOME))
		FSInit(gl_var->home, NULL);
	else
		FSInit(gl_var->home, NULL);
	if (gl_var->force_group != NULL)
	{
		mylog_printf(MYLOG_WARNING, "[%s][%s]Using force group: %s", gl_var->user, gl_var->ip, gl_var->force_group);
		if (setgid(gid) == -1)
			mylog_printf(MYLOG_WARNING,
					"[%s][%s]Unable to force group: %s (%s)",
					gl_var->user, gl_var->ip, gl_var->force_group,
					strerror(errno));
	}
	if (gl_var->force_user != NULL)
	{
		mylog_printf(MYLOG_WARNING, "[%s][%s]Using force user: %s",
				gl_var->user, gl_var->ip, gl_var->force_user);
		if (setuid(uid) == -1)
			mylog_printf(MYLOG_WARNING,
					"[%s][%s]Unable to force user: %s (%s)", gl_var->user,
					gl_var->ip, gl_var->force_user, strerror(errno));
	}
	if (getuid() != geteuid()) //revoke root rights in user mode !
	{
		if (seteuid(uid) == -1 || setegid(gid) == -1)
		{
			mylog_printf(MYLOG_ERROR,
					"[%s][%s]Couldn't revoke root rights : %s",
					gl_var->user, gl_var->ip, strerror(errno));
			exit(255);
		}
	}
}

int CheckRulesAboutMaxFiles()
{
	t_sftpwho *who;
	int i, fileread, filewrite, fileall;

	if ((who = SftWhoGetAllStructs()) != NULL)
	{
		fileread = 0;
		filewrite = 0;
		fileall = 0;
		for (i = 0; i < SFTPWHO_MAXCLIENT; i++)
			if (strcmp(who[i].user, gl_var->user) == 0)
			{
				switch (who[i].status & SFTPWHO_STATUS_MASK)
				{
				case SFTPWHO_PUT:
					filewrite++;
					fileall++;
					break;
				case SFTPWHO_GET:
					fileread++;
					fileall++;
					break;
				}
			}
		if ((fileread > gl_var->max_readfiles && gl_var->max_readfiles != 0)
				|| (filewrite > gl_var->max_writefiles
						&& gl_var->max_writefiles != 0) || (fileall
				> gl_var->max_openfiles && gl_var->max_openfiles != 0))
			return SSH2_FX_PERMISSION_DENIED;
	}
	return SSH2_FX_OK;
}

void UpdateInfoForOpenFiles()
{
	tHandle *lastFile;

	lastFile = HandleGetLastOpen(HANDLE_FILE);
	if (lastFile != NULL)
	{
		(void) snprintf(gl_var->who->file, sizeof(gl_var->who->file), "%s", lastFile->path);
		if (lastFile->flags & O_WRONLY)
			gl_var->who->status = (gl_var->who->status & SFTPWHO_ARGS_MASK) | SFTPWHO_PUT;
		else
			gl_var->who->status = (gl_var->who->status & SFTPWHO_ARGS_MASK) | SFTPWHO_GET;
		if (lastFile->fileSize > 0)
			gl_var->who->download_pos = lastFile->filePos * 100 / lastFile->fileSize;
		else
			gl_var->who->download_pos = 0;
	}
	else
	{
		gl_var->who->file[0] = '\0';
		gl_var->who->status = (gl_var->who->status & SFTPWHO_ARGS_MASK) | SFTPWHO_IDLE;
	}
}

void CloseInfoForOpenFiles()
{
	tHandle *hdl;
	int pourcentage;

	while ((hdl = HandleGetLastOpen(HANDLE_FILE)) != NULL)
	{
		if (hdl->fileSize > 0)
			pourcentage = hdl->filePos * 100 / hdl->fileSize;
		else
			pourcentage = 0;
		if (FILE_IS_UPLOAD(hdl->flags))
		{
			mylog_printf(MYLOG_TRANSFERT,
					"[%s][%s]Interrupt upload into file '%s'",
					gl_var->user, gl_var->ip, hdl->path);
		}
		else
		{
			mylog_printf(MYLOG_TRANSFERT,
					"[%s][%s]Interrupt download file '%s' : %i%%",
					gl_var->user, gl_var->ip, hdl->path, pourcentage);
		}
		HandleClose(hdl->id);
	}
}
