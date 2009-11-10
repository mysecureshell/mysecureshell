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
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>

#if STAT_MACROS_BROKEN
# undef S_ISDIR
#endif

#if !defined S_ISDIR && defined S_IFDIR
# define S_ISDIR(Mode) (((Mode) & S_IFMT) == S_IFDIR)
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif

#include "Encode.h"
#include "Stats.h"
#include "Admin.h"
#include "Defines.h"
#include "Encoding.h"
#include "Handle.h"
#include "Log.h"
#include "Send.h"
#include "SftpExt.h"
#include "SftpServer.h"
#include "SftpWho.h"
#include "Util.h"
#include "../security.h"

#define CONN_INIT	0
#define CONN_SFTP	1
#define	CONN_ADMIN	2

tBuffer		*bIn = NULL;
tBuffer		*bOut = NULL;
u_int32_t	cVersion = SSH2_FILEXFER_VERSION;
static char	connectionStatus = CONN_INIT;
static tStats	*stats = NULL;


void	DoInit()
{
  u_int32_t	clientVersion;
  tBuffer	*b;
	
  clientVersion = BufferGetInt32(bIn);
  b = BufferNew();
  BufferPutInt8FAST(b, SSH2_FXP_VERSION);
  connectionStatus = CONN_SFTP;
  cVersion = clientVersion;
#ifdef MSS_HAVE_ADMIN
  if (clientVersion == SSH2_ADMIN_VERSION)
    {
      if (HAS_BIT(gl_var->flagsGlobals, SFTPWHO_IS_ADMIN)
	  || HAS_BIT(gl_var->flagsGlobals, SFTPWHO_IS_SIMPLE_ADMIN))
	{
	  connectionStatus = CONN_ADMIN;
	  cVersion = clientVersion;
	  if (!HAS_BIT(gl_var->flagsGlobals, SFTPWHO_IS_ADMIN))
	    cVersion = SSH2_SIMPLE_ADMIN_VERSION;
	  DEBUG((MYLOG_DEBUG, "[DoInit]New admin [use version: %i]", cVersion));
	  //Hide admin to sftp-who !
	  gl_var->who->status = SFTPWHO_EMPTY;
	  gl_var->who = NULL;
	  stats = StatsNew();
	  BufferPutInt32(b, cVersion);
	  BufferPutString(b, "space-available");
#ifdef MSSEXT_FILE_HASHING
	  BufferPutString(b, "check-file");
#endif
	}
    }
#endif
  if (connectionStatus == CONN_SFTP)
    {
      DoInitUser();
      if (cVersion < 3)
	cVersion = 3;
      else if (cVersion > SSH2_FILEXFER_VERSION)
	cVersion = SSH2_FILEXFER_VERSION;
      BufferPutInt32(b, cVersion);
      DEBUG((MYLOG_DEBUG, "[DoInit]New client version: %i [use: %i]", clientVersion, cVersion));
      if (cVersion >= 4)
	{
	  BufferPutString(b, "newline");
	  BufferPutString(b, "\n");
	  if (cVersion >= 5)
	    {
	      tBuffer	*opt;
	      
	      BufferPutString(b, "supported");
	      opt = BufferNew();
	      BufferPutInt32(opt, SSH5_FILEXFER_ATTR__MASK);
	      BufferPutInt32(opt, SSH5_FILEXFER_ATTR__BITS);
	      BufferPutInt32(opt, SSH5_FXF__FLAGS);
	      BufferPutInt32(opt, SSH5_FXF_ACCESS__FLAGS);
	      BufferPutInt32(opt, SSH2_MAX_READ);
	      BufferPutString(opt, "space-available");
#ifdef MSSEXT_FILE_HASHING
	      BufferPutString(opt, "check-file");
#endif
	      BufferPutPacket(b, opt);
	    }
	  else
	    {
	      BufferPutString(b, "space-available");
	      BufferPutString(b, "");
#ifdef MSSEXT_FILE_HASHING
	      BufferPutString(b, "check-file");
	      BufferPutString(b, "");
#endif
	    }
	}
    }
  BufferPutPacket(bOut, b);
  BufferDelete(b);
}

void	DoRealPath()
{
  u_int32_t	id;
  char		resolvedName[PATH_MAX];
  char		*path;
	
  id = BufferGetInt32(bIn);
  path = convertFromUtf8(BufferGetString(bIn), 1);
  resolvedName[0] = '\0';
  if (path[0] == '\0')
    {
      free(path);
      path = strdup(".");
    }
  if (realpath(path, resolvedName) == 0)
    SendStatus(bOut, id, errnoToPortable(errno));
  else
    {
      tStat	s;
      
      memset(&s, 0, sizeof(s));
      if (strcmp(path, ".") != 0 && strcmp(path, "./.") != 0)
	ResolvPath(path, resolvedName, sizeof(resolvedName));
      if (cVersion >= 4)
	s.name = convertToUtf8(resolvedName, 0);
      else
	{
	  s.name = s.longName = resolvedName;
	}
      SendStats(bOut, id, 1, &s);
      if (cVersion >= 4)
	free(s.name);
    }
  DEBUG((MYLOG_DEBUG, "[DoRealPath]path:'%s' -> '%s'", path, resolvedName));
  free(path);
}

