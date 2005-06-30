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

#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include "Encode.h"
#include "Handle.h"
#include "Util.h"
#include "Send.h"
#include "Sftp.h"

#include "SftpServer.c"

static tBuffer	*bIn = 0;
static tBuffer	*bOut = 0;
int		cVersion = 0;

#ifdef DODEBUG
#define	DEBUG(_X)	log_printf _X
#else
#define	DEBUG(_X)
#endif

static void	DoInit()
{
  tBuffer	*b;
	
  cVersion = BufferGetInt32(bIn);
  DEBUG((MYLOG_DEBUG, "[DoInit]New client version %i [server: %i]", cVersion, SSH2_FILEXFER_VERSION));
  b = BufferNew();
  BufferPutInt8(b, SSH2_FXP_VERSION);
  if (cVersion >= SSH2_FILEXFER_VERSION)
    {
      BufferPutInt32(b, SSH2_FILEXFER_VERSION);
      cVersion = SSH2_FILEXFER_VERSION;
    }
  else
    BufferPutInt32(b, cVersion);
  if (cVersion == 4)
    {
      BufferPutString(b, "newline");
      BufferPutString(b, "\n");
    }
  BufferPutPacket(bOut, b);
  BufferDelete(b);
}

static void	DoRealPath()
{
  u_int32_t	id;
  char		resolvedName[PATH_MAX];
  char		*path;
	
  id = BufferGetInt32(bIn);
  path = BufferGetString(bIn);
  resolvedName[0] = 0;
  if (!path[0])
    {
      free(path);
      path = strdup(".");
    }
  if (!realpath(path, resolvedName))
    SendStatus(bOut, id, errnoToPortable(errno));
  else
    {
      tStat	s;
      
      memset(&s, 0, sizeof(s));
      if (strcmp(path, ".") && strcmp(path, "./."))
	ResolvPath(path, resolvedName);
      s.name = s.longName = resolvedName;
      SendStats(bOut, id, 1, &s);
    }
  DEBUG((MYLOG_DEBUG, "[DoRealPath]path:'%s' -> '%s'", path, resolvedName));
  free(path);
}

static void	DoOpenDir()
{
  u_int32_t	id;
  char		*path;
  DIR		*dir;
  int		status = (cVersion <= 3 ? SSH2_FX_FAILURE : SSH4_FX_INVALID_HANDLE);
	
  id = BufferGetInt32(bIn);
  path = BufferGetString(bIn);
  if ((status = CheckRules(path, RULES_DIRECTORY, 0, O_RDONLY)) == SSH2_FX_OK)
    {
      if (!(dir = opendir(path)))
	status = errnoToPortable(errno);
      else
	{
	  int	h;
	  
	  if ((h = HandleNew(HANDLE_DIR, path, -1, dir, 0)) < 0)
	    closedir(dir);
	  else
	    {
	      SendHandle(bOut, id, h);
	      status = SSH2_FX_OK;
	    }
	}
    }
  DEBUG((MYLOG_DEBUG, "[DoOpenDir]path:'%s' status:%i", path, status));
  if (status != SSH2_FX_OK)
    SendStatus(bOut, id, status);
  free(path);
}

