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

#include "../config.h"
#include "GetUsersInfos.h"

static t_info	*_users = 0;
static t_info	*_groups = 0;

void		init_usersinfos()
{
  struct passwd	*pw;
  struct group	*grp;
  int		size;

  setpwent();
  for (size = 0; (pw = getpwent()); size++)
    {
      _users = realloc(_users, (size + 2) * sizeof(*_users));
      _users[size].name = strdup(pw->pw_name);
      _users[size].id = pw->pw_uid;
    }
  endpwent();

  setgrent();
  for (size = 0; (grp = getgrent()); size++)
    {
      _groups = realloc(_groups, (size + 2) * sizeof(*_groups));
      _groups[size].name = strdup(grp->gr_name);
      _groups[size].id = grp->gr_gid;
    }
  endgrent();
}

t_info	*mygetpwnam(char *login)
{
  int	i;

  if (_users && login)
    {
      for (i = 0; _users[i].name; i++)
        if (!strcmp(_users[i].name, login))
	  return (&_users[i]);
      mylog_printf(MYLOG_WARNING, "[%s][%s]Couldn't resolve user name %i",
                 gl_var->who->user, gl_var->who->ip, login);
    }
  return (0);
}


t_info	*mygetpwuid(uid_t uid)
{
  int	i;

  if (_users)
    {
      for (i = 0; _users[i].name; i++)
	if (_users[i].id == uid)
	  return (&_users[i]);
      mylog_printf(MYLOG_WARNING, "[%s][%s]Couldn't resolve user id %i",
		 gl_var->who->user, gl_var->who->ip, uid);
    }
  return (0);
}

t_info	*mygetgrnam(char *group)
{
  int	i;

  if (_groups && group)
    {
      for (i = 0; _groups[i].name; i++)
	if (!strcmp(_groups[i].name, group))
	  return (&_groups[i]);
      mylog_printf(MYLOG_WARNING, "[%s][%s]Couldn't resolve group name %i",
		 gl_var->who->user, gl_var->who->ip, group);
    }
  return (0);
}

t_info	*mygetgrgid(gid_t gid)
{
  int	i;

  if (_groups)
    {
      for (i = 0; _groups[i].name; i++)
	if (_groups[i].id == gid)
	  return (&_groups[i]);
      mylog_printf(MYLOG_WARNING, "[%s][%s]Couldn't resolve group id %i",
		 gl_var->who->user, gl_var->who->ip, gid);
    }
  return (0);
}
