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
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include "Global.h"
#include "GetUsersInfos.h"
#include "Log.h"

#define	TB_ALLOC_SIZE	42

static t_info *_users = NULL;
static t_info *_groups = NULL;
static int _usersSize = 0;
static int _groupsSize = 0;

static t_info *add_element_in_table(t_info **tb, int *tbSize, char *name, int id)
{
	DEBUG((MYLOG_DEBUG, "add_element_in_table(%p, %i, %s, %i)", *tb, *tbSize, name, id));
	if ((*tbSize % TB_ALLOC_SIZE) == 0)
	{
		*tb = realloc(*tb, (*tbSize + TB_ALLOC_SIZE) * sizeof(t_info));
		DEBUG((MYLOG_DEBUG, "Increaze size to: %i", *tbSize + TB_ALLOC_SIZE));
	}
	(*tb)[*tbSize].name = name;
	(*tb)[*tbSize].id = id;
	return &((*tb)[(*tbSize)++]);
}

void free_usersinfos()
{
	int i;

	if (_users != NULL)
	{
		for (i = 0; i < _usersSize; i++)
			free(_users[i].name);
		free(_users);
		_users = NULL;
	}
	if (_groups != NULL)
	{
		for (i = 0; i < _groupsSize; i++)
			free(_groups[i].name);
		free(_groups);
		_groups = NULL;
	}
}

t_info *mygetpwnam(const char *login)
{
	struct passwd *pwd;
	int i;

	if (login == NULL)
		return (NULL);
	for (i = 0; i < _usersSize; i++)
		if (strcmp(_users[i].name, login) == 0)
			return (&_users[i]);
	if ((pwd = getpwnam(login)) != NULL)
		return (add_element_in_table(&_users, &_usersSize, strdup(pwd->pw_name), pwd->pw_uid));
	mylog_printf(MYLOG_WARNING, "[%s][%s]Couldn't resolve user name %i", gl_var->user, gl_var->ip, login);
	return (NULL);
}

t_info *mygetpwuid(u_int32_t uid)
{
	struct passwd *pwd;
	int i;

	for (i = 0; i < _usersSize; i++)
		if (_users[i].id == uid)
			return (&_users[i]);
	if ((pwd = getpwuid(uid)) != NULL)
		return (add_element_in_table(&_users, &_usersSize, strdup(pwd->pw_name), pwd->pw_uid));
	mylog_printf(MYLOG_WARNING, "[%s][%s]Couldn't resolve user id %i", gl_var->user, gl_var->ip, uid);
	return (NULL);
}

t_info *mygetgrnam(const char *group)
{
	struct group *grp;
	int i;

	if (group == NULL)
		return (NULL);
	for (i = 0; i < _groupsSize; i++)
		if (strcmp(_groups[i].name, group) == 0)
			return (&_groups[i]);
	if ((grp = getgrnam(group)) != NULL)
		return (add_element_in_table(&_groups, &_groupsSize, strdup(grp->gr_name), grp->gr_gid));
	mylog_printf(MYLOG_WARNING, "[%s][%s]Couldn't resolve group name %i", gl_var->user, gl_var->ip, group);
	return (NULL);
}

t_info *mygetgrgid(u_int32_t gid)
{
	struct group *grp;
	int i;

	for (i = 0; i < _groupsSize; i++)
		if (_groups[i].id == gid)
			return (&_groups[i]);
	if ((grp = getgrgid(gid)) != NULL)
		return (add_element_in_table(&_groups, &_groupsSize, strdup(grp->gr_name), grp->gr_gid));
	mylog_printf(MYLOG_WARNING, "[%s][%s]Couldn't resolve group id %i", gl_var->user, gl_var->ip, gid);
	return (NULL);
}
