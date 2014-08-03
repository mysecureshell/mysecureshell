
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
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../SftpServer/SftpWho.h"
#include "../Core/conf.h"
#include "../Core/hash.h"

static unsigned int	do_loop = 0;
static int		_verbose = 0;
static int		_only_show_pid_and_name = 0;

static int	is_number(const char *av)
{
  int		i;

  if (av)
    {
      for (i = 0; av[i] != '\0'; i++)
	if (av[i] < '0' || av[i] > '9')
	  return (0);
      return (1);
    }
  return (0);
}

static void	parse_args(int ac, char **av)
{
  int		i;

  for (i = 1; i < ac; i++)
    if (strcmp(av[i], "--while") == 0)
      {
	if (is_number(av[i + 1]) == 1)
	  {
	    i++;
	    do_loop = (unsigned int )atoi(av[i]);
	  }
	else
	  do_loop = 1;
      }
    else if (strcmp(av[i], "-v") == 0)
      _verbose = 1;
    else if (strcmp(av[i], "--sftp-kill") == 0)
      _only_show_pid_and_name = 1;
    else
      {
	(void )printf("Usage:\n------\n%s [options]\n\nOptions:\n", av[0]);
	(void )printf("\t-v : verbose mode\n");
	(void )printf("\t--while [time in seconde] : sftp-who refresh informations (use ^C to quit)\n");
	(void )printf("\n");
	exit (0);
      }
}

static char	*make_idle_time(unsigned int t)
{
  static char	buffer[256];

  if (t < 60)
    (void )snprintf(buffer, sizeof(buffer), "%02us", t);
  else if (t < (60 * 60))
    (void )snprintf(buffer, sizeof(buffer), "%02umins %02us", t / 60, t % 60);
  else if (t < (60 * 60 * 24))
    (void )snprintf(buffer, sizeof(buffer), "%02uh %02umins %02us",
		    t / (60 * 60), (t / 60) % 60, t % 60);
  else
    (void )snprintf(buffer, sizeof(buffer), "%udays %02uh %02umins %02us",
		    t / (60 * 60 * 24), (t / (60 * 60)) % 24, (t / 60) % 60, t % 60);
  return (buffer);
}

static char	*make_time(unsigned int t)
{
  static char	b1[256];
  struct tm	*tm;
  time_t	tt = t;

  if ((tm = localtime(&tt)))
    {
      (void )strftime(b1, sizeof(b1), "%G/%m/%d %T", tm);
    }
  else
    b1[0] = '\0';
  return (b1);
}

static char	*make_speed(char *b2, size_t size, unsigned int s, int can_unlimit)
{
  if (can_unlimit == 1 && s == 0)
    (void )snprintf(b2, size, "%s", "unlimited");
  else if (s < 1024)
    (void )snprintf(b2, size, "%u bytes/s", s);
  else if (s < (1024 * 1024))
    (void )snprintf(b2, size, "%.2f kbytes/s", (float )s / 1024.0f);
  else
    (void )snprintf(b2, size, "%.3f mbytes/s", (float )s / 1048576.0f);
  return (b2);
}

static unsigned int	getRealDown(t_sftpwho *who)
{
  if (_sftpglobal->download_by_client != 0&& (who->status & SFTPWHO_BYPASS_GLB_DWN) == 0
      && ((_sftpglobal->download_by_client < who->download_max) || who->download_max == 0))
      return (_sftpglobal->download_by_client);
  return (who->download_max);
}

static unsigned int	getRealUp(t_sftpwho *who)
{
  if (_sftpglobal->upload_by_client != 0 && (who->status & SFTPWHO_BYPASS_GLB_UPL) == 0
      && ((_sftpglobal->upload_by_client < who->upload_max) || who->upload_max == 0))
      return (_sftpglobal->upload_by_client);
  return (who->upload_max);
}

