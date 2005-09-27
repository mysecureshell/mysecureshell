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

#ifndef __LOG_H__
#define __LOG_H__

#define	MYLOG_NORMAL	0
#define	MYLOG_WARNING	1
#define	MYLOG_ERROR	2
#define	MYLOG_DEBUG	3
#define	MYLOG_MAX	4

void	log_open(char *file);
void	log_close();
void	log_printf(int level, char *str, ...);

#ifdef DODEBUG
#define DEBUG(_X)       log_printf _X
#else
#define DEBUG(_X)
#endif

#endif
