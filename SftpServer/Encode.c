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
#include <sys/ioctl.h>
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#ifdef HAVE_LINUX_EXT2_FS_H

#define EXT2_SECRM_FL                   0x00000001 /* Secure deletion */
#define EXT2_UNRM_FL                    0x00000002 /* Undelete */
#define EXT2_COMPR_FL                   0x00000004 /* Compress file */
#define EXT2_SYNC_FL                    0x00000008 /* Synchronous updates */
#define EXT2_IMMUTABLE_FL               0x00000010 /* Immutable file */
#define EXT2_APPEND_FL                  0x00000020 /* writes to file may only append */
#define EXT2_NODUMP_FL                  0x00000040 /* do not dump file */
#define EXT2_NOATIME_FL                 0x00000080 /* do not update atime */
/* Reserved for compression usage... */
#define EXT2_DIRTY_FL                   0x00000100
#define EXT2_COMPRBLK_FL                0x00000200 /* One or more compressed clusters */
#define EXT2_NOCOMP_FL                  0x00000400 /* Don't compress */
#define EXT2_ECOMPR_FL                  0x00000800 /* Compression error */
/* End compression flags --- maybe not all used */
#define EXT2_BTREE_FL                   0x00001000 /* btree format dir */
#define EXT2_INDEX_FL                   0x00001000 /* hash-indexed directory */
#define EXT2_IMAGIC_FL                  0x00002000 /* AFS directory */
#define EXT2_JOURNAL_DATA_FL            0x00004000 /* Reserved for ext3 */
#define EXT2_NOTAIL_FL                  0x00008000 /* file tail should not be merged */
#define EXT2_DIRSYNC_FL                 0x00010000 /* dirsync behaviour (directories only) */
#define EXT2_TOPDIR_FL                  0x00020000 /* Top of directory hierarchies*/
#define EXT2_RESERVED_FL                0x80000000 /* reserved for ext2 lib */

#define EXT2_FL_USER_VISIBLE            0x0003DFFF /* User visible flags */
#define EXT2_FL_USER_MODIFIABLE         0x000380FF /* User modifiable flags */

/*
 * ioctl commands
 */
#define EXT2_IOC_GETFLAGS               _IOR('f', 1, long)
#define EXT2_IOC_SETFLAGS               _IOW('f', 2, long)
#define EXT2_IOC_GETVERSION             _IOR('v', 1, long)
#define EXT2_IOC_SETVERSION             _IOW('v', 2, long)

#endif
#include <unistd.h>
#include "Encode.h"
#include "GetUsersInfos.h"
#include "Log.h"
#include "../security.h"

