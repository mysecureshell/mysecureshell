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

#include "config.h"
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ip.h"
#include "parsing.h"
#include "user.h"

static char	*user_name = NULL;
static char	**user_group = NULL;
static int	restrictions = REST_ALL;

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
      if (strcmp(grp->gr_mem[i], uname) == 0) {
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

int		init_user_info()
{
  struct passwd	*info;
  struct group	*group;
  gid_t		groups[42];
  int		nb_groups = sizeof(groups) / sizeof(*groups);
  int		i;

  if ((info = getpwuid(getuid())))
    {
      if ((user_name = strdup(info->pw_name)) == NULL)
	return (0);
      getgrouplist(user_name, info->pw_gid, groups, &nb_groups);
      if (nb_groups > 0)
        {
          user_group = malloc((nb_groups + 1) * sizeof(*user_group));
          for (i = 0; i < nb_groups; i++)
	    if ((group = getgrgid(groups[i])))
	      user_group[i] = strdup(group->gr_name);
          user_group[i] = NULL;
        }
      hash_set("User", (void *)strdup(info->pw_name));
      hash_set("Home", (void *)strdup(info->pw_dir));
      return (1);
    }
  return (0);
}

void	free_user_info()
{
  int	i;

  free(user_name);
  if (user_group)
    {
      for (i = 0; user_group[i]; i++)
	free(user_group[i]);
      free(user_group);
    }
}

int	is_for_user(const char *user, int verbose)
{
  if (user == NULL)
    return (0);
  if (strcmp(user, TAG_ALL) == 0 && restrictions != REST_USER)
    {
      if (verbose >= 2) (void )printf("--- Apply restrictions for all users ---\n");
      return (1);
    }
  if (strcmp(user, user_name) == 0)
    {
      if (verbose >= 2) (void )printf("--- Apply restrictions for user '%s' ---\n", user);
      restrictions = REST_USER;
      return (1);
    }
  return (0);
}

int	is_for_group(const char *group, int verbose)
{
  int	i;

  if (group == NULL || restrictions == REST_USER)
    return (0);
  if (strcmp(group, TAG_ALL) == 0 && restrictions == REST_ALL)
    {
      if (verbose >= 2) (void )printf("--- Apply restrictions for all groups ---\n");
      return (1);
    }
  if (user_group != NULL && restrictions <= REST_GROUP)
    for (i = 0; user_group[i]; i++)
      if (strcmp(group, user_group[i]) == 0)
	{
	  restrictions = REST_GROUP;
	  if (verbose >= 2) (void )printf("--- Apply restrictions for group '%s' ---\n", group);
	  return (1);
	}
  return (0);
}

int	is_for_virtualhost(const char *host, int port, int verbose)
{
  char	*current_host;
  int	current_port;
  
  current_host = (char *)hash_get("VIRTUALHOST_IP");
  current_port = hash_get_int("VIRTUALHOST_PORT");	
  if (current_host != NULL && (strcmp(host, current_host) == 0
			       || strcmp(current_host, TAG_ALL) == 0))
    if (current_port == 0 || port == current_port)
      {
	if (verbose >= 2)
	  (void )printf("--- Apply restriction for virtualhost '%s:%i' ---\n", current_host, current_port);
	return (1);
      }
  return (0);
}

int	is_for_rangeip(const char *range, int verbose)
{
  char	*bip, *ip;
  int	pos, size;

  if (range == NULL || restrictions == REST_USER)
    return (0);
  size = (int )((unsigned char )range[8]);
  ip = get_ip(0); //don't resolv dns
  bip = parse_range_ip(ip);
  pos = 0;
  while (size >= 8)
    {
      if (range[pos] <= bip[pos] && bip[pos] <= range[pos + 4])
	{
	  pos++;
	  size -= 8;
	}
      else
	goto error_is_for_rangeip;
    }
  if (size > 0)
    {
      bip[pos] = (unsigned char )bip[pos] >> (8 - size);
      bip[pos] = (unsigned char )bip[pos] << (8 - size);
      if (range[pos] < bip[pos] || bip[pos] > range[pos + 4])
	goto error_is_for_rangeip;
    }
  if (verbose >= 2)
    (void )printf("--- Apply restrictions for ip range '%ui.%ui.%ui.%ui-%ui.%ui.%ui.%ui/%ui' ---\n",
		  (unsigned int )range[0], (unsigned int )range[1], (unsigned int )range[2],
		  (unsigned int )range[3], (unsigned int )range[4], (unsigned int )range[5],
		  (unsigned int )range[6], (unsigned int )range[7], (unsigned int )range[8]);
  free(bip);
  free(ip);
  return (1);

 error_is_for_rangeip:
  free(bip);
  free(ip);
  return (0);
}
