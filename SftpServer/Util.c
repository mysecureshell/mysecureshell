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
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include "Sftp.h"
#include "Util.h"
#include "GetUsersInfos.h"
#include "../security.h"

static void	StrMode(mode_t mode, char *d)
{
  switch (mode & S_IFMT)
    {
    case S_IFDIR: *d++ = 'd'; break;
    case S_IFCHR: *d++ = 'c'; break;
    case S_IFBLK: *d++ = 'b'; break;
    case S_IFREG: *d++ = '-'; break;
#ifdef S_IFLNK
    case S_IFLNK: *d++ = 'l'; break;
#endif
#ifdef S_IFSOCK
    case S_IFSOCK: *d++ = 's'; break;
#endif
#ifdef S_IFIFO
    case S_IFIFO: *d++ = 'p'; break;
#endif
#ifdef S_IFWHT
    case S_IFWHT: *d++ = 'w'; break;
#endif
    default: *d++ = '?'; break;
    }

  if (HAS_BIT(mode, S_IRUSR)) *d++ = 'r';
  else *d++ = '-';
  if (HAS_BIT(mode, S_IWUSR)) *d++ = 'w';
  else *d++ = '-';
  switch (mode & (S_IXUSR | S_ISUID))
    {
    case 0: *d++ = '-'; break;
    case S_IXUSR: *d++ = 'x'; break;
    case S_ISUID: *d++ = 'S'; break;
    case S_IXUSR | S_ISUID: *d++ = 's'; break;
    }
  
  if (HAS_BIT(mode, S_IRGRP)) *d++ = 'r';
  else *d++ = '-';
  if (HAS_BIT(mode, S_IWGRP)) *d++ = 'w';
  else *d++ = '-';
  switch (mode & (S_IXGRP | S_ISGID))
    {
    case 0: *d++ = '-'; break;
    case S_IXGRP: *d++ = 'x'; break;
    case S_ISGID: *d++ = 'S'; break;
    case S_IXGRP | S_ISGID: *d++ = 's'; break;
    }
  
  if (HAS_BIT(mode, S_IROTH)) *d++ = 'r';
  else *d++ = '-';
  if (HAS_BIT(mode, S_IWOTH)) *d++ = 'w';
  else *d++ = '-';
  switch (mode & (S_IXOTH | S_ISVTX))
    {
    case 0: *d++ = '-'; break;
    case S_IXOTH: *d++ = 'x'; break;
    case S_ISVTX: *d++ = 'T'; break;
    case S_IXOTH | S_ISVTX: *d++ = 't'; break;
    }
  *d++ = ' '; /* will be a '+' if ACL's implemented */
  *d = '\0';
}

char		*LsFile(const char *name, const struct stat *st)
{
  int		ulen, glen, sz = 0;
  struct tm	*ltime = localtime(&st->st_mtime);
  t_info	*pw;
  t_info	*gr;
  char		*user, *group;
  char		buf[1024], mode[11+1], tbuf[12+1], ubuf[11+1], gbuf[11+1];
  
  StrMode(st->st_mode, mode);
  if ((pw = mygetpwuid(st->st_uid)) != NULL)
    user = pw->name;
  else
    {
      (void )snprintf(ubuf, sizeof(ubuf), "%u", (u_int32_t )st->st_uid);
      user = ubuf;
    }
  if ((gr = mygetgrgid(st->st_gid)) != NULL)
    group = gr->name;
  else
    {
      (void )snprintf(gbuf, sizeof(gbuf), "%u", (u_int32_t )st->st_gid);
      group = gbuf;
    }
  if (ltime != NULL)
    {
      if (time(0) - st->st_mtime < (365*24*60*60)/2)
	sz = strftime(tbuf, sizeof(tbuf), "%b %e %H:%M", ltime);
      else
	sz = strftime(tbuf, sizeof(tbuf), "%b %e  %Y", ltime);
    }
  if (sz == 0)
    tbuf[0] = '\0';
  ulen = MAX(strlen(user), 8);
  glen = MAX(strlen(group), 8);
  (void )snprintf(buf, sizeof(buf), "%s %3u %-*s %-*s %8llu %s %s", mode,
		  (u_int32_t)st->st_nlink, ulen, user, glen, group,
		  (unsigned long long int)st->st_size, tbuf, name);
  return (strdup(buf));
}

int	FlagsFromPortable(int pFlags, int *textMode)
{
  int	flags = 0;

  *textMode = 0;
  if (cVersion >= 5)
    {
      switch (pFlags & SSH5_FXF_ACCESS_DISPOSITION)
	{
	case SSH5_FXF_CREATE_NEW:
	  flags = O_EXCL | O_CREAT;
	  break;
	case SSH5_FXF_CREATE_TRUNCATE:
	  flags = O_TRUNC | O_CREAT;
	  break;
	case SSH5_FXF_OPEN_EXISTING:
	  flags = 0;
	  break;
	case SSH5_FXF_OPEN_OR_CREATE:
	  flags = O_CREAT;
	  break;
	case SSH5_FXF_TRUNCATE_EXISTING:
	  flags = O_TRUNC | O_EXCL | O_CREAT;
	  break;
	}
      if ((HAS_BIT(pFlags, SSH5_FXF_ACCESS_APPEND_DATA)) ||
	  HAS_BIT(pFlags, SSH5_FXF_ACCESS_APPEND_DATA_ATOMIC))
	flags = O_RDWR | O_APPEND;
      if (HAS_BIT(pFlags, SSH5_FXF_ACCESS_TEXT_MODE))
	*textMode = 1;
    }
  else
    {
      if (HAS_BIT(pFlags, SSH2_FXF_READ)
	  && HAS_BIT(pFlags, SSH2_FXF_WRITE))
	flags = O_RDWR;
      else if (HAS_BIT(pFlags, SSH2_FXF_READ))
	flags = O_RDONLY;
      else if (HAS_BIT(pFlags, SSH2_FXF_WRITE))
	flags = O_WRONLY;
      
      if (HAS_BIT(pFlags, SSH2_FXF_CREAT))
	flags |= O_CREAT;
      if (HAS_BIT(pFlags, SSH2_FXF_TRUNC))
	flags |= O_TRUNC;
      if (HAS_BIT(pFlags, SSH2_FXF_EXCL))
	flags |= O_EXCL;
      if (HAS_BIT(pFlags, SSH4_FXF_TEXT))
	*textMode = 1;
    }
  return (flags);
}

