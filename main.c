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
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "conf.h"
#include "ip.h"
#include "parsing.h"
#include "prog.h"
#include "string.h"
#include "SftpServer/Sftp.h"
#include "SftpServer/Encoding.h"
#include "SftpServer/Log.h"

static void	showVersion(int showAll)
{
  printf("MySecureShell is version "PACKAGE_VERSION" build on " __DATE__ "%s",
#ifdef DODEBUG
	       " with DEBUG"
#else
	       ""
#endif
	       );
  if (showAll)
    {
      printf("\n\nOptions:\n  ACL support: "
#if(HAVE_LIBACL)
	     "yes"
#else
	     "no"
#endif
	     "\n  UTF-8 support: "
#if(HAVE_ICONV||HAVE_LIBICONV)
	     "yes"
#else
	     "no"
#endif
	     "\n\nSftp Extensions:\n"
#ifdef MSSEXT_DISKUSAGE
	     "  Disk Usage\n"
#endif
#ifdef MSSEXT_FILE_HASHING
	     "  File Hashing\n"
#endif
	     );
    }
}

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
	exit(0);
      }
    else if (!strcmp(av[i], "--help"))
      {
      help:
	printf("Build:\n\t");
	showVersion(0);
	printf("\nUsage:\n\t%s [verbose] [options]\n\nOptions:\n", av[0]);
	printf("\t--configtest : test the config file and show errors\n");
	printf("\t--help       : show this screen\n");
	printf("\t--version    : show version of MySecureShell\n"); 
	printf("\nVerbose:\n");
	printf("\t-v           : add a level at verbose mode\n");
	exit(0);
      }
    else if (!strcmp(av[i], "--version"))
      {
	showVersion(1);
	exit(0);
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
  int	is_sftp = 0;

  create_hash();
  if (ac == 3 && av[1] && av[2] && !strcmp("-c", av[1]) && strstr(av[2], "sftp-server"))
      is_sftp = 1;
  else
    parse_args(ac, av);
  load_config(0);
  if (is_sftp)
    {
      tGlobal	*params;
      char	*hide_files, *deny_filter, *hostname;
      int	max, fd, sftp_version;

      hostname = get_ip(hash_get_int("ResolveIP"));
      params = malloc(sizeof(*params));
      memset(params, 0, sizeof(*params));
      params->who = SftpWhoGetStruct(1);
      params->who->time_begin = time(0);
      params->who->pid = (unsigned int)getpid();
      snprintf(params->who->home, sizeof(params->who->home), "%s", (char *)hash_get("Home"));
      snprintf(params->who->user, sizeof(params->who->user), "%s", (char *)hash_get("User"));
      snprintf(params->who->ip, sizeof(params->who->ip), "%s", hostname);
      max = hash_get_int("LimitConnectionByUser");
      if (max > 0 && count_program_for_uid((char *)hash_get("User")) > max)
	{//too many connection for the account
	  params->who->status = SFTPWHO_EMPTY;
	  SftpWhoRelaseStruct();
	  delete_hash();
	  exit(10);
	}
      max = hash_get_int("LimitConnectionByIP");
      if (max > 0 && count_program_for_ip(hostname) > max)
	{//too many connection for this IP
	  params->who->status = SFTPWHO_EMPTY;
	  SftpWhoRelaseStruct();
	  delete_hash();
	  exit(11);
	}
      max = hash_get_int("LimitConnection");
      if (max > 0 && count_program_for_uid(0) > max)
	{//too many connection for the server
	  params->who->status = SFTPWHO_EMPTY;
	  SftpWhoRelaseStruct();
	  delete_hash();
	  exit(12);
	}
      if (hash_get_int("DisableAccount"))
	{//account is temporary disable
	  params->who->status = SFTPWHO_EMPTY;
	  SftpWhoRelaseStruct();
	  delete_hash();
	  exit(13);
	}
      //check if the server is up ans user is not admin
      if ((fd = open(SHUTDOWN_FILE, O_RDONLY)) >= 0)
	//server is down
	{
	  close(fd);
	  if (!hash_get_int("IsAdmin"))
	    {
	      params->who->status = SFTPWHO_EMPTY;
	      SftpWhoRelaseStruct();
	      delete_hash();
	      exit(0);
	    }
	}

      if ((hide_files = (char *)hash_get("HideFiles"))) hide_files = strdup(hide_files);
      if ((deny_filter = (char *)hash_get("PathDenyFilter"))) deny_filter = strdup(deny_filter);

      params->who->status |=
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
	(hash_get_int("IsAdmin") ? SFTPWHO_IS_ADMIN : 0) +
	(hash_get_int_with_default("CanRemoveDir", 1) ? SFTPWHO_CAN_RMDIR : 0) +
	(hash_get_int_with_default("CanRemoveFile", 1) ? SFTPWHO_CAN_RMFILE : 0)
	;
      _sftpglobal->download_max = hash_get_int("GlobalDownload");
      _sftpglobal->upload_max = hash_get_int("GlobalUpload");
      if (hash_get_int("Download"))
	{
	  params->download_max = hash_get_int("Download");
	  params->who->download_max = params->download_max;
	}
      if (hash_get_int("Upload"))
	{
	  params->upload_max = hash_get_int("Upload");
	  params->who->upload_max = params->upload_max;
	}
      if (hash_get_int("IdleTimeOut"))
	params->who->time_maxidle = hash_get_int("IdleTimeOut");
      if (hash_get_int("DirFakeMode"))
	params->who->mode = hash_get_int("DirFakeMode");
      if (hide_files && strlen(hide_files) > 0)
	{
	  int	r;

	  if (!(r = regcomp(&params->hide_files_regexp, hide_files, REG_EXTENDED | REG_NOSUB | REG_NEWLINE)))
            params->hide_files = strdup(hide_files);
          else
            {
              char	buffer[256];

              regerror(r, &params->hide_files_regexp, buffer, sizeof(buffer));
              mylog_printf(MYLOG_ERROR, "[%s][%s]Couldn't compile regex : %s",
			   params->who->user, params->who->ip, buffer);
            }
	}
      if (deny_filter && strlen(deny_filter) > 0)
	{
	  int	r;

	  if (!(r = regcomp(&params->deny_filter_regexp, deny_filter, REG_EXTENDED | REG_NOSUB | REG_NEWLINE)))
            params->deny_filter = strdup(deny_filter);
          else
            {
              char      buffer[256];

              regerror(r, &params->deny_filter_regexp, buffer, sizeof(buffer));
              mylog_printf(MYLOG_ERROR, "[%s][%s]Couldn't compile regex : %s",
			   params->who->user, params->who->ip, buffer);
            }
	}
      sftp_version = hash_get_int("SftpProtocol");
      if (hash_get_int("ConnectionMaxLife"))
	params->who->time_maxlife = hash_get_int("ConnectionMaxLife");
      if (hash_get_int("MaxOpenFilesForUser"))
	params->max_openfiles = hash_get_int("MaxOpenFilesForUser");
      if (hash_get_int("MaxReadFilesForUser"))
	params->max_readfiles = hash_get_int("MaxReadFilesForUser");
      if (hash_get_int("MaxWriteFilesForUser"))
	params->max_writefiles = hash_get_int("MaxWriteFilesForUser");
      if (hash_get_int("DefaultRightsDirectory"))
	params->rights_directory = hash_get_int("DefaultRightsDirectory");
      else
	params->rights_directory = 0777;
      if (hash_get_int("DefaultRightsFile"))
	params->rights_file = hash_get_int("DefaultRightsFile");
      else
	params->rights_file = 0666;
      if (hash_get("Charset"))
	  setCharset(strdup((char *)hash_get("Charset")));
      if (hash_get("GMTTime"))
	  mylog_time(atoi((char *)hash_get("GMTTime")));
      if (hash_get("LogFile"))
	mylog_open(strdup((char *)hash_get("LogFile")));
      else
	mylog_open(MSS_LOG);
      delete_hash();
      if (deny_filter)
	free(deny_filter);
      if (hide_files)
	free(hide_files);
      if (hostname)
	free(hostname);
      return (SftpMain(params, sftp_version));
    }
  else
    {
      char	*ptr;

      if (getuid() != geteuid())
	//if we are in utset byte mode then we restore user's rights to avoid security problems
	{
	  seteuid(getuid());
	  setegid(getgid());
	}
      ptr = (char *)hash_get("Shell");
      av[0] = ptr;
      if (ptr)
	{
	  execve(ptr, av, env);
	  exit(1);
	}
    }
  return (0);
}