tAttributes		*GetAttributes(tBuffer *bIn)
{
  static tAttributes	a;

  memset(&a, 0, sizeof(a));
  a.flags = BufferGetInt32(bIn);
  //DEBUG((MYLOG_DEBUG, "FLAGS[%x][%i]", a.flags, a.flags));
  if (cVersion >= 4)
    a.type = BufferGetInt8(bIn);
  if (HAS_BIT(a.flags, SSH2_FILEXFER_ATTR_SIZE))
    a.size = BufferGetInt64(bIn);
  if (cVersion <= 3 && HAS_BIT(a.flags, SSH2_FILEXFER_ATTR_UIDGID))
    {
      a.uid = BufferGetInt32(bIn);
      a.gid = BufferGetInt32(bIn);
    }
  if (cVersion >= 4 && HAS_BIT(a.flags, SSH4_FILEXFER_ATTR_OWNERGROUP))
    {
      t_info	*pw;
      t_info	*gr;
      
      if ((pw = mygetpwnam(BufferGetString(bIn))))
	a.uid = pw->id;
      if ((gr = mygetgrnam(BufferGetString(bIn))))
	a.gid = gr->id;
    }
  if (HAS_BIT(a.flags, SSH2_FILEXFER_ATTR_PERMISSIONS))
    a.perm = BufferGetInt32(bIn);
  if (cVersion <= 3)
    {
      if (HAS_BIT(a.flags, SSH2_FILEXFER_ATTR_ACMODTIME))
	{
	  a.atime = BufferGetInt32(bIn);
	  a.mtime = BufferGetInt32(bIn);
	}
    }
  else //version >= 4
    {
      if (HAS_BIT(a.flags, SSH4_FILEXFER_ATTR_ACCESSTIME))
	a.atime = BufferGetInt64(bIn);
      if (HAS_BIT(a.flags, SSH4_FILEXFER_ATTR_SUBSECOND_TIMES))
	(void )BufferGetInt32(bIn);
      if (HAS_BIT(a.flags, SSH4_FILEXFER_ATTR_CREATETIME))
	a.ctime = BufferGetInt64(bIn);
      if (HAS_BIT(a.flags, SSH4_FILEXFER_ATTR_SUBSECOND_TIMES))
	(void )BufferGetInt32(bIn);
      if (HAS_BIT(a.flags, SSH4_FILEXFER_ATTR_MODIFYTIME))
	a.mtime = BufferGetInt64(bIn);
      if (HAS_BIT(a.flags, SSH4_FILEXFER_ATTR_SUBSECOND_TIMES))
	(void )BufferGetInt32(bIn);
    }
  if (HAS_BIT(a.flags, SSH2_FILEXFER_ATTR_ACL)) //unsupported feature
    {
      free(BufferGetString(bIn));
    }
  if (HAS_BIT(a.flags, SSH2_FILEXFER_ATTR_EXTENDED)) //unsupported feature
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

void	StatToAttributes(const struct stat *st, tAttributes *a, const char *fileName)
{
  memset(a, 0, sizeof(*a));
  a->flags = SSH2_FILEXFER_ATTR_SIZE;
  a->size = st->st_size;
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
      if (cVersion >= 5)
	{
	  if ((st->st_mode & S_IFMT) == S_IFSOCK)
	    a->type = SSH5_FILEXFER_TYPE_SOCKET;
	  else if ((st->st_mode & S_IFMT) == S_IFCHR)
	    a->type = SSH5_FILEXFER_TYPE_CHAR_DEVICE;
	  else if ((st->st_mode & S_IFMT) == S_IFBLK)
	    a->type = SSH5_FILEXFER_TYPE_BLOCK_DEVICE;
	  else if ((st->st_mode & S_IFMT) == S_IFIFO)
	    a->type = SSH5_FILEXFER_TYPE_FIFO;
	}
      a->flags |= SSH4_FILEXFER_ATTR_OWNERGROUP | SSH4_FILEXFER_ATTR_ACCESSTIME
	| SSH4_FILEXFER_ATTR_CREATETIME | SSH4_FILEXFER_ATTR_MODIFYTIME;
    }
  else
    a->flags |= SSH2_FILEXFER_ATTR_UIDGID;
  if (cVersion >= 5 && fileName != NULL)
    {
      int	pos = strlen(fileName) - 1;
#ifdef HAVE_LINUX_EXT2_FS_H
      int	fd;
#endif

      a->attrib = 0;
      a->flags |= SSH5_FILEXFER_ATTR_BITS;
      while (pos >= 1 && fileName[pos - 1] != '/')
	pos--;
      if (pos >= 0 && fileName[pos] == '.')
	a->attrib |= SSH5_FILEXFER_ATTR_FLAGS_HIDDEN;
#ifdef HAVE_LINUX_EXT2_FS_H
      if ((fd = open(fileName, O_RDONLY)) >= 0)
	{
	  int	flags;

	  if (ioctl(fd, EXT2_IOC_GETFLAGS, &flags) != -1)
	    {
	      if (flags & EXT2_COMPR_FL)
		a->attrib |= SSH5_FILEXFER_ATTR_FLAGS_COMPRESSED;
	      if (flags & EXT2_APPEND_FL)
		a->attrib |= SSH5_FILEXFER_ATTR_FLAGS_APPEND_ONLY;
	      if (flags & EXT2_IMMUTABLE_FL)
		a->attrib |= SSH5_FILEXFER_ATTR_FLAGS_IMMUTABLE;
	      if (flags & EXT2_SYNC_FL)
		a->attrib |= SSH5_FILEXFER_ATTR_FLAGS_SYNC;
	    }
	  xclose(fd);
	}
#endif
    }
}

#if(HAVE_LIBACL)

#ifndef HAVE_CYGWIN
#include <acl/libacl.h>
#endif
#include <sys/acl.h>