int	FlagsFromAccess(int access)
{
  int	flags = 0;

  if (HAS_BIT(access, SSH5_ACE4_READ_DATA))
    {
      if (HAS_BIT(access, SSH5_ACE4_WRITE_DATA))
	flags = O_RDWR;
      else
	flags = O_RDONLY;
    }
  else if (HAS_BIT(access, SSH5_ACE4_WRITE_DATA))
    flags = O_WRONLY;
  if (HAS_BIT(access, SSH5_ACE4_APPEND_DATA))
    flags |= O_APPEND;
  else if (HAS_BIT(access, SSH5_ACE4_WRITE_DATA))
    flags |= O_TRUNC;
  if (HAS_BIT(access, SSH5_ACE4_SYNCHRONIZE))
    flags |= O_SYNC;
  return (flags);
}

int	errnoToPortable(int unixErrno)
{
  int	ret = 0;

  switch (unixErrno)
    {
    case 0:
      ret = SSH2_FX_OK;
      break;
    case EROFS:
      ret = cVersion <= 3 ? SSH2_FX_FAILURE : SSH4_FX_WRITE_PROTECT;
      break;
    case EEXIST:
      ret = cVersion <= 3 ? SSH2_FX_FAILURE : SSH4_FX_FILE_ALREADY_EXISTS;
      break;
    case ENOTDIR:
      ret = cVersion <= 3 ? SSH2_FX_NO_SUCH_FILE : SSH4_FX_NO_SUCH_PATH;
      break;
    case EBADF:
      ret = cVersion <= 3 ? SSH2_FX_NO_SUCH_FILE : SSH4_FX_INVALID_HANDLE;
    case ENOENT:
    case ELOOP:
      ret = SSH2_FX_NO_SUCH_FILE;
      break;
    case EPERM:
    case EACCES:
    case EFAULT:
      ret = SSH2_FX_PERMISSION_DENIED;
      break;
    case ENAMETOOLONG:
    case EINVAL:
      ret = SSH2_FX_BAD_MESSAGE;
      break;
    default:
      ret = SSH2_FX_FAILURE;
      break;
    }
  return ret;
}

char	*ExecCommand(char *cmd, int *myRet)
{
  char	*args[2];

  args[0] = cmd;
  args[1] = 0;
  return (ExecCommandWithArgs(args, myRet, NULL, 1));
}

char	*ExecCommandWithArgs(char **args, int *myRet, const char *dataInput, int shouldReturnString)
{
  char	buffer[1024], *str = NULL;
  pid_t	pid;
  int	fdsI[2], fdsO[2], size = 0, ret;

  *myRet = -1;
  if (dataInput != NULL && pipe(fdsI) == -1)
    return (NULL);
  if (pipe(fdsO) == -1)
    {
      if (dataInput != NULL)
	{
	  xclose(fdsI[0]);
	  xclose(fdsI[1]);
	}
      return (NULL);
    }
  if ((pid = fork()) == 0)
    {
      if (dataInput != NULL)
	{
	  xdup2(fdsI[0], 0);
	  xclose(fdsI[0]);
	  xclose(fdsI[1]);
	}
      xdup2(fdsO[1], 1);
      xdup2(fdsO[1], 2);
      xclose(fdsO[0]);
      xclose(fdsO[1]);
      (void )execv(args[0], args);
      exit (1);
    }
  else if (pid == -1)
    {
      if (dataInput != NULL)
	{
	  xclose(fdsI[0]);
	  xclose(fdsI[1]);
	}
      xclose(fdsO[0]);
      xclose(fdsO[1]);
      return (NULL);
    }
  if (dataInput != NULL)
    {
      size_t	len, off, r;

      off = 0;
      len = strlen(dataInput);
      xclose(fdsI[0]);
      while ((r = write(fdsI[1], dataInput + off, len)) > 0)
	{
	  off += r;
	  len -= r;
	  if (len == 0)
	    break;
	}
      xclose(fdsI[1]);
    }
  xclose(fdsO[1]);
  str = malloc(1);
  str[0] = 0;
  while ((ret = read(fdsO[0], buffer, sizeof(buffer))) > 0)
    {
      if (shouldReturnString == 1)
	{
	  str = realloc(str, size + ret + 1);
	  strncat(str, buffer, ret);
	}
      size += ret;
    }
  xclose(fdsO[0]);
  (void )waitpid(pid, myRet, 0);
  if (shouldReturnString == 1)
    return (str);
  free(str);
  return (NULL);
}
