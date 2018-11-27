/*
 MySecureShell permit to add restriction to modified sftp-server
 when using MySecureShell as shell.
 Copyright (C) 2007-2018 MySecureShell Team

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

#ifndef FILESPEC_H_
#define FILESPEC_H_

#include <unistd.h>
#include <sys/types.h>
#include <regex.h>

typedef enum enumFileSpec
{
	FILESPEC_ALLOW_DENY, FILESPEC_DENY_ALLOW,
} eFileSpec;

typedef struct sFileSpec
{
	eFileSpec type;
	char *name;
	int useFullPath;
	int nbExpression;
	regex_t *expressions;
	struct sFileSpec *next;
} tFileSpec;

void FileSpecInit();
void FileSpecDestroy();
void FileSpecEnter(const char *specName);
void FileSpecLeave();
void FileSpecParse(/*@null@*/ char **words);
void FileSpecActiveProfil(const char *specName, const int verbose);
void FileSpecActiveProfils(/*@null@*/ char *specsName, const int verbose);
int FileSpecCheckRights(const char *fullPath, const char *path);

#endif /* FILESPEC_H_ */
