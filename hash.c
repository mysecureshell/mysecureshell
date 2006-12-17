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

#include <stdio.h>
#include <string.h>
#include "hash.h"

#define MAKE_HASH(_D) (int )((_D [0] >> 5) | _D [1])

static t_hash		*_hash = NULL;
static t_element	*_last_key = NULL;

void	create_hash()
{
  _hash = calloc(1, sizeof(*_hash));
  _last_key = NULL;
}

void		delete_hash()
{
  t_element	*t, *n;
  int		nb = sizeof(_hash->hash) / sizeof(*_hash->hash);
  int		i;

  for (i = 0; i < nb; i++)
    {
      t = _hash->hash[i];
      while (t)
	{
	  n = t->next;
	  free(t->key);
	  if (t->str)
	    free(t->str);
	  t = n;
	}
    }
  free(_hash);
  _hash = 0;
  _last_key = 0;
}

void		hash_set(char *key, void *value)
{
  t_element	*t = _hash->hash[MAKE_HASH(key)];

  while (t)
    {
      if (!strcmp(key, t->key))
	{
	  free(t->str);
	  t->str = value;
	  return;
	}
      t = t->next;
    }
  t = calloc(1, sizeof(*t));
  t->key = strdup(key);
  t->str = value;
  t->next = _hash->hash[(int )*key];
  _hash->hash[(int )*key] = t;
}

void		hash_set_int(char *key, int value)
{
  t_element	*t = _hash->hash[MAKE_HASH(key)];

  while (t)
    {
      if (!strcmp(key, t->key))
	{
	  t->number = value;
	  return;
	}
      t = t->next;
    }
  t = calloc(1, sizeof(*t));
  t->key = strdup(key);
  t->number = value;
  t->next = _hash->hash[(int )*key];
  _hash->hash[(int )*key] = t;
}

void		*hash_get(char *key)
{
  t_element	*t = _hash->hash[MAKE_HASH(key)];

  if (_last_key && !strcmp(key, _last_key->key))
    return (_last_key->str);
  while (t)
    {
      if (!strcmp(key, t->key))
	{
	  _last_key = t;
	  return (t->str);
	}
      t = t->next;
    }
  return (0);
}

int		hash_get_int(char *key)
{
  t_element     *t = _hash->hash[MAKE_HASH(key)];

  if (_last_key && !strcmp(key, _last_key->key))
    return (_last_key->number);
  while (t)
    {
      if (!strcmp(key, t->key))
        {
          _last_key = t;
          return (t->number);
        }
      t = t->next;
    }
  return (0);
}

int		hash_get_int_with_default(char *key, int dft)
{
  t_element	*t = _hash->hash[MAKE_HASH(key)];

  if (_last_key && !strcmp(key, _last_key->key))
    return (_last_key->number);
  while (t)
    {
      if (!strcmp(key, t->key))
        {
          _last_key = t;
          return (t->number);
        }
      t = t->next;
    }
  return (dft);
}
