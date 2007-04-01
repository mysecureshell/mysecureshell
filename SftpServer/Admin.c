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
#ifdef MSS_HAVE_ADMIN
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Defines.h"
#include "Global.h"
#include "Admin.h"
#include "Log.h"
#include "Send.h"
#include "Util.h"
#include "../security.h"

void	DoAdminListUsers()
{
  char	*buf;
  int	ret;

  buf = ExecCommand(MSS_SFTPWHO, &ret);
  if (buf != NULL)
    {
      tBuffer	*b;
      
      b = BufferNew();
      BufferPutInt8(b, SSH_ADMIN_LIST_USERS_REPLY);
      BufferPutString(b, buf);
      BufferPutPacket(bOut, b);
      DEBUG((MYLOG_DEBUG, "[DoAdminListUsers]send length:'%i' return:%i", strlen(buf), ret));
      BufferDelete(b);
      free(buf);
    }
  else
    {
      SendStatus(bOut, 0, SSH2_FX_FAILURE);
      DEBUG((MYLOG_DEBUG, "[DoAdminListUsers]Error: %s", strerror(errno)));
    }
}

void	DoAdminKillUser()
{
  t_sftpwho	*who;
  int	pidToKill = (int )BufferGetInt32(bIn);
  int	status = SSH2_FX_OK;

  DEBUG((MYLOG_DEBUG, "[DoAdminKillUser]Try to kill pid:%i", pidToKill));
  who = SftWhoGetAllStructs();
  if (who != NULL)
    {
      unsigned int	pid;
      int		i;

      pid = (unsigned int )getpid();
      for (i = 0; i < SFTPWHO_MAXCLIENT; i++)
	if ((who[i].status & SFTPWHO_STATUS_MASK) != SFTPWHO_EMPTY)
	  if ((who[i].pid == pidToKill || pidToKill == 0) && who[i].pid != pid)
	    {
	      DEBUG((MYLOG_DEBUG, "[DoAdminKillUser]Send kill to pid:%i", who[i].pid));
	      if (kill(who[i].pid, SIGHUP) == -1)
		status = errnoToPortable(errno);
	    }
    }
#ifdef DODEBUG
  else
    DEBUG((MYLOG_DEBUG, "[DoAdminKillUser]No global structure !"));
#endif
  SendStatus(bOut, 0, status);
}

void	DoAdminServerStatus()
{
  int	isActive = BufferGetInt8(bIn);
  int	status = SSH2_FX_OK;
  int	fd;

  if (isActive > 0)
    {
      if (unlink(SHUTDOWN_FILE) == -1)
	status = errnoToPortable(errno);
    }
  else
    {
      if ((fd = open(SHUTDOWN_FILE, O_CREAT | O_TRUNC | O_RDWR, 0644)) >= 0)
	xclose(fd);
      else
	status = errnoToPortable(errno);
    }
  SendStatus(bOut, 0, status);
}

void	DoAdminServerGetStatus()
{
  struct stat	st;
  tBuffer		*b;
  char		state;
  
  b = BufferNew();
  BufferPutInt8(b, SSH_ADMIN_SERVER_GET_STATUS_REPLY);
  if (stat(SHUTDOWN_FILE, &st) == -1)
    state = 1;
  else
    state = 0;
  BufferPutInt8(b, state);
  BufferPutPacket(bOut, b);
  BufferDelete(b);
  DEBUG((MYLOG_DEBUG, "[DoAdminServerGetStatus]state:'%i'", state));
}

void	DoAdminGetLogContent()
{
  u_int32_t	r, status = SSH2_FX_FAILURE;
  char		*buffer;
  off_t		size;
  
  size = BufferGetInt32(bIn);
  if ((buffer = malloc(size)) != NULL)
    {
      int	fd;

      if ((fd = open(MSS_LOG, O_RDONLY)) >= 0)
	{
	  if (lseek(fd, -size, SEEK_END) == 0)
	    {
	      r = read(fd, buffer, size);
	      SendData(bOut, 0, buffer, r);
	      status = SSH2_FX_OK;
	    }
	  else
	    status = errnoToPortable(errno);
	}
      else
	status = errnoToPortable(errno);
      free(buffer);
    }
  DEBUG((MYLOG_DEBUG, "[DoAdminGetLogContent]wanted:%i / read:%i", size, r));
  if (status != SSH2_FX_OK)
    SendStatus(bOut, 0, status);
}

