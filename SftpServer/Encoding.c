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

#if(HAVE_ICONV||HAVE_LIBICONV)

#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include "Encoding.h"

static iconv_t _toUTF8 = (iconv_t) -1;
static iconv_t _fromUTF8 = (iconv_t) -1;

/*@null@*/ char *convertToUtf8(char *str, int freeAfter)
{
	char *newStr = NULL;

	if (_toUTF8 != (iconv_t) -1 && str[0] != '\0')
	{
		char *oldPtr = str;
		char *newPtr;
		size_t iLen, oLen;

		iLen = strlen(str);
		oLen = iLen << 1;
		newStr = malloc(oLen + 1);
		if (newStr != NULL)
		{
			newPtr = newStr;
			if (iconv(_toUTF8, &oldPtr, &iLen, &newPtr, &oLen) == (size_t) -1)
			{
				free(newStr);
				goto justdup;
			}
			*newPtr = '\0';
		}
	}
	else
	{
justdup:
		newStr = strdup(str);
	}
	if (freeAfter == 1)
		free(str);
	return (newStr);
}

/*@null@*/ char *convertFromUtf8(char *str, int freeAfter)
{
	char *newStr = NULL;

	if (_fromUTF8 != (iconv_t) -1 && str[0] != '\0')
	{
		char *oldPtr = str;
		char *newPtr;
		size_t iLen, oLen;

		iLen = oLen = strlen(str);
		newStr = strdup(str);
		if (newStr != NULL)
		{
			newPtr = newStr;
			if (iconv(_fromUTF8, &oldPtr, &iLen, &newPtr, &oLen) == (size_t) -1)
			{
				free(newStr);
				goto justdup2;
			}
			*newPtr = '\0';
		}
	}
	else
	{
justdup2:
		newStr = strdup(str);
	}
	if (freeAfter == 1)
		free(str);
	return (newStr);
}

void setCharset(/*@null@*/ const char *charset)
{
	if (charset != NULL)
	{
		_toUTF8 = iconv_open("UTF-8", charset);
		_fromUTF8 = iconv_open(charset, "UTF-8");
	}
	else
	{
		if (_toUTF8 != (iconv_t) -1)
			(void) iconv_close(_toUTF8);
		if (_fromUTF8 != (iconv_t) -1)
			(void) iconv_close(_fromUTF8);
		_toUTF8 = (iconv_t) -1;
		_fromUTF8 = (iconv_t) -1;
	}
}

#endif