static void	DoReadDir()
{
  struct dirent	*dp;
  u_int32_t	id;
  char 		*path;
  DIR		*dir;
  int		h;

  id = BufferGetInt32(bIn);
  h = BufferGetInt32(bIn);
  dir = HandleGetDir(h);
  path = HandleGetPath(h);
  DEBUG((MYLOG_DEBUG, "[DoReadDir]path:'%s' handle:%i", path, h));
  if (dir == NULL || path == NULL || !path[0])
    SendStatus(bOut, id, (cVersion <= 3 ? SSH2_FX_FAILURE : SSH4_FX_INVALID_HANDLE));
  else
    {
      struct stat	st;
      char		pathName[1024];
      tStat 		*s;
      int		nstats = 10, count = 0, i;
      
      s = malloc(nstats * sizeof(tStat));
      while ((dp = readdir(dir)))
	{
	  if (count >= nstats)
	    {
	      nstats *= 2;
	      s = realloc(s, nstats * sizeof(tStat));
	    }
	  snprintf(pathName, sizeof(pathName), "%s%s%s", path,
		   path[strlen(path) - 1] == '/' ? "" : "/", dp->d_name);
	  if ((gl_var->who->status & SFTPWHO_LINKS_AS_LINKS))
	    {
	      if (lstat(pathName, &st) < 0)
		continue;
	    }
	  else
	    {
	      if (stat(pathName, &st) < 0)
		continue;
	    }
	  if ((dp->d_name[0] == '.' && (!dp->d_name[1] || (dp->d_name[1] == '.' && !dp->d_name[2])))
	      || CheckRules(pathName, RULES_LISTING, &st, 0) == SSH2_FX_OK)
	    {
	      ChangeRights(&st);
	      StatToAttributes(&st, &(s[count].attributes));
	      s[count].name = strdup(dp->d_name);
	      s[count].longName = LsFile(dp->d_name, &st);
	      DEBUG((MYLOG_DEBUG, "[DoReadDir] -> '%s' handle:%i [%i]", pathName, h, count));
	      count++;
	      if (count == 100)
		break;
	    }
	  else
	    DEBUG((MYLOG_DEBUG, "[DoReadDir] REFUSED -> '%s' handle:%i [%i]", pathName, h, count));
	}
      if (count > 0)
	{
	  SendStats(bOut, id, count, s);
	  for (i = 0; i < count; i++)
	    {
	      free(s[i].name);
	      free(s[i].longName);
	    }
	}
      else
	SendStatus(bOut, id, SSH2_FX_EOF);
      free(s);
    }
}

static void	DoClose()
{
  u_int32_t	id;
  int		h, status = (cVersion <= 3 ? SSH2_FX_FAILURE : SSH4_FX_INVALID_HANDLE);
	
  id = BufferGetInt32(bIn);
  h = BufferGetInt32(bIn);
  if ((HandleClose(h)) == -1)
    status = errnoToPortable(errno);
  else
    status = SSH2_FX_OK;
  SendStatus(bOut, id, status);
  gl_var->who->status = (gl_var->who->status & SFTPWHO_ARGS_MASK ) | SFTPWHO_IDLE;
  gl_var->down_max = 0;
  DEBUG((MYLOG_DEBUG, "[DoClose] -> handle:%i", h));
}

static void	DoOpen()
{
  u_int32_t	id, pflags;
  tAttributes	*a;
  char		*path;
  int		fd, flags, mode, status = SSH2_FX_FAILURE;

  id = BufferGetInt32(bIn);
  path = BufferGetString(bIn);
  pflags = BufferGetInt32(bIn);
  a = GetAttributes(bIn);
  flags = FlagsFromPortable(pflags);
  mode = (a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS) ? a->perm : gl_var->rights_file;
  if ((status = CheckRules(path, RULES_FILE, 0, flags)) == SSH2_FX_OK)
    if ((status = CheckRulesAboutMaxFiles()) == SSH2_FX_OK)
      {
	if ((fd = open(path, flags, mode)) < 0)
	  status = errnoToPortable(errno);
	else
	  {
	    int	h;
	    
	    if ((h = HandleNew(HANDLE_FILE, path, fd, NULL, pflags & SSH4_FXF_TEXT ? 1 : 0)) < 0)
	      close(fd);
	    else
	      {
		snprintf(gl_var->who->path, sizeof(gl_var->who->path), "%s", path);
		if (flags & O_WRONLY)
		  {
		    gl_var->who->status = (gl_var->who->status & SFTPWHO_ARGS_MASK ) | SFTPWHO_PUT;
		    log_printf(MYLOG_NORMAL, "[%s][%s]Upload into file '%s'",
			       gl_var->who->user, gl_var->who->ip, path);
		  }
		else
		  {
		    gl_var->who->status = (gl_var->who->status & SFTPWHO_ARGS_MASK ) | SFTPWHO_GET;
		    log_printf(MYLOG_NORMAL, "[%s][%s]Download file '%s'",
			       gl_var->who->user, gl_var->who->ip, path);
		  }
		gl_var->down_size = 0;
		gl_var->down_max = 0;
		if (!(flags & O_WRONLY))
		  {
		    struct stat	st;
		    
		    if (stat(path, &st) != -1)
		      gl_var->down_max = st.st_size;
		  }
		SendHandle(bOut, id, h);
		status = SSH2_FX_OK;
	      }
	  }
      }
  DEBUG((MYLOG_DEBUG, "[DoOpen]file:'%s' pflags:%i perm:0%o fd:%i", path, pflags, mode, fd));
  if (status != SSH2_FX_OK)
    SendStatus(bOut, id, status);
  free(path);
}

