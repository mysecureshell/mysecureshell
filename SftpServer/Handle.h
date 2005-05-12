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

#ifndef _HANDLE_H_
#define _HANDLE_H_

#include <dirent.h>

enum
{
  HANDLE_UNUSED,
  HANDLE_DIR,
  HANDLE_FILE
};

void	HandleInit();
int	HandleNew(int state, char *path, int fd, DIR *dir, int fileIsText);
DIR	*HandleGetDir(int pos);
int	HandleGetFd(int pos, int *fileIsText);
char	*HandleGetPath(int pos);
int	HandleClose(int pos);

#endif //_HANDLE_H_
