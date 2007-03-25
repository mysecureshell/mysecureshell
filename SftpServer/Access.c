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

#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>

static gid_t	*_in_group = 0;

#ifndef HAVE_GETGROUPLIST
static int getgrouplist(const char *uname, gid_t agroup, register gid_t *groups, int *grpcnt)
{
  register struct group *grp;
  register int i, ngroups;
  int ret, maxgroups;
  int bail;

  ret = 0;
  ngroups = 0;
  maxgroups = *grpcnt;
  
  /*
   * install primary group
   */
  if (ngroups >= maxgroups) {
    *grpcnt = ngroups;
    return (-1);
  }
  groups[ngroups++] = agroup;
  
  /*
   * Scan the group file to find additional groups.
   */
  setgrent();
  while ((grp = getgrent())) {
    if (grp->gr_gid == agroup)
      continue;
    for (bail = 0, i = 0; bail == 0 && i < ngroups; i++)
      if (groups[i] == grp->gr_gid)
	bail = 1;
    if (bail)
      continue;
    for (i = 0; grp->gr_mem[i]; i++) {
      if (!strcmp(grp->gr_mem[i], uname)) {
	if (ngroups >= maxgroups) {
	  ret = -1;
	  goto out;
	}
	groups[ngroups++] = grp->gr_gid;
	break;
      }
    }
  }
 out:
  endgrent();
  *grpcnt = ngroups;
  return (ret);
}
#endif

static void	InitAccess()
{
  struct passwd	*info;
  struct group	*group;
  gid_t		*groups;
  int		nb_groups = 42;
  int		i;

  if ((info = getpwuid(getuid())))
    {
      groups = malloc(nb_groups * sizeof(*groups));
      if (getgrouplist(info->pw_name, info->pw_gid, groups, &nb_groups) == -1)
	{
	  groups = realloc(groups, nb_groups * sizeof(*groups));
	  getgrouplist(info->pw_name, info->pw_gid, groups, &nb_groups);
	}
      if (nb_groups > 0)
        {
          _in_group = malloc((nb_groups + 1) * sizeof(*_in_group));
          for (i = 0; i < nb_groups; i++)
	    if ((group = getgrgid(groups[i])) != NULL)
	      _in_group[i] = group->gr_gid;
          _in_group[i] = 0;
        }
      free(groups);
    }
}

static int	UserIsInThisGroup(gid_t grp)
{
  int		i;

  if (_in_group)
    for (i = 0; _in_group[i] != 0; i++)
      if (_in_group[i] == grp)
	return (1);
  return (0);
}
