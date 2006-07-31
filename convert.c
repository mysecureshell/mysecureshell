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

#include "config.h"
#include "SftpServer/Defines.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "conf.h"

char	*convert_to_path(char *path)
{
  int	len = strlen(path);

  if (len > 0)
    {
      if (path[len - 1] == '/' || path[len - 1] == '\\')
	path[len - 1] = 0;
    }
  return (path);
}

char	*convert_str_with_resolv_env_to_str(char *str)
{
  char	*env_var, *env_str, *new;
  int	beg, end;
  int	i, max;

  str = strdup(str);
  max = strlen(str);
  for (i = 0; i < max; i++)
    if (str[i] == '$')
      {
	beg = i + 1;
	while (i < max)
	  {
	    i++;
	    if (!((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z')
		  || (str[i] >= '0' && str[i] <= '9') || (str[i] == '_')))
	      break;
	  }
	end = i;
	env_str = malloc(end - beg + 1);
	strncpy(env_str, str + beg, end - beg);
	env_str[end - beg] = 0;
	if ((env_var = getenv(env_str)))
	  {
	    int	len = strlen(str) - (end - beg) + strlen(env_var) + 1;

	    new = malloc(len);
	    strncpy(new, str, beg - 1);
	    new[beg - 1] = 0;
	    STRCAT(new, env_var, len);
	    STRCAT(new, str + end, len);
	    free(str);
	    str = new;
	    i = 0;
	  }
	free(env_str);
      }
  return (str);
}

int	convert_boolean_to_int(char *str)
{
  if (str)
    if (!strcasecmp(str, "true") || !strcmp(str, "1"))
      return (1);
  return (0);
}

int	convert_speed_to_int(char **tb)
{
  char	*str;
  int	nb = 0;
  int	div = 0;
  int	i, j;
  int	*ptr = &nb;
  int	len = 1;

  for (j = 0; tb[j]; j++)
    {
      str = tb[j];
      for (i = 0; str[i]; i++)
	{
	  if (str[i] >= '0' && str[i] <= '9')
	    {
	      *ptr = *ptr * 10 + (str[i] - '0');
	      len *= 10;
	    }
	  else
	    switch (str[i])
	      {
	      case 'k':
	      case 'K':
		return (nb * 1024 + div * (1024 / len));
		
	      case 'm':
	      case 'M':
		return (nb * 1024 * 1024 + div * ((1024 * 1024) / len));
		
	      case '.':
		ptr = &div;
		len = 1;
		break;
	      }
	}
    }
  return (nb);
}

int	convert_time_to_int(char **tb)
{
  int   nb = 0;
  int   i, j;

  for (j = 0; tb[j]; j++)
    {
      char	*str = tb[j];

      for (i = 0; str[i]; i++)
	{
	  if (str[i] >= '0' && str[i] <= '9')
	    nb = nb * 10 + (str[i] - '0');
	  else
	    switch (str[i])
	      {
	      case 'd':
	      case 'D':
		nb *= 24;
	      case 'h':
	      case 'H':
		nb *= 60;
	      case 'm':
	      case 'M':
		nb *= 60;
	      }
	}
    }
  return (nb);
}

int     convert_mode_to_int(char *str)
{
  int	i;
  int	r;

  r = 0;
  for (i = 0; str[i]; i++)
    r = (r * 8) + (str[i] - '0');
  return (r);
}
