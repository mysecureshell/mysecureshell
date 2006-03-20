/*
MySecureShell permit to add restriction to modified sftp-server
when using MySecureShell as shell.
Copyright (C) 2006 Sebastien Tardif

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
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Defines.h"
#include "Sftp.h"
#include "Admin.h"
#include "Global.h"
#include "Log.h"
#include "Send.h"
#include "Util.h"

void	DoAdminListUsers()
{
  if ((gl_var->who->status & SFTPWHO_IS_ADMIN))
    {
      char	*buf;
      int	ret;

      buf = ExecCommand(MSS_SFTPWHO, &ret);
      if (buf)
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
        SendStatus(bOut, 0, SSH2_FX_FAILURE);
    }
  else
    SendStatus(bOut, 0, SSH2_FX_OP_UNSUPPORTED);
}

void	DoAdminKillUser()
{
  if ((gl_var->who->status & SFTPWHO_IS_ADMIN))
    {
      t_sftpwho	*who;
      int	pidToKill = BufferGetInt32(bIn);
      int	status = SSH2_FX_OK;

      DEBUG((MYLOG_DEBUG, "[DoAdminKillUser]Try to kill pid:%i status:%i", pidToKill, status));
      who = SftWhoGetAllStructs();
      if (who)
        {
          unsigned int	pid;
          int		i;

          pid = getpid();
          for (i = 0; i < SFTPWHO_MAXCLIENT; i++)
            if ((who[i].status & SFTPWHO_STATUS_MASK) != SFTPWHO_EMPTY)
              if ((who[i].pid == pidToKill || pidToKill == 0) && who[i].pid != pid)
                if (kill(who[i].pid, SIGHUP) == -1)
                  status = errnoToPortable(errno);
        }
      SendStatus(bOut, 0, status);
    }
  else
    SendStatus(bOut, 0, SSH2_FX_OP_UNSUPPORTED);
}

void	DoAdminServerStatus()
{
  if ((gl_var->who->status & SFTPWHO_IS_ADMIN))
    {
      int	isActive = BufferGetInt8(bIn);
      int	status = SSH2_FX_OK;
      int	fd;

      if (isActive)
        {
          if (unlink(SHUTDOWN_FILE) == -1)
            status = errnoToPortable(errno);
        }
      else
        {
          if ((fd = open(SHUTDOWN_FILE, O_CREAT | O_TRUNC | O_RDWR, 0644)) >= 0)
            close(fd);
          else
            status = errnoToPortable(errno);
        }
      SendStatus(bOut, 0, status);
    }
  else
    SendStatus(bOut, 0, SSH2_FX_OP_UNSUPPORTED);
}

void	DoAdminServerGetStatus()
{
  if ((gl_var->who->status & SFTPWHO_IS_ADMIN))
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
  else
    SendStatus(bOut, 0, SSH2_FX_OP_UNSUPPORTED);
}

void	DoAdminGetLogContent()
{
  if ((gl_var->who->status & SFTPWHO_IS_ADMIN))
    {
      u_int32_t	size, r;
      tBuffer	*b;
      char	*buffer;

      b = BufferNew();
      BufferPutInt8(b, SSH_ADMIN_GET_LOG_CONTENT_REPLY);
      size = BufferGetInt32(bIn);
      if ((buffer = malloc(size)))
	{
	  int	fd;

	  if ((fd = open(MSS_LOG, O_CREAT | O_TRUNC | O_RDWR, 0644)) >= 0)
	    {
	      lseek(fd, SEEK_END, size);
	      r = read(fd, buffer, size);
	      if (r < 0)
		BufferPutInt32(b, 0);
	      else
		BufferPutData(b, buffer, r);
	      close(fd);
	    }
	}
      BufferPutPacket(bOut, b);
      BufferDelete(b);
      DEBUG((MYLOG_DEBUG, "[DoAdminGetLogContent]wanted:%i / read:%i", size, r));
    }
  else
    SendStatus(bOut, 0, SSH2_FX_OP_UNSUPPORTED);
}
