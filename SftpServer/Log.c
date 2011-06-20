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
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Log.h"
#include "../security.h"

typedef struct s_log
{
	char *file;
	int fd;
	int pid;
	int nextReopen;
#ifdef HAVE_LOG_IN_COLOR
unsigned char color[MYLOG_MAX][3];
#endif
} t_log;

static t_log *_log = NULL;

void mylog_open(char *file)
{
	int fd;

	if ((fd = open(file, O_CREAT | O_APPEND | O_WRONLY, 0644)) != -1)
	{
		if (_log == NULL)
		{
			time_t t;

			t = time(NULL);
			(void) localtime(&t);
			_log = calloc(1, sizeof(*_log));
			_log->pid = getpid();
		}
		_log->file = file;
		_log->fd = fd;
		if (fchown(fd, 0, 0) == -1)
			mylog_printf(MYLOG_ERROR, "Unable to chown log '%s' : %s", file,
					strerror(errno));
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
	if (_log != NULL)
		xclose(_log->fd);
}

void mylog_reopen()
{
	if (_log != NULL)
		_log->nextReopen = 1;
}

void mylog_close_and_free()
{
	if (_log != NULL)
	{
		xclose(_log->fd);
		free(_log->file);
		free(_log);
		_log = NULL;
	}
}

void mylog_printf(int level, const char *str, ...)
{
	va_list ap;
	struct tm *tm;
	time_t t;
	char buffer[1024];
	char fmt[1024];
	int size;

	if (_log != NULL)
	{
		if (level < 0 || level >= MYLOG_MAX)
			level = MYLOG_ERROR;
		if (_log->nextReopen == 1)
		{
			_log->nextReopen = 0;
			mylog_close();
			mylog_open(_log->file);
		}
		va_start(ap, str);
		t = time(NULL);
		if ((tm = localtime(&t)) == NULL)
		{
			if (snprintf(fmt, sizeof(buffer), "[Error with time] [%i]%s\n", _log->pid, str) > 0)
				goto forceShowLog;
		}
#ifndef HAVE_LOG_IN_COLOR
		if (snprintf(fmt, sizeof(buffer), "%i-%02i-%02i %02i:%02i:%02i [%i]%s\n",
				1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec,
				_log->pid, str) > 0)
#else
		if (snprintf(fmt, sizeof(buffer), "%i-%02i-%02i %02i:%02i:%02i \33[%i:%i:%im[%i]%s\33[37:40:0m\n",
				1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec,
				_log->color[level][0], _log->color[level][1], _log->color[level][2], _log->pid, str) > 0)
#endif
		{
forceShowLog:
			if ((size = vsnprintf(buffer, sizeof(buffer), fmt, ap)) > 0)
				(void) write(_log->fd, buffer, size);
		}
		va_end(ap);
	}
}
