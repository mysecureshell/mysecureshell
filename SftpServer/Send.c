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
#include <stdio.h>
#include <string.h>
#include "Encode.h"
#include "Send.h"

void		SendAttributes(tBuffer *bOut, u_int32_t id, const tAttributes *a, const char *file)
{
  tBuffer	*b;
	
  b = BufferNew();
  BufferPutInt8(b, SSH2_FXP_ATTRS);
  BufferPutInt32(b, id);
  EncodeAttributes(b, a, file);
  BufferPutPacket(bOut, b);
  BufferDelete(b);
}

void		SendStats(tBuffer *bOut, u_int32_t id, int count, const tStat *s)
{
  tBuffer	*b;
  int		i;
	
  b = BufferNew();
  BufferPutInt8(b, SSH2_FXP_NAME);
  BufferPutInt32(b, id);
  BufferPutInt32(b, count);
  for (i = 0; i < count; i++)
    {
      BufferPutString(b, s[i].name);
      if (cVersion <= 3)
	BufferPutString(b, s[i].longName);
      EncodeAttributes(b, &s[i].attributes, NULL);
    }
  BufferPutPacket(bOut, b);
  BufferDelete(b);
}

void		SendHandle(tBuffer *bOut, u_int32_t id, int h)
{
  u_int32_t	dataSize;

  dataSize = 1 + 4 + BufferHandleSize;
  BufferEnsureFreeCapacity(bOut, 4 + dataSize);
  BufferPutInt32(bOut, dataSize);
  //START Data
  BufferPutInt8FAST(bOut, SSH2_FXP_HANDLE);
  BufferPutInt32(bOut, id);
  BufferPutHandle(bOut, h);
  //END Data
}

void		SendData(tBuffer *bOut, u_int32_t id, const char *data, int len)
{
  u_int32_t	dataSize;

  dataSize = 1 + 4 + 4 + len;
  BufferEnsureFreeCapacity(bOut, 4 + dataSize);
  BufferPutInt32(bOut, dataSize);
  //START Data
  BufferPutInt8FAST(bOut, SSH2_FXP_DATA);
  BufferPutInt32(bOut, id);
  BufferPutData(bOut, data, len);
  //END Data
}

void		SendStatus(tBuffer *bOut, u_int32_t id, u_int32_t status)
{
  static char	*statusMessages[] =
    {
      "Success",		/* SSH_FX_OK */
      "End of file",		/* SSH_FX_EOF */
      "No such file",		/* SSH_FX_NO_SUCH_FILE */
      "Permission denied",	/* SSH_FX_PERMISSION_DENIED */
      "Failure",		/* SSH_FX_FAILURE */
      "Bad message",		/* SSH_FX_BAD_MESSAGE */
      "No connection",		/* SSH_FX_NO_CONNECTION */
      "Connection lost",	/* SSH_FX_CONNECTION_LOST */
      "Operation unsupported",	/* SSH_FX_OP_UNSUPPORTED */
      "Invalid handle",		/* SSH4_FX_INVALID_HANDLE */
      "No such path",		/* SSH4_FX_NO_SUCH_PATH */
      "File already exists",	/* SSH4_FX_FILE_ALREADY_EXISTS */
      "Write protect",		/* SSH4_FX_WRITE_PROTECT */
      "No media",		/* SSH4_FX_NO_MEDIA */
      "Unknown error"		/* Others */
    };
  u_int32_t	dataSize;
  u_int32_t	msgLength = 0;
  char		*msg = NULL;

  dataSize = 1 + 4 + 4;
  if (cVersion >= 3)
    {
      msg = statusMessages[MIN(status, SSH2_FX_MAX)];
      msgLength = strlen(msg);
      dataSize += 4 + msgLength + 4 + 2;
    }
  BufferEnsureFreeCapacity(bOut, 4 + dataSize);
  BufferPutInt32(bOut, dataSize);
  //START Data
  BufferPutInt8(bOut, SSH2_FXP_STATUS);
  BufferPutInt32(bOut, id);
  BufferPutInt32(bOut, status);
  if (msg != NULL)
    {
      BufferPutData(bOut, msg, msgLength);
      BufferPutData(bOut, "en", 2);
    }
  //END Data
}
