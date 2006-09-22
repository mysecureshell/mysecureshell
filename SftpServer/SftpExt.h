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

#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#define STATFS  statvfs
#endif

#ifdef HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#ifndef STATFS
#define STATFS  statfs
#endif
#endif

#ifdef MSSEXT_DISKUSAGE
void	DoExtDiskSpace(tBuffer *bIn, tBuffer *bOut, u_int32_t id);
#endif

#ifdef MSSEXT_FILE_HASHING
void	DoExtFileHashing_Handle(tBuffer *bIn, tBuffer *bOut, u_int32_t id);
void	DoExtFileHashing_Name(tBuffer *bIn, tBuffer *bOut, u_int32_t id);
#endif
