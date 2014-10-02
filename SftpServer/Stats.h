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

#ifndef _STATS_H_
#define _STATS_H_

#include "Sftp.h"

#define	STATS_SECONDES	300 //5mins

typedef struct	sStats
{
  u_int16_t	users[STATS_SECONDES];
  u_int32_t	download[STATS_SECONDES];
  u_int32_t	upload[STATS_SECONDES];
  int32_t	writePos;
}		tStats;

tStats	*StatsNew();
void	StatsDelete(tStats *stats);
void	StatsUpdate(tStats *stats);
void	StatsSend(tStats *stats, u_int32_t lastRefresh, tBuffer *b);


#endif //_STATS_H_
