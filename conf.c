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
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "conf.h"
#include "FileSpec.h"
#include "ip.h"
#include "parsing.h"
#include "string.h"
#include "security.h"
#include "user.h"

#define CONF_IS_EMPTY				0
#define CONF_IS_STRING				1
#define CONF_IS_STRING_MAYBE_EMPTY	2
#define CONF_IS_PATH_RESOLVE_ENV	3
#define CONF_IS_INT					4
#define CONF_IS_BOOLEAN				5
#define CONF_IS_SPEED				6
#define CONF_IS_MODE				7
#define CONF_IS_TIME				8
#define CONF_IS_FILE_AND_DIR		9
#define CONF_DEPRECATED				10

#define CONF_SHOW			0
#define CONF_SHOW_ALWAYS	1
#define CONF_NOT_SHOW		2

typedef struct sConf
{
	char *name;
	int type;
	int show;
} tConf;

static const tConf confParams[] =
{
	{ "GlobalDownload", CONF_IS_SPEED, CONF_SHOW },
	{ "GlobalUpload", CONF_IS_SPEED, CONF_SHOW },
	{ "Download", CONF_IS_SPEED, CONF_SHOW },
	{ "Upload", CONF_IS_SPEED, CONF_SHOW },
	{ "StayAtHome", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "VirtualChroot", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "LimitConnection", CONF_IS_INT, CONF_SHOW },
	{ "LimitConnectionByUser", CONF_IS_INT, CONF_SHOW },
	{ "LimitConnectionByIP", CONF_IS_INT, CONF_SHOW },
	{ "Home", CONF_IS_PATH_RESOLVE_ENV, CONF_SHOW },
	{ "Shell", CONF_IS_STRING, CONF_SHOW },
	{ "ResolveIP", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "IdleTimeOut", CONF_IS_TIME, CONF_SHOW },
	{ "IgnoreHidden", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "DirFakeUser", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "DirFakeGroup", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "DirFakeMode", CONF_IS_MODE, CONF_SHOW },
	{ "HideNoAccess", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "ByPassGlobalDownload", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "ByPassGlobalUpload", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "MaxOpenFilesForUser", CONF_IS_INT, CONF_SHOW },
	{ "MaxReadFilesForUser", CONF_IS_INT, CONF_SHOW },
	{ "MaxWriteFilesForUser", CONF_IS_INT, CONF_SHOW },
	{ "ShowLinksAsLinks", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "SftpProtocol", CONF_IS_INT, CONF_SHOW_ALWAYS },
	{ "LogFile", CONF_IS_STRING, CONF_SHOW_ALWAYS },
	{ "ConnectionMaxLife", CONF_IS_TIME, CONF_SHOW },
	{ "DisableAccount", CONF_IS_BOOLEAN, CONF_SHOW },
#ifdef MSS_HAVE_ADMIN
	{ "IsAdmin", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "IsSimpleAdmin", CONF_IS_BOOLEAN, CONF_SHOW },
#endif
	{ "DisableRemoveDir", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "DisableRemoveFile", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "DisableReadFile", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "DisableReadDir", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "DisableWriteFile", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "DisableSetAttribute", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "DisableMakeDir", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "DisableRename", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "DisableSymLink", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "DisableOverwrite", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "Charset", CONF_IS_STRING, CONF_SHOW },
	{ "CanRemoveDir", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "CanRemoveFile", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "CanChangeRights", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "CanChangeTime", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "ExpireDate", CONF_IS_STRING_MAYBE_EMPTY, CONF_SHOW },
	{ "ForceUser", CONF_IS_STRING, CONF_SHOW },
	{ "ForceGroup", CONF_IS_STRING, CONF_SHOW },
	{ "CreateHome", CONF_IS_BOOLEAN, CONF_SHOW },
	{ "DefaultRights", CONF_IS_FILE_AND_DIR, CONF_SHOW },
	{ "MinimumRights", CONF_IS_FILE_AND_DIR, CONF_SHOW },
	{ "MaximumRights", CONF_IS_FILE_AND_DIR, CONF_SHOW },
	{ "ForceRights", CONF_IS_FILE_AND_DIR, CONF_SHOW },
	{ "ApplyFileSpec", CONF_IS_STRING, CONF_SHOW_ALWAYS },
	{ "GMTTime", CONF_DEPRECATED, CONF_NOT_SHOW },
	{ "HideFiles", CONF_DEPRECATED, CONF_SHOW },
	{ "PathAllowFilter", CONF_DEPRECATED, CONF_SHOW },
	{ "PathDenyFilter", CONF_DEPRECATED, CONF_SHOW },
	{ NULL, CONF_IS_EMPTY, CONF_NOT_SHOW }
};

