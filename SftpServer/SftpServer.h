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

#ifndef _SFTPSERVER_H_
#define _SFTPSERVER_H_

#define SET_TIMEOUT(_TM, _TSEC, _TUSEC)         _TM .tv_sec = _TSEC; _TM .tv_usec = _TUSEC
#define RULES_NONE              0
#define RULES_FILE              1
#define RULES_DIRECTORY         2
#define RULES_LISTING           3
#define RULES_RMFILE            4
#define RULES_RMDIRECTORY       5

void    DoInitUser();
int     CheckRules(const char *pwd, char operation, const struct stat *st, int flags);
void    ChangeRights(struct stat *st);
int     CheckRulesAboutMaxFiles();
void    ResolvPath(const char *path, char *dst, int dstMaxSize);
void	ParseConf(tGlobal *params, int sftpProtocol);

extern tGlobal *gl_var;

#endif //_SFTPSERVER_H_