void	DoOpenDir()
{
  u_int32_t	id;
  tHandle	*hdl;
  char		*path;
  DIR		*dir;
  int		status = (cVersion <= 3 ? SSH2_FX_FAILURE : SSH4_FX_INVALID_HANDLE);
	
  id = BufferGetInt32(bIn);
  path = convertFromUtf8(BufferGetString(bIn), 1);
  if ((status = CheckRules(path, RULES_DIRECTORY, 0, O_RDONLY)) == SSH2_FX_OK)
    {
      if ((dir = opendir(path)) == NULL)
	status = errnoToPortable(errno);
      else
	{
	  if ((hdl = HandleNewDirectory(path, dir)) == NULL)
	    {
	      (void )closedir(dir);
	      status = errnoToPortable(EMFILE);
	    }
	  else
	    {
	      (void )snprintf(gl_var->who->path, sizeof(gl_var->who->path), "%s", path);
	      SendHandle(bOut, id, hdl->id);
	      status = SSH2_FX_OK;
	    }
	}
    }
  DEBUG((MYLOG_DEBUG, "[DoOpenDir]path:'%s' status:%i", path, status));
  if (status != SSH2_FX_OK)
    {
      SendStatus(bOut, id, status);
      free(path);
    }
}

void	DoReadDir()
{
  struct dirent	*dp;
  u_int32_t	id;
  tHandle	*hdl;
  int		h;

  id = BufferGetInt32(bIn);
  h = BufferGetHandle(bIn);
  if (HAS_BIT(gl_var->flagsDisable, SFTP_DISABLE_READ_DIR))
    {
      DEBUG((MYLOG_DEBUG, "[DoReadDir]Disabled by conf."));
      SendStatus(bOut, id, SSH2_FX_PERMISSION_DENIED);
    }
  else if ((hdl = HandleGetDir(h)) == NULL)
    {
      DEBUG((MYLOG_DEBUG, "[DoReadDir]handle:%i", h));
      SendStatus(bOut, id, (cVersion <= 3 ? SSH2_FX_FAILURE : SSH4_FX_INVALID_HANDLE));
    }
  else
    {
      struct stat	st;
      char		pathName[1024];
      tStat 		*s;
      int		nstats = 100, count = 0, i, len;
      
      DEBUG((MYLOG_DEBUG, "[DoReadDir]path:'%s' handle:%i", hdl->path, h));
      s = malloc(nstats * sizeof(tStat));
      len = strlen(hdl->path);
      if ((len + 256) >= sizeof(pathName))
	{
	  SendStatus(bOut, id, SSH2_FX_FAILURE);
	  goto notEnoughtMemory;
	}
      STRCPY(pathName, hdl->path, sizeof(pathName));
      if (pathName[len - 1] != '/')
	{
	  pathName[len] = '/';
	  len++;
	}
      while ((dp = readdir(hdl->dir)) != NULL)
	{
	  STRCPY(pathName + len, dp->d_name, sizeof(pathName) - len);
	  if (HAS_BIT(gl_var->flagsGlobals, SFTPWHO_LINKS_AS_LINKS))
	    {
	      if (lstat(pathName, &st) < 0)
		{
		  DEBUG((MYLOG_DEBUG, "[DoReadDir]ERROR lstat(%s): %s", pathName, strerror(errno)));
		  continue;
		}
	    }
	  else
	    {
	      if (stat(pathName, &st) < 0)
		{
		  DEBUG((MYLOG_DEBUG, "[DoReadDir]ERROR stat(%s): %s", pathName, strerror(errno)));
		  continue;
		}
	    }
	  if ((dp->d_name[0] == '.' && (dp->d_name[1] == '\0'
					|| (dp->d_name[1] == '.' && dp->d_name[2] == '\0')))
	      || CheckRules(pathName, RULES_LISTING, &st, 0) == SSH2_FX_OK)
	    {
	      ChangeRights(&st);
	      StatToAttributes(&st, &(s[count].attributes), pathName);
	      s[count].name = convertToUtf8(dp->d_name, 0);
	      if (cVersion <= 3)
		s[count].longName = LsFile(dp->d_name, &st);
	      DEBUG((MYLOG_DEBUG, "[DoReadDir] -> '%s' handle:%i [%i]", pathName, h, count));
	      count++;
	      if (count == nstats)
		break;
	    }
#ifdef DODEBUG
	  else
	    DEBUG((MYLOG_DEBUG, "[DoReadDir] REFUSED -> '%s' handle:%i [%i]", pathName, h, count));
#endif
	}
      if (count > 0)
	{
	  SendStats(bOut, id, count, s);
	  for (i = 0; i < count; i++)
	    {
	      free(s[i].name);
	      if (cVersion <= 3)
		free(s[i].longName);
	    }
	}
      else	
	SendStatus(bOut, id, SSH2_FX_EOF);
    notEnoughtMemory:
      free(s);
    }
}

