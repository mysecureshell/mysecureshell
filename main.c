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
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "conf.h"
#include "hash.h"
#include "ip.h"
#include "prog.h"
#include "SftpServer/Sftp.h"
#include "SftpServer/Encoding.h"
#include "SftpServer/Log.h"
#include "security.h"

static void	showVersion(int showAll)
{
  (void )printf("MySecureShell is version "PACKAGE_VERSION" build on " __DATE__ "%s",
#ifdef DODEBUG
	       " with DEBUG"
#else
	       ""
#endif
	       );
  if (showAll)
    {
      (void )printf("\n\nOptions:\n  ACL support: "
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
    if (strcmp(av[i], "-c") == 0)
      i++;
    else if (strcmp(av[i], "--configtest") == 0)
      {
	load_config(verbose);
	(void )printf("Config is valid.\n");
	exit(0);
      }
    else if (strcmp(av[i], "--help") == 0)
      {
      help:
	(void )printf("Build:\n\t");
	showVersion(0);
	(void )printf("\nUsage:\n\t%s [verbose] [options]\n\nOptions:\n", av[0]);
	(void )printf("\t--configtest : test the config file and show errors\n");
	(void )printf("\t--help       : show this screen\n");
	(void )printf("\t--version    : show version of MySecureShell\n"); 
	(void )printf("\nVerbose:\n");
	(void )printf("\t-v           : add a level at verbose mode\n");
	exit(0);
      }
    else if (strcmp(av[i], "--version") == 0)
      {
	showVersion(1);
	exit(0);
      }
    else if (strcmp(av[i], "-v") == 0)
      verbose++;
    else
      {
	(void )printf("--- UNKNOW OPTION: %s ---\n\n", av[i]);
	goto help;
      }
}

int	main(int ac, char **av, char **env)
{
  int	is_sftp = 0;

  create_hash();
  if (ac == 3 && av[1] != NULL && av[2] != NULL
      && strcmp("-c", av[1]) != 0 && strstr(av[2], "sftp-server") == 0)
      is_sftp = 1;
  else
    parse_args(ac, av);
  load_config(0);
  if (is_sftp == 1)
    {
      tGlobal	*params;
      char	*hide_files, *deny_filter, *hostname;
      int	max, fd, sftp_version;

      hostname = get_ip(hash_get_int("ResolveIP"));
      params = calloc(1, sizeof(*params));
      params->who = SftpWhoGetStruct(1);
      if (params->who != NULL)
	{
	  params->who->time_begin = time(0);
	  params->who->pid = (unsigned int)getpid();
	  (void )strncat(params->who->home, (char *)hash_get("Home"), sizeof(params->who->home) - 1);
	  (void )strncat(params->who->user, (char *)hash_get("User"), sizeof(params->who->user) - 1);
	  (void )strncat(params->who->ip, hostname, sizeof(params->who->ip) - 1);
	}
      max = hash_get_int("LimitConnectionByUser");
      if (max > 0 && count_program_for_uid((char *)hash_get("User")) > max)
	{//too many connection for the account
	  if (params->who != NULL) params->who->status = SFTPWHO_EMPTY;
	  SftpWhoRelaseStruct();
	  delete_hash();
	  exit(10);
	}
      max = hash_get_int("LimitConnectionByIP");
      if (max > 0 && count_program_for_ip(hostname) > max)
	{//too many connection for this IP
	  if (params->who != NULL) params->who->status = SFTPWHO_EMPTY;
	  SftpWhoRelaseStruct();
	  delete_hash();
	  exit(11);
	}
      max = hash_get_int("LimitConnection");
      if (max > 0 && count_program_for_uid(0) > max)
	{//too many connection for the server
	  if (params->who != NULL) params->who->status = SFTPWHO_EMPTY;
	  SftpWhoRelaseStruct();
	  delete_hash();
	  exit(12);
	}
      if (hash_get_int("DisableAccount"))
	{//account is temporary disable
	  if (params->who != NULL) params->who->status = SFTPWHO_EMPTY;
	  SftpWhoRelaseStruct();
	  delete_hash();
	  exit(13);
	}
      //check if the server is up ans user is not admin
      if ((fd = open(SHUTDOWN_FILE, O_RDONLY)) >= 0)
	//server is down
	{
	  xclose(fd);
	  if (hash_get_int("IsAdmin") == 0)
	    {
	      if (params->who != NULL) params->who->status = SFTPWHO_EMPTY;
	      SftpWhoRelaseStruct();
	      delete_hash();
	      exit(0);
	    }
	}
      if (hash_get("LogFile") != NULL)
	mylog_open(strdup((char *)hash_get("LogFile")));
      else
	mylog_open(MSS_LOG);
      if (params->who == NULL)
	{
	  mylog_printf(MYLOG_ERROR, "[%s][%s]Server reached maximum connexion (%i clients)",
		       (char *)hash_get("User"), hostname, SFTPWHO_MAXCLIENT);
	  SftpWhoRelaseStruct();
	  delete_hash();
	  mylog_close();
	  exit(14);
	}

      hide_files = (char *)hash_get("HideFiles");
      deny_filter = (char *)hash_get("PathDenyFilter");

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
      _sftpglobal->download_max = (u_int32_t )hash_get_int("GlobalDownload");
      _sftpglobal->upload_max = (u_int32_t )hash_get_int("GlobalUpload");
      if (hash_get_int("Download") > 0)
	{
	  params->download_max = (u_int32_t )hash_get_int("Download");
	  params->who->download_max = params->download_max;
	}
      if (hash_get_int("Upload") > 0)
	{
	  params->upload_max = (u_int32_t )hash_get_int("Upload");
	  params->who->upload_max = params->upload_max;
	}
      if (hash_get_int("IdleTimeOut") > 0)
	params->who->time_maxidle = hash_get_int("IdleTimeOut");
      if (hash_get_int("DirFakeMode") > 0)
	params->who->mode = hash_get_int("DirFakeMode");
      if (hide_files != NULL && strlen(hide_files) > 0)
	{
	  int	r;

	  if ((r = regcomp(&params->hide_files_regexp, hide_files, REG_EXTENDED | REG_NOSUB | REG_NEWLINE)) == 0)
            params->has_hide_files = MSS_TRUE;
          else
            {
              char	buffer[256];

              (void )regerror(r, &params->hide_files_regexp, buffer, sizeof(buffer));
              mylog_printf(MYLOG_ERROR, "[%s][%s]Couldn't compile regex : %s",
			   params->who->user, params->who->ip, buffer);
            }
	}
      if (deny_filter != NULL && strlen(deny_filter) > 0)
	{
	  int	r;

	  if ((r = regcomp(&params->deny_filter_regexp, deny_filter, REG_EXTENDED | REG_NOSUB | REG_NEWLINE)) == 0)
            params->has_deny_filter = MSS_TRUE;
          else
            {
              char      buffer[256];

              (void )regerror(r, &params->deny_filter_regexp, buffer, sizeof(buffer));
              mylog_printf(MYLOG_ERROR, "[%s][%s]Couldn't compile regex : %s",
			   params->who->user, params->who->ip, buffer);
            }
	}
      sftp_version = hash_get_int("SftpProtocol");
      if (hash_get_int("ConnectionMaxLife") > 0)
	params->who->time_maxlife = hash_get_int("ConnectionMaxLife");
      if (hash_get("ExpireDate") != NULL)
	{
	  struct tm	tm;
	  time_t	currentTime, maxTime;

	  if (strptime((const char *)hash_get("ExpireDate"), "%Y-%m-%d %H:%M:%S", &tm) != NULL)
	    {
	      maxTime = mktime(&tm);
	      currentTime = time(NULL);
	      if (currentTime > maxTime) //time elapsed
		{
		  params->who->status = SFTPWHO_EMPTY;
		  SftpWhoRelaseStruct();
		  delete_hash();
		  mylog_close();
		  exit(15);
		}
	      else
		{ //check if expireDate < time_maxlife
		  currentTime = maxTime - currentTime;
		  if (currentTime < params->who->time_maxlife)
		    params->who->time_maxlife = currentTime;
		}
	    }
	  DEBUG((MYLOG_DEBUG, "[%s][%s]ExpireDate time to rest: %i",
		       params->who->user, params->who->ip, params->who->time_maxlife));
	}
      if (hash_get_int("MaxOpenFilesForUser") > 0)
	params->max_openfiles = hash_get_int("MaxOpenFilesForUser");
      if (hash_get_int("MaxReadFilesForUser") > 0)
	params->max_readfiles = hash_get_int("MaxReadFilesForUser");
      if (hash_get_int("MaxWriteFilesForUser") > 0)
	params->max_writefiles = hash_get_int("MaxWriteFilesForUser");
      if (hash_get_int("DefaultRightsDirectory") > 0)
	params->rights_directory = hash_get_int("DefaultRightsDirectory");
      else
	params->rights_directory = 0777;
      if (hash_get_int("DefaultRightsFile") > 0)
	params->rights_file = hash_get_int("DefaultRightsFile");
      else
	params->rights_file = 0666;
      if (hash_get("Charset") != NULL)
	  setCharset((char *)hash_get("Charset"));
      if (hash_get("GMTTime") != NULL)
	  mylog_time(atoi((char *)hash_get("GMTTime")));
      delete_hash();
      if (hostname != NULL)
	free(hostname);
      return (SftpMain(params, sftp_version));
    }
  else
    {
      char	*ptr;

      if (getuid() != geteuid())
	//if we are in utset byte mode then we restore user's rights to avoid security problems
	{
	  if (seteuid(getuid()) == -1 || setegid(getgid()) == -1)
	    {
	      perror("revoke root rights");
	      exit(1);
	    }
	}
      ptr = (char *)hash_get("Shell");
      av[0] = ptr;
      if (ptr != NULL)
	(void )execve(ptr, av, env);
      exit(1);
    }
}
