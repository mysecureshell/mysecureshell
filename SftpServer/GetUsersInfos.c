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

static t_info	*_users = NULL;
static t_info	*_groups = NULL;
static int	_usersSize = 0;
static int	_groupsSize = 0;

static t_info	*add_element_in_table(t_info **tb, int *tbSize, char *name, int id)
{
  DEBUG((MYLOG_DEBUG, "add_element_in_table(%p, %i, %s, %i)", *tb, *tbSize, name, id));
  if ((*tbSize % TB_ALLOC_SIZE) == 0)
    {
      *tb = realloc(*tb, *tbSize + TB_ALLOC_SIZE);
      DEBUG((MYLOG_DEBUG, "Increaze size to: %i", *tbSize + TB_ALLOC_SIZE));
    }
  (*tb)[*tbSize].name = name;
  (*tb)[*tbSize].id = id;
  return &((*tb)[(*tbSize)++]);
}

void		init_usersinfos()
{
  if (HAS_BIT(gl_var->status, SFTPWHO_VIRTUAL_CHROOT))
    {
      struct passwd	*pw;
      struct group	*grp;
      int		size;
     
      DEBUG((MYLOG_DEBUG, "Load all users..."));
      (void )setpwent();
      for (size = 0; (pw = getpwent()) != NULL; size++)
	{
	  _users = realloc(_users, (size + 2) * sizeof(*_users));
	  _users[size].name = strdup(pw->pw_name);
	  _users[size].id = pw->pw_uid;
	}
      endpwent();
      _usersSize = size;
      DEBUG((MYLOG_DEBUG, "Load all groups..."));
      (void )setgrent();
      for (size = 0; (grp = getgrent()) != NULL; size++)
	{
	  _groups = realloc(_groups, (size + 2) * sizeof(*_groups));
	  _groups[size].name = strdup(grp->gr_name);
	  _groups[size].id = grp->gr_gid;
	}
      endgrent();
      _groupsSize = size;
    }
}

t_info	*mygetpwnam(const char *login)
{
  int	i;

  if (login == NULL)
    return (NULL);
  for (i = 0; i < _usersSize; i++)
    if (strcmp(_users[i].name, login) == 0)
      return (&_users[i]);
  if (!HAS_BIT(gl_var->status, SFTPWHO_VIRTUAL_CHROOT))
    {
      struct passwd	*pwd;

      if ((pwd = getpwnam(login)) != NULL)
	return (add_element_in_table(&_users, &_usersSize, strdup(pwd->pw_name), pwd->pw_uid));
    }
  mylog_printf(MYLOG_WARNING, "[%s][%s]Couldn't resolve user name %i",
	       gl_var->who->user, gl_var->who->ip, login);
  return (NULL);
}


t_info	*mygetpwuid(uid_t uid)
{
  int	i;

  for (i = 0; i < _usersSize; i++)
    if (_users[i].id == uid)
      return (&_users[i]);
  if (!HAS_BIT(gl_var->status, SFTPWHO_VIRTUAL_CHROOT))
    {
      struct passwd	*pwd;

      if ((pwd = getpwuid(uid)) != NULL)
	return (add_element_in_table(&_users, &_usersSize, strdup(pwd->pw_name), pwd->pw_uid));
    }
  mylog_printf(MYLOG_WARNING, "[%s][%s]Couldn't resolve user id %i",
	       gl_var->who->user, gl_var->who->ip, uid);
  return (NULL);
}

t_info	*mygetgrnam(const char *group)
{
  int	i;

  if (group == NULL)
    return (NULL);
  for (i = 0; i < _groupsSize; i++)
    if (strcmp(_groups[i].name, group) == 0)
      return (&_groups[i]);
  if (!HAS_BIT(gl_var->status, SFTPWHO_VIRTUAL_CHROOT))
    {
      struct group	*grp;

      if ((grp = getgrnam(group)) != NULL)
	return (add_element_in_table(&_groups, &_groupsSize, strdup(grp->gr_name), grp->gr_gid));
    }
  mylog_printf(MYLOG_WARNING, "[%s][%s]Couldn't resolve group name %i",
	       gl_var->who->user, gl_var->who->ip, group);
  return (NULL);
}

t_info	*mygetgrgid(gid_t gid)
{
  int	i;

  for (i = 0; i < _groupsSize; i++)
    if (_groups[i].id == gid)
      return (&_groups[i]);
  if (!HAS_BIT(gl_var->status, SFTPWHO_VIRTUAL_CHROOT))
    {
      struct group	*grp;

      if ((grp = getgrgid(gid)) != NULL)
	return (add_element_in_table(&_groups, &_groupsSize, strdup(grp->gr_name), grp->gr_gid));
    }
  mylog_printf(MYLOG_WARNING, "[%s][%s]Couldn't resolve group id %i",
	       gl_var->who->user, gl_var->who->ip, gid);
  return (NULL);
}