int		main(int ac, char **av)
{
  t_sftpwho	*who;
  char		b1[18], b2[18], b3[18], b4[18];
  int		nb_clients;
  int		i;

  parse_args(ac, av);
  create_hash();
  load_config(0);
  who = SftpWhoGetStruct(-1);
  do
    {
      if (do_loop > 0)
	(void )printf("\33[H\33[J");
      nb_clients = 0;
      if (who)
	{
	  if (_only_show_pid_and_name == 0)
	    {
	      unsigned int	global_download, global_upload;

	      global_download = 0;
	      global_upload = 0;
	      for (i = 0; i < SFTPWHO_MAXCLIENT; i++)
		if ((who[i].status & SFTPWHO_STATUS_MASK) != SFTPWHO_EMPTY)
		  {
		    nb_clients++;
		    global_download += who[i].download_current;
		    global_upload += who[i].upload_current;
		  }
	      (void )printf("--- %i / %i clients ---\n", nb_clients, hash_get_int("LimitConnection"));
	      (void )printf("Global used bandwith : %s / %s\n",
			    make_speed(b1, sizeof(b1), global_download, 0),
			    make_speed(b2, sizeof(b2), global_upload, 0)
			    );
	    }
	  for (i = 0; i < SFTPWHO_MAXCLIENT; i++)
	    if ((who[i].status & SFTPWHO_STATUS_MASK) != SFTPWHO_EMPTY)
	      {
		if (_only_show_pid_and_name == 1)
		  {
		    (void )printf("%u %s\n", who[i].pid, who[i].user);
		  }
		else
		  {
		    char	*status;
		    
		    switch (who[i].status & SFTPWHO_STATUS_MASK)
		      {
		      case SFTPWHO_IDLE:
			status = "idle";
			break;
		      case SFTPWHO_GET:
			status = "download";
			break;
		      case SFTPWHO_PUT:
			status = "upload";
			break;
		      default:
			status = "unknown";
			break;
		      }
		    (void )printf("PID: %u   Name: %s   IP: %s\n", who[i].pid, who[i].user, who[i].ip);
		    (void )printf("\tHome: %s\n", who[i].home);
		    if (_verbose > 0)
		      (void )printf("\tOptions: %s%s%s%s%s%s%s%s\n",
				    (who[i].status & SFTPWHO_STAY_AT_HOME) ? " StayAtHome" : "",
				    (who[i].status & SFTPWHO_VIRTUAL_CHROOT) ? " VirtualChroot" : "",
				    (who[i].status & SFTPWHO_RESOLVE_IP) ? " ResolveIp" : "",
				    (who[i].status & SFTPWHO_IGNORE_HIDDEN) ? " IgnoreHidden" : "",
				    (who[i].status & SFTPWHO_FAKE_USER) ? " FakeUser" : "",
				    (who[i].status & SFTPWHO_FAKE_GROUP) ? " FakeGroup" : "",
				    (who[i].status & SFTPWHO_FAKE_MODE) ? " FakeMode" : "",
				    (who[i].status & SFTPWHO_HIDE_NO_ACESS) ? " HideNoAccess" : ""
				    );
		    if (_verbose > 0)
		      {
			(void )printf("\tStatus: %s [since %s]   Path: %s\n", status,
				      make_idle_time(
						     (who[i].status & SFTPWHO_STATUS_MASK) == SFTPWHO_IDLE ?
						     who[i].time_idle : who[i].time_transf),
				      who[i].path);
		      }
		    else
		      {
			(void )printf("\tStatus: %s    Path: %s\n", status, who[i].path);
		      }

		    if ((who[i].status & SFTPWHO_STATUS_MASK) != SFTPWHO_GET)
		      {
			(void )printf("\tFile: %s\n",
				      (who[i].status & SFTPWHO_STATUS_MASK) == SFTPWHO_IDLE ?  "" : who[i].file);
		      }
		    else
		      {
			(void )printf("\tFile: %s [%i%%]\n",
				      (who[i].status & SFTPWHO_STATUS_MASK) == SFTPWHO_IDLE ?  "" : who[i].file,
				      who[i].download_pos);
		      }
		    (void )printf("\tConnected: %s [since %s]\n",
				  make_time(who[i].time_begin), make_idle_time(who[i].time_total));
		    (void )printf("\tSpeed: Download: %s [%s]  Upload: %s [%s]\n",
				  make_speed(b1, sizeof(b1), who[i].download_current, 0),
				  make_speed(b2, sizeof(b2), getRealDown(&who[i]), 1),
				  make_speed(b3, sizeof(b3), who[i].upload_current, 0),
				  make_speed(b4, sizeof(b4), getRealUp(&who[i]), 1));
		    (void )printf("\tTotal: Download: %u bytes   Upload: %u bytes\n",
				  who[i].download_total, who[i].upload_total);
		    (void )printf("\n");
		  }
	      }
	}
      else
	{
	  if (_only_show_pid_and_name == 0)
	    (void )printf("--- %i / %i clients ---\n", nb_clients, hash_get_int("LimitConnection"));
	  who = SftpWhoGetStruct(-1);
	}
      if (do_loop > 0)
	(void )sleep(do_loop);
    }
  while (do_loop > 0);
  SftpWhoReleaseStruct(NULL);
  delete_hash();
  return (0);
}
