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

#ifndef _UTIL_H_
#define _UTIL_H_

#include <sys/stat.h>

void	StrMode(int mode, char *d);
char	*LsFile(const char *name, const struct stat *st);
int	FlagsFromPortable(int pFlags);
int	errnoToPortable(int unixErrno);
char	*ExecCommand(char *cmd);

#endif //_UTIL_H_
