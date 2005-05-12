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

#define	REST_ALL	0
#define	REST_GROUP	1
#define	REST_USER	2

int	init_user_info();
int	is_for_user(char *user, int verbose);
int	is_for_group(char *group, int verbose);
int	is_for_rangeip(char *range, int verbose);
int	is_for_virtualhost(char *host, int port, int verbose);
void	free_user_info();
