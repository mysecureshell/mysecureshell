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

#include <string.h>
#include "string.h"

char	*clean_buffer(char *buffer)
{
  delete_comments(buffer);
  buffer = trim_right(trim_left(buffer));
  if (strlen(buffer))
    return (buffer);
  return (0);
}

char	*trim_right(char *buffer)
{
  int	i;

  for (;;)
    {
      i = strlen(buffer) - 1;
      if (buffer[i] == ' ' || buffer[i] == '\t'
	  || buffer[i] == '\r' || buffer[i] == '\n')
	buffer[i] = 0;
      else
	break;
    }
  return (buffer);
}

char	*trim_left(char *buffer)
{
  while (*buffer == ' ' || *buffer == '\t')
    buffer++;
  return (buffer);
}

char	*clean_string(char *buffer)
{
  char	c;
  int	i, max;

  buffer = trim_left(trim_right(buffer));
  for (i = 0, max = strlen(buffer); i < max; i++)
    {
      if (buffer[i] == '"' || buffer[i] == '\'')
	{
	  c = buffer[i];
	  strcpy(buffer + i, buffer + i + 1);
	  while (c != buffer[i] && i < max)
	    i++;
	  if (c == buffer[i])
	    strcpy(buffer + i, buffer + i + 1);
	}
      else if (buffer[i] == '\\')
	strcpy(buffer + i, buffer + i + 1);
    }
  return (buffer);
}

void	delete_comments(char *buffer)
{
  int	len;
  int	i;

  len = strlen(buffer);
  for (i = 0; i < len; i++)
    {
      if (buffer[i] == '\'' ||buffer[i] == '"')
	{
	  char	c = buffer[i];

	  for (i++; i < len && buffer[i] != c; )
	    i++;
	}
      else if (buffer[i] == '\\')
	i++;
      else if (buffer[i] == '#')
	buffer[i] = 0;
    }
}