static void	DoRead()
{
  u_int32_t	id, len;
  char		buf[SSH2_MAX_READ];
  int		h, fileIsText, fd, status = (cVersion <= 3 ? SSH2_FX_FAILURE : SSH4_FX_INVALID_HANDLE);
  u_int64_t	off;
  
  id = BufferGetInt32(bIn);
  h = BufferGetInt32(bIn);
  off = BufferGetInt64(bIn);
  if ((len = BufferGetInt32(bIn)) > sizeof(buf))
    len = sizeof(buf);
  if ((fd = HandleGetFd(h, &fileIsText)) >= 0)
    {
      if (lseek(fd, off, SEEK_SET) < 0)
	status = errnoToPortable(errno);
      else
	{
	  int	ret;
	  
	  ret = read(fd, buf, len);
	  if (fileIsText)
	    for (len = 0; len < ret; len++)
	      if (buf[len] == '\r')
		{
		  memcpy(buf + len + 1, buf + len, ret - len - 1);
		  ret--;
		}
	  if (ret < 0)
	    status = errnoToPortable(errno);
	  else if (ret == 0)
	    status = SSH2_FX_EOF;
	  else
	    {
	      SendData(bOut, id, buf, ret);
	      status = SSH2_FX_OK;
	    }
	}
    }
  //DEBUG((MYLOG_DEBUG, "[DoRead]fd:%i off:%llu len:%i", fd, off, len));
  if (status != SSH2_FX_OK)
    SendStatus(bOut, id, status);
}

static void	DoWrite()
{
  u_int32_t	id;
  u_int64_t	off;
  u_int		len;
  int		fd, fileIsText, ret, status = (cVersion <= 3 ? SSH2_FX_FAILURE : SSH4_FX_INVALID_HANDLE);
  char		*data;

  id = BufferGetInt32(bIn);
  fd = HandleGetFd(BufferGetInt32(bIn), &fileIsText);
  off = BufferGetInt64(bIn);
  data = BufferGetData(bIn, &len);
  if (fd >= 0)
    {
      if (lseek(fd, off, SEEK_SET) < 0)
	status = errnoToPortable(errno);
      else
	{
	  if (fileIsText)
	    {
	      for (ret = 0; ret < len; ret++)
		if (data[ret] == '\r')
		  {
		    memcpy(data + ret + 1, data + ret, len - ret - 1);
		    len--;
		  }
	    }
	  ret = write(fd, data, len);
	  if (ret == -1)
	    status = errnoToPortable(errno);
	  else if (ret == len)
	    status = SSH2_FX_OK;
	}
    }
  //DEBUG((MYLOG_DEBUG, "[DoWrite]fd:%i off:%llu len:%i fileIsText:%i", fd, off, len, fileIsText));
  SendStatus(bOut, id, status);
}

