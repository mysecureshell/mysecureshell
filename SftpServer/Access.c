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
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static gid_t	*_in_group = 0;

void	InitAccess()
{
  gid_t	*groups;
  int	nb_groups = 8, ret;

  groups = malloc(nb_groups * sizeof(*groups));
  if ((ret = getgroups(nb_groups, groups)) == -1)
    {
      if (errno == EINVAL)
	nb_groups = 63;
      else
	nb_groups = 0;
    }
  else
    nb_groups = ret;
  _in_group = malloc((nb_groups + 2) * sizeof(*_in_group));
  if (nb_groups > 0)
    if (getgroups(nb_groups, _in_group) == -1)
      nb_groups = 0;
  _in_group[nb_groups] = getgid();
  _in_group[nb_groups + 1] = -1;
  free(groups);
}

int	UserIsInThisGroup(gid_t grp)
{
  int	i;

  if (_in_group != NULL)
    for (i = 0; _in_group[i] != -1; i++)
      if (_in_group[i] == grp)
	return (1);
  return (0);
}
