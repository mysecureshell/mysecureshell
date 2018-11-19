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

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif //HAVE_SYS_PARAM_H

#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#define STATFS  statvfs
#endif //HAVE_SYS_STATVFS_H

#ifdef HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#ifndef STATFS
#define STATFS  statfs
#endif //STATFS
#endif //HAVE_SYS_STATFS_H

#if (HAVE_SYS_MOUNT_H&&HAVE_STATFS)
#include <sys/mount.h>
#ifndef STATFS
#define STATFS  statfs
#endif //STATFS
#endif //(HAVE_SYS_MOUNT_H&&HAVE_STATFS)

#ifdef MSSEXT_DISKUSAGE
void	DoExtDiskSpace(tBuffer *bIn, tBuffer *bOut, u_int32_t id);
#endif
#ifdef MSSEXT_DISKUSAGE_SSH
void	DoExtDiskSpaceOpenSSH_Handle(tBuffer *bIn, tBuffer *bOut, u_int32_t id);
void	DoExtDiskSpaceOpenSSH_Name(tBuffer *bIn, tBuffer *bOut, u_int32_t id);
#endif

#ifdef MSSEXT_FILE_HASHING
void	DoExtFileHashing_Handle(tBuffer *bIn, tBuffer *bOut, u_int32_t id);
void	DoExtFileHashing_Name(tBuffer *bIn, tBuffer *bOut, u_int32_t id);
#endif

void	DoExtHardLink(tBuffer *bIn, tBuffer *bOut, u_int32_t id);
