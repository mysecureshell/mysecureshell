/*
MySecureShell permit to add restriction to modified sftp-server
when using MySecureShell as shell.
Copyright (C) 2006 Sebastien Tardif

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

#define MYLOG_CONNECTION	0
#define MYLOG_TRANSFERT		1
#define	MYLOG_NORMAL		2
#define	MYLOG_WARNING		3
#define	MYLOG_ERROR		4
#define	MYLOG_DEBUG		5
#define	MYLOG_MAX		6

void	mylog_open(char *file);
void	mylog_time(int hours);
void	mylog_close();
void	mylog_printf(int level, char *str, ...);

#ifdef DODEBUG
#define DEBUG(_X)	mylog_printf _X
#else
#define DEBUG(_X)
#endif

#endif
