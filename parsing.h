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

#include "hash.h"

typedef enum enumTagType
{
	VTAG_DEFAULT, VTAG_USER, VTAG_GROUP, VTAG_RANGEIP, VTAG_VIRTUALHOST, VTAG_FILESPEC,
} eTagType;

typedef struct sTag
{
	eTagType type;
	void *next;
	char *data1;
	int data2;
} tTag;

#define	TAG_GROUP		"group"
#define TAG_USER		"user"
#define	TAG_RANGEIP		"iprange"
#define	TAG_DEFAULT		"default"
#define TAG_VIRTUALHOST	"virtualhost"
#define	TAG_ALL			"*"
#define TAG_FILESPEC	"filespec"

int TagIsActive(int verbose);
int TagIsOpen(eTagType tagType);
int TagParse(char *buffer);
void TagParseOpen(char *str);
void TagParseClose();
void TagParseVirtualHost(const char *str, tTag *newTag);
/*@null@*/ char *TagParseRangeIP(const char *str);
/*@null@*/ char **ParseCutString(char *str);
