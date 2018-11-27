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

#ifndef __SFTPWHO_H__
#define __SFTPWHO_H__

#define	SFTPWHO_MAXCLIENT	128 //128 x 716o = 89.5 ko

#define	SFTPWHO_EMPTY		0
#define	SFTPWHO_IDLE		1
#define SFTPWHO_GET			2
#define	SFTPWHO_PUT			3
#define	SFTPWHO_STATUS_MASK	0xff

//Flags globals
#define SFTPWHO_STAY_AT_HOME	(1 << 8)
#define SFTPWHO_VIRTUAL_CHROOT	(1 << 9)
#define	SFTPWHO_RESOLVE_IP		(1 << 10)
#define SFTPWHO_IGNORE_HIDDEN	(1 << 11)
#define SFTPWHO_FAKE_USER		(1 << 12)
#define SFTPWHO_FAKE_GROUP		(1 << 13)
#define SFTPWHO_FAKE_MODE		(1 << 14)
#define SFTPWHO_HIDE_NO_ACESS	(1 << 15)
#define SFTPWHO_BYPASS_GLB_DWN	(1 << 16)
#define SFTPWHO_BYPASS_GLB_UPL	(1 << 17)
#define SFTPWHO_LINKS_AS_LINKS	(1 << 18)
#define SFTPWHO_IS_ADMIN		(1 << 19)
#define SFTPWHO_IS_SIMPLE_ADMIN	(1 << 20)
#define SFTPWHO_CAN_CHG_RIGHTS	(1 << 23)
#define SFTPWHO_CAN_CHG_TIME	(1 << 24)
#define SFTPWHO_CREATE_HOME		(1 << 25)
#define SFTPWHO_ARGS_MASK		0xffffff00

//Flags disabled
#define SFTP_DISABLE_REMOVE_DIR		(1 << 1)
#define SFTP_DISABLE_REMOVE_FILE	(1 << 2)
#define SFTP_DISABLE_READ_DIR		(1 << 3)
#define SFTP_DISABLE_READ_FILE		(1 << 4)
#define SFTP_DISABLE_WRITE_FILE		(1 << 5)
#define SFTP_DISABLE_SET_ATTRIBUTE	(1 << 6)
#define SFTP_DISABLE_MAKE_DIR		(1 << 7)
#define SFTP_DISABLE_RENAME			(1 << 8)
#define SFTP_DISABLE_SYMLINK		(1 << 9)
#define SFTP_DISABLE_OVERWRITE		(1 << 10)
#define SFTP_DISABLE_STATSFS		(1 << 11)

typedef struct	s_sftpglobal
{
	u_int32_t	download_max;
	u_int32_t	upload_max;
	u_int32_t	download_by_client;
	u_int32_t	upload_by_client;
}				t_sftpglobal;

typedef struct	s_sftpwho
{
//5 items = 882 octets
  char		user[30];
  char		ip[256];
  char		path[200];
  char		file[200];
  char		home[196];

//2 items = 8 octets
  u_int32_t	status;
  u_int32_t	pid;

//13 items = 52 octets
  u_int32_t	download_pos;
  u_int32_t	download_current;
  u_int32_t	download_total;
  u_int32_t	download_max;
  u_int32_t	upload_current;
  u_int32_t	upload_total;
  u_int32_t	upload_max;

  u_int32_t	time_maxidle;
  u_int32_t	time_maxlife;
  u_int32_t	time_idle;
  u_int32_t	time_total;
  u_int32_t	time_begin;
  u_int32_t	time_transf;
}		t_sftpwho;
//total : 942 octets

extern t_sftpglobal     *_sftpglobal;

t_sftpwho	*SftWhoGetAllStructs();
t_sftpwho	*SftpWhoGetStruct(int create);
int		SftpWhoCleanBuggedClient();
void	SftpWhoReleaseStruct(/*@null@*/ t_sftpwho *currentSession);
int		SftpWhoDeleteStructs();

#endif
