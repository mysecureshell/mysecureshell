/*
** parsing.h for MySecureShell in /root/MySecureShell
** 
** Made by root
** Login   <root@localhost>
** 
** Started on  Sat Aug 14 15:44:45 2004 root
** Last update Tue Jan 11 13:50:20 2005 root
*/

#include "hash.h"

void	parse_tag(char *buffer);
void	parse_tag_open(char *str);
void	parse_tag_close(char *str);
void	parse_virtualhost(char *str);
char	*parse_range_ip(char *str);
char	**parse_cut_string(char *str);

extern int	parse_opened_tag;

#define	TAG_GROUP	"group"
#define TAG_USER	"user"
#define	TAG_RANGEIP	"iprange"
#define	TAG_DEFAULT	"default"
#define TAG_VIRTUALHOST "virtualhost"
#define	TAG_ALL		"*"
