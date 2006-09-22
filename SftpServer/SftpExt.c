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
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "Buffer.h"
#include "Encoding.h"
#include "Handle.h"
#include "Send.h"
#include "SftpExt.h"
#include "SftpServer.h"
#include "Util.h"


#ifdef MSSEXT_DISKUSAGE
void	DoExtDiskSpace(tBuffer *bIn, tBuffer *bOut, u_int32_t id)
{
  struct STATFS	stfs;
  char		*path;
  int		status;

  path = convertFromUtf8(BufferGetString(bIn), 1);
  if ((status = CheckRules(path, RULES_DIRECTORY, 0, O_RDONLY)) == SSH2_FX_OK)
    {
      if (!STATFS(path, &stfs))
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
#endif

#ifdef MSSEXT_FILE_HASHING
#include <openssl/evp.h>

static void DoExtFileHashing_FD(tBuffer *bIn, tBuffer *bOut, u_int32_t id, int fd)
{
  u_int64_t	offset, length;
  u_int32_t	blockSize;
  char		*algo;

  algo = BufferGetString(bIn);
  offset = BufferGetInt64(bIn);
  length = BufferGetInt64(bIn);
  blockSize = BufferGetInt32(bIn);
  if (lseek(fd, offset, SEEK_SET) == -1)
    SendStatus(bOut, id, errnoToPortable(errno));
  else
    {
      const EVP_MD	*md;

      if (!blockSize)
	blockSize = length;
      if ((md = EVP_get_digestbyname(algo)))
	{
	  unsigned char	md_value[EVP_MAX_MD_SIZE];
	  EVP_MD_CTX	mdctx;
	  u_int32_t	md_len;
	  tBuffer	*b;
	  char		*data;
	  
	  EVP_MD_CTX_init(&mdctx);
	  EVP_DigestInit_ex(&mdctx, md, NULL);
	  data = malloc(blockSize);
	  if (data != NULL)
	    {
	      u_int64_t	off = 0;
	      u_int32_t r;

	      while ((r = read(fd, data, blockSize)) > 0)
		{
		  EVP_DigestUpdate(&mdctx, data, r);
		  off += (u_int64_t )r;
		  if ((off + (u_int64_t )blockSize) > length)
		    blockSize = (u_int32_t )(length - off);
		  if (off == length)
		    break;
		}
	      free(data);
	    }
	  EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
	  EVP_MD_CTX_cleanup(&mdctx);
          b = BufferNew();
          BufferPutInt8(b, SSH2_FXP_EXTENDED_REPLY);
          BufferPutInt32(b, id);
	  BufferPutString(b, algo);
	  BufferPutData(b, md_value, md_len);
          BufferPutPacket(bOut, b);
	}
      else
	SendStatus(bOut, id, SSH2_FX_OP_UNSUPPORTED);
    }
  free(algo);
}

void    DoExtFileHashing_Handle(tBuffer *bIn, tBuffer *bOut, u_int32_t id)
{
  int	fd, fileIsText;

  if ((fd = HandleGetFd(BufferGetStringAsInt(bIn), &fileIsText)) >= 0)
    DoExtFileHashing_FD(bIn, bOut, id, fd);
  else
    SendStatus(bOut, id, SSH4_FX_INVALID_HANDLE);
}

void    DoExtFileHashing_Name(tBuffer *bIn, tBuffer *bOut, u_int32_t id)
{
  char	*file = BufferGetString(bIn);
  int	fd;

  if ((fd = open(file, O_RDONLY)) != -1)
    {
      DoExtFileHashing_FD(bIn, bOut, id, fd);
      close(fd);
    }
  else
    SendStatus(bOut, id, errnoToPortable(errno));
  free(file);
}
#endif
