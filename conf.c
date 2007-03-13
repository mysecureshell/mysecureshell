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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "conf.h"
#include "config.h"
#include "ip.h"
#include "parsing.h"
#include "string.h"
#include "user.h"

#define CONF_IS_EMPTY			0
#define CONF_IS_STRING			1
#define CONF_IS_STRING_MAYBE_EMPTY	2
#define CONF_IS_PATH_RESOLVE_ENV	3
#define CONF_IS_INT			4
#define CONF_IS_BOOLEAN			5
#define CONF_IS_SPEED			6
#define CONF_IS_MODE			7
#define CONF_IS_TIME			8

#define CONF_SHOW		0
#define CONF_SHOW_IF_NOT_NULL	1
#define CONF_NOT_SHOW		2

typedef struct	sConf
{
  char		*name;
  char		type;
  char		show;
}		tConf;

static const tConf	confParams[] =
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
    { "IdleTimeOut", CONF_IS_INT, CONF_SHOW },
    { "IgnoreHidden", CONF_IS_BOOLEAN, CONF_SHOW },
    { "DirFakeUser", CONF_IS_BOOLEAN, CONF_SHOW },
    { "DirFakeGroup", CONF_IS_BOOLEAN, CONF_SHOW },
    { "DirFakeMode", CONF_IS_MODE, CONF_SHOW },
    { "HideFiles", CONF_IS_STRING_MAYBE_EMPTY, CONF_SHOW },
    { "HideNoAccess", CONF_IS_BOOLEAN, CONF_SHOW },
    { "ByPassGlobalDownload", CONF_IS_BOOLEAN, CONF_SHOW },
    { "ByPassGlobalUpload", CONF_IS_BOOLEAN, CONF_SHOW },
    { "MaxOpenFilesForUser", CONF_IS_INT, CONF_SHOW },
    { "MaxReadFilesForUser", CONF_IS_INT, CONF_SHOW },
    { "MaxWriteFilesForUser", CONF_IS_INT, CONF_SHOW },
    { "ShowLinksAsLinks", CONF_IS_BOOLEAN, CONF_SHOW },
    { "PathDenyFilter", CONF_IS_STRING, CONF_SHOW },
    { "SftpProtocol", CONF_IS_INT, CONF_SHOW_IF_NOT_NULL },
    { "LogFile", CONF_IS_STRING, CONF_SHOW_IF_NOT_NULL },
    { "ConnectionMaxLife", CONF_IS_TIME, CONF_SHOW },
    { "DisableAccount", CONF_IS_BOOLEAN, CONF_SHOW },
#ifdef MSS_HAVE_ADMIN
    { "IsAdmin", CONF_IS_BOOLEAN, CONF_SHOW },
#endif
    { "Charset", CONF_IS_STRING, CONF_SHOW },
    { "GMTTime", CONF_IS_STRING_MAYBE_EMPTY, CONF_SHOW },
    { "CanRemoveDir", CONF_IS_BOOLEAN, CONF_SHOW },
    { "CanRemoveFile", CONF_IS_BOOLEAN, CONF_SHOW },
    { "ExpireDate", CONF_IS_STRING_MAYBE_EMPTY, CONF_SHOW },
    { 0, CONF_IS_EMPTY },
  };

void	load_config(char verbose)
{
  if (!init_user_info())
    {
      if (verbose) printf("[ERROR]Error when fetching user informations\n");
      exit (2);
    }
  hash_set_int("SERVER_PORT", get_port_server());
  hash_set("SERVER_IP", get_ip_server());
  if (!load_config_file(CONFIG_FILE, verbose, 10))
    if (!load_config_file(CONFIG_FILE2, verbose, 10) && verbose)
      {
	printf("[ERROR]No valid config file were found.\nPlease correct this.\n");
	exit (2);
      }
  free_user_info();
  if (verbose)
    {
      int	i, r, r2, maxLen;

      printf("--- %s ---\n", (char *)hash_get("User"));
      for (i = 0, maxLen = 0; confParams[i].type != CONF_IS_EMPTY; i++)
	{
	  int	len = strlen(confParams[i].name);

	  if (len > maxLen)
	    maxLen = len;
	}
      for (i = 0; confParams[i].type != CONF_IS_EMPTY; i++)
	{
	  char	*ptr;
	  int	vInt, j;

	  printf("%s", confParams[i].name);
	  for (j = maxLen - strlen(confParams[i].name); j >= 0; j--)
	    printf(" ");
	  printf("= ");
	  switch (confParams[i].type)
	    {
	    case CONF_IS_STRING:
	    case CONF_IS_PATH_RESOLVE_ENV:
	      ptr = (char *)hash_get(confParams[i].name);
	      if (ptr == NULL && confParams[i].show == CONF_SHOW_IF_NOT_NULL)
		printf("{default}");
	      else
		printf("%s", ptr);
	      break;
	    case CONF_IS_STRING_MAYBE_EMPTY:
	      ptr = (char *)hash_get(confParams[i].name);
	      printf("%s", ptr ? ptr : "{nothing}");
	      break;
	    case CONF_IS_INT:
	      vInt = hash_get_int(confParams[i].name);
	      if (vInt == 0 && confParams[i].show == CONF_SHOW_IF_NOT_NULL)
		printf("{default}");
	      else
		printf("%i", vInt);
	      break;
	    case CONF_IS_BOOLEAN:
	      printf("%s", hash_get_int(confParams[i].name) == 0 ? "false" : "true");
	      break;
	    case CONF_IS_SPEED:
	      printf("%i bytes/s", hash_get_int(confParams[i].name));
	      break;
	    case CONF_IS_MODE:
	      vInt = hash_get_int(confParams[i].name);
	      if (vInt == 0)
		printf("{default}");
	      else
		printf("%i", vInt);
	      break;
	    case CONF_IS_TIME:
	      printf("%is", hash_get_int(confParams[i].name));
	      break;
	    }
	  printf("\n");
	}
      r = hash_get_int("DefaultRightsFile");
      r2 = hash_get_int("DefaultRightsDirectory");
      printf("DefaultRights         = %i%i%i%i %i%i%i%i\n",
	     r / (8 * 8 * 8), (r / ( 8 * 8)) % 8, (r / 8) % 8, r % 8,
	     r2 / (8 * 8 * 8), (r2 / ( 8 * 8)) % 8, (r2 / 8) % 8, r2 % 8);
    }
}