void	DoClose()
{
  u_int32_t	id;
  tHandle	*hdl;
  int		h, pourcentage;
  int		status = (cVersion <= 3 ? SSH2_FX_FAILURE : SSH4_FX_INVALID_HANDLE);
	
  id = BufferGetInt32(bIn);
  h = BufferGetHandle(bIn);
  if ((hdl = HandleGet(h)) != NULL)
    status = SSH2_FX_OK;
  SendStatus(bOut, id, status);
  if (hdl->state == HANDLE_FILE)
    {
      if (hdl->fileSize > 0)
	pourcentage = hdl->filePos * 100 / hdl->fileSize;
      else
	pourcentage = 0;
      if (FILE_IS_UPLOAD(hdl->flags))
	{
	  mylog_printf(MYLOG_TRANSFERT, "[%s][%s]End upload into file '%s'",
		       gl_var->who->user, gl_var->who->ip, hdl->path);
	}
      else
	{
	  mylog_printf(MYLOG_TRANSFERT, "[%s][%s]End download file '%s' : %i%%",
		       gl_var->who->user, gl_var->who->ip, hdl->path,
		       pourcentage);
	  BufferSetFastClean(bIn, 0);
	  BufferSetFastClean(bOut, 0);
	}
      HandleClose(h);
      UpdateInfoForOpenFiles();
    }
  else
      HandleClose(h);
  DEBUG((MYLOG_DEBUG, "[DoClose] -> handle:%i status:%i", h, status));
}

void	DoOpen()
{
  u_int32_t	id, pflags;
  tAttributes	*a;
  tHandle	*hdl;
  struct stat	st;
  char		*path;
  int		fd, flags, mode, textMode, status = SSH2_FX_FAILURE;

  id = BufferGetInt32(bIn);
  path = convertFromUtf8(BufferGetString(bIn), 1);
  if (cVersion >= 5)
    flags = FlagsFromAccess(BufferGetInt32(bIn));
  else
    flags = 0;
  pflags = BufferGetInt32(bIn);
  a = GetAttributes(bIn);
  flags |= FlagsFromPortable(pflags, &textMode);
  mode = gl_var->rights_file ? gl_var->rights_file :
    (a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS) ? a->perm : 0644;
  mode |= gl_var->minimum_rights_file;
  mode &= gl_var->maximum_rights_file;
  if ((HAS_BIT(gl_var->flagsDisable, SFTP_DISABLE_READ_FILE)
		&& HAS_BIT(pflags, SSH2_FXF_READ))
	|| (HAS_BIT(gl_var->flagsDisable, SFTP_DISABLE_WRITE_FILE)
		&& HAS_BIT(pflags, SSH2_FXF_WRITE))
	|| (HAS_BIT(gl_var->flagsDisable, SFTP_DISABLE_OVERWRITE)
		&& HAS_BIT(pflags, SSH2_FXF_APPEND)
		&& stat(path, &st) != -1)
	)
    {
      DEBUG((MYLOG_DEBUG, "[DoOpen]Disabled by conf."));
      status = SSH2_FX_PERMISSION_DENIED;
    }
  else if ((status = CheckRules(path, RULES_FILE, 0, flags)) == SSH2_FX_OK)
    if ((status = CheckRulesAboutMaxFiles()) == SSH2_FX_OK)
      {
	if ((fd = open(path, flags, mode)) < 0)
	  status = errnoToPortable(errno);
	else
	  {
	    if ((hdl = HandleNewFile(path, fd, textMode, flags)) == NULL)
	      {
		xclose(fd);
		status = errnoToPortable(EMFILE);
	      }
	    else
	      {
		if (FILE_IS_UPLOAD(flags))
		  {
		    mylog_printf(MYLOG_TRANSFERT, "[%s][%s]Start upload into file '%s'",
			       gl_var->who->user, gl_var->who->ip, path);
		  }
		else
		  {
		    mylog_printf(MYLOG_TRANSFERT, "[%s][%s]Start download file '%s'",
			       gl_var->who->user, gl_var->who->ip, path);
		    BufferSetFastClean(bIn, 1);
		    BufferSetFastClean(bOut, 1);
		  }
		hdl->filePos = 0;
		hdl->fileSize = 0;
		if (stat(path, &st) != -1)
		  hdl->fileSize = st.st_size;
		UpdateInfoForOpenFiles();
		SendHandle(bOut, id, hdl->id);
		status = SSH2_FX_OK;
	      }
	  }
      }
  DEBUG((MYLOG_DEBUG, "[DoOpen]file:'%s' pflags:%i[%i] perm:0%o fd:%i status:%i",
	 path, pflags, flags, mode, fd, status));
  if (status != SSH2_FX_OK)
    {
      SendStatus(bOut, id, status);
      free(path);
    }
}

