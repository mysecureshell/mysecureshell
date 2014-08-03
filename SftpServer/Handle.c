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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Handle.h"
#include "../Core/security.h"

/*@null@*/ static tHandle *gHandle = NULL;

void HandleInit()
{
	int i;

	gHandle = calloc(HANDLE_NUMBER, sizeof(*gHandle));
	if (gHandle != NULL)
		for (i = 0; i < HANDLE_NUMBER; i++)
		{
			gHandle[i].id = i;
			gHandle[i].fd = -1;
		}
}

void HandleCloseAll()
{
	if (gHandle != NULL)
	{
		int i;

		for (i = 0; i < HANDLE_NUMBER; i++)
			if (gHandle[i].state != HANDLE_UNUSED)
				HandleClose(i);
		free(gHandle);
		gHandle = NULL;
	}
}

tHandle *HandleNew(int state, char *path, int fd, DIR *dir, int fileIsText, int flags)
{
	int i;

	if (gHandle != NULL)
		for (i = 0; i < HANDLE_NUMBER; i++)
			if (gHandle[i].state == HANDLE_UNUSED)
			{
				gHandle[i].state = state;
				gHandle[i].dir = dir;
				gHandle[i].fd = fd;
				gHandle[i].path = path;
				gHandle[i].fileIsText = fileIsText;
				gHandle[i].flags = flags;
				return (&gHandle[i]);
			}
	return (NULL);
}

tHandle *HandleGet(int pos)
{
	if (gHandle != NULL && pos >= 0 && pos < HANDLE_NUMBER)
		return (&gHandle[pos]);
	return (NULL);
}

tHandle *HandleGetFile(int pos)
{
	if (gHandle != NULL && pos >= 0 && pos < HANDLE_NUMBER && gHandle[pos].state == HANDLE_FILE)
		return (&gHandle[pos]);
	return (NULL);
}

tHandle *HandleGetDir(int pos)
{
	if (gHandle != NULL && pos >= 0 && pos < HANDLE_NUMBER && gHandle[pos].state == HANDLE_DIR)
		return (&gHandle[pos]);
	return (NULL);
}

tHandle *HandleGetLastOpen(int state)
{
	tHandle *lastHdl = NULL;
	int i;

	if (gHandle != NULL)
		for (i = 0; i < HANDLE_NUMBER; i++)
			if (gHandle[i].state == state)
				lastHdl = &gHandle[i];
	return (lastHdl);
}

void HandleClose(int pos)
{
	if (gHandle != NULL && pos >= 0 && pos < HANDLE_NUMBER)
	{
		if (gHandle[pos].state == HANDLE_DIR)
			(void) closedir(gHandle[pos].dir);
		else
			xclose(gHandle[pos].fd);
		free(gHandle[pos].path);

		gHandle[pos].dir = NULL;
		gHandle[pos].fd = -1;
		gHandle[pos].path = NULL;
		gHandle[pos].state = HANDLE_UNUSED;
	}
}