void	DoAdminConfigSet()
{
  u_int32_t	size, status = SSH2_FX_FAILURE;
  char		*buffer;
      
  buffer = BufferGetData(bIn, &size);
  if (buffer != NULL)
    {
      int	fd;

      if ((fd = open(CONFIG_FILE, O_WRONLY | O_TRUNC | O_CREAT, 0644)) >= 0)
	{
	  fchown(fd, 0, 0);
	  if (write(fd, buffer, size) == -1)
	    status = errnoToPortable(errno);
	  else
	    status = SSH2_FX_OK;
	}
      else
	status = errnoToPortable(errno);
    }
  DEBUG((MYLOG_DEBUG, "[DoAdminSetLogContent]send:%i / write:%i", size, r));
  SendStatus(bOut, 0, status);
}

void	DoAdminConfigGet()
{
  struct stat	st;
  u_int32_t	r, status = SSH2_FX_FAILURE;
  char		*buffer;
      
  if (stat(CONFIG_FILE, &st) == -1)
    status = errnoToPortable(errno);
  else if ((buffer = malloc(st.st_size)) != NULL)
    {
      tBuffer	*b = BufferNew();
      int	fd;

      BufferPutInt8FAST(b, SSH2_FXP_DATA);
      BufferPutInt32(b, 0);
      if ((fd = open(CONFIG_FILE, O_RDONLY)) >= 0)
	{
	  r = read(fd, buffer, st.st_size);
	  xclose(fd);
	  BufferPutData(b, buffer, r);
	  status = SSH2_FX_OK;
	  if (stat("/etc/shells", &st) != -1
	      && (fd = open("/etc/shells", O_RDONLY)) >= 0)
	    {
	      if ((buffer = realloc(buffer, st.st_size)) != NULL)
		{
		  r = read(fd, buffer, st.st_size);
		  xclose(fd);
		  BufferPutData(b, buffer, r);
		}
	      else
		BufferPutInt32(b, 0);
	      xclose(fd);
	      BufferPutPacket(bOut, b);
	    }
	  else
	    BufferPutInt32(b, 0);
	}
      else
	status = errnoToPortable(errno);
      BufferDelete(b);
      free(buffer);
    }
  DEBUG((MYLOG_DEBUG, "[DoAdminGetLogContent]status: %i", status));
  if (status != SSH2_FX_OK)
    SendStatus(bOut, 0, status);
}

void	DoAdminUserCreate()
{
  u_int32_t	status = SSH2_FX_FAILURE;
  char		*args[5];
  char		*userName;
  char		*passWord;
  char		*homePath;
  int		ret;

  userName = BufferGetString(bIn);
  passWord = BufferGetString(bIn);
  homePath = BufferGetString(bIn);
  args[0] = MSS_SFTPUSER;
  args[1] = "create";
  args[2] = userName;
  args[3] = homePath;
  args[4] = NULL;
  (void )ExecCommandWithArgs(args, &ret, passWord, 0);
  if (ret == 0)
    {
      args[1] = "hide";
      args[3] = "0";
      (void )ExecCommandWithArgs(args, &ret, NULL, 0);
      status = SSH2_FX_OK;
    }
  DEBUG((MYLOG_DEBUG, "[DoAdminUserCreate]User:%s Home:%s Pass:%s status:%i", userName, homePath, passWord, status));
  SendStatus(bOut, 0, status);
  free(userName);
  free(passWord);
  free(homePath);
}

void	DoAdminUserDelete()
{
  u_int32_t	status = SSH2_FX_FAILURE;
  char		*args[5];
  char		*userName;
  int		ret;

  userName = BufferGetString(bIn);
  args[0] = MSS_SFTPUSER;
  args[1] = "delete";
  args[2] = userName;
  args[3] = "0";
  args[4] = NULL;
  (void )ExecCommandWithArgs(args, &ret, NULL, 0);
  if (ret == 0)
    status = SSH2_FX_OK;
  DEBUG((MYLOG_DEBUG, "[DoAdminUserDelete]User:%s status:%i", userName, status));
  SendStatus(bOut, 0, status);
  free(userName);
}

void    DoAdminUserList()
{
  char      *args[3];
  char      *users;
  int       ret;
  
  args[0] = MSS_SFTPUSER;
  args[1] = "list";
  args[2] = NULL;
  users = ExecCommandWithArgs(args, &ret, NULL, 1);
  DEBUG((MYLOG_DEBUG, "[DoAdminUserList]Return:%i Users:%s", ret, users));
  if (ret == 0)
    SendData(bOut, 0, users, strlen(users));
  else
    SendStatus(bOut, 0, SSH2_FX_FAILURE);
  if (users != NULL)
    free(users);
}

#endif
