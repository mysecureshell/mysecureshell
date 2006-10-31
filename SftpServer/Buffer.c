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
#include <stdlib.h>
#include <string.h>
#include "Buffer.h"

tBuffer		*BufferNew()
{
  tBuffer	*b;
  
  b = malloc(sizeof(*b));
  b->size = DEFAULT_GROW;
  b->data = malloc(b->size);
  b->length = 0;
  b->read = 0;
  b->fastClean = 0;
  return (b);
}

void	BufferGrow(tBuffer *b, u_int32_t toAdd)
{
  b->size += toAdd;
  b->data = realloc(b->data, b->size);
}

void	BufferClean(tBuffer *b)
{
  if (b->read > 0)
    {
      if (b->length > b->read)
	{
	  memcpy(b->data, b->data + b->read, b->length - b->read);
	  b->length -= b->read;
	}
      else
	b->length = 0;
      b->read = 0;
      if (b->fastClean == 0)
	{
	  u_int32_t	nextSize;

	  nextSize = b->size >> 2;
	  if (b->length < nextSize && nextSize >= DEFAULT_GROW)
	    {
	      b->size = nextSize;
	      b->data = realloc(b->data, b->size);
	    }
	}
    }
}

void	BufferDelete(tBuffer *b)
{
  free(b->data);
  free(b);
}

void	BufferPutInt8(tBuffer *b, u_int8_t nb)
{
  if ((b->length + 1) > b->size)
    BufferGrow(b, DEFAULT_GROW);
  b->data[b->length++] = nb;
}

void	BufferPutInt32(tBuffer *b, u_int32_t nb)
{
  if ((b->length + 4) > b->size)
    BufferGrow(b, DEFAULT_GROW);
  b->data[b->length++] = (nb >> 24);
  b->data[b->length++] = (nb >> 16);
  b->data[b->length++] = (nb >> 8);
  b->data[b->length++] = nb;
}

void	BufferPutInt64(tBuffer *b, u_int64_t nb)
{
  u_int32_t	n1, n2;

  n1 = (u_int64_t )nb >> (u_int64_t )32;
  n2 = (u_int64_t )nb & (u_int64_t )0xffffffff;
  if ((b->length + 8) > b->size)
    BufferGrow(b, DEFAULT_GROW);
  b->data[b->length++] = (n1 >> 24);
  b->data[b->length++] = (n1 >> 16);
  b->data[b->length++] = (n1 >> 8);
  b->data[b->length++] = n1;
  b->data[b->length++] = (n2 >> 24);
  b->data[b->length++] = (n2 >> 16);
  b->data[b->length++] = (n2 >> 8);
  b->data[b->length++] = n2;
}

void	BufferPutRawData(tBuffer *b, void *data, int size)
{
  if ((b->length + size) > b->size)
    BufferGrow(b, b->length + size - b->size + DEFAULT_GROW);
  memcpy(b->data + b->length, data, size);
  b->length += size;
}

void	BufferPutString(tBuffer *b, char *data)
{
  int	size;

  size = strlen(data);
  if ((b->length + size + 4) > b->size)
    BufferGrow(b, b->length + size + 4 - b->size + DEFAULT_GROW);
  b->data[b->length++] = (size >> 24);
  b->data[b->length++] = (size >> 16);
  b->data[b->length++] = (size >> 8);
  b->data[b->length++] = size;
  memcpy(b->data + b->length, data, size);
  b->length += size;
}

void	BufferPutHandle(tBuffer *b, int h)
{
  if ((b->length + 5) > b->size)
    BufferGrow(b, b->length + 5 - b->size + DEFAULT_GROW);
  b->data[b->length++] = 0;
  b->data[b->length++] = 0;
  b->data[b->length++] = 0;
  b->data[b->length++] = 1;
  BufferPutInt8FAST(b, h);
}

#ifdef DODEBUG
#include "Log.h"

static char	*ASCII = "0123456789ABCDEF";

static void	dumpPacket(tBuffer *b, int trySize)
{
  char	*buffer;
  int   i, pos;

  
  mylog_printf(MYLOG_DEBUG, "[dumpPacket][length:%i][read:%i][size:%i][trySize:%i]",
	       b->length, b->read, b->size, trySize);
  buffer = malloc(b->size * 2 + 1);
  for (i = 0, pos = 0; i < b->size; i++)
    {
      unsigned char	c = (unsigned char)b->data[i];

      buffer[pos++] = ASCII[c / 16];
      buffer[pos++] = ASCII[c % 16];
    }
  buffer[pos] = 0;
  mylog_printf(MYLOG_DEBUG, "[%s]", buffer);
  free(buffer);
}

#endif

void	BufferReadData(tBuffer *b, u_int32_t size)
{
  if ((b->read + size) <= b->length)
    b->read += size;
#ifdef DODEBUG
  else
    dumpPacket(b, size);
#endif
}

u_int8_t	BufferGetInt8(tBuffer *b)
{
  u_int8_t	nb;
	
  if ((b->read + 1) > b->length)
    {
#ifdef DODEBUG
      dumpPacket(b, 1);
#endif
      return (0);
    }
  nb = (u_int8_t )b->data[b->read++];
  return (nb);
}

u_int32_t	BufferGetInt32(tBuffer *b)
{
  u_int32_t	nb;
	
  if ((b->read + 4) > b->length)
    {
#ifdef DODEBUG
      dumpPacket(b, 4);
#endif
      return (0);
    }
  nb = (u_int32_t )b->data[b->read++] << 24;
  nb += (u_int32_t )b->data[b->read++] << 16;
  nb += (u_int32_t )b->data[b->read++] << 8;
  nb += (u_int32_t )b->data[b->read++];
  return (nb);
}

u_int64_t	BufferGetInt64(tBuffer *b)
{
  u_int64_t	nb;
	
  if ((b->read + 8) > b->length)
    {
#ifdef DODEBUG
      dumpPacket(b, 8);
#endif
      return (0);
    }
  nb = (u_int64_t )b->data[b->read++] << 56;
  nb += (u_int64_t )b->data[b->read++] << 48;
  nb += (u_int64_t )b->data[b->read++] << 40;
  nb += (u_int64_t )b->data[b->read++] << 32;
  nb += (u_int64_t )b->data[b->read++] << 24;
  nb += (u_int64_t )b->data[b->read++] << 16;
  nb += (u_int64_t )b->data[b->read++] << 8;
  nb += (u_int64_t )b->data[b->read++];
  return (nb);
}

char	*BufferGetData(tBuffer *b, u_int32_t *size)
{
  char	*data;

  *size = BufferGetInt32(b);
  if ((b->read + *size) > b->length)
    {
#ifdef DODEBUG
      dumpPacket(b, *size);
#endif
      return (0);
    }
  data = (char *)(b->data + b->read);
  b->read += *size;
  return (data);
}

char		*BufferGetString(tBuffer *b)
{
  char		*data;
  u_int32_t	size;
  
  size = BufferGetInt32(b);
  if ((b->read + size) > b->length)
    {
#ifdef DODEBUG
      dumpPacket(b, size);
#endif
      return (0);
    }
  data = malloc(size + 1);
  if (data)
    {
      memcpy(data, b->data + b->read, size);
      data[size] = 0;
      b->read += size;
    }
  return (data);
}

int	BufferGetHandle(tBuffer *b)
{
  u_int32_t	size;

  size = BufferGetInt32(b);
  if (size == 1)
    return (BufferGetInt8(b));
  return (-1);
}
