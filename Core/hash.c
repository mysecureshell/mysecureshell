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

#include "../config.h"
#include <stdio.h>
#include <string.h>
#include "hash.h"

#define MAKE_HASH(_D) (unsigned int )((((unsigned int)_D [0]) >> 5) | ((unsigned int )_D [1]))

/*@null@*/ static t_hash *_hash = NULL;
/*@null@*/ static t_element *_last_key = NULL;

void create_hash()
{
	_hash = calloc(1, sizeof(*_hash));
	_last_key = NULL;
}

void delete_hash()
{
	t_element *t, *n;
	int i;

	if (_hash != NULL)
	{
		for (i = 0; i < MSS_HASH_SIZE; i++)
		{
			t = _hash->hash[i];
			while (t != NULL)
			{
				n = t->next;
				free(t->key);
				if (t->str != NULL)
					free(t->str);
				free(t);
				t = n;
			}
		}
		free(_hash);
	}
	_hash = NULL;
	_last_key = NULL;
}

void hash_set(const char *key, /*@null@*/ char *value)
{
	if (_hash != NULL && value != NULL)
	{
		t_element *t = _hash->hash[MAKE_HASH(key)];

		while (t != NULL)
		{
			if (strcmp(key, t->key) == 0)
			{
				free(t->str);
				t->str = value;
				return;
			}
			t = t->next;
		}
		t = calloc(1, sizeof(*t));
		if (t != NULL)
		{
			if ((t->key = strdup(key)) != NULL)
			{
				t->str = value;
				t->next = _hash->hash[MAKE_HASH(key)];
				_hash->hash[MAKE_HASH(key)] = t;
			}
			else
				free(t);
		}
	}
}

void hash_set_int(const char *key, int value)
{
	if (_hash != NULL)
	{
		t_element *t = _hash->hash[MAKE_HASH(key)];

		while (t != NULL)
		{
			if (strcmp(key, t->key) == 0)
			{
				t->number = value;
				return;
			}
			t = t->next;
		}
		t = calloc(1, sizeof(*t));
		if (t != NULL)
		{
			if ((t->key = strdup(key)) != NULL)
			{
				t->number = value;
				t->next = _hash->hash[MAKE_HASH(key)];
				_hash->hash[MAKE_HASH(key)] = t;
			}
			else
				free(t);
		}
	}
}

int hash_exists(const char *key)
{
	if (_hash != NULL)
	{
		t_element *t = _hash->hash[MAKE_HASH(key)];

		if (_last_key != NULL && strcmp(key, _last_key->key) == 0)
			return (1);
		while (t != NULL)
		{
			if (strcmp(key, t->key) == 0)
			{
				_last_key = t;
				return (1);
			}
			t = t->next;
		}
	}
	return (0);
}

/*@null@*/ char *hash_get(const char *key)
{
	if (_hash != NULL)
	{
		t_element *t = _hash->hash[MAKE_HASH(key)];

		if (_last_key != NULL && strcmp(key, _last_key->key) == 0)
			return (_last_key->str);
		while (t != NULL)
		{
			if (strcmp(key, t->key) == 0)
			{
				_last_key = t;
				return (t->str);
			}
			t = t->next;
		}
	}
	return (NULL);
}

int hash_get_int(const char *key)
{
	if (_hash != NULL)
	{
		t_element *t = _hash->hash[MAKE_HASH(key)];

		if (_last_key != NULL && strcmp(key, _last_key->key) == 0)
			return (_last_key->number);
		while (t != NULL)
		{
			if (strcmp(key, t->key) == 0)
			{
				_last_key = t;
				return (t->number);
			}
			t = t->next;
		}
	}
	return (0);
}