static void	DoReadLink()
{
  u_int32_t	id, status;
  char		readLink[PATH_MAX];
  char		*path;
  int		len;

  id = BufferGetInt32(bIn);
  path = BufferGetString(bIn);
  if ((status = CheckRules(path, RULES_FILE, 0, O_RDONLY)) == SSH2_FX_OK)
    {
      if ((len = readlink(path, readLink, sizeof(readLink) - 1)) == -1)
	SendStatus(bOut, id, errnoToPortable(errno));
      else
	{
	  tStat	s;
	  
	  memset(&s.attributes, 0, sizeof(s.attributes));
	  readLink[len] = 0;
	  s.name = s.longName = readLink;
	  SendStats(bOut, id, 1, &s);
	}
      DEBUG((MYLOG_DEBUG, "[DoReadLink]file:'%s' -> '%s'", path, readLink));
    }
  else
    SendStatus(bOut, id, status);
  free(path);
}

static void	DoStat(int (*f_stat)(const char *, struct stat *))
{
  tAttributes	a;
  struct stat	st;
  u_int32_t	id, status, flags = 0;
  char		*path;
  int		r;
  
  id = BufferGetInt32(bIn);
  path = BufferGetString(bIn);
  if (cVersion >= 4)
    flags = BufferGetInt32(bIn);
  if ((status = CheckRules(path, RULES_NONE, 0, 0)) == SSH2_FX_OK)
    {
      DEBUG((MYLOG_DEBUG, "CheckRules => OK", status));
      if ((r = f_stat(path, &st)) < 0)
	SendStatus(bOut, id, errnoToPortable(errno));
      else
	{
	  ChangeRights(&st);
	  StatToAttributes(&st, &a);
	  if (cVersion >= 4)
	    a.flags = flags;
	  SendAttributes(bOut, id, &a);
	}
    }
  else
    SendStatus(bOut, id, status);
  DEBUG((MYLOG_DEBUG, "[Do%sStat]path:'%s' -> '%i'", f_stat == stat ? "" : "L", path, r));
  DEBUG((MYLOG_DEBUG, "status=%i flags=%x", status, flags));
  free(path);
}

static void	DoFStat()
{
  tAttributes	a;
  struct stat	st;
  u_int32_t	id, flags = 0;
  int		fd, fileIsText;
	
  id = BufferGetInt32(bIn);
  fd = HandleGetFd(BufferGetInt32(bIn), &fileIsText);
  if (cVersion >= 4)
    flags = BufferGetInt32(bIn);
  if (fd >= 0)
    {
      int	r;
      
      if ((r = fstat(fd, &st)) < 0)
	SendStatus(bOut, id, errnoToPortable(errno));
      else
	{
	  StatToAttributes(&st, &a);
	  if (cVersion >= 4)
	    a.flags = flags;
	  SendAttributes(bOut, id, &a);
	}
      DEBUG((MYLOG_DEBUG, "[DoFStat]fd:'%i' -> '%i'", fd, r));
    }
  else
    SendStatus(bOut, id, (cVersion <= 3 ? SSH2_FX_FAILURE : SSH4_FX_INVALID_HANDLE));
}

static void 	DoSetStat()
{
  tAttributes	*a;
  u_int32_t	id;
  char		*path;
  int		status = SSH2_FX_OK;

  id = BufferGetInt32(bIn);
  path = BufferGetString(bIn);
  a = GetAttributes(bIn);
  if ((status = CheckRules(path, RULES_NONE, 0, 0)) == SSH2_FX_OK)
    {
      if (a->flags & SSH2_FILEXFER_ATTR_SIZE)
	{
	  if (truncate(path, a->size) == -1)
	    status = errnoToPortable(errno);
	}
      if (a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS)
	{
	  if (chmod(path, a->perm & 0777) == -1)
	    status = errnoToPortable(errno);
	}
      if (a->flags & SSH2_FILEXFER_ATTR_ACMODTIME)
	{
	  if (utimes(path, AttributesToTimeval(a)) == -1)
	    status = errnoToPortable(errno);
	}
      if (a->flags & SSH2_FILEXFER_ATTR_UIDGID)
	{
	  if (chown(path, a->uid, a->gid) == -1)
	    status = errnoToPortable(errno);
	}
    }
  DEBUG((MYLOG_DEBUG, "[DoSetStat]path:'%s' -> '%i'", path, status));
  SendStatus(bOut, id, status);
  free(path);
}