#ifndef HAVE_CYGWIN
static void	EncodeACL(tBuffer *b, const char *file)
{
  tBuffer	*bAcl = BufferNew();
  acl_entry_t	entry;
  u_int32_t	posNew, nb = 0;
  acl_t		acl;

  BufferPutInt32(bAcl, 0);//Number of ACL
  if ((acl = acl_get_file(file, ACL_TYPE_ACCESS)) != NULL)
    {
      if (acl_get_entry(acl, ACL_FIRST_ENTRY, &entry) == 1)
	{
	  acl_permset_t	permset;
	  acl_tag_t	tag;
	  int		*data;
	  
	  do
	    {
	      if (acl_get_tag_type(entry, &tag) == 0 && acl_get_permset(entry, &permset) == 0)
		{
		  if (tag == ACL_MASK)
		    continue;
		  nb++;
		  BufferPutInt32(bAcl, SSH5_ACE4_ACCESS_ALLOWED_ACE_TYPE);
		  BufferPutInt32(bAcl, 0);//ace-flag ???
		  BufferPutInt32(bAcl,
				 (acl_get_perm(permset, ACL_READ) == 1 ? SSH5_ACE4_READ_DATA : 0) |
				 (acl_get_perm(permset, ACL_WRITE) == 1 ? SSH5_ACE4_WRITE_DATA : 0) |
				 (acl_get_perm(permset, ACL_EXECUTE) == 1 ? SSH5_ACE4_EXECUTE : 0));
		  switch (tag)
		    {
		    case ACL_USER_OBJ: BufferPutString(bAcl, "USER"); break;
		    case ACL_GROUP_OBJ: BufferPutString(bAcl, "GROUP"); break;
		    case ACL_OTHER: BufferPutString(bAcl, "OTHER"); break;
		    case ACL_USER:
		      data = (int *)acl_get_qualifier(entry);
		      if (data)
			{
			  t_info	*pw;
			  char		buf[11+1];
			  char		*str;
			  
			  if ((pw = mygetpwuid(*data)))
			    str = pw->name;
			  else
			    {
			      (void )snprintf(buf, sizeof(buf), "%i", *data);
			      str = buf;
			    }
			  BufferPutString(bAcl, str);
			}
		      break;
		    case ACL_GROUP:
		      data = (int *)acl_get_qualifier(entry);
		      if (data)
			{
			  t_info	*gr;
			  char		buf[11+1];
			  char		*str;
			  
			  if ((gr = mygetgrgid(*data)))
			    str = gr->name;
			  else
			    {
			      (void )snprintf(buf, sizeof(buf), "%i", *data);
			      str = buf;
			    }
			  BufferPutString(bAcl, str);
			}
		      break;
		    }
		}
	    }
	  while (acl_get_entry(acl, ACL_NEXT_ENTRY, &entry) == 1);
	}
      (void )acl_free(acl);
    }
  posNew = bAcl->length;
  bAcl->length = 0;
  BufferPutInt32(bAcl, nb);//Number of ACLs
  bAcl->length = posNew;
  BufferPutPacket(b, bAcl);
  BufferDelete(bAcl);
}

#else //ifdef HAVE_CYGWIN

static void     EncodeACL(tBuffer *b, const char *file)
{
  tBuffer	*bAcl = BufferNew();
  aclent_t	acls[MAX_ACL_ENTRIES];
  int		nbAcls;

  nbAcls = acl(file, GETACL, MAX_ACL_ENTRIES, acls);
  if (nbAcls < 0)
    nbAcls = 0;
  BufferPutInt32(bAcl, nbAcls);
  if (nbAcls > 0)
    {
      int	i;

      for (i = 0; i < nbAcls; i++)
	{
	  switch (acls[i].a_type)
	    {
	    case USER_OBJ:
	    case USER:
	    case GROUP_OBJ:
	    case GROUP:
	    case OTHER:
	      break;
	    default:
	      continue;
	    }
	  BufferPutInt32(bAcl, SSH5_ACE4_ACCESS_ALLOWED_ACE_TYPE);
	  BufferPutInt32(bAcl, 0);//ace-flag ???
	  BufferPutInt32(bAcl,
			 ((acls[i].a_perm & 2) ? SSH5_ACE4_READ_DATA : 0) |
			 ((acls[i].a_perm & 4) ? SSH5_ACE4_WRITE_DATA : 0) |
			 ((acls[i].a_perm & 1) ? SSH5_ACE4_EXECUTE : 0));
	  switch (acls[i].a_type)
	    {
	    case ACL_USER_OBJ: BufferPutString(bAcl, "USER"); break;
	    case ACL_GROUP_OBJ: BufferPutString(bAcl, "GROUP"); break;
	    case ACL_OTHER: BufferPutString(bAcl, "OTHER"); break;
	    case ACL_USER:
	      {
		t_info	*pw;
		char	buf[11+1];
		char	*str;

		if ((pw = mygetpwuid(acls[i].a_id)))
		  str = pw->name;
		else
		  {
		    (void )snprintf(buf, sizeof(buf), "%u", *data);
		    str = buf;
		  }
		BufferPutString(bAcl, str);
	      }
	    case ACL_GROUP:
	      {
		t_info	*gr;
		char		buf[11+1];
		char		*str;
		
		if ((gr = mygetgrgid(*data)))
		  str = gr->name;
		else
		  {
		    (void )snprintf(buf, sizeof(buf), "%u", *data);
		    str = buf;
		  }
		BufferPutString(bAcl, str);
		break;
	      }
	    }
	}
    }
  BufferPutPacket(b, bAcl);
  BufferDelete(bAcl);
}

