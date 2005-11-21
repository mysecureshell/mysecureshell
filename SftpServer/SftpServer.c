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

#include <regex.h>
#include <signal.h>
#include "Log.h"
#include "SftpWho.c"

typedef struct	s_global
{
  t_sftpwho	*who;

  char		*hide_files;
  char		*deny_filter;
  regex_t	hide_files_regexp;
  regex_t	deny_filter_regexp;
  int		max_openfiles;
  int		max_readfiles;
  int		max_writefiles;
  int		rights_file;
  int		rights_directory;
  unsigned int	download_current;
  unsigned int	upload_current;
  unsigned int	download_max;
  unsigned int	upload_max;
  off_t		down_size;
  off_t		down_max;
}		t_global;

static t_global	*gl_var = 0;

#include "Access.c"
#include "GetUsersInfos.h"
#include "GetUsersInfos.c"


#define	SET_TIMEOUT(_TM, _TSEC, _TUSEC)		_TM .tv_sec = _TSEC; _TM .tv_usec = _TUSEC

static void	end_sftp()
{
  mylog_printf(MYLOG_NORMAL, "[%s][%s]Quit.", gl_var->who->user, gl_var->who->ip);
  mylog_close();
  gl_var->who->status = SFTPWHO_EMPTY;
  SftpWhoRelaseStruct();
  regfree(&gl_var->hide_files_regexp);
  free(gl_var);
}

static void	end_sftp_by_signal(int signal)
{
  end_sftp();
}

static void	parse_conf(int ac, char **av)
{
  int		i, r;
  
  mylog_open(MSS_LOG);
  gl_var = malloc(sizeof(*gl_var));
  memset(gl_var, 0, sizeof(*gl_var));
  gl_var->who = SftpWhoGetStruct(1);
  gl_var->who->time_begin = time(0);
  gl_var->who->pid = (unsigned int)getpid();
  gl_var->rights_directory = 0777;
  gl_var->rights_file = 0666;
  atexit(end_sftp);
  signal(SIGHUP, end_sftp_by_signal);
  signal(SIGINT, end_sftp_by_signal);
  for (i = 1; i < ac; i++)
    {
      if (!strcmp(av[i], "--home") && av[i + 1])
	snprintf(gl_var->who->home, sizeof(gl_var->who->home), "%s", av[i + 1]);
      else if (!strcmp(av[i], "--user") && av[i + 1])
	snprintf(gl_var->who->user, sizeof(gl_var->who->user), "%s", av[i + 1]);
      else if (!strcmp(av[i], "--ip") && av[i + 1])
	snprintf(gl_var->who->ip, sizeof(gl_var->who->ip), "%s", av[i + 1]);
      else if (!strcmp(av[i], "--mode") && av[i + 1])
	gl_var->who->status |= atoi(av[i + 1]);
      else if (!strcmp(av[i], "--upload") && av[i + 1])
	{
	  gl_var->who->upload_max = atoi(av[i + 1]);
	  gl_var->upload_max = gl_var->who->upload_max;
	}
      else if (!strcmp(av[i], "--download") && av[i + 1])
	{
	  gl_var->who->download_max = atoi(av[i + 1]);
	  gl_var->download_max = gl_var->who->download_max;
	}
      else if (!strcmp(av[i], "--global-upload") && av[i + 1])
	_sftpglobal->upload_max = atoi(av[i + 1]);
      else if (!strcmp(av[i], "--global-download") && av[i + 1])
	_sftpglobal->download_max = atoi(av[i + 1]);
      else if (!strcmp(av[i], "--idle") && av[i + 1])
	gl_var->who->time_maxidle = atoi(av[i + 1]);
      else if (!strcmp(av[i], "--fake-mode") && av[i + 1])
	gl_var->who->mode = atoi(av[i + 1]);
      else if (!strcmp(av[i], "--hide-files") && av[i + 1])
	{
	  if (!(r = regcomp(&gl_var->hide_files_regexp, av[i + 1], REG_EXTENDED | REG_NOSUB | REG_NEWLINE)))
	    gl_var->hide_files = av[i + 1];
	  else
	    {
	      char	buffer[256];

	      regerror(r, &gl_var->hide_files_regexp, buffer, sizeof(buffer));
	      mylog_printf(MYLOG_ERROR, "[%s][%s]Couldn't compile regex : %s",
			 gl_var->who->user, gl_var->who->ip, buffer);
	    }
	}
       else if (!strcmp(av[i], "--max-open-files") && av[i + 1])
	gl_var->max_openfiles = atoi(av[i + 1]);
      else if (!strcmp(av[i], "--max-read-files") && av[i + 1])
	gl_var->max_readfiles = atoi(av[i + 1]);
      else if (!strcmp(av[i], "--max-write-files") && av[i + 1])
	gl_var->max_writefiles = atoi(av[i + 1]);
      else if (!strcmp(av[i], "--rights-directory") && av[i + 1])
	gl_var->rights_directory = atoi(av[i + 1]);
      else if (!strcmp(av[i], "--rights-file") && av[i + 1])
	gl_var->rights_file = atoi(av[i + 1]);
      else if (!strcmp(av[i], "--deny-filter") && av[i + 1])
	{
	  if (!(r = regcomp(&gl_var->deny_filter_regexp, av[i + 1], REG_EXTENDED | REG_NOSUB | REG_NEWLINE)))
	    gl_var->deny_filter = av[i + 1];
	  else
	    {
	      char	buffer[256];

	      regerror(r, &gl_var->deny_filter_regexp, buffer, sizeof(buffer));
	      mylog_printf(MYLOG_ERROR, "[%s][%s]Couldn't compile regex : %s",
			 gl_var->who->user, gl_var->who->ip, buffer);
	    }
	}
      else if (!strcmp(av[i], "--protocol") && av[i + 1])
	cVersion = atoi(av[i + 1]);
      else if (!strcmp(av[i], "--max-life") && av[i + 1])
	gl_var->who->time_maxlife = atoi(av[i + 1]);
      else if (!strcmp(av[i], "--charset") && av[i + 1])
	setCharset(av[i + 1]);
    }
  mylog_printf(MYLOG_NORMAL, "New client [%s] from [%s]", gl_var->who->user, gl_var->who->ip);
  init_usersinfos();//load users / groups into memory
  InitAccess();
  chdir(gl_var->who->home);
  if (gl_var->who->status & SFTPWHO_VIRTUAL_CHROOT)
    {
      if (chroot(gl_var->who->home) != -1)
	{
	  gl_var->who->status &= ~SFTPWHO_STAY_AT_HOME;
	  chdir("/");
	}
      else
	{
	  mylog_printf(MYLOG_ERROR, "[%s][%s]Couldn't chroot : %s",
		     gl_var->who->user, gl_var->who->ip, strerror(errno));
	  gl_var->who->status &= ~SFTPWHO_VIRTUAL_CHROOT;
	  gl_var->who->status |= SFTPWHO_STAY_AT_HOME;
	}
    }
  if (getuid() != geteuid() && !(gl_var->who->status & SFTPWHO_IS_ADMIN))
    //if we are in utset byte mode then we restore user's rights to avoid security problems
    {
      if (seteuid(getuid()) == -1 || setegid(getgid()) == -1)
	{
	  mylog_printf(MYLOG_ERROR, "[%s][%s]Couldn't revoke root rights : %s",
		       gl_var->who->user, gl_var->who->ip, strerror(errno));
	  exit(255);
	}
    }
}