static void 	DoFSetStat()
{
  tAttributes	*a;
  u_int32_t	id;
  char		*path;
  int		status = SSH2_FX_OK;

  id = BufferGetInt32(bIn);
  path = HandleGetPath(BufferGetInt32(bIn));
  a = GetAttributes(bIn);
  if (!path)
    status = (cVersion <= 3 ? SSH2_FX_FAILURE : SSH4_FX_INVALID_HANDLE);
  else
    {
      if (a->flags & SSH2_FILEXFER_ATTR_SIZE)
	{
	  if (truncate(path, a->size) == -1)
	    status = errnoToPortable(errno);
	}
      if (a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS)
	{
	  if (chmod(path, a->perm & 0777) == -1)
	    status = errnoToPortable(errno);
	}
      if (a->flags & SSH2_FILEXFER_ATTR_ACMODTIME)
	{
	  if (utimes(path, AttributesToTimeval(a)) == -1)
	    status = errnoToPortable(errno);
	}
      if (a->flags & SSH2_FILEXFER_ATTR_UIDGID)
	{
	  if (chown(path, a->uid, a->gid) == -1)
	    status = errnoToPortable(errno);
	}
    }
  DEBUG((MYLOG_DEBUG, "[DoFSetStat]path:'%s' -> '%i'", path, status));
  SendStatus(bOut, id, status);
}

static void	DoRemove()
{
  u_int32_t	id;
  char		*path;
  int		status = SSH2_FX_OK;

  id = BufferGetInt32(bIn);
  path = BufferGetString(bIn);
  if ((status = CheckRules(path, RULES_RMFILE, 0, 0)) == SSH2_FX_OK)
    {
      if (unlink(path) == -1)
	status = errnoToPortable(errno);
      log_printf(MYLOG_WARNING, "[%s][%s]Try to remove file '%s'",
		 gl_var->who->user, gl_var->who->ip, path);
    }
  DEBUG((MYLOG_DEBUG, "[DoRemove]path:'%s' -> '%i'", path, status));
  SendStatus(bOut, id, status);
  free(path);
}

static void	DoMkDir()
{
  tAttributes	*a;
  u_int32_t	id;
  char		*path;
  int		mode, status = SSH2_FX_OK;

  id = BufferGetInt32(bIn);
  path = BufferGetString(bIn);
  a = GetAttributes(bIn);
  mode = (a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS) ? a->perm & 0777 : gl_var->rights_directory;
  if ((status = CheckRules(path, RULES_DIRECTORY, 0, O_WRONLY)) == SSH2_FX_OK)
    {
      if (mkdir(path, mode) == -1)
	status = errnoToPortable(errno);
      log_printf(MYLOG_WARNING, "[%s][%s]Try to create directory '%s' : %s",
		 gl_var->who->user, gl_var->who->ip, path, (status != SSH2_FX_OK ? "error" : "success"));
    }
  SendStatus(bOut, id, status);
  DEBUG((MYLOG_DEBUG, "[DoMkDir]path:'%s' -> '%i'", path, status));
  free(path);
}

