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

#ifndef __SFTPWHO_H__
#define __SFTPWHO_H__

#define	SFTPWHO_MAXCLIENT	128 //128 x 712o = 89 ko

#define	SFTPWHO_EMPTY		0
#define	SFTPWHO_IDLE		1
#define SFTPWHO_GET		2
#define	SFTPWHO_PUT		3
#define	SFTPWHO_STATUS_MASK	0xff

#define SFTPWHO_STAY_AT_HOME	(1 << 8)
#define SFTPWHO_VIRTUAL_CHROOT	(1 << 9)
#define	SFTPWHO_RESOLVE_IP	(1 << 10)
#define SFTPWHO_IGNORE_HIDDEN	(1 << 11)
#define SFTPWHO_FAKE_USER	(1 << 12)
#define SFTPWHO_FAKE_GROUP	(1 << 13)
#define SFTPWHO_FAKE_MODE	(1 << 14)
#define SFTPWHO_HIDE_NO_ACESS	(1 << 15)
#define SFTPWHO_BYPASS_GLB_DWN	(1 << 16)
#define SFTPWHO_BYPASS_GLB_UPL	(1 << 17)
#define SFTPWHO_LINKS_AS_LINKS	(1 << 18)
#define SFTPWHO_IS_ADMIN	(1 << 19)
#define SFTPWHO_CAN_RMDIR	(1 << 20)
#define SFTPWHO_CAN_RMFILE	(1 << 21)
#define SFTPWHO_ARGS_MASK	0xffffff00

typedef struct	s_sftpglobal
{
  unsigned int	download_max;
  unsigned int	upload_max;
  unsigned int	download_by_client;
  unsigned int	upload_by_client;
}		t_sftpglobal;

typedef struct	s_sftpwho
{
//4 items = 656 octets
  char		user[30];
  char		ip[30];
  char		path[200];
  char		file[200];
  char		home[196];

//3 items = 10 octets
  unsigned int	status;
  unsigned int	pid;
  unsigned short mode;

//12 items = 46 octets
  unsigned short dowload_pos;
  unsigned int	download_current;
  unsigned int	download_total;
  unsigned int	download_max;
  unsigned int	upload_current;
  unsigned int	upload_total;
  unsigned int	upload_max;

  unsigned int	time_maxidle;
  unsigned int	time_maxlife;
  unsigned int	time_idle;
  unsigned int	time_total;
  unsigned int	time_begin;
  unsigned int	time_transf;
}		t_sftpwho;
//total : 512 octets

//#define	SHM_SFTP_SIZE	(SFTPWHO_MAXCLIENT * sizeof(t_sftpwho) + sizeof(t_sftpglobal))

extern t_sftpglobal     *_sftpglobal;

t_sftpwho	*SftWhoGetAllStructs();
t_sftpwho	*SftpWhoGetStruct(int create);
int		SftpWhoCleanBuggedClient();
void		SftpWhoRelaseStruct();
int		SftpWhoDeleteStructs();

#endif
