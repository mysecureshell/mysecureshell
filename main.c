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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "conf.h"
#include "defines.h"
#include "ip.h"
#include "parsing.h"
#include "prog.h"
#include "string.h"
#include "SftpServer/SftpWho.h"

static void	parse_args(int ac, char **av)
{
  int		verbose = 1;
  int		i;

  if (ac == 1)
    return;
  for (i = 1; i < ac; i++)
    if (!strcmp(av[i], "-c"))
      i++;
    else if (!strcmp(av[i], "--configtest"))
      {
	load_config(verbose);
	printf("Config is valid.\n");
	exit (0);
      }
    else if (!strcmp(av[i], "--help"))
      {
      help:
	printf("Build:\n\t%s is version %s build on " __DATE__ "\n\n", av[0], "0.7b2");
	printf("Usage:\n\t%s [verbose] [options]\n\nOptions:\n", av[0]);
	printf("\t--configtest : test the config file and show errors\n");
	printf("\t--help       : show this screen\n");
	printf("\nVerbose:\n");
	printf("\t-v           : add a level at verbose mode\n");
	exit (0);
      }
    else if (!strcmp(av[i], "-v"))
      verbose++;
    else
      {
	printf("--- UNKNOW OPTION: %s ---\n\n", av[i]);
	goto help;
      }
}