static void	DoRmDir()
{
  u_int32_t	id;
  char		*path;
  int		status = SSH2_FX_OK;

  id = BufferGetInt32(bIn);
  path = BufferGetString(bIn);
  if ((status = CheckRules(path, RULES_RMDIRECTORY, 0, 0)) == SSH2_FX_OK)
    {
      if (rmdir(path) == -1)
	status = errnoToPortable(errno);
      log_printf(MYLOG_WARNING, "[%s][%s]Try to remove directory '%s' : %s",
		 gl_var->who->user, gl_var->who->ip, path, (status != SSH2_FX_OK ? "error" : "success"));
    }
  SendStatus(bOut, id, status);
  DEBUG((MYLOG_DEBUG, "[DoRmDir]path:'%s' -> '%i'", path, status));
  free(path);
}

static void	DoRename()
{
  struct stat	sb;
  u_int32_t	id;
  char		*oldPath, *newPath;
  int		status = SSH2_FX_FAILURE;

  id = BufferGetInt32(bIn);
  oldPath = BufferGetString(bIn);
  newPath = BufferGetString(bIn);
  if ((status = CheckRules(oldPath, RULES_RMFILE, 0, 0)) == SSH2_FX_OK
      && (status = CheckRules(newPath, RULES_FILE, 0, O_WRONLY)) == SSH2_FX_OK)
    {
      if (lstat(oldPath, &sb) == -1)
	status = errnoToPortable(errno);
      else if (S_ISREG(sb.st_mode))
	{
	  if (link(oldPath, newPath) == -1)
	    {
	      if (errno == EOPNOTSUPP
#ifdef LINK_OPNOTSUPP_ERRNO
		  || errno == LINK_OPNOTSUPP_ERRNO
#endif
		  )
		{
		  struct stat	st;
		  
		  if (stat(newPath, &st) == -1)
		    {
		      if (rename(oldPath, newPath) == -1)
			status = errnoToPortable(errno);
		      else
			status = SSH2_FX_OK;
		    }
		}
	      else
		{
		  status = errnoToPortable(errno);
		}
	    }
	  else if (unlink(oldPath) == -1)
	    {
	      status = errnoToPortable(errno);
	      unlink(newPath);
	    }
	  else
	    status = SSH2_FX_OK;
	}
      else if (stat(newPath, &sb) == -1)
	{
	  if (rename(oldPath, newPath) == -1)
	    status = errnoToPortable(errno);
	  else
	    status = SSH2_FX_OK;
	}
      log_printf(MYLOG_WARNING, "[%s][%s]Try to rename '%s' -> '%s' : %s",
		 gl_var->who->user, gl_var->who->ip, oldPath, newPath,
		 (status != SSH2_FX_OK ? "error" : "success"));
    }
  else
    status = SSH2_FX_PERMISSION_DENIED;
  DEBUG((MYLOG_DEBUG, "[DoRename]oldPath:'%s' newPath:'%s' -> '%i'", oldPath, newPath, status));
  SendStatus(bOut, id, status);
  free(oldPath);
  free(newPath);
}

static void	DoSymLink()
{
  u_int32_t	id;
  char		*oldPath, *newPath;
  int		status = SSH2_FX_OK;

  id = BufferGetInt32(bIn);
  oldPath = BufferGetString(bIn);
  newPath = BufferGetString(bIn);
  if ((status = CheckRules(oldPath, RULES_FILE, 0, O_RDONLY)) == SSH2_FX_OK
      && (status = CheckRules(newPath, RULES_FILE, 0, O_WRONLY)) == SSH2_FX_OK)
    {
      if (symlink(oldPath, newPath) == -1)
	status = errnoToPortable(errno);
      DEBUG((MYLOG_DEBUG, "[DoSymLink]oldPath:'%s' newPath:'%s' -> '%i'", oldPath, newPath, status));
    }
  SendStatus(bOut, id, status);
  free(oldPath);
  free(newPath);
}

static void	DoUnsupported(int msgType, int msgLen)
{
  u_int32_t	id;
		
  id = BufferGetInt32(bIn);
  SendStatus(bOut, id, SSH2_FX_OP_UNSUPPORTED);
  DEBUG((MYLOG_DEBUG, "msgType:%i msgLen:%i", msgType, msgLen));
}

