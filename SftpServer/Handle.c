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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Handle.h"

#define	HANDLE_NUMBER	100

typedef struct	sHandle
{
  int		state;
  char		*path;
  DIR		*dir;
  int		fd;
  int		fileIsText;
}		tHandle;

static tHandle	*gHandle = 0;

void	HandleInit()
{
  int	i;
	
  gHandle = calloc(HANDLE_NUMBER, sizeof(*gHandle));
  for (i = 0; i < HANDLE_NUMBER; i++)
    gHandle[i].fd = -1;
}

int	HandleNew(int state, char *path, int fd, DIR *dir, int fileIsText)
{
  int	i;
	
  for (i = 0; i < HANDLE_NUMBER; i++)
    if (gHandle[i].state == HANDLE_UNUSED)
      {
	gHandle[i].state = state;
	gHandle[i].dir = dir;
	gHandle[i].fd = fd;
	gHandle[i].path = strdup(path);
	gHandle[i].fileIsText = fileIsText;
	return (i);
      }
  return (-1);
}

DIR	*HandleGetDir(int pos)
{
  if (pos >= 0 && pos < HANDLE_NUMBER)
    return (gHandle[pos].dir);
  return (NULL);
}

int	HandleGetFd(int pos, int *fileIsText)
{
  if (pos >= 0 && pos < HANDLE_NUMBER)
    {
      *fileIsText = gHandle[pos].fileIsText;
      return (gHandle[pos].fd);
    }
  *fileIsText = 0;
  return (-1);
}

char	*HandleGetPath(int pos)
{
  if (pos >= 0 && pos < HANDLE_NUMBER)
    return (gHandle[pos].path);
  return (0);
}

int	HandleClose(int pos, int *isCloseFile)
{
  if (pos >= 0 && pos < HANDLE_NUMBER)
    {
      int	ret;
      
      if (gHandle[pos].state == HANDLE_DIR)
	{
	  ret = closedir(gHandle[pos].dir);
	  *isCloseFile = 0;
	}
      else
	{
	  ret = close(gHandle[pos].fd);
	  *isCloseFile = 1;
	}
      free(gHandle[pos].path);
      
      gHandle[pos].dir = NULL;
      gHandle[pos].fd = -1;
      gHandle[pos].path = NULL;
      gHandle[pos].state = HANDLE_UNUSED;
      return (ret);
    }
  return (-1);
}