int	main(int ac, char **av, char **env)
{
  char	*exe = "";
  int	is_sftp = 0;

  create_hash();
  if (ac == 3 && av[1] && av[2] &&
      !strcmp("-c", av[1]) &&
      strstr(av[2], "sftp-server"))
    {
      int	len = strlen(av[2]);
      int	is_exe = 0;

      if (!strcmp(av[2] + len - 4, ".exe"))
	{
	  is_exe = 1;
	  av[2][len - 4] = 0;
	}
      exe = malloc(strlen(av[2]) + 5 + (is_exe == 1 ? 4 : 0));
      strcpy(exe, av[2]);
      strcat(exe, "_MSS");
      if (is_exe)
	strcat(exe, ".exe");
      is_sftp = 1;
    }
  else
    parse_args(ac, av);
  load_config(0);
  if (is_sftp)
    {
      char	**args;
      char	*hide_files, *deny_filter;
      char	bmode[12];
      int	max, nb_args, fd;

      max = hash_get_int("LimitConnectionByUser");
      if (max > 0 && count_program_for_uid(getuid(), (char *)hash_get("User")) >= max)
	//too many connection for the account
	exit(10);
      max = hash_get_int("LimitConnectionByIP");
      if (max > 0 && count_program_for_ip(getuid(), get_ip(hash_get_int("ResolveIP"))) >= max)
	//too many connection for this IP
	exit(11);
      max = hash_get_int("LimitConnection");
      if (max > 0 && count_program_for_uid(-1, 0) >= max)
	//too many connection for the server
        exit(12);
      if (hash_get_int("DisableAccount"))
	//account is temporary disable
	exit (13);
      if (getuid() != geteuid())
	//if we are in utset byte mode then we restore user's rights to avoid security problems
	{
	  seteuid(getuid());
	  setegid(getgid());
	}

      if ((hide_files = (char *)hash_get("HideFiles"))) hide_files = strdup(hide_files);
      if ((deny_filter = (char *)hash_get("PathDenyFilter"))) deny_filter = strdup(deny_filter);

      snprintf(bmode, sizeof(bmode), "%i",
	       (hash_get_int("StayAtHome") ? SFTPWHO_STAY_AT_HOME : 0) +
	       (hash_get_int("VirtualChroot") ? SFTPWHO_VIRTUAL_CHROOT : 0) +
	       (hash_get_int("ResolveIP") ? SFTPWHO_RESOLVE_IP : 0) +
	       (hash_get_int("IgnoreHidden") ? SFTPWHO_IGNORE_HIDDEN : 0) +
	       (hash_get_int("DirFakeUser") ? SFTPWHO_FAKE_USER : 0) +
	       (hash_get_int("DirFakeGroup") ? SFTPWHO_FAKE_GROUP : 0) +
	       (hash_get_int("DirFakeMode") ? SFTPWHO_FAKE_MODE : 0) +
	       (hash_get_int("HideNoAccess") ? SFTPWHO_HIDE_NO_ACESS : 0) +
	       (hash_get_int("ByPassGlobalDownload") ? SFTPWHO_BYPASS_GLB_DWN : 0) +
	       (hash_get_int("ByPassGlobalUpload") ? SFTPWHO_BYPASS_GLB_UPL : 0) +
	       (hash_get_int("ShowLinksAsLinks") ? SFTPWHO_LINKS_AS_LINKS : 0) + 
	       (hash_get_int("IsAdmin") ? SFTPWHO_IS_ADMIN : 0)
	       );

      args = calloc(sizeof(*args), 40);//be aware of the buffer overflow
      nb_args = 0;
      args[nb_args++] = exe;
      args[nb_args++] = "--user";
      args[nb_args++] = strdup((char *)hash_get("User"));
      args[nb_args++] = "--home";
      args[nb_args++] = strdup((char *)hash_get("Home"));
      args[nb_args++] = "--ip";
      args[nb_args++] = get_ip(hash_get_int("ResolveIP"));
      args[nb_args++] = "--mode";
      args[nb_args++] = bmode;
      if (hash_get_int("GlobalDownload"))
	{
	  args[nb_args++] = "--global-download";
	  args[nb_args++] = hash_get_int_to_char("GlobalDownload");
	}
      if (hash_get_int("GlobalUpload"))
	{
	  args[nb_args++] = "--global-upload";
          args[nb_args++] = hash_get_int_to_char("GlobalUpload");
	}
      if (hash_get_int("Download"))
	{
	  args[nb_args++] = "--download";
	  args[nb_args++] = hash_get_int_to_char("Download");
	}
      if (hash_get_int("Upload"))
	{
	  args[nb_args++] = "--upload";
          args[nb_args++] = hash_get_int_to_char("Upload");
	}
      if (hash_get_int("IdleTimeOut"))
	{
	  args[nb_args++] = "--idle";
	  args[nb_args++] = hash_get_int_to_char("IdleTimeOut");
	}
      if (hash_get_int("DirFakeMode"))
	{
	  args[nb_args++] = "--fake-mode";
	  args[nb_args++] = hash_get_int_to_char("DirFakeMode");
	}
      if (hide_files && strlen(hide_files) > 0)
	{
	  args[nb_args++] = "--hide-files";
          args[nb_args++] = hide_files;
	}
      if (hash_get_int("MaxOpenFilesForUser"))
	{
	  args[nb_args++] = "--max-open-files";
	  args[nb_args++] = hash_get_int_to_char("MaxOpenFilesForUser");
	}
      if (hash_get_int("MaxReadFilesForUser"))
	{
	  args[nb_args++] = "--max-read-files";
	  args[nb_args++] = hash_get_int_to_char("MaxReadFilesForUser");
	}
      if (hash_get_int("MaxWriteFilesForUser"))
	{
	  args[nb_args++] = "--max-write-files";
	  args[nb_args++] = hash_get_int_to_char("MaxWriteFilesForUser");
	}
      if (hash_get_int("DefaultRightsDirectory"))
	{
	  args[nb_args++] = "--rights-directory";
	  args[nb_args++] = hash_get_int_to_char("DefaultRightsDirectory");
	}
      if (hash_get_int("DefaultRightsFile"))
	{
	  args[nb_args++] = "--rights-file";
	  args[nb_args++] = hash_get_int_to_char("DefaultRightsFile");
	}
      if (deny_filter && strlen(deny_filter) > 0)
	{
	  args[nb_args++] = "--deny-filter";
	  args[nb_args++] = deny_filter;
	}
      if (hash_get_int("SftpProtocol"))
	{
	  args[nb_args++] = "--protocol";
          args[nb_args++] = hash_get_int_to_char("SftpProtocol");
	}
      if (hash_get_int("ConnectionMaxLife"))
	{
          args[nb_args++] = "--max-life";
          args[nb_args++] = hash_get_int_to_char("ConnectionMaxLife");
        }
      //check if the server is up ans user is not admin
      if ((fd = open(SHUTDOWN_FILE, O_RDONLY)) >= 0 && !(hash_get_int("IsAdmin")))
	//server is down
	{
	  close(fd);
	  exit(0);
	}
      delete_hash();
      execve(exe, args, env);
      exit (1);
    }
  else
    {
      char	**tb;
      char	*ptr;

      if (getuid() != geteuid())
	//if we are in utset byte mode then we restore user's rights to avoid security problems
	{
	  seteuid(getuid());
	  setegid(getgid());
	}
      ptr = (char *)hash_get("Shell");
      tb = calloc(2, sizeof(*tb));
      tb[0] = ptr;
      if (ptr)
	{
	  execve(ptr, tb, env);
	  exit (1);
	}
    }
  return (0);
}
