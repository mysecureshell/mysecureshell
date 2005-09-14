/*
MySecureShell permit to add restriction to modified sftp-server
when using MySecureShell as shell.
Copyright (C) 2004 Sebastien Tardif

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

#ifndef __LOG_C__
#define __LOG_C__

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define	MYLOG_NORMAL	0
#define	MYLOG_WARNING	1
#define	MYLOG_ERROR	2
#define	MYLOG_DEBUG	3
#define	MYLOG_MAX	4

typedef struct	s_log
{
  int		fd;
  int		pid;
#ifdef HAVE_LOG_IN_COLOR
  unsigned char	color[MYLOG_MAX][3];
#endif
}		t_log;

static t_log	*_log = 0;

static void	log_open(char *file)
{
  int		fd;

  if ((fd = open(file, O_CREAT | O_APPEND | O_WRONLY, 0644)) != -1)
    {
      _log = calloc(sizeof(*_log), 1);
      _log->pid = getpid();
      _log->fd = fd;
      fchown(fd, 0, 0);
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
      _log->color[MYLOG_NORMAL][0] = 37;
      _log->color[MYLOG_NORMAL][1] = 40;
      _log->color[MYLOG_NORMAL][2] = 0;

      _log->color[MYLOG_WARNING][0] = 33;
      _log->color[MYLOG_WARNING][1] = 40;
      _log->color[MYLOG_WARNING][2] = 1;

      _log->color[MYLOG_ERROR][0] = 31;
      _log->color[MYLOG_ERROR][1] = 40;
      _log->color[MYLOG_ERROR][2] = 1;

      _log->color[MYLOG_DEBUG][0] = 30;
      _log->color[MYLOG_DEBUG][1] = 47;
      _log->color[MYLOG_DEBUG][2] = 8;
#endif
    }
}

static void	log_close()
{
  if (_log)
    {
      close(_log->fd);
      free(_log);
      _log = 0;
    }
}

static void	log_printf(int level, char *str, ...)
{
  va_list	ap;
  struct tm	*tm;
  time_t	t;
  char		buffer[1024];
  char		fmt[1024];

  if  (_log)
    {
      if (level < 0 || level >= MYLOG_MAX)
	level = MYLOG_ERROR;
      va_start(ap, str);
      t = time(0);
      tm = localtime(&t);
#ifndef HAVE_LOG_IN_COLOR
      if (snprintf(fmt, sizeof(buffer),	"%i-%02i-%02i %02i:%02i:%02i [%i]%s\n",
		   1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec,
		   _log->pid, str) > 0)
#else
      if (snprintf(fmt, sizeof(buffer),	"%i-%02i-%02i %02i:%02i:%02i \33[%i:%i:%im[%i]%s\33[37:40:0m\n",
		   1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec,
		   _log->color[level][0], _log->color[level][1], _log->color[level][2], _log->pid, str) > 0)
#endif
	if (vsnprintf(buffer, sizeof(buffer), fmt, ap) > 0)
	  write(_log->fd, buffer, strlen(buffer));
      va_end(ap);
    }
}

#endif
