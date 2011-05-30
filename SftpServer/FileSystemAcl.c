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

#include "../config.h"
#include <sys/types.h>
#include "FileSystem.h"
#include "Log.h"
#include "Sftp.h"

#if(HAVE_LIBACL)

#ifndef HAVE_CYGWIN
#include <acl/libacl.h>
#endif
#include <sys/acl.h>

#ifndef HAVE_CYGWIN

int FSEnumAcl(const char *file, int resolvePath, void (*callback)(void *data, int type, int id, int mode), void *data, int *nbEntries)
{
	tFSPath	*path = NULL;
	acl_t acl;

	if (resolvePath == 1)
	{
		path = FSResolvePath(file, NULL, 0);
		DEBUG((MYLOG_DEBUG, "[FSEnumAcl]file:'%s' realPath:'%s' exposedPath:'%s' path:'%s'", file, path->realPath, path->exposedPath, path->path));
		if (FSCheckSecurity(path->realPath, path->path) != SSH2_FX_OK)
		{
			FSDestroyPath(path);
			return SSH2_FX_PERMISSION_DENIED;
		}
		acl = acl_get_file(path->realPath, ACL_TYPE_ACCESS);
	}
	else
		acl = acl_get_file(file, ACL_TYPE_ACCESS);
	*nbEntries = 0;
	DEBUG((MYLOG_DEBUG, "[FSEnumAcl]Call acl_get_file -> '%s'", file));
	//Ignore all errors because FS should not support ACL
	if (acl != NULL)
	{
		acl_entry_t entry;

		if (acl_get_entry(acl, ACL_FIRST_ENTRY, &entry) == 1)
		{
			do
			{
				acl_permset_t permset;
				acl_tag_t tag;
				int *id = NULL;
				int mode;

				if (acl_get_tag_type(entry, &tag) == 0 && acl_get_permset(entry, &permset) == 0)
				{
					if (tag == ACL_MASK)
						continue;
					mode = (acl_get_perm(permset, ACL_READ) == 1 ? SSH5_ACE4_READ_DATA : 0) |
							(acl_get_perm(permset, ACL_WRITE) == 1 ? SSH5_ACE4_WRITE_DATA : 0) |
							(acl_get_perm(permset, ACL_EXECUTE) == 1 ? SSH5_ACE4_EXECUTE : 0);
					switch (tag)
					{
					 case ACL_USER:
						 id = (int *)acl_get_qualifier(entry);
						 if (id != NULL)
							 (*callback)(data, FS_ENUM_USER, *id, mode);
						 (*nbEntries)++;
						 break;
					 case ACL_GROUP:
						 id = (int *)acl_get_qualifier(entry);
						 if (id != NULL)
							 (*callback)(data, FS_ENUM_GROUP, *id, mode);
						 (*nbEntries)++;
						 break;
					 case ACL_OTHER:
						 (*callback)(data, FS_ENUM_OTHER, -1, mode);
						 (*nbEntries)++;
						 break;
					}
					DEBUG((MYLOG_DEBUG, "[FSEnumAcl]enum tag=%i id=%i mode=%i (permset: %i)", tag, id == NULL ? -42 : *id, mode));
					DEBUG((MYLOG_DEBUG, "[FSEnumAcl]permset ACL_READ=%i", acl_get_perm(permset, ACL_READ)));
					DEBUG((MYLOG_DEBUG, "[FSEnumAcl]permset ACL_WRITE=%i", acl_get_perm(permset, ACL_WRITE)));
					DEBUG((MYLOG_DEBUG, "[FSEnumAcl]permset ACL_EXECUTE=%i", acl_get_perm(permset, ACL_EXECUTE)));
				}
			}
			while (acl_get_entry(acl, ACL_NEXT_ENTRY, &entry) == 1);
		}
		(void )acl_free(acl);
	}
	if (path != NULL)
		FSDestroyPath(path);
	DEBUG((MYLOG_DEBUG, "[FSEnumAcl]nbEntries=%i", *nbEntries));
	return SSH2_FX_OK;
}

#else //ifdef HAVE_CYGWIN

int FSEnumAcl(const char *file, int resolvePath, void (*callback)(void *data, int type, int id, int mode), void *data, int *nbEntries)
{
	aclent_t acls[MAX_ACL_ENTRIES];
	tFSPath	*path;
	int nbAcls;

	path = FSResolvePath(file, NULL, 0);
	DEBUG((MYLOG_DEBUG, "[FSEnumAcl]file:'%s' realPath:'%s' exposedPath:'%s' path:'%s'", file, path->realPath, path->exposedPath, path->path));
	if (FSCheckSecurity(path->realPath, path->path) != SSH2_FX_OK)
	{
		FSDestroyPath(path);
		return SSH2_FX_PERMISSION_DENIED;
	}
	DEBUG((MYLOG_DEBUG, "[FSEnumAcl]Call acl"));
	nbAcls = acl(file, GETACL, MAX_ACL_ENTRIES, acls);
	//Ignore all errors because FS should not support ACL
	if (nbAcls > 0)
	{
		int i;

		for (i = 0; i < nbAcls; i++)
		{
			int mode;

			mode = (acls[i].a_perm & 2) ? SSH5_ACE4_READ_DATA : 0) |
			 ((acls[i].a_perm & 4) ? SSH5_ACE4_WRITE_DATA : 0) |
			 ((acls[i].a_perm & 1) ? SSH5_ACE4_EXECUTE : 0);
			switch (acls[i].a_type)
			{
			case ACL_USER: (*callback)(data, FS_ENUM_USER, acls[i].a_id, mode); break;
			case ACL_GROUP: (*callback)(data, FS_ENUM_GROUP, acls[i].a_id, mode); break;
			case ACL_OTHER: (*callback)(data, FS_ENUM_OTHER, -1, mode); break;
			}
			DEBUG((MYLOG_DEBUG, "[FSEnumAcl]enum tag=%i id=%i mode=%i", acls[i].a_type, acls[i].a_id, mode));
		}
		*nbEntries = nbAcls;
	}
	else
		*nbEntries = 0;
	FSDestroyPath(path);
	DEBUG((MYLOG_DEBUG, "[FSEnumAcl]nbEntries=%i", *nbEntries));
	return SSH2_FX_OK;
}


#endif //HAVE_CYGWIN

#endif //HAVE_LIBACL