#define	RULES_NONE		0
#define	RULES_FILE		1
#define	RULES_DIRECTORY		2
#define RULES_LISTING		3
#define	RULES_RMFILE		4
#define	RULES_RMDIRECTORY	5

static int	CheckRules(char *pwd, char operation, struct stat *st, int flags)
{
  char		*str;

  if (gl_var->hide_files && (operation >= RULES_LISTING
			     || (operation == RULES_DIRECTORY && (flags & O_RDONLY))
			     || (operation == RULES_FILE && (flags & O_RDONLY))))
    {
      if ((str = strrchr(pwd, '/')))
	str++;
      else //should not exist...
	str = pwd;
      if (regexec(&gl_var->hide_files_regexp, str, 0, 0, 0) != REG_NOMATCH)
	return SSH2_FX_NO_SUCH_FILE;
    }
  if (gl_var->who->status & SFTPWHO_STAY_AT_HOME)
    {
      if (!strncmp(pwd, gl_var->who->home, strlen(gl_var->who->home))
	  && (strlen(pwd) < 3 || strcmp(pwd + strlen(pwd) - 3, "/..")))
	;
      else
	return SSH2_FX_PERMISSION_DENIED;
    }
  if ((gl_var->who->status & SFTPWHO_IGNORE_HIDDEN)
      && ((operation >= RULES_DIRECTORY && (flags & O_RDONLY))
	  || operation == RULES_FILE
	  || operation == RULES_LISTING))
    {
      if ((str = strstr(pwd, "/.")) && strcmp(str, "/.."))
	return SSH2_FX_NO_SUCH_FILE;
    }
  if (gl_var->deny_filter && ((operation == RULES_FILE && ((flags & O_WRONLY) || (flags & O_RDWR)))
			      || (operation == RULES_DIRECTORY && (flags & O_WRONLY))))
    {
      if ((str = strrchr(pwd, '/')))
	str++;
      else
	str = pwd;
      if (regexec(&gl_var->deny_filter_regexp, str, 0, 0, 0) != REG_NOMATCH)
	return SSH2_FX_PERMISSION_DENIED;
    }
  
  //This code should always be at the end of this function
  if ((gl_var->who->status & SFTPWHO_HIDE_NO_ACESS) && operation == RULES_LISTING && st)
    {
      if ((gl_var->who->status & SFTPWHO_LINKS_AS_LINKS))
	{
	  struct stat	localst;
	  
	  if ((st->st_mode & S_IFMT) == S_IFLNK && stat(pwd, &localst) != -1)
	    st = &localst;
	}
      if ((st->st_uid == getuid() && (st->st_mode & S_IRUSR))
	  || (UserIsInThisGroup(st->st_gid) && (st->st_mode & S_IRGRP))
	  || (st->st_mode & S_IROTH))
	return SSH2_FX_OK;
      return SSH2_FX_NO_SUCH_FILE;
    }
  return SSH2_FX_OK;
}

