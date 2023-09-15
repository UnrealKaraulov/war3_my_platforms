#ifndef PVPGN_CONFIG_H
#define PVPGN_CONFIG_H

#define HAVE_FCNTL_H
/* #undef HAVE_SYS_TIME_H */
/* #undef HAVE_SYS_SELECT_H */
/* #undef HAVE_UNISTD_H */
/* #undef HAVE_SYS_UTSNAME_H */
#define HAVE_SYS_TIMEB_H
/* #undef HAVE_SYS_SOCKET_H */
/* #undef HAVE_SYS_PARAM_H */
/* #undef HAVE_NETINET_IN_H */
/* #undef HAVE_ARPA_INET_H */
/* #undef HAVE_NETDB_H */
/* #undef HAVE_TERMIOS_H */
#define HAVE_SYS_TYPES_H
/* #undef HAVE_SYS_WAIT_H */
/* #undef HAVE_SYS_FILE_H */
/* #undef HAVE_POLL_H */
/* #undef HAVE_SYS_POLL_H */
#define HAVE_SYS_STAT_H
/* #undef HAVE_PWD_H */
/* #undef HAVE_GRP_H */
/* #undef HAVE_DIR_H */
/* #undef HAVE_DIRENT_H */
/* #undef HAVE_NDIR_H */
/* #undef HAVE_SYS_DIR_H */
/* #undef HAVE_SYS_NDIR_H */
#define HAVE_DIRECT_H
/* #undef HAVE_SYS_MMAN_H */
/* #undef HAVE_SYS_EVENT_H */
/* #undef HAVE_SYS_EPOLL_H */
/* #undef HAVE_SYS_RESOURCE_H */
#define HAVE_WINDOWS_H
#define HAVE_WINSOCK2_H
#define HAVE_WS2TCPIP_H
#define HAVE_PROCESS_H

#define HAVE_CHDIR
/* #undef HAVE_EPOLL_CREATE */
/* #undef HAVE_FORK */
#define HAVE_FTIME
/* #undef HAVE_GETGID */
/* #undef HAVE_GETGRNAM */
#define HAVE_GETHOSTBYNAME
#define HAVE_GETHOSTNAME
/* #undef HAVE_GETLOGIN */
/* #undef HAVE_GETOPT */
#define HAVE_GETPID
/* #undef HAVE_GETPWNAME */
/* #undef HAVE_GETRLIMIT */
#define HAVE_GETSERVBYNAME
#define HAVE_MICROSOFT_GMTIME_S
/* #undef HAVE_GMTIME_S */
/* #undef HAVE_GMTIME_R */
/* #undef HAVE_GETTIMEOFDAY */
/* #undef HAVE_GETUID */
/* #undef HAVE_IOCTL */
/* #undef HAVE_KQUEUE */
#define HAVE_MICROSOFT_LOCALTIME_S
/* #undef HAVE_LOCALTIME_S */
/* #undef HAVE_LOCALTIME_R */
#define HAVE__MKDIR
#define HAVE_MKDIR
/* #undef HAVE_MMAP */
/* #undef HAVE_PIPE */
/* #undef HAVE_POLL */
#define HAVE_RECV
#define HAVE_RECVFROM
#define HAVE_SELECT
#define HAVE_SEND
#define HAVE_SENDTO
/* #undef HAVE_SETITIMER */
/* #undef HAVE_SETPGID */
/* #undef HAVE_SETPGRP */
/* #undef HAVE_SETSID */
/* #undef HAVE_SETUID */
/* #undef HAVE_SIGACTION */
/* #undef HAVE_SIGADDSET */
/* #undef HAVE_SIGPROCMASK */
#define HAVE_SOCKET
/* #undef HAVE_STRCASECMP */
#define HAVE_STRDUP
#define HAVE_STRICMP
/* #undef HAVE_STRNCASECMP */
#define HAVE_STRNICMP
/* #undef HAVE_STRSEP */
/* #undef HAVE_UNAME */
/* #undef HAVE_WAIT */
/* #undef HAVE_WAITPID */
#define MKDIR_TAKES_ONE_ARG

#define BNETD_DEFAULT_CONF_FILE "conf/bnetd.conf"
#define D2CS_DEFAULT_CONF_FILE "conf/d2cs.conf"
#define D2DBS_DEFAULT_CONF_FILE "conf/d2dbs.conf"

#endif
