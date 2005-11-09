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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "conf.h"
#include "config.h"
#include "ip.h"
#include "parsing.h"
#include "string.h"
#include "user.h"

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
      char	*ptr;
      int	r, r2;

      printf("--- %s ---\n", (char *)hash_get("User"));
      printf("Home\t\t\t= %s\n", (char *)hash_get("Home"));
      printf("ByPassGlobalDownload\t= %s\n", hash_get_int("ByPassGlobalDownload") == 0 ? "false" : "true");
      printf("ByPassGlobalUpload\t= %s\n", hash_get_int("ByPassGlobalUpload") == 0 ? "false" : "true");
      printf("GlobalDownload\t\t= %i bytes/s\n", hash_get_int("GlobalDownload"));
      printf("GlobalUpload\t\t= %i bytes/s\n", hash_get_int("GlobalUpload"));
      printf("Download\t\t= %i bytes/s\n", hash_get_int("Download"));
      printf("Upload\t\t\t= %i bytes/s\n", hash_get_int("Upload"));
      printf("StayAtHome\t\t= %s\n", hash_get_int("StayAtHome") == 0 ? "false" : "true");
      printf("VirtualChroot\t\t= %s\n", hash_get_int("VirtualChroot") == 0 ? "false" : "true");
      printf("LimitConnection\t\t= %i\n", hash_get_int("LimitConnection"));
      printf("LimitConnectionByUser\t= %i\n", hash_get_int("LimitConnectionByUser"));
      printf("LimitConnectionByIP\t= %i\n", hash_get_int("LimitConnectionByIP"));
      printf("IdleTimeOut\t\t= %is\n", hash_get_int("IdleTimeOut"));
      printf("ResolveIP\t\t= %s\n", hash_get_int("ResolveIP") == 0 ? "false" : "true");
      printf("DirFakeUser\t\t= %s\n", hash_get_int("DirFakeUser") == 0 ? "false" : "true");
      printf("DirFakeGroup\t\t= %s\n", hash_get_int("DirFakeGroup") == 0 ? "false" : "true");
      r = hash_get_int("DirFakeMode");
      printf("DirFakeMode\t\t= %i%i%i%i\n", r / (8 * 8 * 8), (r / ( 8 * 8)) % 8, (r / 8) % 8, r % 8);
      ptr = (char *)hash_get("HideFiles");
      printf("HideFiles\t\t= %s\n", ptr ? ptr : "{nothing to hide}");
      printf("HideNoAccess\t\t= %s\n", hash_get_int("HideNoAccess") == 0 ? "false" : "true");
      printf("MaxOpenFilesForUser\t= %i\n", hash_get_int("MaxOpenFilesForUser"));
      printf("MaxReadFilesForUser\t= %i\n", hash_get_int("MaxReadFilesForUser"));
      printf("MaxWriteFilesForUser\t= %i\n", hash_get_int("MaxWriteFilesForUser"));
      printf("PathDenyFilter\t\t= %s\n", (char *)hash_get("PathDenyFilter"));
      ptr = (char *)hash_get("Shell");
      printf("Shell\t\t\t= %s\n", ptr ? ptr : "{no shell}");
      printf("ShowLinksAsLinks\t= %s\n", hash_get_int("ShowLinksAsLinks") == 0 ? "false" : "true");
      r = hash_get_int("DefaultRightsFile");
      r2 = hash_get_int("DefaultRightsDirectory");
      printf("DefaultRights\t\t= %i%i%i%i %i%i%i%i\n",
	     r / (8 * 8 * 8), (r / ( 8 * 8)) % 8, (r / 8) % 8, r % 8,
	     r2 / (8 * 8 * 8), (r2 / ( 8 * 8)) % 8, (r2 / 8) % 8, r2 % 8);
      printf("ConnectionMaxLife\t= %is\n", hash_get_int("ConnectionMaxLife"));
      r = hash_get_int("SftpProtocol");
      if (r)
	printf("SftpProtocol\t\t= %i\n", r);
      printf("DisableAccount\t\t= %s\n", hash_get_int("DisableAccount") == 0 ? "false" : "true");
      printf("IsAdmin\t\t\t= %s\n", hash_get_int("IsAdmin") == 0 ? "false" : "true");
    }
}

