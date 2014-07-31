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

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SftpServer/Sftp.h"
#include "FileSpec.h"

/*@null@*/ static tFileSpec *_allSpecs = NULL;
/*@null@*/ static tFileSpec *_selectedSpecs = NULL;

void FileSpecInit()
{
	;
}

void FileSpecDestroy()
{
	tFileSpec *current;
	tFileSpec *next;

	for (next = _allSpecs; next != NULL; )
	{
		int	i;

		current = next;
		next = current->next;
		for (i = 0; i < current->nbExpression; i++)
			regfree(&current->expressions[i]);
		free(current->expressions);
		free(current->name);
		free(current);
	}
	for (next = _selectedSpecs; next != NULL; )
	{
		current = next;
		next = current->next;
		free(current);
	}
}

void FileSpecEnter(const char *specName)
{
	tFileSpec *newSpec;

	newSpec = malloc(sizeof(*newSpec));
	if (newSpec != NULL)
	{
		newSpec->name = strdup(specName);
		newSpec->type = FILESPEC_DENY_ALLOW;
		newSpec->useFullPath = 0;
		newSpec->nbExpression = 0;
		newSpec->expressions = NULL;
		newSpec->next = _allSpecs;
		_allSpecs = newSpec;
	}
}

void FileSpecLeave()
{
	;
}

void FileSpecParse(/*@null@*/ char **words)
{
	if (_allSpecs == NULL)
		return;
	if (words == NULL || words[0] == NULL || words[1] == NULL)
		return;
	if (strcmp("Order", words[0]) == 0)
	{
		if (strcmp("AllowDeny", words[1]) == 0)
			_allSpecs->type = FILESPEC_ALLOW_DENY;
	}
	else if (strcmp("UseFullPath", words[0]) == 0)
	{
		if (strcmp("true", words[1]) == 0)
			_allSpecs->useFullPath = 1;
	}
	else
	{
		int r;

		if (strcmp("all", words[1]) == 0)
			return;
		if (strcmp("Allow", words[0]) != 0 && _allSpecs->type == FILESPEC_ALLOW_DENY)
			return;
		if (strcmp("Deny", words[0]) != 0 && _allSpecs->type == FILESPEC_DENY_ALLOW)
			return;

		_allSpecs->expressions = (regex_t *) realloc(_allSpecs->expressions, (_allSpecs->nbExpression + 1) * sizeof(regex_t));
		if (_allSpecs->expressions != NULL)
		{
			r = regcomp(&_allSpecs->expressions[_allSpecs->nbExpression], words[1], REG_EXTENDED | REG_NOSUB | REG_NEWLINE);
			if (r == 0)
				_allSpecs->nbExpression++;
			else
			{
				char buffer[256];

				(void) regerror(r, &_allSpecs->expressions[_allSpecs->nbExpression], buffer, sizeof(buffer));
				(void) printf("[ERROR]Couldn't compile regex \"%s\" : %s\n", words[1], buffer);
			}
		}
		else
			perror("unable to allocate list of regexp");
	}
}

void FileSpecActiveProfils(/*@null@*/ char *specsName, const int verbose)
{
	if (specsName != NULL)
	{
		size_t lenSpecsName, len;
		char *specName = specsName;

		lenSpecsName = strlen(specsName);
		for (len = lenSpecsName - 1; len > 0; len--)
			if (specsName[len] == ',')
			{
				specsName[len] = '\0';
				specName = specsName + len + 1;
				FileSpecActiveProfil(specName, verbose);
			}
		FileSpecActiveProfil(specsName, verbose);
	}
}

void FileSpecActiveProfil(const char *specName, const int verbose)
{
	tFileSpec *next = _allSpecs;

	if (verbose > 0)
		(void) printf("--- Apply profile FileSpec '%s'---\n", specName);
	while (next != NULL)
	{
		if (strcmp(next->name, specName) == 0)
		{
			tFileSpec *new = malloc(sizeof(tFileSpec));

			if (new != NULL)
			{
				memcpy(new, next, sizeof(tFileSpec));
				new->next = _selectedSpecs;
				_selectedSpecs = new;
			}
			return;
		}
		next = next->next;
	}
	if (verbose > 0)
			(void) printf("[ERROR]Unkown profile FileSpec '%s'\n", specName);
}

int FileSpecCheckRights(const char *fullPath, const char *path)
{
	tFileSpec *next = _selectedSpecs;
	int nb;

	while (next != NULL)
	{
		const char *p;

		if (next->useFullPath == 1)
			p = fullPath;
		else
			p = path;
		if (next->type == FILESPEC_ALLOW_DENY)
		{
			for (nb = next->nbExpression - 1; nb >= 0; nb--)
				if (regexec(next->expressions + nb, p, 0, NULL, 0) != REG_NOMATCH)
					goto nextSpec;
			if (next->nbExpression > 0)
				return SSH2_FX_PERMISSION_DENIED;
		}
		else //FILESPEC_DENY_ALLOW
		{
			if (next->nbExpression == 0)
				return SSH2_FX_PERMISSION_DENIED;
			for (nb = next->nbExpression - 1; nb >= 0; nb--)
				if (regexec(next->expressions + nb, p, 0, NULL, 0) != REG_NOMATCH)
					return SSH2_FX_PERMISSION_DENIED;
		}
nextSpec:
		next = next->next;
	}
	return SSH2_FX_OK;
}
