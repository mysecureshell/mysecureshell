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

#ifndef __LOG_H__
#define __LOG_H__

#define	MYLOG_OFF			0
#define MYLOG_CONNECTION	1
#define MYLOG_TRANSFERT		2
#define	MYLOG_ERROR			3
#define	MYLOG_WARNING		4
#define	MYLOG_NORMAL		5
#define	MYLOG_DEBUG			6
#define	MYLOG_MAX			7

void	mylog_level(int level);
void	mylog_open(char *file, int useSyslog);
void	mylog_close_and_free();
void	mylog_close();
void	mylog_reopen();
void	mylog_time(int hours);
void	mylog_printf(int level, const char *str, ...);

#ifdef DODEBUG
#define DEBUG(_X)	mylog_printf _X
#else
#define DEBUG(_X)
#endif

#endif
