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
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif //HAVE_SYSLOG_H
#include "Log.h"
#include "../Core/security.h"

typedef struct s_log
{
	char *file;
	int fd;
	int pid;
	int nextReopen;
	int useSyslog;
#ifdef HAVE_LOG_IN_COLOR
unsigned char color[MYLOG_MAX][3];
#endif
} t_log;

/*@null@*/ static t_log *_log = NULL;

void mylog_open(char *file, int useSyslog)
{
	int fd;

	if (_log == NULL)
	{
		_log = calloc(1, sizeof(*_log));
		if (_log == NULL)
		{
			perror("unable to allocate log structure");
			return;
		}
		_log->pid = getpid();
		_log->fd = -1;
	}
#ifdef HAVE_OPENLOG
	if (useSyslog == 1)
	{
		_log->useSyslog = 1;
		openlog("MySecureShell", LOG_PID, LOG_FTP);
	}
#endif //HAVE_OPENLOG

	if (file != NULL && (fd = open(file, O_CREAT | O_APPEND | O_WRONLY, 0644)) != -1)
	{
		time_t t;

		t = time(NULL);
		(void) localtime(&t);
		if (_log != NULL)
		{
			_log->file = file;
			_log->fd = fd;
			if (fchown(fd, 0, 0) == -1)
				mylog_printf(MYLOG_ERROR, "Unable to chown log '%s' : %s", file, strerror(errno));
		}
		/*
		 Text color codes:
		 30=black 31=red 32=green 33=yellow 34=blue 35=magenta 36=cyan 37=white
		 Background color codes:
		 40=black 41=red 42=green 43=yellow 44=blue 45=magenta 46=cyan 47=white
		 Style:
		 00=none 01=bold 04=underscore 05=blink 07=reverse 08=concealed
		 Form:
		 text_color:background_color:style
		 */
#ifdef HAVE_LOG_IN_COLOR
		_log->color[MYLOG_CONNECTION][0] = (unsigned char )32;
		_log->color[MYLOG_CONNECTION][1] = (unsigned char )40;
		_log->color[MYLOG_CONNECTION][2] = (unsigned char )1;

		_log->color[MYLOG_TRANSFERT][0] = (unsigned char )34;
		_log->color[MYLOG_TRANSFERT][1] = (unsigned char )40;
		_log->color[MYLOG_TRANSFERT][2] = (unsigned char )1;

		_log->color[MYLOG_NORMAL][0] = (unsigned char )37;
		_log->color[MYLOG_NORMAL][1] = (unsigned char )40;
		_log->color[MYLOG_NORMAL][2] = (unsigned char )0;

		_log->color[MYLOG_WARNING][0] = (unsigned char )31;
		_log->color[MYLOG_WARNING][1] = (unsigned char )40;
		_log->color[MYLOG_WARNING][2] = (unsigned char )1;

		_log->color[MYLOG_ERROR][0] = (unsigned char )31;
		_log->color[MYLOG_ERROR][1] = (unsigned char )40;
		_log->color[MYLOG_ERROR][2] = (unsigned char )7;

		_log->color[MYLOG_DEBUG][0] = (unsigned char )30;
		_log->color[MYLOG_DEBUG][1] = (unsigned char )47;
		_log->color[MYLOG_DEBUG][2] = (unsigned char )8;
#endif
	}
}

void mylog_close()
{
	if (_log != NULL && _log->fd != -1)
		xclose(_log->fd);
}

void mylog_reopen()
{
	if (_log != NULL)
		_log->nextReopen = 1;
}

void mylog_close_and_free()
{
	mylog_close();
	if (_log != NULL)
	{
#ifdef HAVE_CLOSELOG
		if (_log->useSyslog == 1)
			closelog();
#endif //HAVE_CLOSELOG
		if (_log->file != NULL)
			free(_log->file);
		free(_log);
		_log = NULL;
	}
}

void mylog_printf(int level, const char *str, ...)
{
	va_list ap;
	char buffer[1024];
	size_t size;

	if (_log != NULL && _log->useSyslog == 1)
	{
		int logprio;

		switch (level)
		{
		case MYLOG_DEBUG: logprio = LOG_DEBUG; break;
		case MYLOG_WARNING: logprio = LOG_WARNING; break;
		case MYLOG_ERROR: logprio = LOG_ERR; break;
		case MYLOG_TRANSFERT: logprio = LOG_NOTICE; break;
		default: logprio = LOG_INFO; break;
		}
		va_start(ap, str);
		vsyslog(logprio, str, ap);
		va_end(ap);
	}
	if (_log != NULL && _log->file != NULL)
	{
		struct tm *tm;
		time_t t;
		char fmt[1024];

		if (_log->nextReopen == 1)
		{
			_log->nextReopen = 0;
			mylog_close();
			mylog_open(_log->file, _log->useSyslog);
		}
		t = time(NULL);
		if ((tm = localtime(&t)) == NULL)
		{
			if (snprintf(fmt, sizeof(buffer), "[Error with time] [%i]%s\n", _log->pid, str) > 0)
				goto forceShowLog;
			return;
		}
#ifndef HAVE_LOG_IN_COLOR
		if (snprintf(fmt, sizeof(buffer), "%i-%02i-%02i %02i:%02i:%02i [%i]%s\n",
				1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec,
				_log->pid, str) > 0)
#else
		if (level < 0 || level >= MYLOG_MAX)
			level = MYLOG_ERROR;
		if (snprintf(fmt, sizeof(buffer), "%i-%02i-%02i %02i:%02i:%02i \33[%i:%i:%im[%i]%s\33[37:40:0m\n",
				1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec,
				_log->color[level][0], _log->color[level][1], _log->color[level][2], _log->pid, str) > 0)
#endif
		{
forceShowLog:
			va_start(ap, str);
			if ((size = vsnprintf(buffer, sizeof(buffer), fmt, ap)) > 0)
				(void) write(_log->fd, buffer, size);
			va_end(ap);
		}
	}
}

void mylog_syslog(int level, const char *str, ...)
{
	;
}
