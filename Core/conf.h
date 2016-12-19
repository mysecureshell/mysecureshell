/*
MySecureShell permit to add restriction to modified sftp-server
when using MySecureShell as shell.
Copyright (C) 2007-2014 MySecureShell Team

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

void	load_config(int verbose);
int	convert_mode_to_int(const char *str);
int	convert_boolean_to_int(const char *str);
int	convert_speed_to_int(char **tb);
int	convert_time_to_int(char **tb);
int	load_config_file(const char *file, int verbose, int max_recursive_left);
void	processLine(char **tb, int max_recursive_left, int verbose);
/*@null@*/ char	*convert_str_with_resolv_env_to_str(const char *str);
char	*convert_to_path(char *path);
void	set_custom_config_file(const char *config_file);