static void	DoExtended()
{
  u_int32_t	id;
  char		*request;
	
  id = BufferGetInt32(bIn);
  request = BufferGetString(bIn);
  SendStatus(bOut, id, SSH2_FX_OP_UNSUPPORTED);
  DEBUG((MYLOG_DEBUG, "[DoExtended]request:'%s'", request));
  free(request);
}

static void	DoProtocol()
{
  int		oldRead, msgLen, msgType;

 parsePacket:
  if (bIn->length < 5) //header too small
    return;
  oldRead = bIn->read;
  msgLen = BufferGetInt32(bIn);
  if (msgLen > (256 * 1024)) //message too long
    {
      log_printf(MYLOG_ERROR, "[%s][%s]Error: message is too long (%i)", gl_var->who->user, gl_var->who->ip, msgLen);
      exit (1);
    }
  if ((bIn->length - bIn->read) < msgLen) //message not complete
    {
      bIn->read = oldRead;//cancel read
      return;
    }
  oldRead += 4; //size of msgLen
  msgType = BufferGetInt8(bIn);
  DEBUG((MYLOG_DEBUG, "[DoProtocol] msgType:%i msgLen:%i", msgType, msgLen));
  switch (msgType)
    {
    case SSH2_FXP_INIT:
      DoInit();
      break;
    case SSH2_FXP_OPEN:
      DoOpen();
      break;
    case SSH2_FXP_CLOSE:
      DoClose();
      break;
    case SSH2_FXP_READ:
      DoRead();
      break;
    case SSH2_FXP_WRITE:
      DoWrite();
      break;
    case SSH2_FXP_LSTAT:
      DoStat(lstat);
      break;
    case SSH2_FXP_FSTAT:
      DoFStat();
      break;
    case SSH2_FXP_SETSTAT:
      DoSetStat();
      break;
    case SSH2_FXP_FSETSTAT:
      DoFSetStat();
      break;
    case SSH2_FXP_OPENDIR:
      DoOpenDir();
      break;
    case SSH2_FXP_READDIR:
      DoReadDir();
      break;
    case SSH2_FXP_REMOVE:
      DoRemove();
      break;
    case SSH2_FXP_MKDIR:
      DoMkDir();
      break;
    case SSH2_FXP_RMDIR:
      DoRmDir();
      break;
    case SSH2_FXP_REALPATH:
      DoRealPath();
      break;
    case SSH2_FXP_STAT:
      DoStat(stat);
      break;
    case SSH2_FXP_RENAME:
      DoRename();
      break;
    case SSH2_FXP_READLINK:
      DoReadLink();
      break;
    case SSH2_FXP_SYMLINK:
      DoSymLink();
      break;
    case SSH2_FXP_EXTENDED:
      DoExtended();
      break;
      
    default:
      DoUnsupported(msgType, msgLen);
      break;
    }
  if ((bIn->read - oldRead) < msgLen)//read entire message
    {
      DEBUG((MYLOG_DEBUG, "ZAP DATA len:%i [bIn->read=%i, oldRead=%i]",
	     msgLen - (bIn->read - oldRead), bIn->read, oldRead));
      BufferReadData(bIn, msgLen - (bIn->read - oldRead));
    }
  BufferClean(bIn);
  goto parsePacket;
}

