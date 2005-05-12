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

#include "defines.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "hash.h"
#include "ip.h"
#include "prog.h"

#ifdef HAVE_NOTHING
#include "SftpServer/SftpWho.c"

int		count_program_for_uid(int uid, char *login)
{
  t_sftpwho	*who;
  int		i, nb;

  nb = 0;
  if ((who = SftpWhoGetStruct(-1)))
    {
      for (i = 0; i < SFTPWHO_MAXCLIENT; i++)
	if ((who[i].status & SFTPWHO_STATUS_MASK) != SFTPWHO_EMPTY)
	  if (!login || !strcmp(who[i].user, login))
	    nb++;
      SftpWhoRelaseStruct();
    }
  return (nb);
}

int		count_program_for_ip(int uid, char *host)
{
  t_sftpwho	*who;
  int		i, nb;

  nb = 0;
  if ((who = SftpWhoGetStruct(-1)))
    {
      for (i = 0; i < SFTPWHO_MAXCLIENT; i++)
	if ((who[i].status & SFTPWHO_STATUS_MASK) != SFTPWHO_EMPTY)
	  if (!host || !strcmp(who[i].ip, host))
	    nb++;
      SftpWhoRelaseStruct();
    }
  return (nb);
}

#elif(HAVE_LIBKVM)

#include <fcntl.h>
#include <kvm.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>

int			count_program_for_uid(int uid, char *login)
{
  struct kinfo_proc	*ki;
  kvm_t			*kd;
  char			**tb;
  int			i, ret, size;

  ret = 0;
#ifdef KVM_NO_FILES
  if ((kd = kvm_open(0, 0, 0, KVM_NO_FILES, 0)))
#else
  if ((kd = kvm_open("/dev/null", "/dev/null", 0, O_RDONLY, 0)))
#endif
    {
      if (uid == -1)
	ki = kvm_getprocs(kd, KERN_PROC_ALL, 0, &size);
      else
	ki = kvm_getprocs(kd, KERN_PROC_UID, uid, &size);
      for (i = 0; i < size; i++)
	if ((tb = kvm_getargv(kd, &ki[i], sizeof(*ki))))
	  if (strstr(tb[0], MSS_EXECFILENAME))
	    ret++;
      kvm_close(kd);
    }
  return (ret);
}

int		count_program_for_ip(int uid, char *host)
{
  struct kinfo_proc	*ki;
  kvm_t			*kd;
  char			**tb;
  char			*ip, *ptr;
  int			i, j, ret, size;

  ret = 0;
  ip = get_ip((int )hash_get("ResolveIP"));
  if ((ptr = strchr(ip, ' ')))
    *ptr = 0;
#ifdef KVM_NO_FILES
  if ((kd = kvm_open(0, 0, 0, KVM_NO_FILES, 0)))
#else
  if ((kd = kvm_open("/dev/null", "/dev/null", 0, O_RDONLY, 0)))
#endif
    {
      if (uid == -1)
	ki = kvm_getprocs(kd, KERN_PROC_ALL, 0, &size);
      else
	ki = kvm_getprocs(kd, KERN_PROC_UID, uid, &size);
      for (i = 0; i < size; i++)
	if ((tb = kvm_getargv(kd, &ki[i], sizeof(*ki))))
	  if (strstr(tb[0], MSS_EXECFILENAME))
	    for (j = 0; tb[j]; j++)
	      if (!strcmp(tb[j], "--ip") && tb[j + 1])
		if (!strcmp(tb[j + 1], ip))
		  ret++;
      kvm_close(kd);
    }
  return (ret);
}

#else

int		count_program_for_uid(int uid, char *login)
{
  struct dirent	*item;
  FILE		*fh;
  char		buffer[256];
  char		buid[12];
  DIR		*dir;
  int		ret = 0;

  snprintf(buid, sizeof(buid), "%i", uid);
  if ((dir = opendir("/proc")))
    {
      while ((item = readdir(dir)))
	if (is_number(item->d_name))
	  {
	    int	step = 0;
	    snprintf(buffer, sizeof(buffer), "/proc/%s/status", item->d_name);
	    if ((fh = fopen(buffer, "r")))
	      {
		while (fgets(buffer, sizeof(buffer), fh))
		  {
		    if (strstr(buffer, "Name:") && strstr(buffer, MSS_EXECFILENAME))
		      step |= 1;
		    else if (strstr(buffer, "Uid:") && (strstr(buffer, buid) || uid == -1))
		      step |= 2;
		  }
		if (step == 3)
		  ret++;
		fclose(fh);
	      }
	  }
      closedir(dir);
    }
  return (ret);
}

int		count_program_for_ip(int uid, char *host)
{
  struct dirent	*item;
  FILE		*fh;
  char		buffer[4096], buid[12];
  char		*ip, *ptr;
  DIR		*dir;
  int		ret = 0, i;

  snprintf(buid, sizeof(buid), "%i", uid);
  ip = get_ip((int )hash_get("ResolveIP"));
  if ((ptr = strchr(ip, ' ')))
    *ptr = 0;
  if ((dir = opendir("/proc")))
    {
      while ((item = readdir(dir)))
	if (is_number(item->d_name))
	  {
	    int step = 0;

	    snprintf(buffer, sizeof(buffer), "/proc/%s/cmdline", item->d_name);
            if ((fh = fopen(buffer, "r")))
              {
                while (fgets(buffer, sizeof(buffer), fh))
                  {
                    if (strstr(buffer, MSS_EXECFILENAME))
		      {
			step = 1;
			break;
		      }
                  }
                if (step == 1)
		  {
		    ptr = buffer;
		    for (i = 0; i < (sizeof(buffer) - 1); i++)
		      if (!buffer[i])
			{
			  if (!buffer[i + 1])
			    break;
			  ptr = buffer + i + 1;
			  if (!strcmp(ptr, "--ip"))
			    {
			      ptr += 5;
			      if (!strcmp(ptr, ip))
				ret++;
			    }
			}
		  }
		fclose(fh);
	      }
	  }
      closedir(dir);
    }
  free(ip); 
  return (ret);
}

int	is_number(char *str)
{
  while (*str)
    {
      if (*str < '0' || *str > '9')
	return (0);
      str++;
    }
  return (1);
}


#endif
