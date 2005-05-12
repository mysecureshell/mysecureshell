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

#include <stdlib.h>

typedef struct		s_element
{
  char			*key;
  void			*value;
  struct s_element	*next;
  int			free;
}			t_element;

typedef struct		s_hash
{
  t_element		*hash[256];
}			t_hash;

void	delete_hash();
char	*hash_get_int_to_char(char *key);
void	*hash_get(char *key);
void	hash_set(char *key, void *value, char free_old);
void	create_hash();
