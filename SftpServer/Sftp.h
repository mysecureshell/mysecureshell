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

#ifndef _SFTP_H_
#define _SFTP_H_

#include <unistd.h>
#include <sys/types.h>
#include <regex.h>
#include "SftpWho.h"

#define HAS_BIT(_A, _B) (((_A) & (_B)) == (_B))

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

#define SSH2_MAX_PACKET	131072
#define SSH2_MAX_READ	131072
#define SSH2_READ_HASH	131072

/* version */
#define	SSH2_FILEXFER_VERSION		5
#define SSH2_SIMPLE_ADMIN_VERSION	254
#define SSH2_ADMIN_VERSION		255

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

#define SSH_ADMIN_LIST_USERS			210
#define SSH_ADMIN_LIST_USERS_REPLY		211
#define SSH_ADMIN_KILL_USER			212
#define SSH_ADMIN_SERVER_STATUS			213
#define SSH_ADMIN_SERVER_GET_STATUS		214
#define SSH_ADMIN_SERVER_GET_STATUS_REPLY	215
#define SSH_ADMIN_GET_LOG_CONTENT		216
#define SSH_ADMIN_CONFIG_GET			217
#define SSH_ADMIN_CONFIG_SET			218
#define SSH_ADMIN_USER_CREATE			219
#define SSH_ADMIN_USER_DELETE			220
#define SSH_ADMIN_USER_LIST			221
#define SSH_ADMIN_STATS				222
#define SSH_ADMIN_STATS_REPLY			223

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
#define SSH5_FILEXFER_ATTR_BITS			0x00000200
#define SSH2_FILEXFER_ATTR_EXTENDED		0x80000000
#define SSH5_FILEXFER_ATTR__MASK		0x8FFFFFFF
#define SSH5_FILEXFER_ATTR__BITS		0x800003FF

#define SSH5_FILEXFER_ATTR_FLAGS_READONLY	0x00000001
#define SSH5_FILEXFER_ATTR_FLAGS_SYSTEM		0x00000002
#define SSH5_FILEXFER_ATTR_FLAGS_HIDDEN		0x00000004
#define SSH5_FILEXFER_ATTR_FLAGS_CASE_INSENSITIVE 0x00000008
#define SSH5_FILEXFER_ATTR_FLAGS_ARCHIVE	0x00000010
#define SSH5_FILEXFER_ATTR_FLAGS_ENCRYPTED	0x00000020
#define SSH5_FILEXFER_ATTR_FLAGS_COMPRESSED	0x00000040
#define SSH5_FILEXFER_ATTR_FLAGS_SPARSE		0x00000080
#define SSH5_FILEXFER_ATTR_FLAGS_APPEND_ONLY	0x00000100
#define SSH5_FILEXFER_ATTR_FLAGS_IMMUTABLE	0x00000200
#define SSH5_FILEXFER_ATTR_FLAGS_SYNC		0x00000400

/* portable open modes */
#define SSH2_FXF_READ			0x00000001
#define SSH2_FXF_WRITE			0x00000002
#define SSH2_FXF_APPEND			0x00000004
#define SSH2_FXF_CREAT			0x00000008
#define SSH2_FXF_TRUNC			0x00000010
#define SSH2_FXF_EXCL			0x00000020
#define SSH4_FXF_TEXT			0x00000040

#define SSH5_FXF_CREATE_NEW		0x00000000
#define SSH5_FXF_CREATE_TRUNCATE	0x00000001
#define SSH5_FXF_OPEN_EXISTING		0x00000002
#define SSH5_FXF_OPEN_OR_CREATE		0x00000003
#define SSH5_FXF_TRUNCATE_EXISTING	0x00000004
#define SSH5_FXF_ACCESS_DISPOSITION	0x00000007
#define SSH5_FXF__FLAGS			0x0000007F
#define SSH5_FXF_ACCESS_APPEND_DATA		0x00000008
#define SSH5_FXF_ACCESS_APPEND_DATA_ATOMIC	0x00000010
#define SSH5_FXF_ACCESS_TEXT_MODE		0x00000020
#define SSH5_FXF_ACCESS_READ_LOCK		0x00000040
#define SSH5_FXF_ACCESS_WRITE_LOCK		0x00000080
#define SSH5_FXF_ACCESS_DELETE_LOCK		0x00000100
#define SSH5_FXF_ACCESS__FLAGS			0x000001F8

/* message flags */
#define SSH5_FXP_RENAME_OVERWRITE	0x00000001
#define SSH5_FXP_RENAME_ATOMIC		0x00000002
#define SSH5_FXP_RENAME_NATIVE		0x00000004