int			main(int ac, char **av)
{
  struct timeval	tm;
  fd_set		fdR, fdW;
  int			len, ret;

  bIn = BufferNew();
  bOut = BufferNew();
  HandleInit();
  parse_conf(ac, av);
  SET_TIMEOUT(tm, 1, 0);
  for (;;)
    {
      FD_ZERO(&fdR);
      FD_ZERO(&fdW);
      
      if (!gl_var->upload_max || (gl_var->upload_current < gl_var->upload_max))
	FD_SET(0, &fdR);
      if (bOut->length > 0 && (!gl_var->download_max
			       || (gl_var->download_current < gl_var->download_max)))
	FD_SET(1, &fdW);
      if ((ret = select(2, &fdR, &fdW, 0, &tm)) == -1)
	exit (1);
      else if (!ret)
	{
	  if (gl_var->upload_current || gl_var->download_current)
	    gl_var->who->time_transf++;
	  else
	    gl_var->who->time_idle++;
	  gl_var->who->upload_current = gl_var->upload_current;
	  gl_var->who->download_current = gl_var->download_current;
	  gl_var->upload_current = 0;
	  gl_var->download_current = 0;
	  gl_var->who->time_total = time(0) - gl_var->who->time_begin;
	  if (gl_var->who->time_maxidle &&
	      gl_var->who->time_idle >= gl_var->who->time_maxidle)
	    {
	      log_printf(MYLOG_NORMAL, "[%s][%s]Connection time out",
			 gl_var->who->user, gl_var->who->ip);
	      exit (0);
	    }
	  if (gl_var->who->time_idle > 2)
	    {
	      gl_var->who->time_transf = 0;
	      gl_var->who->upload_current = 0;
	      gl_var->who->download_current = 0;
	    }
	  SET_TIMEOUT(tm, 1, 0);
	  SftpWhoCleanBuggedClient();
	  
	  gl_var->download_max = gl_var->who->download_max;
	  if (_sftpglobal->download_by_client && !(gl_var->who->status & SFTPWHO_BYPASS_GLB_DWN) &&
	      ((_sftpglobal->download_by_client < gl_var->download_max) || !gl_var->download_max))
	    gl_var->download_max = _sftpglobal->download_by_client;
	  
	  gl_var->upload_max = gl_var->who->upload_max;
	  if (_sftpglobal->upload_by_client && !(gl_var->who->status & SFTPWHO_BYPASS_GLB_UPL) &&
	      ((_sftpglobal->upload_by_client < gl_var->upload_max) || !gl_var->upload_max))
	    gl_var->upload_max = _sftpglobal->upload_by_client;
	}
      else
	{
	  gl_var->who->time_idle = 0;
	  if (FD_ISSET(0, &fdR))
	    {
	      char	buffer[16384];
	      int	todo;
	      
	      if (gl_var->upload_max)
		todo = sizeof(buffer) < (gl_var->upload_max - gl_var->upload_current) ?
		  sizeof(buffer) : (gl_var->upload_max - gl_var->upload_current);
	      else
		todo = sizeof(buffer);
	      len = read(0, buffer, todo);
	      if (len < 0)
		exit(1);
	      else if (len == 0)
		exit(1);
	      else
		{
		  BufferPutRawData(bIn, buffer, len);
		  gl_var->upload_current += len;
		  gl_var->who->upload_total += len;
		}
	      DoProtocol();
	    }
	  if (FD_ISSET(1, &fdW))
	    {
	      int	len = bOut->length - bOut->read;
	      
	      //DEBUG((MYLOG_DEBUG, "PID:%i Down [%i:%i:%i - %i:%i]", getpid(), gl_var->download_current, gl_var->download_max, gl_var->who->download_max,_sftpglobal->download_by_client, _sftpglobal->download_max));
	      if (gl_var->download_max)
		len = len < (gl_var->download_max - gl_var->download_current) ?
		  len : (gl_var->download_max - gl_var->download_current);
	      len = write(1, bOut->data + bOut->read, len);
	      if (len < 0)
		exit(1);
	      else
		bOut->read += len;
	      BufferClean(bOut);
	      gl_var->download_current += len;
	      gl_var->who->download_total += len;
	      if (gl_var->down_max > 0)
		{
		  gl_var->down_size += len;
		  if (gl_var->down_size > gl_var->down_max)
		    gl_var->down_size = gl_var->down_max;
		  gl_var->who->dowload_pos = gl_var->down_size * 100 / gl_var->down_max;
		}
	    }
	}
    }
  return (0);
}
