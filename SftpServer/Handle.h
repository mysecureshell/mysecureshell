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

#ifndef _HANDLE_H_
#define _HANDLE_H_

#include <dirent.h>

#define HANDLE_NUMBER	100

enum
{
  HANDLE_UNUSED,
  HANDLE_DIR,
  HANDLE_FILE
};

typedef struct	sHandle
{
  int		id;
  int		state;
  char		*path;
  DIR		*dir;
  int		fd;
  int		fileIsText;
  int		flags;
  u_int64_t	filePos;
  u_int64_t	fileSize;
}		tHandle;


void	HandleInit();
void	HandleCloseAll();
tHandle	*HandleNew(int state, char *path, int fd, DIR *dir, int fileIsText, int flags);
tHandle	*HandleGet(int pos);
tHandle	*HandleGetFile(int pos);
tHandle	*HandleGetDir(int pos);
tHandle	*HandleGetLastOpen(int state);
void	HandleClose(int pos);

#define HandleNewFile(_PATH, _FD, _FILE_IS_TEXT, _F)	HandleNew(HANDLE_FILE, _PATH, _FD, NULL, _FILE_IS_TEXT, _F)
#define HandleNewDirectory(_PATH, _DIR)			HandleNew(HANDLE_DIR, _PATH, -1, _DIR, 0, 0)

#endif //_HANDLE_H_
