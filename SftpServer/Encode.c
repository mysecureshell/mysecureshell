/*
 MySecureShell permit to add restriction to modified sftp-server
 when using MySecureShell as shell.
 Copyright (C) 2007-2018 MySecureShell Team

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
#include "FileSystem.h"
#include "GetUsersInfos.h"
#include "Log.h"
#include "../Core/security.h"

tAttributes *GetAttributes(tBuffer *bIn)
{
	static tAttributes a;

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
		t_info *pw;
		t_info *gr;
		char *user, *group;

		user = BufferGetString(bIn);
		group = BufferGetString(bIn);
		if ((pw = mygetpwnam(user)) != NULL)
			a.uid = pw->id;
		if ((gr = mygetgrnam(group)) != NULL)
			a.gid = gr->id;
		free(user);
		free(group);
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
			(void) BufferGetInt32(bIn);
		if (HAS_BIT(a.flags, SSH4_FILEXFER_ATTR_CREATETIME))
			a.ctime = BufferGetInt64(bIn);
		if (HAS_BIT(a.flags, SSH4_FILEXFER_ATTR_SUBSECOND_TIMES))
			(void) BufferGetInt32(bIn);
		if (HAS_BIT(a.flags, SSH4_FILEXFER_ATTR_MODIFYTIME))
			a.mtime = BufferGetInt64(bIn);
		if (HAS_BIT(a.flags, SSH4_FILEXFER_ATTR_SUBSECOND_TIMES))
			(void) BufferGetInt32(bIn);
	}
	if (HAS_BIT(a.flags, SSH2_FILEXFER_ATTR_ACL)) //unsupported feature
	{
		free(BufferGetString(bIn));
	}
	if (HAS_BIT(a.flags, SSH2_FILEXFER_ATTR_EXTENDED)) //unsupported feature
	{
		u_int32_t i, count;

		count = BufferGetInt32(bIn);
		for (i = 0; i < count; i++)
		{
			free(BufferGetString(bIn));
			free(BufferGetString(bIn));
		}
	}
	return (&a);
}

void StatToAttributes(const struct stat *st, tAttributes *a, const char *fileName)
{
	memset(a, 0, sizeof(*a));
	a->flags = SSH2_FILEXFER_ATTR_SIZE;
	a->size = (u_int64_t) st->st_size;
	a->uid = (u_int32_t) st->st_uid;
	a->gid = (u_int32_t) st->st_gid;
	a->flags |= SSH2_FILEXFER_ATTR_PERMISSIONS;
	a->perm = (u_int32_t) st->st_mode;
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
		a->flags |= SSH4_FILEXFER_ATTR_OWNERGROUP
				| SSH4_FILEXFER_ATTR_ACCESSTIME | SSH4_FILEXFER_ATTR_CREATETIME
				| SSH4_FILEXFER_ATTR_MODIFYTIME;
	}
	else
		a->flags |= SSH2_FILEXFER_ATTR_UIDGID;
	if (cVersion >= 5 && fileName != NULL)
	{
		size_t pos = strlen(fileName) - 1;
#ifdef HAVE_LINUX_EXT2_FS_H
		int fd;
#endif

		a->attrib = 0;
		a->flags |= SSH5_FILEXFER_ATTR_BITS;
		while (pos >= 1 && fileName[pos - 1] != '/')
			pos--;
		if (fileName[pos] == '.')
			a->attrib |= SSH5_FILEXFER_ATTR_FLAGS_HIDDEN;
#ifdef HAVE_LINUX_EXT2_FS_H
		if ((fd = open(fileName, O_RDONLY)) >= 0)
		{
			int flags;

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

static void EncodeACLCallBack(void *data, int type, u_int32_t id, u_int32_t mode)
{
	tBuffer *bAcl = (tBuffer *) data;

	BufferPutInt32(bAcl, SSH5_ACE4_ACCESS_ALLOWED_ACE_TYPE);
	BufferPutInt32(bAcl, 0);//ace-flag ???
	BufferPutInt32(bAcl, mode);
	switch (type)
	{
	case FS_ENUM_USER_OBJ:
		BufferPutString(bAcl, "USER");
		break;
	case FS_ENUM_GROUP_OBJ:
		BufferPutString(bAcl, "GROUP");
		break;
	case FS_ENUM_OTHER:
		BufferPutString(bAcl, "OTHER");
		break;
	case FS_ENUM_USER:
		{
			t_info *pw;
			char buf[11 + 1];
			char *str;

			if ((pw = mygetpwuid(id)))
				str = pw->name;
			else
			{
				(void) snprintf(buf, sizeof(buf), "%u", (unsigned int) id);
				str = buf;
			}
			BufferPutString(bAcl, str);
		}
		break;
	case FS_ENUM_GROUP:
		{
			t_info *gr;
			char buf[11 + 1];
			char *str;

			if ((gr = mygetgrgid(id)))
				str = gr->name;
			else
			{
				(void) snprintf(buf, sizeof(buf), "%u", (unsigned int) id);
				str = buf;
			}
			BufferPutString(bAcl, str);
		}
		break;
	}
}

//Only call for SSH2_FXP_STAT, SSH2_FXP_LSTAT or SSH2_FXP_FSTAT
static void EncodeACL(tBuffer *b, const char *file)
{
	u_int32_t nbEntries;
	tBuffer *bAcl;

	bAcl = BufferNew();
	if (bAcl != NULL)
	{
		BufferPutInt32(bAcl, 0);//Number of ACL
		if (FSEnumAcl(file, 1, EncodeACLCallBack, bAcl, &nbEntries) == SSH2_FX_OK)
		{
			u_int32_t posNew;

			posNew = BufferGetCurrentWritePosition(bAcl);
			BufferSetCurrentWritePosition(bAcl, 0);
			BufferPutInt32(bAcl, nbEntries);//Number of ACLs
			BufferSetCurrentWritePosition(bAcl, posNew);
		}
		BufferPutPacket(b, bAcl);
		BufferDelete(bAcl);
	}
	else
		BufferPutInt32(b, 0);
}

void EncodeAttributes(tBuffer *b, const tAttributes *a, /*@null@*/ const char *file)
{
	DEBUG((MYLOG_DEBUG, "[EncodeAttributes]flags=%i", a->flags));
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
		t_info *pw;
		t_info *gr;
		char buf[11 + 1];
		char *str;

		if ((pw = mygetpwuid(a->uid)))
			str = pw->name;
		else
		{
			(void) snprintf(buf, sizeof(buf), "%u", (unsigned int) a->uid);
			str = buf;
		}
		BufferPutString(b, str);
		if ((gr = mygetgrgid(a->gid)))
			str = gr->name;
		else
		{
			(void) snprintf(buf, sizeof(buf), "%u", (unsigned int) a->gid);
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
#if(MSS_ACL)
		if (file == NULL)
#endif
			BufferPutString(b, ""); //unsupported feature
#if(MSS_ACL)
		else
			EncodeACL(b, file);
#endif
	}
	if (HAS_BIT(a->flags, SSH5_FILEXFER_ATTR_BITS))
		BufferPutInt32(b, a->attrib);
	if (cVersion >= 5 && HAS_BIT(a->flags, SSH2_FILEXFER_ATTR_EXTENDED))
		BufferPutInt32(b, 0); //unsupported feature
}

struct timeval *AttributesToTimeval(const tAttributes *a)
{
	static struct timeval tv[2];

	tv[0].tv_sec = a->atime;
	tv[0].tv_usec = 0;
	tv[1].tv_sec = a->mtime;
	tv[1].tv_usec = 0;
	return (tv);
}
