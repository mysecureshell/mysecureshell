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

#include "config.h"
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#ifdef HAVE_CYGWIN_SOCKET_H
#include <cygwin/socket.h>
#endif
#include <sys/types.h>
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <arpa/inet.h>
#include "FileSpec.h"
#include "string.h"
#include "parsing.h"
#include "user.h"

/*@null@*/ static tTag *_tags = NULL;

int TagIsActive(int verbose)
{
	tTag *currentTag = _tags;

	while (currentTag != NULL)
	{
		switch (currentTag->type)
		{
		case VTAG_FILESPEC:
		case VTAG_DEFAULT:
			return 1;
		case VTAG_USER:
			return is_for_user(currentTag->data1, verbose);
		case VTAG_GROUP:
			return is_for_group(currentTag->data1, verbose);
		case VTAG_RANGEIP:
			return is_for_rangeip(currentTag->data1, verbose);
		case VTAG_VIRTUALHOST:
			return is_for_virtualhost(currentTag->data1, currentTag->data2, verbose);
		}
		currentTag = currentTag->next;
	}
	return 1;
}

int TagIsOpen(eTagType tagType)
{
	if (_tags != NULL && _tags->type == tagType)
		return 1;
	return 0;
}

int TagParse(char *buffer)
{
	size_t len;
	char *str;
	int is_close_tag = 0;

	str = trim_left(buffer + 1);
	if (*str == '/')
	{
		is_close_tag = 1;
		str = trim_left(str + 1);
	}
	len = *str != '\0' ? strlen(str) - 1 : 0;
	str[len] = '\0';
	str = trim_right(str);
	if (is_close_tag == 1)
		TagParseClose();
	else
		TagParseOpen(str);
	return (is_close_tag == 1 ? -1 : 1);
}

void TagParseClose()
{
	tTag *deleteMe = _tags;

	if (TagIsOpen(VTAG_FILESPEC) == 1)
		FileSpecLeave();
	if (deleteMe != NULL)
	{
		_tags = deleteMe->next;
		if (deleteMe->data1 != NULL)
			free(deleteMe->data1);
		free(deleteMe);
	}
}

void TagParseOpen(char *str)
{
	tTag *newTag;
	char *s;

	if ((s = strchr(str, ' ')) != NULL || (s = strchr(str, '\t')) != NULL)
	{
		*s = '\0';
		s = trim_left(s + 1);
	}
	if (s == NULL || *s == '\0')
		return;
	newTag = calloc(1, sizeof(*newTag));
	if (newTag == NULL)
		return;
	str = trim_right(str);
	if (strcasecmp(str, TAG_GROUP) == 0)
	{
		newTag->type = VTAG_GROUP;
		newTag->data1 = strdup(s);
	}
	else if (strcasecmp(str, TAG_USER) == 0)
	{
		newTag->type = VTAG_USER;
		newTag->data1 = strdup(s);
	}
	else if (strcasecmp(str, TAG_RANGEIP) == 0)
	{
		newTag->type = VTAG_RANGEIP;
		newTag->data1 = TagParseRangeIP(s);
		if (newTag->data1 == NULL)
		{
			free(newTag);
			return;
		}
	}
	else if (strcasecmp(str, TAG_VIRTUALHOST) == 0)
	{
		newTag->type = VTAG_VIRTUALHOST;
		TagParseVirtualHost(s, newTag);
	}
	else if (strcasecmp(str, TAG_FILESPEC) == 0)
	{
		newTag->type = VTAG_FILESPEC;
		FileSpecEnter(s);
	}
	else
		//TAG_DEFAULT
		newTag->type = VTAG_DEFAULT;
	newTag->next = _tags;
	_tags = newTag;
}

void TagParseVirtualHost(const char *str, tTag *newTag)
{
	struct hostent *h;
	char *ptr;
	int port = 0;

	if ((ptr = strchr(str, ':')))
	{
		*ptr = '\0';
		port = atoi(ptr + 1);
	}
	if (!(str[0] == '*' && str[1] == '\0') && (h = gethostbyname(str)) != NULL
			&& h->h_addr_list != NULL && h->h_addr_list[0] != NULL)
	{
		char buffer[32];
#ifdef HAVE_INET_NTOA
		struct in_addr in;

		(void) memcpy(&in.s_addr, *h->h_addr_list, sizeof(in.s_addr));

		(void) snprintf(buffer, sizeof(buffer), "%s", inet_ntoa(in));
#else //!HAVE_INET_NTOA
		(void )snprintf(buffer, sizeof(buffer), "%u.%u.%u.%u",
				(unsigned int)h->h_addr_list[0][0],
				(unsigned int)h->h_addr_list[0][1],
				(unsigned int)h->h_addr_list[0][2],
				(unsigned int)h->h_addr_list[0][3]
		);
#endif
		newTag->data1 = strdup(buffer);
	}
	else
		newTag->data1 = strdup(str);
	newTag->data2 = port;
}

/*@null@*/ char *TagParseRangeIP(const char *str)
{
	char *mask = calloc(10, sizeof(char));
	int i, nb, pos;

	if (mask == NULL)
		return (NULL);
	mask[8] = (char) 32;
	for (i = 0, nb = 0, pos = 0; str[i] != '\0'; i++)
		if (str[i] >= '0' && str[i] <= '9')
			nb = nb * 10 + ((int) str[i] - (int) '0');
		else if ((str[i] == '.' || str[i] == '-' || str[i] == '/') && pos <= 7)
		{
			mask[pos] = (char) nb;
			if (pos >= 0 && pos <= 3)
				mask[pos + 4] = (char) nb;
			pos++;
			nb = 0;
		}
	mask[pos] = (char) nb;
	if (pos >= 0 && pos <= 3)
		mask[pos + 4] = (char) nb;
	if (mask[8] > (char) 32)
		mask[8] = (char) 32;
	return (mask);
}

/*@null@*/ char **ParseCutString(char *str)
{
	char **tb = NULL;
	char *word = NULL;
	int nb = 0;

	while (*str != '\0')
	{
		if (*str == ' ' || *str == '\t')
		{
			*str = '\0';
			if (word != NULL)
			{
				word = clean_string(word);
				if (*word != '\0' && strcmp(word, "=") != 0)
				{
					tb = realloc(tb, (nb + 2) * sizeof(*tb));
					if (tb == NULL)
						return (NULL);
					tb[nb++] = word;
					tb[nb] = 0;
				}
				word = NULL;
			}
		}
		else
		{
			if (word == NULL)
				word = str;
			if (*str == '\'' || *str == '"')
			{
				char c = *str;

				str++;
				while (c != *str && *str != '\0')
					str++;
			}
			else if (*str == '\\')
				str++;
		}
		str++;
	}
	if (word != NULL)
	{
		word = clean_string(word);
		if (*word != '\0' && strcmp(word, "=") != 0)
		{
			tb = realloc(tb, (nb + 2) * sizeof(*tb));
			if (tb == NULL)
				return (NULL);
			tb[nb++] = word;
			tb[nb] = 0;
		}
	}
	return (tb);
}