static void	ChangeRights(struct stat *st)
{
  if (gl_var->who->status & SFTPWHO_FAKE_USER)
    st->st_uid = getuid();
  if (gl_var->who->status & SFTPWHO_FAKE_GROUP)
    st->st_gid = getgid();
  if (gl_var->who->status & SFTPWHO_FAKE_MODE)
    {
      st->st_mode = (st->st_mode & ~0x1fff) | gl_var->who->mode;
      if (st->st_mode & S_IFDIR)
	{
	  if (gl_var->who->mode & S_IRUSR)
	    st->st_mode |= S_IXUSR;
	  if (gl_var->who->mode & S_IRGRP)
	    st->st_mode |= S_IXGRP;
	  if (gl_var->who->mode & S_IROTH)
	    st->st_mode |= S_IXOTH;
	}
    }
}

static int	CheckRulesAboutMaxFiles()
{
  t_sftpwho	*who;
  int		i, fileread, filewrite, fileall;

  if ((who = SftWhoGetAllStructs()))
    {
      fileread = 0;
      filewrite = 0;
      fileall = 0;
      for (i = 0; i < SFTPWHO_MAXCLIENT; i++)
	if (!strcmp(who[i].user, gl_var->who->user))
	  {
	    switch (who[i].status & SFTPWHO_STATUS_MASK)
	      {
	      case SFTPWHO_PUT:
		filewrite++;
		fileall++;
		break;
	      case SFTPWHO_GET:
		fileread++;
		fileall++;
		break;
	      }
	  }
      if ((fileread > gl_var->max_readfiles && gl_var->max_readfiles) ||
	  (filewrite > gl_var->max_writefiles && gl_var->max_writefiles) ||
	  (fileall > gl_var->max_openfiles && gl_var->max_openfiles))
	return SSH2_FX_PERMISSION_DENIED;
    }
  return SSH2_FX_OK;
}

static void     ResolvPath(char *path, char *dst)
{
  char          *ptr, *s = path;
  int           i, beg, end, len;

  dst[0] = 0;
  beg = 0;
  len = strlen(path);
  strcpy(dst, path);
  s = dst;
  while ((ptr = strstr(s, "..")))
    {
      beg = len - strlen(ptr);
      end = beg + 2;
      if ((dst[beg - 1] == '/' || !beg) && (!dst[end] || dst[end] == '/'))
        {
          while (beg >= 1 && dst[beg - 1] == '/')
            beg--;
          for (i = beg - 1; i >= 0; i--)
            if (dst[i] == '/' && (i == 0 || dst[i - 1] != '/'))
              break;
          if (i < 0) i = 0;
          if (dst[end])
            strcpy(dst + i, dst + end);
          else
	    dst[i] = 0;
          len = strlen(dst);
        }
      else
        s = ptr + 2;
    }
  if (!dst[0])
    {
      if (path[0] == '/')
        dst[0] = path[0];
      else
        dst[0] = '.';
      dst[1] = 0;
    }
  len = strlen(dst);
  if (len >= 2 && dst[len - 2] == '/' && dst[len - 1] == '.')
    dst[len - 1] = 0;
  else if (len >= 1 && dst[len - 1] != '/')
    {
      struct stat       st;

      if (stat(dst, &st) != -1 && (st.st_mode & S_IFMT) != S_IFREG)
        strcat(dst, "/");
    }
}
