/*
Mysecureshell permit to add restriction to modified sftp-server
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
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "../SftpServer/Sftp.h"
#include "../SftpServer/Buffer.h"
#include "../security.h"

static int	_sftpIn = 0;
static int	_sftpOut = 0;

int	DoProtocol(tBuffer *bIn);

static pid_t	execSftpServer(int ac, char **av)
{
  pid_t		pid;
  char		**args;
  int		fdIn[2], fdOut[2];

  if (pipe(fdIn) == -1 ||
      pipe(fdOut) == -1)
    {
      perror(av[0]);
      exit (1);
    }
  if ((pid = fork()) == 0)
    {
      xdup2(fdIn[0], 0);
      xdup2(fdOut[1], 1);
      xdup2(fdOut[1], 2);
      xclose(fdIn[0]); xclose(fdIn[1]);
      xclose(fdOut[0]); xclose(fdOut[1]);
      args = calloc(ac + 6, sizeof(*args));
      args[0] = "ssh";
      args[1] = "-oForwardX11 no";
      args[2] = "-oForwardAgent no";
      args[3] = "-oClearAllForwardings yes";
      args[4] = "-s";
      (void )memcpy(args + 5, av + 1, (ac - 1) * sizeof(char *));
      args[ac + 4] = "sftp";
      (void )signal(SIGINT, SIG_IGN);
      if (execvp(args[0], args) == -1)
	perror(args[0]);
      exit (1);
    }
  else if (pid != -1)
    {
      xclose(fdIn[0]);
      xclose(fdOut[1]);
      _sftpIn = fdIn[1];
      _sftpOut = fdOut[0];
    }
  else
    {
      perror(av[0]);
      exit (1);
    }
  return (pid);
}

static int	WritePacket(tBuffer *bOut)
{
  ssize_t	ret;
  size_t	len;

  len = (size_t )(bOut->length - bOut->read);
  ret = write(_sftpIn, bOut->data + bOut->read, len);
  if (ret == -1)
    return (1);
  bOut->read += ret;
  BufferClean(bOut);
  return (0);
}

static int	ReadPacket(tBuffer *bIn)
{
  ssize_t	len;
  char		buffer[16384];

  len = read(_sftpOut, buffer, sizeof(buffer));
  if (len == -1)
    return (1);
  BufferPutRawData(bIn, buffer, (u_int32_t )len);
  if (DoProtocol(bIn) == 1)
    return (1);
  return (0);
}

static void	SendInit(tBuffer *bOut)
{
  tBuffer	*b;

  b = BufferNew();
  BufferPutInt8(b, SSH2_FXP_INIT);
  BufferPutInt32(b, SSH2_ADMIN_VERSION);
  BufferPutPacket(bOut, b);
  BufferDelete(b);
  (void )WritePacket(bOut);
}

static void	SendListUsers(tBuffer *bOut)
{
  tBuffer       *b;

  b = BufferNew();
  BufferPutInt8(b, SSH_ADMIN_LIST_USERS);
  BufferPutPacket(bOut, b);
  BufferDelete(b);
}

static void	SendKillUser(tBuffer *bOut, const char *arg)
{
  tBuffer	*b;

  b = BufferNew();
  BufferPutInt8(b, SSH_ADMIN_KILL_USER);
  BufferPutInt32(b, (u_int32_t )atoi(arg));
  BufferPutPacket(bOut, b);
  BufferDelete(b);
}

static void	SendServerStatus(tBuffer *bOut, const char *arg)
{
  tBuffer       *b;

  b = BufferNew();
  BufferPutInt8(b, SSH_ADMIN_SERVER_STATUS);
  BufferPutInt8(b, (u_int8_t )(strcmp(arg, "start") == 0 ? 1 : 0));
  BufferPutPacket(bOut, b);
  BufferDelete(b);
}

static void	SendServerGetStatus(tBuffer *bOut)
{
  tBuffer	*b;

  b = BufferNew();
  BufferPutInt8(b, SSH_ADMIN_SERVER_GET_STATUS);
  BufferPutPacket(bOut, b);
  BufferDelete(b);
}

static void	SendGetLog(tBuffer *bOut, u_int32_t size)
{
  tBuffer	*b;

  b = BufferNew();
  BufferPutInt8(b, SSH_ADMIN_GET_LOG_CONTENT);
  BufferPutInt32(b, size);
  BufferPutPacket(bOut, b);
  BufferDelete(b);
}

static void	DoVersion(tBuffer *bIn)
{
  (void )BufferGetInt32(bIn);
  while (bIn->read < bIn->length)
    {
      free(BufferGetString(bIn));
      free(BufferGetString(bIn));
    }
}

static void	DoListUsersReply(tBuffer *bIn)
{
  char		*lists = BufferGetString(bIn);

  (void )printf("%s\n", lists);
  free(lists);
}

static void	DoGetServerStatusReply(tBuffer *bIn)
{
  u_int8_t	state;

  state = BufferGetInt8(bIn);
  (void )printf("Server is %s.\n", state == 0 ? "offline" : "online");
}

static void	DoGetLogContentReply(tBuffer *bIn)
{
  u_int32_t	size;
  void		*str;

  (void )BufferGetInt32(bIn);
  str = BufferGetData(bIn, &size);
  if (size > 0)
    {
      (void )fflush(stdout);
      (void )write(1, str, size);
      (void )write(1, "\n", 1);
#ifdef HAVE_LOG_IN_COLOR
      (void )printf("\33[37:40:0m");
#endif
    }
}

static void	DoStatus(tBuffer *bIn)
{
  char		*msg;
  u_int32_t	status;

  (void )BufferGetInt32(bIn);
  status = BufferGetInt32(bIn);
  msg = BufferGetString(bIn);
  free(BufferGetString(bIn));
  if (status == SSH2_FX_OK)
    (void )printf("Done.\n");
  else
    (void )printf("Error : %s\n", msg);
  free(msg);
}

int	DoProtocol(tBuffer *bIn)
{
  u_int32_t	oldRead, msgLen, msgType;
  
 parsePacket:
  if (bIn->length < 5) //header too small
    return (0);
  oldRead = bIn->read;
  msgLen = BufferGetInt32(bIn);
  if (msgLen > (256 * 1024)) //message too long
      return (1);
  if ((bIn->length - bIn->read) < msgLen) //message not complete
    {
      bIn->read = oldRead;//cancel read
      return (0);
    }
  oldRead += 4; //ignore size of msgLen
  msgType = BufferGetInt8(bIn);
  switch (msgType)
    {
    case SSH2_FXP_STATUS:
      DoStatus(bIn);
      break;
    case SSH2_FXP_VERSION:
      DoVersion(bIn);
      break;
    case SSH_ADMIN_LIST_USERS_REPLY:
      DoListUsersReply(bIn);
      break;
    case SSH_ADMIN_SERVER_GET_STATUS_REPLY:
      DoGetServerStatusReply(bIn);
      break;
    case SSH2_FXP_DATA:
      DoGetLogContentReply(bIn);
      break;
    default:
      (void )printf("[ERROR]Unkown message type : %u\n", msgType);
      break;
    }
  if ((bIn->read - oldRead) < msgLen)//read entire message
      BufferReadData(bIn, msgLen - (bIn->read - oldRead));
  BufferClean(bIn);
  goto parsePacket;
}

static int	DoCommandLine(char *cmd, tBuffer *bIn, tBuffer *bOut)
{
  if (strcmp(cmd, "quit") == 0)
    return (1);
  else if (strncmp(cmd, "kill", 4) == 0)
    {
      char	*arg = strchr(cmd, ' ');

      if (arg)
	{
	  while (*arg == ' ')
	    arg++;
	  SendKillUser(bOut, arg);
	  if (WritePacket(bOut) == 1 || ReadPacket(bIn) == 1)
	    return (1);
	}
    }
  else if (strcmp(cmd, "list") == 0)
    {
      SendListUsers(bOut);
      if (WritePacket(bOut) == 1 || ReadPacket(bIn) == 1)
	return (1);
    }
  else if (strncmp(cmd, "log", 3) == 0)
    {
      char	*arg = strchr(cmd, ' ');
 
      if (arg != NULL)
	{
	  int	size;

	  while (*arg == ' ')
            arg++;
	  size = atoi(arg);
	  SendGetLog(bOut, (u_int32_t )size);
	  if (WritePacket(bOut) == 1 || ReadPacket(bIn) == 1)
	    return (1);
	}
    }
  else if (strncmp(cmd, "server", 6) == 0)
    {
      char      *arg = strchr(cmd, ' ');

      if (arg != NULL)
	{
	  SendServerStatus(bOut, arg);
	  if (WritePacket(bOut) == 1 || ReadPacket(bIn) == 1)
	    return (1);
	}
      else
	{
	  SendServerGetStatus(bOut);
          if (WritePacket(bOut) == 1 || ReadPacket(bIn) == 1)
	    return (1);
	}
    }
  else
    {
      (void )printf("Usage:\n");
      (void )printf("\t kill [0 or PID] : kill user with PID or 0 to kill all users\n");
      (void )printf("\t list : list online users\n");
      (void )printf("\t log [x bytes] : show last x bytes of log\n");
      (void )printf("\t quit : quit program\n");
      (void )printf("\t server [start or stop] : start or stop server\n");
      (void )printf("\n");
    }
  (void )printf("> ");
  (void )fflush(stdout);
  return (0);
}

int		main(int ac, char **av)
{
  tBuffer	*bIn, *bOut;
  pid_t		pid;
  int		status, max;

  if (ac == 1)
    {
      (void )printf("Usage:\n");
      (void )printf("%s [ssh options] user@hostname\n", av[0]);
      exit (0);
    }
  bIn = BufferNew();
  bOut = BufferNew();
  pid = execSftpServer(ac, av);
  max = _sftpOut > _sftpIn ? _sftpOut + 1 : _sftpIn + 1;
  SendInit(bOut);
  (void )ReadPacket(bIn);
  (void )printf("> ");
  (void )fflush(stdout);
  for (;;)
    {
      fd_set	fdr, fdw;

      FD_ZERO(&fdr);
      FD_ZERO(&fdw);
      FD_SET(0, &fdr);
      FD_SET(_sftpOut, &fdr);
      if (bOut->length > 0)
	FD_SET(_sftpIn, &fdw);
      if (select(max, &fdr, &fdw, NULL, NULL) == -1)
	break;
      if (FD_ISSET(0, &fdr))
	{
	  char	buffer[1024];
	  int	len;

	  len = read(0, buffer, sizeof(buffer));
	  if (len == -1)
	    break;
	  buffer[len > 0 ? len - 1 : len] = 0;
	  if (DoCommandLine(buffer, bIn, bOut))
	    break;
	}
      if (FD_ISSET(_sftpOut, &fdr))
	{
	  if (ReadPacket(bIn))
	    break;
	}
      if (FD_ISSET(_sftpIn, &fdw))
	{
	  if (WritePacket(bOut))
	    break;
	}
    }
  (void )kill(pid, SIGHUP);
  xclose(_sftpIn);
  xclose(_sftpOut);
  (void )waitpid(pid, &status, 0);
  return (0);
}
