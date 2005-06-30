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

#ifndef _SFTP_H_
#define _SFTP_H_

#include <sys/types.h>

#ifndef MIN
#define MIN(_A, _B)	((_A) < (_B) ? (_A) : (_B))
#endif
#ifndef MAX
#define MAX(_A, _B)	((_A) > (_B) ? (_A) : (_B))
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef S_ISVTX
#define S_ISVTX	01000
#endif

#define SSH2_MAX_READ	(64 * 1024)

/* version */
#define	SSH2_FILEXFER_VERSION		4

/* client to server */
#define SSH2_FXP_INIT			1
#define SSH2_FXP_OPEN			3
#define SSH2_FXP_CLOSE			4
#define SSH2_FXP_READ			5
#define SSH2_FXP_WRITE			6
#define SSH2_FXP_LSTAT			7
#define SSH2_FXP_FSTAT			8
#define SSH2_FXP_SETSTAT		9
#define SSH2_FXP_FSETSTAT		10
#define SSH2_FXP_OPENDIR		11
#define SSH2_FXP_READDIR		12
#define SSH2_FXP_REMOVE			13
#define SSH2_FXP_MKDIR			14
#define SSH2_FXP_RMDIR			15
#define SSH2_FXP_REALPATH		16
#define SSH2_FXP_STAT			17
#define SSH2_FXP_RENAME			18
#define SSH2_FXP_READLINK		19
#define SSH2_FXP_SYMLINK		20

/* server to client */
#define SSH2_FXP_VERSION		2
#define SSH2_FXP_STATUS			101
#define SSH2_FXP_HANDLE			102
#define SSH2_FXP_DATA			103
#define SSH2_FXP_NAME			104
#define SSH2_FXP_ATTRS			105

#define SSH2_FXP_EXTENDED		200
#define SSH2_FXP_EXTENDED_REPLY		201

/* attributes */
#define SSH2_FILEXFER_ATTR_SIZE			0x00000001
#define SSH2_FILEXFER_ATTR_UIDGID		0x00000002
#define SSH2_FILEXFER_ATTR_PERMISSIONS		0x00000004
#define SSH2_FILEXFER_ATTR_ACMODTIME		0x00000008
#define SSH4_FILEXFER_ATTR_ACCESSTIME		0x00000008
#define SSH4_FILEXFER_ATTR_CREATETIME		0x00000010
#define SSH4_FILEXFER_ATTR_MODIFYTIME		0x00000020
#define SSH2_FILEXFER_ATTR_ACL			0x00000040
#define SSH4_FILEXFER_ATTR_OWNERGROUP		0x00000080
#define SSH4_FILEXFER_ATTR_SUBSECOND_TIMES	0x00000100
#define SSH2_FILEXFER_ATTR_EXTENDED		0x80000000

/* portable open modes */
#define SSH2_FXF_READ			0x00000001
#define SSH2_FXF_WRITE			0x00000002
#define SSH2_FXF_APPEND			0x00000004
#define SSH2_FXF_CREAT			0x00000008
#define SSH2_FXF_TRUNC			0x00000010
#define SSH2_FXF_EXCL			0x00000020
#define SSH4_FXF_TEXT			0x00000040

/* status messages */
#define SSH2_FX_OK			0
#define SSH2_FX_EOF			1
#define SSH2_FX_NO_SUCH_FILE		2
#define SSH2_FX_PERMISSION_DENIED	3
#define SSH2_FX_FAILURE			4
#define SSH2_FX_BAD_MESSAGE		5
#define SSH2_FX_NO_CONNECTION		6
#define SSH2_FX_CONNECTION_LOST		7
#define SSH2_FX_OP_UNSUPPORTED		8
#define SSH4_FX_INVALID_HANDLE		9
#define SSH4_FX_NO_SUCH_PATH		10
#define SSH4_FX_FILE_ALREADY_EXISTS	11
#define SSH4_FX_WRITE_PROTECT		12
#define SSH4_FX_NO_MEDIA		13
#define SSH2_FX_MAX			13

/* file type */
#define SSH4_FILEXFER_TYPE_REGULAR	1
#define SSH4_FILEXFER_TYPE_DIRECTORY	2
#define SSH4_FILEXFER_TYPE_SYMLINK	3
#define SSH4_FILEXFER_TYPE_SPECIAL	4
#define SSH4_FILEXFER_TYPE_UNKNOWN	5

typedef struct		sAttributes
{
	u_int32_t	flags;
	u_int8_t	type;
	u_int64_t	size;
	u_int32_t	uid;
	u_int32_t	gid;
	u_int32_t	perm;
	u_int32_t	atime;
	u_int32_t	ctime;
	u_int32_t	mtime;
}			tAttributes;

typedef struct		sStat
{
	char		*name;
	char		*longName;
	tAttributes	attributes;
}			tStat;
 
extern	int	cVersion;
 
#endif //_SFTP_H_
