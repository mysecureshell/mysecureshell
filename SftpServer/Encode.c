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

#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Encode.h"
#include "GetUsersInfos.h"

tAttributes		*GetAttributes(tBuffer *bIn)
{
  static tAttributes	a;

  memset(&a, 0, sizeof(a));
  a.flags = BufferGetInt32(bIn);
  if (cVersion >= 4)
    a.type = BufferGetInt8(bIn);
  if (a.flags & SSH2_FILEXFER_ATTR_SIZE)
    a.size = BufferGetInt64(bIn);
  if (a.flags & SSH2_FILEXFER_ATTR_UIDGID)
    {
      a.uid = BufferGetInt32(bIn);
      a.gid = BufferGetInt32(bIn);
    }
  if (a.flags & SSH4_FILEXFER_ATTR_OWNERGROUP)
    {
      struct passwd	*pw;
      struct group	*gr;
      
      if ((pw = mygetpwnam(BufferGetString(bIn))))
	a.uid = pw->pw_uid;
      if ((gr = mygetgrnam(BufferGetString(bIn))))
	a.gid = gr->gr_gid;
    }
  if (a.flags & SSH2_FILEXFER_ATTR_PERMISSIONS)
    a.perm = BufferGetInt32(bIn);
  if (cVersion <= 3)
    {
      if (a.flags & SSH2_FILEXFER_ATTR_ACMODTIME)
	{
	  a.atime = BufferGetInt32(bIn);
	  a.mtime = BufferGetInt32(bIn);
	}
    }
  else //version >= 4
    {
      if (a.flags & SSH4_FILEXFER_ATTR_ACCESSTIME)
	a.atime = BufferGetInt64(bIn);
      if (a.flags & SSH4_FILEXFER_ATTR_SUBSECOND_TIMES)
	BufferGetInt32(bIn);
      if (a.flags & SSH4_FILEXFER_ATTR_CREATETIME)
	a.ctime = BufferGetInt64(bIn);
      if (a.flags & SSH4_FILEXFER_ATTR_SUBSECOND_TIMES)
	BufferGetInt32(bIn);
      if (a.flags & SSH4_FILEXFER_ATTR_MODIFYTIME)
	a.mtime = BufferGetInt64(bIn);
      if (a.flags & SSH4_FILEXFER_ATTR_SUBSECOND_TIMES)
	BufferGetInt32(bIn);
      if (a.flags & SSH4_FILEXFER_ATTR_ACL) //unsupported feature
	{
	  free(BufferGetString(bIn));
	}
    }
  if (a.flags & SSH2_FILEXFER_ATTR_EXTENDED) //unsupported feature
    {
      int	i, count;

      count = BufferGetInt32(bIn);
      for (i = 0; i < count; i++)
	{
	  free(BufferGetString(bIn));
	  free(BufferGetString(bIn));
	}
    }
  return (&a);
}

void	StatToAttributes(struct stat *st, tAttributes *a)
{
  memset(a, 0, sizeof(*a));
  a->flags = SSH2_FILEXFER_ATTR_SIZE;
  a->size = st->st_size;
  a->flags |= SSH2_FILEXFER_ATTR_UIDGID;
  a->uid = st->st_uid;
  a->gid = st->st_gid;
  a->flags |= SSH2_FILEXFER_ATTR_PERMISSIONS;
  a->perm = st->st_mode;
  a->flags |= SSH2_FILEXFER_ATTR_ACMODTIME;
  a->atime = st->st_atime;
  a->mtime = st->st_mtime;
  a->ctime = st->st_ctime;
  if (cVersion >= 4)
    {
      if ((st->st_mode & S_IFMT) == S_IFREG)
	a->type = SSH4_FILEXFER_TYPE_REGULAR;
      else if ((st->st_mode & S_IFMT) == S_IFDIR)
	a->type = SSH4_FILEXFER_TYPE_DIRECTORY;
      else if ((st->st_mode & S_IFMT) == S_IFLNK)
	a->type = SSH4_FILEXFER_TYPE_SYMLINK;
      else
	a->type = SSH4_FILEXFER_TYPE_SPECIAL;
      a->flags ^= SSH2_FILEXFER_ATTR_UIDGID;
      a->flags |= SSH4_FILEXFER_ATTR_OWNERGROUP | SSH4_FILEXFER_ATTR_ACCESSTIME
	| SSH4_FILEXFER_ATTR_CREATETIME | SSH4_FILEXFER_ATTR_MODIFYTIME;
    }
}

void	EncodeAttributes(tBuffer *b, tAttributes *a)
{
  BufferPutInt32(b, a->flags);
  if (cVersion >= 4)
    BufferPutInt8(b, a->type);
  if (a->flags & SSH2_FILEXFER_ATTR_SIZE)
    BufferPutInt64(b, a->size);
  if (cVersion <= 3 && a->flags & SSH2_FILEXFER_ATTR_UIDGID)
    {
      BufferPutInt32(b, a->uid);
      BufferPutInt32(b, a->gid);
    }
  if (a->flags & SSH4_FILEXFER_ATTR_OWNERGROUP)
    {
      struct passwd	*pw;
      struct group	*gr;
      char		buf[11+1];
      char		*str;
		
      if ((pw = getpwuid(a->uid)))
	str = pw->pw_name;
      else
	{
	  snprintf(buf, sizeof(buf), "%u", a->uid);
	  str = buf;
	}
      BufferPutString(b, str);
      if ((gr = getgrgid(a->gid)))
	str = gr->gr_name;
      else
	{
	  snprintf(buf, sizeof(buf), "%u", a->gid);
	  str = buf;
	}
      BufferPutString(b, str);
    }
  if (a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS)
    BufferPutInt32(b, a->perm);
  if (cVersion <= 3)
    {
      if (a->flags & SSH2_FILEXFER_ATTR_ACMODTIME)
	{
	  BufferPutInt32(b, a->atime);
	  BufferPutInt32(b, a->mtime);
	}
    }
  else //version >= 4
    {
      if (a->flags & SSH4_FILEXFER_ATTR_ACCESSTIME)
	BufferPutInt64(b, a->atime);
      if (a->flags & SSH4_FILEXFER_ATTR_SUBSECOND_TIMES)
	BufferPutInt32(b, 0);
      if (a->flags & SSH4_FILEXFER_ATTR_CREATETIME)
	BufferPutInt64(b, a->ctime);
      if (a->flags & SSH4_FILEXFER_ATTR_SUBSECOND_TIMES)
	BufferPutInt32(b, 0);	
      if (a->flags & SSH4_FILEXFER_ATTR_MODIFYTIME)
	BufferPutInt64(b, a->mtime);
      if (a->flags & SSH4_FILEXFER_ATTR_SUBSECOND_TIMES)
	BufferPutInt32(b, 0);
    }
  if (a->flags & SSH2_FILEXFER_ATTR_ACL)
    BufferPutString(b, ""); //unsupported feature
  if (a->flags & SSH2_FILEXFER_ATTR_EXTENDED)
    BufferPutInt32(b, 0); //unsupported feature
}

struct timeval		*AttributesToTimeval(const tAttributes *a)
{
  static struct timeval	tv[2];

  tv[0].tv_sec = a->atime;
  tv[0].tv_usec = 0;
  tv[1].tv_sec = a->mtime;
  tv[1].tv_usec = 0;
  return (tv);
}
