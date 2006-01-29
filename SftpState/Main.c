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
      if (!strcmp(av[i], "fullshutdown"))
	{
	  assume_yes_to_all = 1;
	  do_clean = 1;
	  goto doShutdown;
	}
      else if (!strcmp(av[i], "shutdown") || !strcmp(av[i], "stop"))
	{
	doShutdown:
	  if ((fd = open(SHUTDOWN_FILE, O_CREAT | O_TRUNC | O_RDWR, 0644)) >= 0)
	    {
	      char	buf[4];

	      fchmod(fd, 0644);
	      printf("Shutdown server for new connection (active connection are keeped)\n");

	      printf("Do you want to kill all users ? [YES/no] ");
	      fflush(stdout);
	      if (!assume_yes_to_all)
		i = read(0, buf, sizeof(buf));
	      else
		printf("yes\n");
	      buf[i >= 1 ? i - 1 : 0] = 0;
	      if (assume_yes_to_all || !strcasecmp(buf, "yes") || !strcasecmp(buf, "y"))
		{
		  system("sftp-kill all > /dev/null");
		  if (do_clean)
		    {
		      if (SftpWhoDeleteStructs() == 0)
			printf("Can't clean server: %s\n", strerror(errno));
		    }
		}
	      else
		printf("Clients aren't disconnected\n");
	      close(fd);
	    }
	  else
	    printf("Can't shutdown server: %s\n", strerror(errno));
	}
      else if (!strcmp(av[i], "active") || !strcmp(av[i], "start"))
	{
	  if (!unlink(SHUTDOWN_FILE) || errno == ENOENT)
	    printf("Server is now online.\n");
	  else
	    printf("Can't wake up server: %s\n", strerror(errno));
	}
      else if (!strcmp(av[i], "-yes"))
	assume_yes_to_all = 1;
      else
	{
	  printf("Usage:\n------\n\n");
	  printf("%s {options} {states}\n\n", av[0]);
	  printf("\nOptions:\n");
	  printf("\t-yes : assume yes to all questions\n");
	  printf("\nStates:\n");
	  printf("\t- active : wake up server\n");
	  printf("\t- start : same as 'active'\n");
	  printf("\t- shutdown : shutdown the server (but don't kill current connections)\n");
	  printf("\t- stop : same as 'shutdown'\n");
	  printf("\t- fullshutdown : shutdown the server (kill all connections and clean memory)\n");
	}
  else
    {
      if ((fd = open(SHUTDOWN_FILE, O_RDONLY)) >= 0)
	close(fd);
      printf("Server is %s\n", fd == -1 ? "up" : "down");
    }
  return (0);
}