#endif //HAVE_CYGWIN

#endif //HAVE_LIBACL

void	EncodeAttributes(tBuffer *b, const tAttributes *a, const char *file)
{
  BufferPutInt32(b, a->flags);
  if (cVersion >= 4)
    BufferPutInt8(b, a->type);
  if (HAS_BIT(a->flags, SSH2_FILEXFER_ATTR_SIZE))
    BufferPutInt64(b, a->size);
  if (cVersion <= 3 && HAS_BIT(a->flags, SSH2_FILEXFER_ATTR_UIDGID))
    {
      BufferPutInt32(b, a->uid);
      BufferPutInt32(b, a->gid);
    }
  if (HAS_BIT(a->flags, SSH4_FILEXFER_ATTR_OWNERGROUP))
    {
      t_info	*pw;
      t_info	*gr;
      char	buf[11+1];
      char	*str;
	
      if ((pw = mygetpwuid(a->uid)))
	str = pw->name;
      else
	{
	  (void )snprintf(buf, sizeof(buf), "%u", a->uid);
	  str = buf;
	}
      BufferPutString(b, str);
      if ((gr = mygetgrgid(a->gid)))
	str = gr->name;
      else
	{
	  (void )snprintf(buf, sizeof(buf), "%u", a->gid);
	  str = buf;
	}
      BufferPutString(b, str);
    }
  if (HAS_BIT(a->flags, SSH2_FILEXFER_ATTR_PERMISSIONS))
    BufferPutInt32(b, a->perm);
  if (cVersion <= 3)
    {
      if (HAS_BIT(a->flags, SSH2_FILEXFER_ATTR_ACMODTIME))
	{
	  BufferPutInt32(b, a->atime);
	  BufferPutInt32(b, a->mtime);
	}
    }
  else //cVersion >= 4
    {
      if (HAS_BIT(a->flags, SSH4_FILEXFER_ATTR_ACCESSTIME))
	BufferPutInt64(b, a->atime);
      if (HAS_BIT(a->flags, SSH4_FILEXFER_ATTR_SUBSECOND_TIMES))
	BufferPutInt32(b, 0);
      if (HAS_BIT(a->flags, SSH4_FILEXFER_ATTR_CREATETIME))
	BufferPutInt64(b, a->ctime);
      if (HAS_BIT(a->flags, SSH4_FILEXFER_ATTR_SUBSECOND_TIMES))
	BufferPutInt32(b, 0);	
      if (HAS_BIT(a->flags, SSH4_FILEXFER_ATTR_MODIFYTIME))
	BufferPutInt64(b, a->mtime);
      if (HAS_BIT(a->flags, SSH4_FILEXFER_ATTR_SUBSECOND_TIMES))
	BufferPutInt32(b, 0);
    }
  if (HAS_BIT(a->flags, SSH2_FILEXFER_ATTR_ACL))
    {
#if(HAVE_LIBACL)
      if (file != NULL)
#endif
	BufferPutString(b, ""); //unsupported feature
#if(HAVE_LIBACL)
      else
	EncodeACL(b, file);
#endif
    }
  if (HAS_BIT(a->flags, SSH5_FILEXFER_ATTR_BITS))
    BufferPutInt32(b, a->attrib);
  if (HAS_BIT(a->flags, SSH2_FILEXFER_ATTR_EXTENDED))
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
