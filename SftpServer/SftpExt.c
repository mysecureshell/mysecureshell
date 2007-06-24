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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Buffer.h"
#include "Encoding.h"
#include "Handle.h"
#include "Log.h"
#include "Send.h"
#include "SftpExt.h"
#include "SftpServer.h"
#include "Util.h"
#include "../security.h"


#ifdef MSSEXT_DISKUSAGE
void	DoExtDiskSpace(tBuffer *bIn, tBuffer *bOut, u_int32_t id)
{
  struct STATFS	stfs;
  char		*path;
  int		status;

  path = convertFromUtf8(BufferGetString(bIn), 1);
  if ((status = CheckRules(path, RULES_DIRECTORY, NULL, O_RDONLY)) == SSH2_FX_OK)
    {
      if (STATFS(path, &stfs) == 0)
	{
	  tBuffer       *b;

	  b = BufferNew();
	  BufferPutInt8(b, SSH2_FXP_EXTENDED_REPLY);
	  BufferPutInt32(b, id);
	  BufferPutInt64(b, (u_int64_t )stfs.f_blocks * (u_int64_t )stfs.f_bsize);
	  BufferPutInt64(b, (u_int64_t )stfs.f_bfree * (u_int64_t )stfs.f_bsize);
	  BufferPutInt64(b, 0);
	  BufferPutInt64(b, (u_int64_t )stfs.f_bavail * (u_int64_t )stfs.f_bsize);
	  BufferPutInt32(b, stfs.f_bsize);
	  BufferPutPacket(bOut, b);
	}
      else
	SendStatus(bOut, id, errnoToPortable(errno));
    }
  else
    SendStatus(bOut, id, status);
  free(path);
}
#endif //MSSEXT_DISKUSAGE

#ifdef MSSEXT_FILE_HASHING
#include <openssl/evp.h>

static void DoExtFileHashing_FD(tBuffer *bIn, tBuffer *bOut, u_int32_t id, int fd)
{
  const EVP_MD	*md;
  u_int64_t	offset, length;
  u_int32_t	blockSize;
  char		*algo;

  algo = BufferGetString(bIn);
  offset = BufferGetInt64(bIn);
  length = BufferGetInt64(bIn);
  blockSize = BufferGetInt32(bIn);
  if (lseek(fd, offset, SEEK_SET) == -1)
    {
      SendStatus(bOut, id, errnoToPortable(errno));
      DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Error lseek1"));
      goto endOfFileHashing;
    }
  if (length == 0)//read the file to the end
    {
      u_int64_t	endOfFile;
      
      if ((endOfFile = lseek(fd, 0, SEEK_END)) == -1)
	{
	  SendStatus(bOut, id, errnoToPortable(errno));
	  DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Error lseek2"));
	  goto endOfFileHashing;
	}
      length = endOfFile - offset;
      if (lseek(fd, offset, SEEK_SET) == -1)
	{
	  SendStatus(bOut, id, errnoToPortable(errno));
	  DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Error lseek3"));
	  goto endOfFileHashing;
	}
    }
  if (blockSize == 0)//read length in one time
    blockSize = length;
  DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Algo:%s Fd:%i Offset:%llu Length:%llu BlockSize:%i",
	 algo, fd, offset, length, blockSize));
  if ((md = EVP_get_digestbyname(algo)) != NULL)
    {
      unsigned char	md_value[EVP_MAX_MD_SIZE];
      EVP_MD_CTX	mdctx;
      u_int32_t		md_len;
      tBuffer		*b;
      char		data[SSH2_READ_HASH];
      int		inError = 0;
      
      b = BufferNew();
      BufferPutInt8FAST(b, SSH2_FXP_EXTENDED_REPLY);
      BufferPutInt32(b, id);
      BufferPutString(b, algo);
      EVP_MD_CTX_init(&mdctx);
      while (length > 0)
	{
	  length = (length > (u_int64_t )blockSize) ? length - (u_int64_t )blockSize : 0;
	  if (EVP_DigestInit_ex(&mdctx, md, NULL) == 1)
	    {
	      u_int32_t	r, off, len;
	      
	      off = blockSize;
	      len = sizeof(data);
	      DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Read:%i Rest:%llu", len, length));
	      while ((r = read(fd, data, len)) > 0)
		{
		  DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Compute block (%u/%u %u)", len, r, off));
		  if (EVP_DigestUpdate(&mdctx, data, r) == 0)
		    {
		      DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Error EVP_DigestUpdate"));
		      inError = 1;
		      break;
		    }
		  off -= r;
		  if (off < sizeof(data))
		    len = off;
		  if (off == 0)
		    break;
		}
	      if (EVP_DigestFinal_ex(&mdctx, md_value, &md_len) == 0)
		{
		  DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Error EVP_DigestFinal_ex"));
		  inError = 1;
		}
	      if (inError == 1)
		{
		  SendStatus(bOut, id, SSH2_FX_FAILURE);
		  break;
		}
	      else
		{
		  BufferPutRawData(b, md_value, md_len);
		  DEBUG((MYLOG_DEBUG, "[Hash: %X%X%X ...", md_value[0], md_value[1], md_value[2]));
		}
	    }
	  else
	    {
	      SendStatus(bOut, id, SSH2_FX_FAILURE);
	      DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]Error EVP_DigestInit_ex"));
	      break;
	    }
	}
      if (inError == 0)
	BufferPutPacket(bOut, b);
      BufferDelete(b);
      (void )EVP_MD_CTX_cleanup(&mdctx);
    }
  else
    {
      DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]No algorithm: %s", algo));
      SendStatus(bOut, id, SSH2_FX_OP_UNSUPPORTED);
    }
 endOfFileHashing:
  DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_FD]End"));
  free(algo);
}

void    DoExtFileHashing_Handle(tBuffer *bIn, tBuffer *bOut, u_int32_t id)
{
  int	fd, fileIsText;

  DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_Handle]..."));
  if ((fd = HandleGetFd(BufferGetHandle(bIn), &fileIsText)) >= 0)
    DoExtFileHashing_FD(bIn, bOut, id, fd);
  else
    SendStatus(bOut, id, SSH4_FX_INVALID_HANDLE);
}

void    DoExtFileHashing_Name(tBuffer *bIn, tBuffer *bOut, u_int32_t id)
{
  char	*file = BufferGetString(bIn);
  int	status, fd;

  DEBUG((MYLOG_DEBUG, "[DoExtFileHashing_Name]File: %s", file));
  if ((status = CheckRules(file, RULES_FILE, NULL, O_RDONLY)) == SSH2_FX_OK)
    {
      if ((fd = open(file, O_RDONLY)) != -1)
	{
	  DoExtFileHashing_FD(bIn, bOut, id, fd);
	  xclose(fd);
	}
      else
	SendStatus(bOut, id, errnoToPortable(errno));
    }
  else
    SendStatus(bOut, id, status);
  free(file);
}
#endif //MSSEXT_FILE_HASHING