int	load_config_file(char *file, char verbose, int max_recursive_left)
{
  FILE	*fh;
  char	buffer[1024];
  char	**tb, *str;
  int	len, line, processTag;

  if (!max_recursive_left)
    {
      if (verbose)
	printf("[ERROR]Too much inclusions !!!\n");
      return (0);
    }
  processTag = 1;
  if ((fh = fopen(file, "r")))
    {
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
		      parse_tag(str);
		      if (parse_opened_tag < 0)
			{
			  if (verbose) printf("[ERROR]Too much tag closed at line %i in file '%s'!\n", line, file);
			  exit (2);
			}
		    }
		  else
		    {
		      if (verbose) printf("[ERROR]Error parsing line %i is not valid in file '%s'!\n", line, file);
		      exit (2);
		    }
		  if (is_for_user((char *)hash_get("USER"), verbose)
		      || is_for_group((char *)hash_get("GROUP"), verbose)
		      || is_for_rangeip((char *)hash_get("RANGEIP"), verbose)
		      || is_for_virtualhost((char *)hash_get("SERVER_IP"),
					    hash_get_int("SERVER_PORT"),
					    verbose)
		      || hash_get_int("DEFAULT") == 1)
		    processTag = 1;
		  else
		    processTag = 0;
		}
	      else if (processTag == 0)
		continue;
	      else if ((tb = parse_cut_string(str))) 
		{
		  if (tb[0])
		    processLine(tb, max_recursive_left, verbose);
		  free(tb);
		}
	    }
	}
      if (parse_opened_tag != 0)
	{
	  if (verbose) printf("[ERROR]Missing %i close(s) tag(s) in file '%s'!!!\n", parse_opened_tag, file);
	  exit (2);
	}
      fclose(fh);
    }
  else
    {
      if (verbose && strcmp(file, CONFIG_FILE))
	printf("[ERROR]Couldn't load config file '%s'. Error : %s\n", file, strerror(errno));
      return (0);
    }
  return (1);
}

void	processLine(char **tb, int max_recursive_left, char verbose)
{
  int	notRecognized;
  int	i;
  
  notRecognized = 1;
  for (i = 0; confParams[i].type != CONF_IS_EMPTY; i++)
    if (!strcmp(tb[0], confParams[i].name)
	&& (tb[1] || confParams[i].type == CONF_IS_STRING_MAYBE_EMPTY))
      {
	notRecognized = 0;
	switch (confParams[i].type)
	  {
	  case CONF_IS_STRING:
	    hash_set(tb[0], (void *)strdup(tb[1]));
	    break;
	  case CONF_IS_STRING_MAYBE_EMPTY:
	    hash_set(tb[0], (void *)(tb[1] ? strdup(tb[1]) : 0));
	    break;
	  case CONF_IS_PATH_RESOLVE_ENV:
	    {
	      char	*path = convert_str_with_resolv_env_to_str(tb[1]);
	      
	      hash_set(tb[0], (void *)convert_to_path(path));
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
	  }
	break;
      }
  if (notRecognized)
    {
      if (!strcmp(tb[0], "DefaultRights") && tb[1])
	{
	  notRecognized = 0;
	  hash_set_int("DefaultRightsFile", convert_mode_to_int(tb[1]));
	  if (tb[2])
	    hash_set_int("DefaultRightsDirectory", convert_mode_to_int(tb[2]));
	}
      else if (!strcmp(tb[0], "Include") && tb[1])
	{
	  notRecognized = 0;
	  load_config_file(tb[1], verbose, max_recursive_left - 1);
	}
      if (verbose && notRecognized)
	printf("Property '%s' is not recognized !\n", tb[0]);
    }
}
