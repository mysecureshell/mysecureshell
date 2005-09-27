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

#include "../defines.h"
#include <stdlib.h>
#include <string.h>
#include "Buffer.h"

tBuffer		*BufferNew()
{
  tBuffer	*b;
  
  b = MALLOC(sizeof(*b));
  b->size = DEFAULT_GROW;
  b->data = MALLOC(b->size);
  b->length = 0;
  b->read = 0;
  return (b);
}

void	BufferGrow(tBuffer *b, u_int32_t toAdd)
{
  b->size += toAdd;
  b->data = REALLOC(b->data, b->size);
}

void	BufferClean(tBuffer *b)
{
  int	nextSize;

  if (b->read > 0)
    {
      memcpy(b->data, b->data + b->read, b->length - b->read);
      b->length -= b->read;
      b->read = 0;
      nextSize = b->size >> 2;
      if (b->length < nextSize && nextSize >= DEFAULT_GROW)
	{
	  b->size = nextSize;
	  b->data = REALLOC(b->data, b->size);
	}
    }
}

void	BufferDelete(tBuffer *b)
{
  FREE(b->data);
  FREE(b);
}

void	BufferReadData(tBuffer *b, u_int32_t size)
{
  if ((b->read + size) <= b->length)
    b->read += size;
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
  if ((b->length + 8) > b->size)
    BufferGrow(b, DEFAULT_GROW);
  b->data[b->length++] = (nb >> 56);
  b->data[b->length++] = (nb >> 48);
  b->data[b->length++] = (nb >> 40);
  b->data[b->length++] = (nb >> 32);
  b->data[b->length++] = (nb >> 24);
  b->data[b->length++] = (nb >> 16);
  b->data[b->length++] = (nb >> 8);
  b->data[b->length++] = nb;
}

void	BufferPutData(tBuffer *b, void *data, int size)
{
  BufferPutInt32(b, size);
  BufferPutRawData(b, data, size);
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
  BufferPutData(b, data, strlen(data));
}

u_int8_t	BufferGetInt8(tBuffer *b)
{
  u_int8_t	nb;
	
  if ((b->read + 1) > b->size)
    return 0;
  nb = (u_int8_t )b->data[b->read++];
  return (nb);
}

u_int32_t	BufferGetInt32(tBuffer *b)
{
  u_int32_t	nb;
	
  if ((b->read + 4) > b->size)
    return 0;
  nb = (u_int32_t )b->data[b->read++] << 24;
  nb += (u_int32_t )b->data[b->read++] << 16;
  nb += (u_int32_t )b->data[b->read++] << 8;
  nb += (u_int32_t )b->data[b->read++];
  return (nb);
}

u_int64_t	BufferGetInt64(tBuffer *b)
{
  u_int64_t	nb;
	
  if ((b->read + 8) > b->size)
    return 0;
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
    return (0);
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
    return (0);
  data = MALLOC(size + 1);
  if (data)
    {
      memcpy(data, b->data + b->read, size);
      data[size] = 0;
      b->read += size;
    }
  return (data);
}

int	BufferGetStringAsInt(tBuffer *b)
{
  char	*data;

  data = BufferGetString(b);
  if (data)
    {
      int	nb = atoi(data);

      FREE(data);
      return (nb);
    }
  return (-1);
}
