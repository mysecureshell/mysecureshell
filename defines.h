#ifndef __DEFINES_H__
#define __DEFINES_H__

#ifdef DODEBUG
#include <debug/log.h>
#include <debug/memory.h>

#define	MALLOC(_PTR)		mem_alloc(_PTR)
#define	REALLOC(_PTR, _SIZE)	mem_realloc(_PTR, _SIZE)
#define	FREE(_PTR)		mem_free(_PTR)

#else

#define	MALLOC(_PTR)		malloc(_PTR)
#define	REALLOC(_PTR, _SIZE)	realloc(_PTR, _SIZE)
#define	FREE(_PTR)		free(_PTR)
#endif

#define MSS_EXECFILENAME "sftp-server_MSS"

#define	_FILE_OFFSET_BITS	64

#ifdef __OSLinux
#define HAVE_NOTHING		1
#define HAVE_LOG_IN_COLOR	1
#define	CONFIG_FILE		"/etc/ssh/sftp_config"
#define	CONFIG_FILE2	"/etc/sshd/sftp_config"
#define	SHUTDOWN_FILE	"/etc/sftp.shut"
#endif

#ifdef __OSDarwin
#define HAVE_LIBKVM			1
#define HAVE_LOG_IN_COLOR	1
#define	CONFIG_FILE		"/etc/ssh/sftp_config"
#define	CONFIG_FILE2	"/etc/sshd/sftp_config"
#define	SHUTDOWN_FILE	"/etc/sftp.shut"
#endif

#ifdef __OSCYGWIN_NT_5_1
#define HAVE_NOTHING	1
#define	CONFIG_FILE		"/etc/ssh/sftp_config"
#define	CONFIG_FILE2	"/etc/sshd/sftp_config"
#define	SHUTDOWN_FILE	"/etc/sftp.shut"
#endif

#if(defined(__OSOpenBSD)||defined(__OSNetBSD)||defined(__OSFreeBSD))
#define HAVE_LIBKVM			1
#define HAVE_LOG_IN_COLOR	1
#define	CONFIG_FILE		"/etc/ssh/sftp_config"
#define	CONFIG_FILE2	"/etc/sshd/sftp_config"
#define	SHUTDOWN_FILE	"/etc/sftp.shut"
#endif

#endif