void load_config(int verbose)
{
	if (init_user_info() == 0)
	{
		(void) fprintf(stderr, "[ERROR]Error when fetching user informations\n");
		exit(2);
	}
	hash_set_int("SERVER_PORT", get_port_server());
	hash_set("SERVER_IP", get_ip_server());
	hash_set_int("CanRemoveDir", 1);
	hash_set_int("CanRemoveFile", 1);
	hash_set_int("CanChangeRights", 1);
	hash_set_int("CanChangeTime", 1);
	if (load_config_file(CONFIG_FILE, verbose, 10) == 0)
		if (load_config_file(CONFIG_FILE2, verbose, 10) == 0)
		{
			(void) fprintf(stderr,
					"[ERROR]No valid config file were found. Please correct this.\n");
			exit(2);
		}
	free_user_info();
	if (verbose > 0)
	{
		size_t maxLen;
		char bTmp[256];
		int i, r;

		(void) printf("--- %s ---\n", (char *) hash_get("User"));
		for (i = 0, maxLen = 0; confParams[i].type != CONF_IS_EMPTY; i++)
		{
			size_t len = strlen(confParams[i].name);

			if (len > maxLen)
				maxLen = len;
		}
		for (i = 0; confParams[i].type != CONF_IS_EMPTY; i++)
		{
			size_t j;
			char *ptr;
			int vInt;

			if (confParams[i].show != CONF_SHOW_ALWAYS && hash_exists(
					confParams[i].name) == 0)
				continue;
			(void) printf("%s", confParams[i].name);
			for (j = maxLen - strlen(confParams[i].name) + 1; j > 0; j--)
				(void) printf(" ");
			(void) printf("= ");
			switch (confParams[i].type)
			{
			case CONF_IS_STRING:
			case CONF_IS_PATH_RESOLVE_ENV:
				ptr = (char *) hash_get(confParams[i].name);
				if (ptr == NULL && confParams[i].show == CONF_SHOW_ALWAYS)
					(void) printf("{default}");
				else
					(void) printf("%s", ptr);
				break;
			case CONF_IS_STRING_MAYBE_EMPTY:
				ptr = (char *) hash_get(confParams[i].name);
				(void) printf("%s", ptr ? ptr : "{nothing}");
				break;
			case CONF_IS_INT:
				vInt = hash_get_int(confParams[i].name);
				if (vInt == 0 && confParams[i].show == CONF_SHOW_ALWAYS)
					(void) printf("{default}");
				else
					(void) printf("%i", vInt);
				break;
			case CONF_IS_BOOLEAN:
				(void) printf("%s",
						hash_get_int(confParams[i].name) == 0 ? "false"
								: "true");
				break;
			case CONF_IS_SPEED:
				(void) printf("%i bytes/s", hash_get_int(confParams[i].name));
				break;
			case CONF_IS_MODE:
				vInt = hash_get_int(confParams[i].name);
				if (vInt == 0)
					(void) printf("{default}");
				else
					(void) printf("%i", vInt);
				break;
			case CONF_IS_TIME:
				(void) printf("%is", hash_get_int(confParams[i].name));
				break;
			case CONF_IS_FILE_AND_DIR:
				(void) snprintf(bTmp, sizeof(bTmp), "%sFile",
						confParams[i].name);
				r = hash_get_int(bTmp);
				(void) printf("%i%i%i%i", r / (8 * 8 * 8), (r / (8 * 8)) % 8,
						(r / 8) % 8, r % 8);
				(void) snprintf(bTmp, sizeof(bTmp), "%sDirectory",
						confParams[i].name);
				r = hash_get_int(bTmp);
				if (r > 0)
				{
					(void) printf(" %i%i%i%i", r / (8 * 8 * 8), (r / (8 * 8))
							% 8, (r / 8) % 8, r % 8);
				}
				break;
			case CONF_DEPRECATED:
				(void) printf("%s is deprecated and unused", confParams[i].name);
				break;
			}
			(void) printf("\n");
		}
	}
}

