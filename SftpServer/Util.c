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

#include "../defines.h"
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "Sftp.h"
#include "Util.h"

void	StrMode(int mode, char *d)
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

  if (mode & S_IRUSR) *d++ = 'r';
  else *d++ = '-';
  if (mode & S_IWUSR) *d++ = 'w';
  else *d++ = '-';
  switch (mode & (S_IXUSR | S_ISUID))
    {
    case 0: *d++ = '-'; break;
    case S_IXUSR: *d++ = 'x'; break;
    case S_ISUID: *d++ = 'S'; break;
    case S_IXUSR | S_ISUID: *d++ = 's'; break;
    }
  
  if (mode & S_IRGRP) *d++ = 'r';
  else *d++ = '-';
  if (mode & S_IWGRP) *d++ = 'w';
  else *d++ = '-';
  switch (mode & (S_IXGRP | S_ISGID))
    {
    case 0: *d++ = '-'; break;
    case S_IXGRP: *d++ = 'x'; break;
    case S_ISGID: *d++ = 'S'; break;
    case S_IXGRP | S_ISGID: *d++ = 's'; break;
    }
  
  if (mode & S_IROTH) *d++ = 'r';
  else *d++ = '-';
  if (mode & S_IWOTH) *d++ = 'w';
  else *d++ = '-';
  switch (mode & (S_IXOTH | S_ISVTX))
    {
    case 0: *d++ = '-'; break;
    case S_IXOTH: *d++ = 'x'; break;
    case S_ISVTX: *d++ = 'T'; break;
    case S_IXOTH | S_ISVTX: *d++ = 't'; break;
    }
  *d++ = ' '; /* will be a '+' if ACL's implemented */
  *d = 0;
}

char		*LsFile(const char *name, const struct stat *st)
{
  int		ulen, glen, sz = 0;
  struct passwd	*pw;
  struct group	*gr;
  struct tm	*ltime = localtime(&st->st_mtime);
  char		*user, *group;
  char		buf[1024], mode[11+1], tbuf[12+1], ubuf[11+1], gbuf[11+1];
  
  StrMode(st->st_mode, mode);
  if ((pw = getpwuid(st->st_uid)))
    user = pw->pw_name;
  else
    {
      snprintf(ubuf, sizeof(ubuf), "%u", (u_int)st->st_uid);
      user = ubuf;
    }
  if ((gr = getgrgid(st->st_gid)))
    group = gr->gr_name;
  else
    {
      snprintf(gbuf, sizeof(gbuf), "%u", (u_int)st->st_gid);
      group = gbuf;
    }
  if (ltime)
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
  snprintf(buf, sizeof(buf), "%s %3u %-*s %-*s %8llu %s %s", mode,
	   (u_int)st->st_nlink, ulen, user, glen, group,
	   (unsigned long long int)st->st_size, tbuf, name);
  return strdup(buf);
}

int	FlagsFromPortable(int pFlags)
{
  int	flags = 0;

  if ((pFlags & SSH2_FXF_READ) && (pFlags & SSH2_FXF_WRITE))
    flags = O_RDWR;
  else if (pFlags & SSH2_FXF_READ)
    flags = O_RDONLY;
  else if (pFlags & SSH2_FXF_WRITE)
    flags = O_WRONLY;
  
  if (pFlags & SSH2_FXF_CREAT)
    flags |= O_CREAT;
  if (pFlags & SSH2_FXF_TRUNC)
    flags |= O_TRUNC;
  if (pFlags & SSH2_FXF_EXCL)
    flags |= O_EXCL;
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
