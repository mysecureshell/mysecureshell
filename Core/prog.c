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

#include "../config.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "hash.h"
#include "ip.h"
#include "prog.h"
#include "../SftpServer/SftpWho.h"

int count_program_for_uid(/*@null@*/ const char *login)
{
	t_sftpwho *who;
	int i, nb;

	nb = 0;
	if ((who = SftWhoGetAllStructs()) != NULL)
	{
		for (i = 0; i < SFTPWHO_MAXCLIENT; i++)
			if ((who[i].status & SFTPWHO_STATUS_MASK) != SFTPWHO_EMPTY)
				if (login == NULL || strcmp(who[i].user, login) == 0)
					nb++;
	}
	return (nb);
}

int count_program_for_ip(/*@null@*/ const char *host)
{
	t_sftpwho *who;
	int i, nb;

	nb = 0;
	if ((who = SftWhoGetAllStructs()) != NULL)
	{
		for (i = 0; i < SFTPWHO_MAXCLIENT; i++)
			if ((who[i].status & SFTPWHO_STATUS_MASK) != SFTPWHO_EMPTY)
				if (host == NULL || strcmp(who[i].ip, host) == 0)
					nb++;
	}
	return (nb);
}