void	DoRead()
{
  u_int32_t	id, len;
  u_int64_t	off, pos;
  tHandle	*hdl;
  int		h, status;
  
  id = BufferGetInt32(bIn);
  h = BufferGetHandle(bIn);
  off = BufferGetInt64(bIn);
  if ((len = BufferGetInt32(bIn)) > SSH2_MAX_READ)
    len = SSH2_MAX_READ;
  if ((hdl = HandleGetFile(h)) != NULL)
    {
      pos = 0;
      if (hdl->fileIsText == 0 && (pos = lseek(hdl->fd, off, SEEK_SET)) < 0)
	status = errnoToPortable(errno);
      else
	{ 
	  unsigned char	*buf;
	  u_int32_t	dataSize, oldPos, newPos;
	  int		ret;
	  
	  oldPos = BufferGetCurrentWritePosition(bOut);
	  dataSize = 1 + 4 + 4 + len;
	  BufferEnsureFreeCapacity(bOut, 4 + dataSize);
	  BufferPutInt32(bOut, 0);//Size of the packet - unknown before read
	  BufferPutInt8FAST(bOut, SSH2_FXP_DATA);
	  BufferPutInt32(bOut, id);
	  BufferPutInt32(bOut, 0);//Size of the data - unknown before read
	  buf = BufferGetWritePointer(bOut);
	  ret = read(hdl->fd, buf, len);
	  if (hdl->fileIsText == 1)
	    {
	      for (len = 0; len < ret; len++)
		if (buf[len] == '\r')
		  {
		    memcpy(buf + len + 1, buf + len, ret - len - 1);
		    ret--;
		  }
	    }
	  if (ret <= 0)
	    {
	      status = ret == 0 ? SSH2_FX_EOF : errnoToPortable(errno);
	      bOut->length = oldPos; //Cancel all uncomplete data
	    }
	  else
	    {
	      hdl->filePos = off + ret;
	      UpdateInfoForOpenFiles();
	      newPos = BufferGetCurrentWritePosition(bOut) + (u_int32_t )ret;
	      BufferSetCurrentWritePosition(bOut, oldPos);
	      dataSize = 1 + 4 + 4 + (u_int32_t )ret;
	      BufferPutInt32(bOut, dataSize);//Size of the packet
	      BufferIncrCurrentWritePosition(bOut, 5);//sizeof(SSH2_FXP_DATA) + sizeof(id)
	      BufferPutInt32(bOut, (u_int32_t )ret);//Size of the data
	      BufferSetCurrentWritePosition(bOut, newPos);
	      status = SSH2_FX_OK;
	    }
	  //DEBUG((MYLOG_WARNING, "[DoRead]fd:%i[isText:%i] off:%llu len:%i (ret:%i) status:%i", fd, fileIsText, off, len, ret, status));
	}
    }
  else
    status = (cVersion <= 3 ? SSH2_FX_FAILURE : SSH4_FX_INVALID_HANDLE);
  if (status != SSH2_FX_OK)
    SendStatus(bOut, id, status);
}

void	DoWrite()
{
  u_int64_t	off, pos;
  u_int32_t	id, dec, len;
  tHandle	*hdl;
  ssize_t	ret;
  int		status, h;
  char		*data;

  id = BufferGetInt32(bIn);
  h = BufferGetHandle(bIn);
  off = BufferGetInt64(bIn);
  data = BufferGetData(bIn, &len);
  if ((hdl = HandleGetFile(h)) != NULL)
    {
      if (hdl->fileIsText == 0 && (pos = lseek(hdl->fd, off, SEEK_SET)) < 0)
	status = errnoToPortable(errno);
      else
	{
	  if (hdl->fileIsText == 1)
	    {
	      for (dec = 0; (dec + 1) < len; dec++)
		if (data[dec] == '\r' && data[dec + 1] == '\n')
		  {
		    memcpy(data + dec, data + dec + 1, len - dec - 1);
		    len--;
		  }
	    }
	  ret = write(hdl->fd, data, len);
	  if (ret == -1)
	    status = errnoToPortable(errno);
	  else if (ret == len)
	    status = SSH2_FX_OK;
	  else
	    status = SSH2_FX_FAILURE;
	  hdl->filePos = off + ret;
	  UpdateInfoForOpenFiles();
	}
    }
  else
    status = (cVersion <= 3 ? SSH2_FX_FAILURE : SSH4_FX_INVALID_HANDLE);
  //DEBUG((MYLOG_DEBUG, "[DoWrite]hdl:%p off:%llu len:%i ret:%i status:%i", hdl, off, len, ret, status));
  SendStatus(bOut, id, status);
}

