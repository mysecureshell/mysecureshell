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

#include <stdlib.h>

#define MSS_HASH_SIZE	256

typedef struct		s_element
{
  char			*key;
  char			*str;
  int			number;
  struct s_element	*next;
}			t_element;

typedef struct		s_hash
{
  t_element		*hash[MSS_HASH_SIZE];
}			t_hash;

void	delete_hash();
int	hash_exists(const char *key);
/*@null@*/ char	*hash_get(const char *key);
int	hash_get_int(const char *key);
void	hash_set(const char *key, /*@null@*/ char *value);
void	hash_set_int(const char *key, int value);
void	create_hash();
