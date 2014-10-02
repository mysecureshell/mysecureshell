/*
MySecureShell permit to add restriction to modified sftp-server
when using MySecureShell as shell.
Copyright (C) 2007-2014 MySecureShell Team

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
#include <stdlib.h>
#include <time.h>
#include "Buffer.h"
#include "SftpWho.h"
#include "Stats.h"

tStats		*StatsNew()
{
  tStats	*stats;

  stats = calloc(1, sizeof(*stats));
  return (stats);
}

void    StatsDelete(tStats *stats)
{
  free(stats);
}


void    StatsUpdate(tStats *stats)
{
  t_sftpwho	*who = SftWhoGetAllStructs();

  if (who != NULL)
    {
      u_int32_t	download = 0, upload = 0;
      u_int16_t	users = 0;
      int	i;
      
      for (i = 0; i < SFTPWHO_MAXCLIENT; i++)
	{
	  if ((who[i].status & SFTPWHO_STATUS_MASK) != SFTPWHO_EMPTY)
	    {
	      users++;
	      download += who[i].download_current;
	      upload += who[i].upload_current;
	    }
	}
      stats->users[stats->writePos] = users;
      stats->download[stats->writePos] = download;
      stats->upload[stats->writePos] = upload;
      stats->writePos = (stats->writePos + 1) % STATS_SECONDES;
    }
}

void    StatsSend(tStats *stats, u_int32_t lastRefresh, tBuffer *b)
{
  u_int32_t	currentTime, showTime;
  int		firstPos, i;

  currentTime = (u_int32_t )time(NULL);
  showTime = currentTime - lastRefresh;
  if (showTime >= STATS_SECONDES)
    showTime = STATS_SECONDES - 1;
  firstPos = (stats->writePos - (int )showTime + STATS_SECONDES) % STATS_SECONDES;
  BufferPutInt32(b, showTime);
  for (i = firstPos; i != stats->writePos; )
    {
      BufferPutInt16(b, stats->users[i]);
      BufferPutInt32(b, stats->download[i]);
      BufferPutInt32(b, stats->upload[i]);
      i = (i + 1) % STATS_SECONDES;
    }
}
