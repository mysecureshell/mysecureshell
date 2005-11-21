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

#include "../config.h"

#ifdef HAVE_LIBIDN

static char	*_charset = 0;

#include <stdlib.h>
#include <string.h>
#include <stringprep.h>
#include "Encoding.h"


char	*convertToUtf8(char *str, int freeAfter)
{
  char	*newStr = 0;

  if (_charset)
    {
      if (!(newStr = stringprep_convert(str, "UTF-8", _charset)))
	goto justdup;
    }
  else
    {
    justdup:
      newStr = strdup(str);
    }
  if (freeAfter)
    free(str);
  return (newStr);
}

char	*convertFromUtf8(char *str, int freeAfter)
{
  char	*newStr = 0;

  if (_charset)
    {      
      if (!(newStr = stringprep_convert(str, _charset, "UTF-8")))
	goto justdup2;
    }
  else
    {
    justdup2:
      newStr = strdup(str);
    }
  if (freeAfter)
    free(str);
  return (newStr);
}

void	setCharset(char *charset)
{
  _charset = charset;
}

#endif
