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
#include "Send.h"
#include "SftpExt.h"
#include "SftpServer.h"
#include "Util.h"


#ifdef SUPPORT_EXT_SPACE
void	DoExtSpace(tBuffer *bIn, tBuffer *bOut, u_int32_t id)
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
