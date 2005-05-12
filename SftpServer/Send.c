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

#include "Encode.h"
#include "Send.h"

void		SendAttributes(tBuffer *bOut, u_int32_t id, tAttributes *a)
{
  tBuffer	*b;
	
  b = BufferNew();
  BufferPutInt8(b, SSH2_FXP_ATTRS);
  BufferPutInt32(b, id);
  EncodeAttributes(b, a);
  BufferPutPacket(bOut, b);
  BufferDelete(b);
}

void		SendStats(tBuffer *bOut, u_int32_t id, int count, tStat *s)
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
      EncodeAttributes(b, &s[i].attributes);
    }
  BufferPutPacket(bOut, b);
  BufferDelete(b);
}

void		SendHandle(tBuffer *bOut, u_int32_t id, int h)
{
  tBuffer	*b;
  
  b = BufferNew();
  BufferPutInt8(b, SSH2_FXP_HANDLE);
  BufferPutInt32(b, id);
  BufferPutInt32(b, h);
  BufferPutPacket(bOut, b);
  BufferDelete(b);
}

void		SendData(tBuffer *bOut, u_int32_t id, char *data, int len)
{
  tBuffer	*b;
	
  b = BufferNew();
  BufferPutInt8(b, SSH2_FXP_DATA);
  BufferPutInt32(b, id);
  BufferPutData(b, data, len);
  BufferPutPacket(bOut, b);
  BufferDelete(b);
}

void		SendStatus(tBuffer *bOut, u_int32_t id, u_int32_t status)
{
  tBuffer	*b;
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

  b = BufferNew();
  BufferPutInt8(b, SSH2_FXP_STATUS);
  BufferPutInt32(b, id);
  BufferPutInt32(b, status);
  if (cVersion >= 3)
    {
      BufferPutString(b, statusMessages[MIN(status, SSH2_FX_MAX)]);
      BufferPutString(b, "en");
    }
  BufferPutPacket(bOut, b);
  BufferDelete(b);
}
