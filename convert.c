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
#include "SftpServer/Defines.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "conf.h"

char *convert_to_path(char *path)
{
	size_t len = strlen(path);

	if (len > 0)
	{
		if (path[len - 1] == '/' || path[len - 1] == '\\')
			path[len - 1] = '\0';
	}
	return (path);
}

char *convert_str_with_resolv_env_to_str(const char *str)
{
	size_t beg, end, i, max;
	char *env_var, *env_str, *new, *res;

	if ((res = strdup(str)) == NULL)
		return NULL;
	max = strlen(res);
	for (i = 0; i < max; i++)
		if (res[i] == '$')
		{
			int firstIsBlock = 0;

			beg = i + 1;
			if (res[beg] == '{')
			{
				firstIsBlock = 1;
				i++;
			}
			while (i < max)
			{
				i++;
				if (!((res[i] >= 'a' && res[i] <= 'z') || (res[i] >= 'A'
						&& res[i] <= 'Z') || (res[i] >= '0' && res[i] <= '9')
						|| (res[i] == '_')))
					break;
			}
			end = i;
			env_str = malloc(end - beg + 1);
			strncpy(env_str, res + beg + firstIsBlock, end - beg - firstIsBlock);
			env_str[end - beg - firstIsBlock] = '\0';
			if (firstIsBlock && (end + 1) <= max)
				end++;
			if ((env_var = getenv(env_str)))
			{
				size_t len;

				len = strlen(res) - (end - beg) + strlen(env_var) + 1;
				new = malloc(len);
				strncpy(new, res, beg - 1);
				new[beg - 1] = '\0';
				STRCAT(new, env_var, len);
				STRCAT(new, res + end, len);
				free(res);
				res = new;
				i = 0;
				max = len - 1;
			}
			free(env_str);
		}
	return (res);
}

int convert_boolean_to_int(const char *str)
{
	if (str)
		if (strcasecmp(str, "true") == 0 || strcmp(str, "1") == 0)
			return (1);
	return (0);
}

int convert_speed_to_int(char **tb)
{
	const char *str;
	int nb = 0;
	int div = 0;
	int i, j;
	int *ptr = &nb;
	int len = 1;

	for (j = 0; tb[j]; j++)
	{
		str = tb[j];
		for (i = 0; str[i] != '\0'; i++)
		{
			if (str[i] >= '0' && str[i] <= '9')
			{
				*ptr = *ptr * 10 + ((int) str[i] - (int) '0');
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

int convert_time_to_int(char **tb)
{
	int nb = 0;
	int i, j;

	for (j = 0; tb[j]; j++)
	{
		const char *str = tb[j];

		for (i = 0; str[i] != '\0'; i++)
		{
			if (str[i] >= '0' && str[i] <= '9')
				nb = nb * 10 + ((int) str[i] - (int) '0');
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

int convert_mode_to_int(const char *str)
{
	int i;
	int r;

	r = 0;
	for (i = 0; str[i] != '\0'; i++)
		r = (r * 8) + ((int) str[i] - (int) '0');
	return (r);
}
