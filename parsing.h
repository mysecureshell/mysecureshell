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

typedef struct	sTag
{
  int		type;
  void		*next;
  char		*data1;
  int		data2;
}		tTag;

#define VTAG_DEFAULT		0
#define VTAG_USER		1
#define VTAG_GROUP		2
#define VTAG_RANGEIP		3
#define VTAG_VIRTUALHOST	4

int	tag_is_active(int verbose);
int	parse_tag(char *buffer);
void	parse_tag_open(char *str);
void	parse_tag_close();
void	parse_virtualhost(const char *str, tTag *newTag);
char	*parse_range_ip(const char *str);
char	**parse_cut_string(char *str);

#define	TAG_GROUP	"group"
#define TAG_USER	"user"
#define	TAG_RANGEIP	"iprange"
#define	TAG_DEFAULT	"default"
#define TAG_VIRTUALHOST "virtualhost"
#define	TAG_ALL		"*"