/* ace type */
#define SSH5_ACE4_ACCESS_ALLOWED_ACE_TYPE	0x00000000
#define SSH5_ACE4_ACCESS_DENIED_ACE_TYPE	0x00000001
#define SSH5_ACE4_SYSTEM_AUDIT_ACE_TYPE		0x00000002
#define SSH5_ACE4_SYSTEM_ALARM_ACE_TYPE		0x00000003

/* ace mask */
#define SSH5_ACE4_READ_DATA		0x00000001
#define SSH5_ACE4_LIST_DIRECTORY	0x00000001
#define SSH5_ACE4_WRITE_DATA		0x00000002
#define SSH5_ACE4_ADD_FILE		0x00000002
#define SSH5_ACE4_APPEND_DATA		0x00000004
#define SSH5_ACE4_ADD_SUBDIRECTORY	0x00000004
#define SSH5_ACE4_READ_NAMED_ATTRS	0x00000008
#define SSH5_ACE4_WRITE_NAMED_ATTRS	0x00000010
#define SSH5_ACE4_EXECUTE		0x00000020
#define SSH5_ACE4_DELETE_CHILD		0x00000040
#define SSH5_ACE4_READ_ATTRIBUTES	0x00000080
#define SSH5_ACE4_WRITE_ATTRIBUTES	0x00000100
#define SSH5_ACE4_DELETE		0x00010000
#define SSH5_ACE4_READ_ACL		0x00020000
#define SSH5_ACE4_WRITE_ACL		0x00040000
#define SSH5_ACE4_WRITE_OWNER		0x00080000
#define SSH5_ACE4_SYNCHRONIZE		0x00100000

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
#define SSH5_FX_NO_SPACE_ON_FILESYSTEM	14
#define SSH5_FX_QUOTA_EXCEEDED		15
#define SSH5_FX_UNKNOWN_PRINCIPLE	16
#define SSH5_FX_LOCK_CONFlICT		17
#define SSH2_FX_MAX			18


/* file type */
#define SSH4_FILEXFER_TYPE_REGULAR	1
#define SSH4_FILEXFER_TYPE_DIRECTORY	2
#define SSH4_FILEXFER_TYPE_SYMLINK	3
#define SSH4_FILEXFER_TYPE_SPECIAL	4
#define SSH4_FILEXFER_TYPE_UNKNOWN	5
#define SSH5_FILEXFER_TYPE_SOCKET	6
#define SSH5_FILEXFER_TYPE_CHAR_DEVICE	7
#define SSH5_FILEXFER_TYPE_BLOCK_DEVICE	8
#define SSH5_FILEXFER_TYPE_FIFO		9

typedef struct	sAttributes
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
  u_int32_t	attrib;
}		tAttributes;

typedef struct	sStat
{
  char		*name;
  char		*longName;
  tAttributes	attributes;
}		tStat;

/* Global defines */

#define MSS_TRUE	1
#define MSS_FALSE	0

/* End defines */


typedef struct	sGlobal
{
  t_sftpwho	*who;
  uid_t		current_user;
  gid_t		current_group;
  u_int32_t	flagsGlobals;
  u_int32_t	flagsDisable;
  int		has_hide_files;
  int		has_allow_filter;
  int		has_deny_filter;
  int		must_shutdown;
  regex_t	hide_files_regexp;
  regex_t	allow_filter_regexp;
  regex_t	deny_filter_regexp;
  int		max_openfiles;
  int		max_readfiles;
  int		max_writefiles;
  int		rights_file;
  int		rights_directory;
  int		minimum_rights_file;
  int		minimum_rights_directory;
  int		maximum_rights_file;
  int		maximum_rights_directory;
  unsigned int	download_current;
  unsigned int	upload_current;
  unsigned int	download_max;
  unsigned int	upload_max;
  char		*force_user;
  char		*force_group;
}		tGlobal;

 
extern	u_int32_t	cVersion;

int	SftpMain(tGlobal *params, int sftpProtocol);
void	DoInit();
void	DoRealPath();
void	DoOpenDir();
void	DoReadDir();
void	DoClose();
void	DoOpen();
void	DoRead();
void	DoWrite();
void	DoReadLink();
void	DoStat();
void	DoFStat();
void	DoSetStat(int usePath);
void	DoRemove();
void	DoMkDir();
void	DoRmDir();
void	DoRename();
void	DoSymLink();
void	DoUnsupported();
void	DoExtended();
void	DoSFTPProtocol();
 
#endif //_SFTP_H_