int	load_config_file(char *file, char verbose, int max_recursive_left)
{
  FILE	*fh;
  char	buffer[1024];
  char	**tb, *str;
  int	len, line, err1, err2;

  if (!max_recursive_left)
    {
      if (verbose)
	printf("[ERROR]Too much inclusions !!!\n");
      return (0);
    }
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
		}
	      else if ((tb = parse_cut_string(str))) 
		{
		  if (tb[0])
		    {
		      err1 = 0;
		      if (is_for_user((char *)hash_get("USER"), verbose)
			  || is_for_group((char *)hash_get("GROUP"), verbose)
			  || is_for_rangeip((char *)hash_get("RANGEIP"), verbose)
			  || is_for_virtualhost((char *)hash_get("SERVER_IP"),
			  						hash_get_int("SERVER_PORT"),
			  						verbose)
			  || hash_get_int("DEFAULT") == 1)
			{
			  if (!strcmp(tb[0], "GlobalDownload") && tb[1])
			    hash_set_int(tb[0], convert_speed_to_int(tb + 1));
			  else if (!strcmp(tb[0], "GlobalUpload") && tb[1])
			    hash_set_int(tb[0], convert_speed_to_int(tb + 1));
			  else if (!strcmp(tb[0], "Download") && tb[1])
			    hash_set_int(tb[0], convert_speed_to_int(tb + 1));
			  else if (!strcmp(tb[0], "Upload") && tb[1])
			    hash_set_int(tb[0], convert_speed_to_int(tb + 1));
			  else if (!strcmp(tb[0], "StayAtHome") && tb[1])
			    hash_set_int(tb[0], convert_boolean_to_int(tb[1]));
			  else if (!strcmp(tb[0], "VirtualChroot") && tb[1])
			    hash_set_int(tb[0], convert_boolean_to_int(tb[1]));
			  else if (!strcmp(tb[0], "LimitConnection") && tb[1])
			    hash_set_int(tb[0], atoi(tb[1]));
			  else if (!strcmp(tb[0], "LimitConnectionByUser") && tb[1])
			    hash_set_int(tb[0], atoi(tb[1]));
			  else if (!strcmp(tb[0], "LimitConnectionByIP") && tb[1])
			    hash_set_int(tb[0], atoi(tb[1]));
			  else if (!strcmp(tb[0], "Home") && tb[1])
			    hash_set(tb[0], (void *)convert_str_with_resolv_env_to_str(tb[1]));
			  else if (!strcmp(tb[0], "Shell") && tb[1])
			    hash_set(tb[0], (void *)strdup(tb[1]));
			  else if (!strcmp(tb[0], "ResolveIP") && tb[1])
			    hash_set_int(tb[0], convert_boolean_to_int(tb[1]));
			  else if (!strcmp(tb[0], "IdleTimeOut") && tb[1])
			    hash_set_int(tb[0], atoi(tb[1]));
			  else if (!strcmp(tb[0], "IgnoreHidden") && tb[1])
			    hash_set_int(tb[0], convert_boolean_to_int(tb[1]));
			  else if (!strcmp(tb[0], "DirFakeUser") && tb[1])
			    hash_set_int(tb[0], convert_boolean_to_int(tb[1]));
			  else if (!strcmp(tb[0], "DirFakeGroup") && tb[1])
			    hash_set_int(tb[0], convert_boolean_to_int(tb[1]));
			  else if (!strcmp(tb[0], "DirFakeMode") && tb[1])
			    hash_set_int(tb[0], convert_mode_to_int(tb[1]));
			  else if (!strcmp(tb[0], "HideFiles"))
			    hash_set(tb[0], (void *)(tb[1] ? strdup(tb[1]) : 0));
			  else if (!strcmp(tb[0], "HideNoAccess") && tb[1])
			    hash_set_int(tb[0], convert_boolean_to_int(tb[1]));
			  else if (!strcmp(tb[0], "ByPassGlobalDownload") && tb[1])
			    hash_set_int(tb[0], convert_boolean_to_int(tb[1]));
			  else if (!strcmp(tb[0], "ByPassGlobalUpload") && tb[1])
			    hash_set_int(tb[0], convert_boolean_to_int(tb[1]));
			  else if (!strcmp(tb[0], "MaxOpenFilesForUser") && tb[1])
			    hash_set_int(tb[0], atoi(tb[1]));
			  else if (!strcmp(tb[0], "MaxReadFilesForUser") && tb[1])
			    hash_set_int(tb[0], atoi(tb[1]));
			  else if (!strcmp(tb[0], "MaxWriteFilesForUser") && tb[1])
			    hash_set_int(tb[0], atoi(tb[1]));
			  else if (!strcmp(tb[0], "ShowLinksAsLinks") && tb[1])
                            hash_set_int(tb[0], convert_boolean_to_int(tb[1]));
			  else if (!strcmp(tb[0], "DefaultRights") && tb[1])
			    {
			      hash_set_int("DefaultRightsFile", convert_mode_to_int(tb[1]));
			      if (tb[2])
				hash_set_int("DefaultRightsDirectory", convert_mode_to_int(tb[2]));
			    }
			  else if (!strcmp(tb[0], "PathDenyFilter") && tb[1])
			    hash_set(tb[0], (void *)strdup(tb[1]));
			  else if (!strcmp(tb[0], "SftpProtocol") && tb[1])
                            hash_set_int(tb[0], atoi(tb[1]));
			  else if (!strcmp(tb[0], "ConnectionMaxLife") && tb[1])
			    hash_set_int(tb[0], convert_time_to_int(tb + 1));
			  else if (!strcmp(tb[0], "DisableAccount") && tb[1])
			    hash_set_int(tb[0], convert_boolean_to_int(tb[1]));
			  else if (!strcmp(tb[0], "IsAdmin") && tb[1])
                            hash_set_int(tb[0], convert_boolean_to_int(tb[1]));
			  else
			    err1 = 1;
			}
		      err2 = 0;
		      if (!strcmp(tb[0], "Include") && tb[1])
			load_config_file(tb[1], verbose, max_recursive_left - 1);
		      else
			err2 = 1;

		      if (verbose && err1 && err2)
			printf("Property '%s' is not recognized !\n", tb[0]);
		    }
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
