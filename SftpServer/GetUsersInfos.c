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

#include "../config.h"

typedef struct	s_info
{
  int		id;
  char		*name;
}		t_info;

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

struct passwd		*mygetpwnam(char *login)
{
  static struct passwd	pwd;
  int			i;

  if (_users && login)
    {
      for (i = 0; _users[i].name; i++)
        if (!strcmp(_users[i].name, login))
          {
            memset(&pwd, 0, sizeof(pwd));
            pwd.pw_name = login;
            pwd.pw_uid = _users[i].id;
            return (&pwd);
          }
      mylog_printf(MYLOG_WARNING, "[%s][%s]Couldn't resolve user name %i",
                 gl_var->who->user, gl_var->who->ip, login);
    }
  return (0);
}


struct passwd		*mygetpwuid(uid_t uid)
{
  static struct passwd	pwd;
  int			i;

  if (_users)
    {
      for (i = 0; _users[i].name; i++)
	if (_users[i].id == uid)
	  {
	    memset(&pwd, 0, sizeof(pwd));
	    pwd.pw_name = _users[i].name;
	    pwd.pw_uid = uid;
	    return (&pwd);
	  }
      mylog_printf(MYLOG_WARNING, "[%s][%s]Couldn't resolve user id %i",
		 gl_var->who->user, gl_var->who->ip, uid);
    }
  return (0);
}

struct group		*mygetgrnam(char *group)
{
  static struct group	grp;
  int			i;

  if (_groups && group)
    {
      for (i = 0; _groups[i].name; i++)
	if (!strcmp(_groups[i].name, group))
	  {
	    memset(&grp, 0, sizeof(grp));
	    grp.gr_name = group;
	    grp.gr_gid = _groups[i].id;
	    return (&grp);
	}
      mylog_printf(MYLOG_WARNING, "[%s][%s]Couldn't resolve group name %i",
		 gl_var->who->user, gl_var->who->ip, group);
    }
  return (0);
}

struct group		*mygetgrgid(gid_t gid)
{
  static struct group	grp;
  int			i;

  if (_groups)
    {
      for (i = 0; _groups[i].name; i++)
	if (_groups[i].id == gid)
	  {
	    memset(&grp, 0, sizeof(grp));
	    grp.gr_name = _groups[i].name;
	    grp.gr_gid = gid;
	    return (&grp);
	}
      mylog_printf(MYLOG_WARNING, "[%s][%s]Couldn't resolve group id %i",
		 gl_var->who->user, gl_var->who->ip, gid);
    }
  return (0);
}
