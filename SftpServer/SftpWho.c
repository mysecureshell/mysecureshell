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
#include <sys/types.h>
#include <sys/shm.h>
#include "SftpWho.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct		s_shm
{
	t_sftpglobal	global;
	t_sftpwho	who[SFTPWHO_MAXCLIENT];
}			t_shm;

static t_sftpwho	*_sftpwho_ptr = 0;
t_sftpglobal		*_sftpglobal = 0;

t_sftpwho	*SftWhoGetAllStructs()
{
  return	(_sftpwho_ptr);
}

t_sftpwho	*SftpWhoGetStruct(int create)
{
  t_sftpwho	*who = 0;
  void		*ptr;
  key_t		key;
  int		shmid, shmkey = 0x0782;
  int		eraze = 0;
  int		i, try, tryshm = 3;

 try_shm:
  if ((key = ftok("/dev/null", shmkey)) != -1)
    {
      //try to join to existing shm
      if ((shmid = shmget(key, sizeof(t_shm), 0)) == -1)
	{
	  if (create == 1)
	    shmid = shmget(key, sizeof(t_shm), IPC_CREAT | IPC_EXCL | 0666);
	  eraze = 1;
	}
      if (shmid == -1 && create == 1 &&
	  (errno == EINVAL || errno == EEXIST))
	{
	  //huho we have a old shm memory
	  if (tryshm)
	    {
	      tryshm--;
	      shmkey++;
	      goto try_shm;
	    }
	}
      if (shmid != -1 && (int )(ptr = shmat(shmid, 0, 0)) != -1)
	{
	  t_shm	*shm = ptr;
		
	  _sftpglobal = &shm->global;
	  who = shm->who;
	  _sftpwho_ptr = who;
	  if (eraze)
	    memset(shm, 0, sizeof(t_shm));
	  else
	    //clean all sessions of bugged client (abnormally quit)
	    SftpWhoCleanBuggedClient();
	  if (create == -1)
	    return (who);
	  //search a empty place :)
	  //try to search 3 times to prevent infinite loop
	  for (try = 0; try < 3; try++)
	      for (i = 0; i < SFTPWHO_MAXCLIENT; i++)
		if (who[i].status == SFTPWHO_EMPTY)
		  {
		    usleep(100);
		    if (who[i].status == SFTPWHO_EMPTY)
		      {
			//clean all old infos
			memset(&who[i], 0, sizeof(*who));
			//marked structure as occuped :)
			who[i].status = SFTPWHO_IDLE;
			return (&who[i]);
		      }
		  }
	}
    }
  if (create == 1)
    {
      _sftpglobal = calloc(1, sizeof(*_sftpglobal));
      who = calloc(1, sizeof(*who));
    }
  return (who);
}

//return number of connected clients
int		SftpWhoCleanBuggedClient()
{
  unsigned int	t;
  int		i, nb, nbdown, nbup;

  if (!_sftpwho_ptr)
    return (0);
  t = time(0);
  nb = 0;
  nbdown = 0;
  nbup = 0;
  for (i = 0; i < SFTPWHO_MAXCLIENT; i++)
    if ((_sftpwho_ptr[i].status & SFTPWHO_STATUS_MASK) != SFTPWHO_EMPTY)
      {
	//add 10s to make sure that the session is definitively dead
	if ((_sftpwho_ptr[i].time_begin + _sftpwho_ptr[i].time_total + 10) < t)
	  _sftpwho_ptr[i].status = SFTPWHO_EMPTY;
	else
	  {
	    nb++;
	    if ((_sftpwho_ptr[i].status & SFTPWHO_STATUS_MASK) == SFTPWHO_GET)
	      nbdown++;
	    else if ((_sftpwho_ptr[i].status & SFTPWHO_STATUS_MASK) == SFTPWHO_PUT)
	      nbup++;
	  }
      }
  if (nbdown > 0)
    _sftpglobal->download_by_client = _sftpglobal->download_max / nbdown;
  else
    _sftpglobal->download_by_client = _sftpglobal->download_max;
  if (nbup > 0)
    _sftpglobal->upload_by_client = _sftpglobal->upload_max / nbup;
  else
    _sftpglobal->upload_by_client = _sftpglobal->upload_max;
  return (nb);
}

void	SftpWhoRelaseStruct()
{
  if (_sftpwho_ptr)
    {
      shmdt(_sftpwho_ptr);
      _sftpwho_ptr = 0;
    }
}
