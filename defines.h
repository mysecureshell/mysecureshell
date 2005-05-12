#define MSS_EXECFILENAME "sftp-server_MSS"

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
