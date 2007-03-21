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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../config.h"
#include "SftpWho.h"

int	main(int ac, char **av)
{
  int	i, fd, assume_yes_to_all, do_clean;

  assume_yes_to_all = 0;
  do_clean = 0;
  if (ac > 1)
    for (i = 1; i < ac; i++)
      if (strcmp(av[i], "fullstop") == 0)
	{
	  assume_yes_to_all = 1;
	  do_clean = 1;
	  goto doShutdown;
	}
      else if (strcmp(av[i], "shutdown") == 0 || strcmp(av[i], "stop") == 0)
	{
	doShutdown:
	  if ((fd = open(SHUTDOWN_FILE, O_CREAT | O_TRUNC | O_RDWR, 0644)) >= 0)
	    {
	      char	buf[4];

	      fchmod(fd, 0644);
	      (void )printf("Shutdown server for new connection (active connection are keeped)\n");
	      (void )printf("Do you want to kill all users ? [YES/no] ");
	      (void )fflush(stdout);
	      if (assume_yes_to_all == 0)
		i = read(0, buf, sizeof(buf));
	      else
		(void )printf("yes\n");
	      buf[i >= 1 ? i - 1 : 0] = '\0';
	      if (assume_yes_to_all == 1 || strcasecmp(buf, "yes") == 0 || strcasecmp(buf, "y") == 0)
		{
		  (void )system("sftp-kill all > /dev/null");
		  if (do_clean == 1)
		    {
		      if (SftpWhoDeleteStructs() == 0)
			(void )printf("Can't clean server: %s\n", strerror(errno));
		    }
		}
	      else
		(void )printf("Clients aren't disconnected\n");
	      (void )close(fd);
	    }
	  else
	    (void )printf("Can't shutdown server: %s\n", strerror(errno));
	}
      else if (strcmp(av[i], "active") == 0 || strcmp(av[i], "start") == 0)
	{
	  if (unlink(SHUTDOWN_FILE) == 0 || errno == ENOENT)
	    (void )printf("Server is now online.\n");
	  else
	    (void )printf("Can't wake up server: %s\n", strerror(errno));
	}
      else if (strcmp(av[i], "-yes") == 0)
	assume_yes_to_all = 1;
      else
	{
	  (void )printf("Usage:\n------\n\n");
	  (void )printf("%s {options} {states}\n\n", av[0]);
	  (void )printf("\nOptions:\n");
	  (void )printf("\t-yes : assume yes to all questions\n");
	  (void )printf("\nStates:\n");
	  (void )printf("\t- active : wake up server\n");
	  (void )printf("\t- start : same as 'active'\n");
	  (void )printf("\t- shutdown : shutdown the server (but don't kill current connections)\n");
	  (void )printf("\t- stop : same as 'shutdown'\n");
	  (void )printf("\t- fullstop : shutdown the server (kill all connections and clean memory)\n");
	}
  else
    {
      if ((fd = open(SHUTDOWN_FILE, O_RDONLY)) >= 0)
	(void )close(fd);
      (void )printf("Server is %s\n", fd == -1 ? "up" : "down");
    }
  return (0);
}