int load_config_file(const char *file, int verbose, int max_recursive_left)
{
	size_t len;
	FILE *fh;
	char buffer[1024];
	char **tb, *str;
	int line, processTag;
	int openedTag = 0;

	if (max_recursive_left == 0)
	{
		(void) fprintf(stderr, "[ERROR]Too much inclusions !!!\n");
		return (0);
	}
	processTag = 1;
	if ((fh = fopen(file, "r")))
	{
		if (verbose > 1)
			(void) printf("- Parse config file: %s -\n", file);
		line = 0;
		while (fgets(buffer, sizeof(buffer), fh))
		{
			line++;
			if ((str = clean_buffer(buffer)))
			{
				len = strlen(str) - 1;
				if (*str == '<')
				{
					if (str[len] == '>')
					{
						openedTag += TagParse(str);
						if (openedTag < 0)
						{
							(void) fprintf(
									stderr,
									"[ERROR]Too much tag closed at line %i in file '%s'!\n",
									line, file);
							exit(2);
						}
					}
					else
					{
						(void) fprintf(
								stderr,
								"[ERROR]Error parsing line %i is not valid in file '%s'!\n",
								line, file);
						exit(2);
					}
					processTag = TagIsActive(verbose);
				}
				else if (processTag == 0)
					continue;
				else if ((tb = ParseCutString(str)))
				{
					if (tb[0] != NULL)
					{
						if (TagIsOpen(VTAG_FILESPEC) == 1)
							FileSpecParse(tb, verbose);
						else
							processLine(tb, max_recursive_left, verbose);
					}
					free(tb);
				}
			}
		}
		if (openedTag != 0)
		{
			(void) fprintf(stderr,
					"[ERROR]Missing %i close(s) tag(s) in file '%s'!!!\n",
					openedTag, file);
			exit(2);
		}
		xfclose(fh);
	}
	else
	{
		(void) fprintf(stderr,
				"[ERROR]Couldn't load config file '%s'. Error : %s\n", file,
				strerror(errno));
		return (0);
	}
	return (1);
}

void processLine(char **tb, int max_recursive_left, int verbose)
{
	char bTmp[256];
	int notRecognized;
	int i;

	notRecognized = 1;
	for (i = 0; confParams[i].type != CONF_IS_EMPTY; i++)
		if (strcmp(tb[0], confParams[i].name) == 0 && (tb[1] != NULL
				|| confParams[i].type == CONF_IS_STRING_MAYBE_EMPTY))
		{
			notRecognized = 0;
			switch (confParams[i].type)
			{
			case CONF_IS_STRING:
				hash_set(tb[0], (void *) strdup(tb[1]));
				break;
			case CONF_IS_STRING_MAYBE_EMPTY:
				hash_set(tb[0], (void *) (tb[1] ? strdup(tb[1]) : 0));
				break;
			case CONF_IS_PATH_RESOLVE_ENV:
			{
				char *path = convert_str_with_resolv_env_to_str(tb[1]);

				hash_set(tb[0], (void *) convert_to_path(path));
			}
				break;
			case CONF_IS_INT:
				hash_set_int(tb[0], atoi(tb[1]));
				break;
			case CONF_IS_BOOLEAN:
				hash_set_int(tb[0], convert_boolean_to_int(tb[1]));
				break;
			case CONF_IS_SPEED:
				hash_set_int(tb[0], convert_speed_to_int(tb + 1));
				break;
			case CONF_IS_MODE:
				hash_set_int(tb[0], convert_mode_to_int(tb[1]));
				break;
			case CONF_IS_TIME:
				hash_set_int(tb[0], convert_time_to_int(tb + 1));
				break;
			case CONF_IS_FILE_AND_DIR:
				hash_set_int(tb[0], 42);
				(void) snprintf(bTmp, sizeof(bTmp), "%sFile", tb[0]);
				hash_set_int(bTmp, convert_mode_to_int(tb[1]));
				if (tb[2] != NULL)
				{
					(void) snprintf(bTmp, sizeof(bTmp), "%sDirectory", tb[0]);
					hash_set_int(bTmp, convert_mode_to_int(tb[2]));
				}
				break;
			}
			break;
		}
	if (notRecognized == 1)
	{
		if (strcmp(tb[0], "Include") == 0 && tb[1] != NULL)
		{
			notRecognized = 0;
			(void) load_config_file(tb[1], verbose, max_recursive_left - 1);
		}
		if (notRecognized == 1)
			(void) fprintf(stderr, "Property '%s' is not recognized !\n", tb[0]);
	}
}