void	DoReadLink()
{
  u_int32_t	id, status;
  char		readLink[PATH_MAX];
  char		*path;
  int		len;

  id = BufferGetInt32(bIn);
  path = convertFromUtf8(BufferGetString(bIn), 1);
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

void	DoStat(int (*f_stat)(const char *, struct stat *))
{
  tAttributes	a;
  struct stat	st;
  u_int32_t	id, status, flags = 0;
  char		*path;
  int		r;
  
  id = BufferGetInt32(bIn);
  path = convertFromUtf8(BufferGetString(bIn), 1);
  if (cVersion >= 4)
    flags = BufferGetInt32(bIn);
  if ((status = CheckRules(path, RULES_NONE, 0, 0)) == SSH2_FX_OK)
    {
      if ((r = f_stat(path, &st)) < 0)
	SendStatus(bOut, id, errnoToPortable(errno));
      else
	{
	  ChangeRights(&st);
	  StatToAttributes(&st, &a, path);
	  if (cVersion >= 4)
	    a.flags = flags;
	  SendAttributes(bOut, id, &a, path);
	}
    }
  else
    SendStatus(bOut, id, status);
  DEBUG((MYLOG_DEBUG, "[Do%sStat]path:'%s' -> '%i'", f_stat == stat ? "" : "L", path, r));
  free(path);
}

void	DoFStat()
{
  tAttributes	a;
  struct stat	st;
  u_int32_t	id, flags = 0;
  tHandle	*hdl;
  int		fh;
	
  id = BufferGetInt32(bIn);
  fh = BufferGetHandle(bIn);
  if (cVersion >= 4)
    flags = BufferGetInt32(bIn);
  if ((hdl = HandleGetFile(fh)) != NULL)
    {
      int	r;
      
      if ((r = fstat(hdl->fd, &st)) < 0)
	SendStatus(bOut, id, errnoToPortable(errno));
      else
	{
	  char	*path = hdl->path;

	  StatToAttributes(&st, &a, path);
	  if (cVersion >= 4)
	    a.flags = flags;
	  SendAttributes(bOut, id, &a, path);
	}
      DEBUG((MYLOG_DEBUG, "[DoFStat]fd:'%i' -> '%i'", hdl->fd, r));
    }
  else
    SendStatus(bOut, id, (cVersion <= 3 ? SSH2_FX_FAILURE : SSH4_FX_INVALID_HANDLE));
}

void 	DoSetStat(int usePath)
{
  tAttributes	*a;
  u_int32_t	id;
  tHandle	*hdl = NULL;
  char		*path = NULL;
  int		status = SSH2_FX_OK;
  struct stat	stats;

  id = BufferGetInt32(bIn);
  if (usePath == 1)
    path = convertFromUtf8(BufferGetString(bIn), 1);
  else if ((hdl = HandleGet(BufferGetHandle(bIn))) != NULL)
    path = hdl->path;
  a = GetAttributes(bIn);
  if (usePath == 0 && hdl == NULL)
    status = (cVersion <= 3 ? SSH2_FX_FAILURE : SSH4_FX_INVALID_HANDLE);
  else if (HAS_BIT(gl_var->flagsDisable, SFTP_DISABLE_SET_ATTRIBUTE))
    {
      DEBUG((MYLOG_DEBUG, "[DoSetStat]Disabled by conf."));
      status = SSH2_FX_PERMISSION_DENIED;
    }
  else if ((status = CheckRules(path, RULES_NONE, 0, 0)) == SSH2_FX_OK)
    {
      if (a->flags & SSH2_FILEXFER_ATTR_SIZE)
	{
	  if (truncate(path, a->size) == -1)
	    status = errnoToPortable(errno);
	}
      if (a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS
	  && HAS_BIT(gl_var->flagsGlobals, SFTPWHO_CAN_CHG_RIGHTS))
	{
	  if (stat(path, &stats) == 0 && S_ISDIR(stats.st_mode))
	  {
	    a->perm |= gl_var->minimum_rights_directory;
	    a->perm &= gl_var->maximum_rights_directory;
	  }
	  else
	  {
	    a->perm |= gl_var->minimum_rights_file;
	    a->perm &= gl_var->maximum_rights_file;
	  }
	  if (chmod(path, (a->perm & 0777)) == -1)
	    status = errnoToPortable(errno);
	}
      if (a->flags & SSH2_FILEXFER_ATTR_ACMODTIME
	  && HAS_BIT(gl_var->flagsGlobals, SFTPWHO_CAN_CHG_TIME))
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
  DEBUG((MYLOG_DEBUG, "[DoSetStat]path:'%s'[hdl: %p] -> '%i'",
          path, hdl, status));
  SendStatus(bOut, id, status);
  if (usePath == 1)
    free(path);
}

void	DoRemove()
{
  u_int32_t	id;
  char		*path;
  int		status = SSH2_FX_OK;

  id = BufferGetInt32(bIn);
  path = convertFromUtf8(BufferGetString(bIn), 1);
  if (HAS_BIT(gl_var->flagsDisable, SFTP_DISABLE_REMOVE_FILE))
    {
      DEBUG((MYLOG_DEBUG, "[DoRemove]Disabled by conf."));
      status = SSH2_FX_PERMISSION_DENIED;
    }
  else if ((status = CheckRules(path, RULES_RMFILE, 0, 0)) == SSH2_FX_OK)
    {
      if (HAS_BIT(gl_var->flagsGlobals, SFTPWHO_CAN_RMFILE))
	{
	  if (unlink(path) == -1)
	    status = errnoToPortable(errno);
	  mylog_printf(MYLOG_WARNING, "[%s][%s]Try to remove file '%s' : %s",
		       gl_var->who->user, gl_var->who->ip, path,
		       (status != SSH2_FX_OK ? strerror(errno) : "success"));
	}
      else
	status = SSH2_FX_PERMISSION_DENIED;
    }
  DEBUG((MYLOG_DEBUG, "[DoRemove]path:'%s' -> '%i'", path, status));
  SendStatus(bOut, id, status);
  free(path);
}

void	DoMkDir()
{
  tAttributes	*a;
  u_int32_t	id;
  char		*path;
  int		mode, status = SSH2_FX_OK;

  id = BufferGetInt32(bIn);
  path = convertFromUtf8(BufferGetString(bIn), 1);
  a = GetAttributes(bIn);
  mode = gl_var->rights_directory ? gl_var->rights_directory :
    (a->flags & SSH2_FILEXFER_ATTR_PERMISSIONS) ? a->perm & 0777 : 0755;
  mode |= gl_var->minimum_rights_directory;
  mode &= gl_var->maximum_rights_directory;
  if (HAS_BIT(gl_var->flagsDisable, SFTP_DISABLE_MAKE_DIR))
    {
      DEBUG((MYLOG_DEBUG, "[DoMkDir]Disabled by conf."));
      status = SSH2_FX_PERMISSION_DENIED;
    }
  else if ((status = CheckRules(path, RULES_DIRECTORY, 0, O_WRONLY)) == SSH2_FX_OK)
    {
      if (mkdir(path, mode) == -1)
        status = errnoToPortable(errno);
      mylog_printf(MYLOG_WARNING, "[%s][%s]Try to create directory '%s' : %s",
		            gl_var->who->user, gl_var->who->ip, path,
                (status != SSH2_FX_OK ? strerror(errno) : "success"));
    }
  SendStatus(bOut, id, status);
  DEBUG((MYLOG_DEBUG, "[DoMkDir]path:'%s', mode:%o -> '%i'", path, mode, status));
  free(path);
}

void	DoRmDir()
{
  u_int32_t	id;
  char		*path;
  int		status = SSH2_FX_OK;

  id = BufferGetInt32(bIn);
  path = convertFromUtf8(BufferGetString(bIn), 1);
  if (HAS_BIT(gl_var->flagsDisable, SFTP_DISABLE_REMOVE_DIR))
    {
      DEBUG((MYLOG_DEBUG, "[DoRmDir]Disabled by conf."));
      status = SSH2_FX_PERMISSION_DENIED;
    }
  else if ((status = CheckRules(path, RULES_RMDIRECTORY, 0, 0)) == SSH2_FX_OK)
    {
      if (HAS_BIT(gl_var->flagsGlobals, SFTPWHO_CAN_RMDIR))
	{
	  if (rmdir(path) == -1)
	    status = errnoToPortable(errno);
	  mylog_printf(MYLOG_WARNING, "[%s][%s]Try to remove directory '%s' : %s",
		       gl_var->who->user, gl_var->who->ip, path, (status != SSH2_FX_OK ? strerror(errno) : "success"));
	}
      else
	status = SSH2_FX_PERMISSION_DENIED;
    }
  SendStatus(bOut, id, status);
  DEBUG((MYLOG_DEBUG, "[DoRmDir]path:'%s' -> '%i'", path, status));
  free(path);
}

void	DoRename()
{
  struct stat	sb;
  u_int32_t	id;
  char		*oldPath, *newPath;
  int		flags = 0, status = SSH2_FX_FAILURE;

  id = BufferGetInt32(bIn);
  oldPath = convertFromUtf8(BufferGetString(bIn), 1);
  newPath = convertFromUtf8(BufferGetString(bIn), 1);
  if (cVersion >= 5)
    flags = BufferGetInt32(bIn);
  if (HAS_BIT(gl_var->flagsDisable, SFTP_DISABLE_RENAME))
    {
      DEBUG((MYLOG_DEBUG, "[DoRename]Disabled by conf."));
      status = SSH2_FX_PERMISSION_DENIED;
    }
  else if ((status = CheckRules(oldPath, RULES_RMFILE, 0, 0)) == SSH2_FX_OK
      && (status = CheckRules(newPath, RULES_FILE, 0, O_WRONLY)) == SSH2_FX_OK)
    {
      if (stat(newPath, &sb) == 0 && HAS_BIT(flags, SSH5_FXP_RENAME_OVERWRITE))
	if (unlink(newPath) == -1)
	  {
	    status = errnoToPortable(errno);
	    goto dont_rename;
	  }
      if (rename(oldPath, newPath) == -1)
	status = errnoToPortable(errno);
      else
	status = SSH2_FX_OK;
    dont_rename:
      mylog_printf(MYLOG_WARNING, "[%s][%s]Try to rename '%s' -> '%s' : %s",
		   gl_var->who->user, gl_var->who->ip, oldPath, newPath,
		   (status != SSH2_FX_OK ? strerror(errno) : "success"));
    }
  else
    status = SSH2_FX_PERMISSION_DENIED;
  DEBUG((MYLOG_DEBUG, "[DoRename]oldPath:'%s' newPath:'%s' -> '%i'", oldPath, newPath, status));
  SendStatus(bOut, id, status);
  free(oldPath);
  free(newPath);
}

void	DoSymLink()
{
  u_int32_t	id;
  char		*oldPath, *newPath;
  int		status = SSH2_FX_OK;

  id = BufferGetInt32(bIn);
  newPath = convertFromUtf8(BufferGetString(bIn), 1);
  oldPath = convertFromUtf8(BufferGetString(bIn), 1);
  if (HAS_BIT(gl_var->flagsDisable, SFTP_DISABLE_SYMLINK))
    {
      DEBUG((MYLOG_DEBUG, "[DoSymLink]Disabled by conf."));
      status = SSH2_FX_PERMISSION_DENIED;
    }
  else if ((status = CheckRules(oldPath, RULES_FILE, 0, O_RDONLY)) == SSH2_FX_OK
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

void	DoUnsupported(int msgType, int msgLen)
{
  u_int32_t	id;
		
  id = BufferGetInt32(bIn);
  SendStatus(bOut, id, SSH2_FX_OP_UNSUPPORTED);
  DEBUG((MYLOG_DEBUG, "[DoUnsupported]msgType:%i msgLen:%i", msgType, msgLen));
}

void	DoExtended()
{
  u_int32_t	id;
  char		*request;
	
  id = BufferGetInt32(bIn);
  request = BufferGetString(bIn);
  DEBUG((MYLOG_DEBUG, "[DoExtended]request:'%s'", request));
#ifdef MSSEXT_DISKUSAGE
  if (strcmp(request, "space-available") == 0)
      DoExtDiskSpace(bIn, bOut, id);
  else
#endif
#ifdef MSSEXT_FILE_HASHING
    if (strcmp(request, "check-file-handle") == 0)
      DoExtFileHashing_Handle(bIn, bOut, id);
    else if (strcmp(request, "check-file-name") == 0)
      DoExtFileHashing_Name(bIn, bOut, id);
    else
#endif
    SendStatus(bOut, id, SSH2_FX_OP_UNSUPPORTED);
  free(request);
}

void	DoSFTPProtocol()
{
  int		oldRead, msgLen, msgType;

 parsePacket:
  if ((bIn->length - bIn->read) < 5) //header too small
    {
      BufferClean(bIn);
      return;
    }
  oldRead = bIn->read;
  msgLen = BufferGetInt32(bIn);
  if (msgLen > (256 * 1024)) //message too long
    {
      mylog_printf(MYLOG_ERROR, "[%s][%s]Error: message is too long (%i)", gl_var->who->user, gl_var->who->ip, msgLen);
      exit(1);
    }
  if ((bIn->length - bIn->read) < msgLen) //message not complete
    {
      bIn->read = oldRead;//cancel read
      return;
    }
  oldRead += 4; //ignore size of msgLen
  msgType = BufferGetInt8FAST(bIn);
  //DEBUG((MYLOG_DEBUG, "[DoSFTPProtocol] msgType:%i msgLen:%i", msgType, msgLen));
  if (connectionStatus == CONN_INIT)
    {
      switch (msgType)
        {
        case SSH2_FXP_INIT: DoInit(); break;
	default: DoUnsupported(msgType, msgLen); break;
	}
    }
  else if (connectionStatus == CONN_SFTP)
    {
      switch (msgType)
	{
	case SSH2_FXP_OPEN: DoOpen(); break;
	case SSH2_FXP_CLOSE: DoClose(); break;
	case SSH2_FXP_READ: DoRead(); break;
	case SSH2_FXP_WRITE: DoWrite(); break;
	case SSH2_FXP_LSTAT: DoStat(lstat); break;
	case SSH2_FXP_FSTAT: DoFStat(); break;
	case SSH2_FXP_SETSTAT: DoSetStat(1); break;
	case SSH2_FXP_FSETSTAT: DoSetStat(0); break;
	case SSH2_FXP_OPENDIR: DoOpenDir(); break;
	case SSH2_FXP_READDIR: DoReadDir(); break;
	case SSH2_FXP_REMOVE: DoRemove(); break;
	case SSH2_FXP_MKDIR: DoMkDir(); break;
	case SSH2_FXP_RMDIR: DoRmDir(); break;
	case SSH2_FXP_REALPATH: DoRealPath(); break;
	case SSH2_FXP_STAT: DoStat(stat); break;
	case SSH2_FXP_RENAME: DoRename(); break;
	case SSH2_FXP_READLINK: DoReadLink(); break;
	case SSH2_FXP_SYMLINK: DoSymLink(); break;
	case SSH2_FXP_EXTENDED: DoExtended(); break;
	default: DoUnsupported(msgType, msgLen); break;
	}
    }
#ifdef MSS_HAVE_ADMIN
  else if (connectionStatus == CONN_ADMIN)
    {
      if (cVersion == SSH2_SIMPLE_ADMIN_VERSION)
	{
	  switch (msgType)
	    {
	    case SSH_ADMIN_LIST_USERS: DoAdminListUsers(); break;
	    case SSH_ADMIN_KILL_USER: DoAdminKillUser(); break;
	    case SSH_ADMIN_SERVER_STATUS: DoAdminServerStatus(); break;
	    case SSH_ADMIN_SERVER_GET_STATUS: DoAdminServerGetStatus(); break;
	    case SSH_ADMIN_STATS: DoAdminStats(stats); break;
	    default: DoUnsupported(msgType, msgLen); break;
	    }
	}
      else
	{
	  switch (msgType)
	    {
	    case SSH_ADMIN_LIST_USERS: DoAdminListUsers(); break;
	    case SSH_ADMIN_KILL_USER: DoAdminKillUser(); break;
	    case SSH_ADMIN_SERVER_STATUS: DoAdminServerStatus(); break;
	    case SSH_ADMIN_SERVER_GET_STATUS: DoAdminServerGetStatus(); break;
	    case SSH_ADMIN_GET_LOG_CONTENT: DoAdminGetLogContent(); break;
	    case SSH_ADMIN_CONFIG_GET: DoAdminConfigGet(); break;
	    case SSH_ADMIN_CONFIG_SET: DoAdminConfigSet(); break;
	    case SSH_ADMIN_USER_CREATE: DoAdminUserCreate(); break;
	    case SSH_ADMIN_USER_DELETE: DoAdminUserDelete(); break;
	    case SSH_ADMIN_USER_LIST: DoAdminUserList(); break;
	    case SSH_ADMIN_STATS: DoAdminStats(stats); break;
	    default: DoUnsupported(msgType, msgLen); break;
	    }
	}
    }
#endif
  if ((bIn->read - oldRead) < msgLen)//read entire message
    {
      DEBUG((MYLOG_DEBUG, "ZAP DATA len:%i [bIn->read=%i, oldRead=%i msgLen=%i]",
	     msgLen - (bIn->read - oldRead), bIn->read, oldRead, msgLen));
      BufferReadData(bIn, msgLen - (bIn->read - oldRead));
    }
  goto parsePacket;
}

int			SftpMain(tGlobal *params, int sftpProtocol)
{
  struct timeval	tm;
  long long		tmLast, tmCur, tmNeeded;
  fd_set		fdR, fdW;
  int			len, ret;

  bIn = BufferNew();
  bOut = BufferNew();
  HandleInit();
  ParseConf(params, sftpProtocol);
  tmNeeded = 1000000;
  gettimeofday(&tm, NULL);
  tmLast = tm.tv_sec * (long long )1000000 + tm.tv_usec;
  for (;;)
    {
      FD_ZERO(&fdR);
      FD_ZERO(&fdW);
      if (gl_var->must_shutdown)
	exit(0);
      if (gl_var->upload_max == 0 || (gl_var->upload_current < gl_var->upload_max))
	FD_SET(0, &fdR);
      if (bOut->length > 0 && (gl_var->download_max == 0
			       || (gl_var->download_current < gl_var->download_max)))
	FD_SET(1, &fdW);
      gettimeofday(&tm, NULL);
      tmCur = tm.tv_sec * (long long )1000000 + tm.tv_usec;
      tmCur -= tmLast;
      if (tmCur > tmNeeded)
	{
	  SET_TIMEOUT(tm, 0, 0);
	}
      else if (tmCur == 0)
	{
	  SET_TIMEOUT(tm, 1, 0);
	}
      else
	{
	  tmNeeded -= tmCur;
	  SET_TIMEOUT(tm, 0, tmNeeded);
	}
      //DEBUG((MYLOG_DEBUG, "[select: %i]tmLast: %lli tmCur: %lli tmNeeded: %lli", ret, tmLast, tmCur, tmNeeded));
      if ((ret = select(2, &fdR, &fdW, NULL, &tm)) == -1)
	{
	  if (errno != EINTR)
	    exit(1);
	}
      else if (ret == 0)
	{
	  tmNeeded = 1000000;
	  if (gl_var->who == NULL) //dont check anything for administrator
	    {
	      StatsUpdate(stats);
	      goto bypassChecks;
	    }
	  if (gl_var->upload_current > 0 || gl_var->download_current > 0)
	    gl_var->who->time_transf++;
	  else
	    gl_var->who->time_idle++;

	  gl_var->who->upload_current = gl_var->upload_current;
	  gl_var->who->download_current = gl_var->download_current;
	  gl_var->upload_current = 0;
	  gl_var->download_current = 0;
	  gl_var->who->time_total = time(0) - gl_var->who->time_begin;
	  if (gl_var->who->time_maxidle > 0 &&
	      gl_var->who->time_idle >= gl_var->who->time_maxidle)
	    {
	      mylog_printf(MYLOG_CONNECTION, "[%s][%s]Connection time out",
			 gl_var->who->user, gl_var->who->ip);
	      exit(0);
	    }
	  if (gl_var->who->time_idle > 2)
	    {
	      gl_var->who->time_transf = 0;
	      gl_var->who->upload_current = 0;
	      gl_var->who->download_current = 0;
	    }
	  (void )SftpWhoCleanBuggedClient();
	  
	  gl_var->download_max = gl_var->who->download_max;
	  if (_sftpglobal->download_by_client > 0 && (gl_var->flagsGlobals & SFTPWHO_BYPASS_GLB_DWN) == 0
	      && ((_sftpglobal->download_by_client < gl_var->download_max) || gl_var->download_max == 0))
	    gl_var->download_max = _sftpglobal->download_by_client;

	  gl_var->upload_max = gl_var->who->upload_max;
	  if (_sftpglobal->upload_by_client > 0 && (gl_var->flagsGlobals & SFTPWHO_BYPASS_GLB_UPL) == 0
	      && ((_sftpglobal->upload_by_client < gl_var->upload_max) || gl_var->upload_max == 0))
	    gl_var->upload_max = _sftpglobal->upload_by_client;

	  if (gl_var->who->time_maxlife > 0)
	    {
	      gl_var->who->time_maxlife--;
	      if (gl_var->who->time_maxlife == 0)
		{
		  mylog_printf(MYLOG_CONNECTION, "[%s][%s]Connection max life !",
			       gl_var->who->user, gl_var->who->ip);
		  exit(0);
		}
	    }	      
	}
      else
	{
	  if (gl_var->who != NULL)
	    gl_var->who->time_idle = 0;
	  if (FD_ISSET(0, &fdR))
	    {
	      u_int32_t	todo;
	      
	      if (gl_var->upload_max > 0)
		todo = SSH2_MAX_PACKET < (gl_var->upload_max - gl_var->upload_current) ?
		  SSH2_MAX_PACKET : (gl_var->upload_max - gl_var->upload_current);
	      else
		todo = SSH2_MAX_PACKET;
	      BufferEnsureFreeCapacity(bIn, todo);
	      len = read(0, BufferGetWritePointer(bIn), todo);
	      if (len < 0)
		exit(1);
	      else if (len == 0)
		exit(1);
	      else
		{
		  BufferIncrCurrentWritePosition(bIn, len);
		  if (gl_var->who != NULL)
		    {
		      gl_var->upload_current += len;
		      gl_var->who->upload_total += len;
		    }
		}
	      DoSFTPProtocol();
	    }
	  if (FD_ISSET(1, &fdW))
	    {
	      u_int32_t	todo = bOut->length - bOut->read;

	      if (gl_var->download_max > 0)
		todo = todo < (gl_var->download_max - gl_var->download_current) ?
		  todo : (gl_var->download_max - gl_var->download_current);
	      len = write(1, BufferGetReadPointer(bOut), todo);
	      if (len < 0)
		exit(1);
	      else
		BufferIncrCurrentReadPosition(bOut, len);
	      BufferClean(bOut);
	      if (gl_var->who != NULL)
		{
		  gl_var->download_current += len;
		  gl_var->who->download_total += len;
		}
	    }
	}
    bypassChecks:
      gettimeofday(&tm, NULL);
      tmLast = tm.tv_sec * (long long )1000000 + tm.tv_usec;
    }
  return (0);
}
