/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2007 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Kristian Koehntopp <kris@koehntopp.de>                       |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include <unistd.h>
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "php_posix.h"


#if HAVE_POSIX

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/times.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#if HAVE_SYS_MKDEV_H
# include <sys/mkdev.h>
#endif

ZEND_DECLARE_MODULE_GLOBALS(posix)

/* {{{ posix_functions[]
 */
zend_function_entry posix_functions[] = {
    /* POSIX.1, 3.3 */
	PHP_FE(posix_kill,		NULL)

	/* POSIX.1, 4.1 */
	PHP_FE(posix_getpid,	NULL)
	PHP_FE(posix_getppid,	NULL)

	/* POSIX.1,  4.2 */
	PHP_FE(posix_getuid,	NULL)
	PHP_FE(posix_setuid,	NULL)
	PHP_FE(posix_geteuid,	NULL)
#ifdef HAVE_SETEUID
	PHP_FE(posix_seteuid,	NULL)
#endif
	PHP_FE(posix_getgid,	NULL)
	PHP_FE(posix_setgid,	NULL)
	PHP_FE(posix_getegid,	NULL)
#ifdef HAVE_SETEGID
	PHP_FE(posix_setegid,	NULL)
#endif
#ifdef HAVE_GETGROUPS
	PHP_FE(posix_getgroups,	NULL)
#endif
#ifdef HAVE_GETLOGIN
	PHP_FE(posix_getlogin,	NULL)
#endif

	/* POSIX.1, 4.3 */
	PHP_FE(posix_getpgrp,	NULL)
#ifdef HAVE_SETSID
	PHP_FE(posix_setsid,	NULL)
#endif
	PHP_FE(posix_setpgid,	NULL)
	/* Non-Posix functions which are common */
#ifdef HAVE_GETPGID
	PHP_FE(posix_getpgid,	NULL)
#endif /* HAVE_GETPGID */
#ifdef HAVE_GETSID
	PHP_FE(posix_getsid,	NULL)
#endif /* HAVE_GETSID */

	/* POSIX.1, 4.4 */
	PHP_FE(posix_uname,		NULL)

	/* POSIX.1, 4.5 */
	PHP_FE(posix_times,		NULL)

	/* POSIX.1, 4.7 */
#ifdef HAVE_CTERMID
	PHP_FE(posix_ctermid,	NULL)
#endif
	PHP_FE(posix_ttyname,	NULL)
	PHP_FE(posix_isatty,	NULL)

    /* POSIX.1, 5.2 */
	PHP_FE(posix_getcwd,	NULL)

	/* POSIX.1, 5.4 */
#ifdef HAVE_MKFIFO
	PHP_FE(posix_mkfifo,	NULL)
#endif
#ifdef HAVE_MKNOD
	PHP_FE(posix_mknod,	NULL)
#endif

	/* POSIX.1, 5.6 */
	PHP_FE(posix_access,	NULL)
	/* POSIX.1, 9.2 */
	PHP_FE(posix_getgrnam,	NULL)
	PHP_FE(posix_getgrgid,	NULL)
	PHP_FE(posix_getpwnam,	NULL)
	PHP_FE(posix_getpwuid,	NULL)

#ifdef HAVE_GETRLIMIT
	PHP_FE(posix_getrlimit,	NULL)
#endif

	PHP_FE(posix_get_last_error,					NULL)
	PHP_FALIAS(posix_errno, posix_get_last_error,	NULL)
	PHP_FE(posix_strerror,							NULL)

	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
static PHP_MINFO_FUNCTION(posix)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Revision", "$Revision$");
	php_info_print_table_end();
}
/* }}} */

static void php_posix_init_globals(zend_posix_globals *posix_globals TSRMLS_DC)
{
	posix_globals->last_error = 0;
}

/* {{{ PHP_MINIT_FUNCTION(posix)
 */
static PHP_MINIT_FUNCTION(posix)
{
	ZEND_INIT_MODULE_GLOBALS(posix, php_posix_init_globals, NULL);
	REGISTER_LONG_CONSTANT("POSIX_F_OK", F_OK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("POSIX_X_OK", X_OK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("POSIX_W_OK", W_OK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("POSIX_R_OK", R_OK, CONST_CS | CONST_PERSISTENT);
#ifdef S_IFREG
	REGISTER_LONG_CONSTANT("POSIX_S_IFREG", S_IFREG, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef S_IFCHR
	REGISTER_LONG_CONSTANT("POSIX_S_IFCHR", S_IFCHR, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef S_IFBLK
	REGISTER_LONG_CONSTANT("POSIX_S_IFBLK", S_IFBLK, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef S_IFIFO
	REGISTER_LONG_CONSTANT("POSIX_S_IFIFO", S_IFIFO, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef S_IFSOCK
	REGISTER_LONG_CONSTANT("POSIX_S_IFSOCK", S_IFSOCK, CONST_CS | CONST_PERSISTENT);
#endif

	return SUCCESS;
}
/* }}} */

static PHP_MINFO_FUNCTION(posix);

/* {{{ posix_module_entry
 */
zend_module_entry posix_module_entry = {
	STANDARD_MODULE_HEADER,
	"posix", 
	posix_functions, 
	PHP_MINIT(posix),
	NULL,
	NULL,
	NULL, 
	PHP_MINFO(posix),
    NO_VERSION_YET,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_POSIX
ZEND_GET_MODULE(posix)
#endif

#define PHP_POSIX_NO_ARGS	if (ZEND_NUM_ARGS()) return;

#define PHP_POSIX_RETURN_LONG_FUNC(func_name)	\
	PHP_POSIX_NO_ARGS	\
	RETURN_LONG(func_name());

#define PHP_POSIX_SINGLE_ARG_FUNC(func_name)	\
	long val;	\
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &val) == FAILURE) RETURN_FALSE;	\
	if (func_name(val) < 0) {	\
		POSIX_G(last_error) = errno;	\
		RETURN_FALSE;	\
	}	\
	RETURN_TRUE;

/* {{{ proto bool posix_kill(int pid, int sig)
   Send a signal to a process (POSIX.1, 3.3.2) */

PHP_FUNCTION(posix_kill)
{
	long pid, sig;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &pid, &sig) == FAILURE) {
		RETURN_FALSE;
	}
		
	if (kill(pid, sig) < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
  	}
  	
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int posix_getpid(void)
   Get the current process id (POSIX.1, 4.1.1) */
PHP_FUNCTION(posix_getpid)
{
	PHP_POSIX_RETURN_LONG_FUNC(getpid);
}
/* }}} */

/* {{{ proto int posix_getppid(void)
   Get the parent process id (POSIX.1, 4.1.1) */
PHP_FUNCTION(posix_getppid)
{
	PHP_POSIX_RETURN_LONG_FUNC(getppid);
}
/* }}} */

/* {{{ proto int posix_getuid(void)
   Get the current user id (POSIX.1, 4.2.1) */
PHP_FUNCTION(posix_getuid)
{
	PHP_POSIX_RETURN_LONG_FUNC(getuid);
}
/* }}} */

/* {{{ proto int posix_getgid(void)
   Get the current group id (POSIX.1, 4.2.1) */
PHP_FUNCTION(posix_getgid)
{
	PHP_POSIX_RETURN_LONG_FUNC(getgid);
}
/* }}} */

/* {{{ proto int posix_geteuid(void)
   Get the current effective user id (POSIX.1, 4.2.1) */
PHP_FUNCTION(posix_geteuid)
{
	PHP_POSIX_RETURN_LONG_FUNC(geteuid);
}
/* }}} */

/* {{{ proto int posix_getegid(void)
   Get the current effective group id (POSIX.1, 4.2.1) */
PHP_FUNCTION(posix_getegid)
{
	PHP_POSIX_RETURN_LONG_FUNC(getegid);
}
/* }}} */

/* {{{ proto bool posix_setuid(long uid)
   Set user id (POSIX.1, 4.2.2) */
PHP_FUNCTION(posix_setuid)
{
	PHP_POSIX_SINGLE_ARG_FUNC(setuid);
}
/* }}} */

/* {{{ proto bool posix_setgid(int uid)
   Set group id (POSIX.1, 4.2.2) */
PHP_FUNCTION(posix_setgid)
{
	PHP_POSIX_SINGLE_ARG_FUNC(setgid);
}
/* }}} */

/* {{{ proto bool posix_seteuid(long uid)
   Set effective user id */
#ifdef HAVE_SETEUID
PHP_FUNCTION(posix_seteuid)
{
	PHP_POSIX_SINGLE_ARG_FUNC(seteuid);
}
#endif
/* }}} */

/* {{{ proto bool posix_setegid(long uid)
   Set effective group id */
#ifdef HAVE_SETEGID
PHP_FUNCTION(posix_setegid)
{
	PHP_POSIX_SINGLE_ARG_FUNC(setegid);
}
#endif
/* }}} */

/* {{{ proto array posix_getgroups(void)
   Get supplementary group id's (POSIX.1, 4.2.3) */
#ifdef HAVE_GETGROUPS
PHP_FUNCTION(posix_getgroups)
{
	gid_t  gidlist[NGROUPS_MAX];
	int    result;
	int    i;

	PHP_POSIX_NO_ARGS;
	
	if ((result = getgroups(NGROUPS_MAX, gidlist)) < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	array_init(return_value);

	for (i=0; i<result; i++) {
		add_next_index_long(return_value, gidlist[i]);
	}
}
#endif
/* }}} */

/* {{{ proto string posix_getlogin(void) 
   Get user name (POSIX.1, 4.2.4) */
#ifdef HAVE_GETLOGIN
PHP_FUNCTION(posix_getlogin)
{
	char *p;
	
	PHP_POSIX_NO_ARGS;
	
	if (NULL == (p = getlogin())) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}
	
	RETURN_STRING(p, 1);
}
#endif
/* }}} */

/* {{{ proto int posix_getpgrp(void)
   Get current process group id (POSIX.1, 4.3.1) */
PHP_FUNCTION(posix_getpgrp)
{
	PHP_POSIX_RETURN_LONG_FUNC(getpgrp);
}
/* }}} */

/* {{{ proto int posix_setsid(void)
   Create session and set process group id (POSIX.1, 4.3.2) */
#ifdef HAVE_SETSID
PHP_FUNCTION(posix_setsid)
{
	PHP_POSIX_RETURN_LONG_FUNC(setsid);
}
#endif
/* }}} */

/* {{{ proto bool posix_setpgid(int pid, int pgid)
   Set process group id for job control (POSIX.1, 4.3.3) */
PHP_FUNCTION(posix_setpgid)
{
	long pid, pgid;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &pid, &pgid) == FAILURE) {
		RETURN_FALSE;
	}
	
	if (setpgid(pid, pgid) < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}
	
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto int posix_getpgid(void)
   Get the process group id of the specified process (This is not a POSIX function, but a SVR4ism, so we compile conditionally) */
#ifdef HAVE_GETPGID
PHP_FUNCTION(posix_getpgid)
{
	long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	
	if ((val = getpgid(val)) < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}
	RETURN_LONG(val);
}
#endif
/* }}} */

/* {{{ proto int posix_getsid(void)
   Get process group id of session leader (This is not a POSIX function, but a SVR4ism, so be compile conditionally) */
#ifdef HAVE_GETSID
PHP_FUNCTION(posix_getsid)
{
	long val;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &val) == FAILURE) {
		RETURN_FALSE;
	}
	
	if ((val = getsid(val)) < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}
	RETURN_LONG(val);
}
#endif
/* }}} */

/* {{{ proto array posix_uname(void)
   Get system name (POSIX.1, 4.4.1) */
PHP_FUNCTION(posix_uname)
{
	struct utsname u;

	PHP_POSIX_NO_ARGS;

	if (uname(&u) < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	array_init(return_value);

	add_assoc_string(return_value, "sysname",  u.sysname,  1);
	add_assoc_string(return_value, "nodename", u.nodename, 1);
	add_assoc_string(return_value, "release",  u.release,  1);
	add_assoc_string(return_value, "version",  u.version,  1);
	add_assoc_string(return_value, "machine",  u.machine,  1);
#ifdef _GNU_SOURCE
	add_assoc_string(return_value, "domainname", u.domainname, 1);
#endif
}
/* }}} */

/* POSIX.1, 4.5.1 time() - Get System Time
							already covered by PHP
 */

/* {{{ proto array posix_times(void)
   Get process times (POSIX.1, 4.5.2) */
PHP_FUNCTION(posix_times)
{
	struct tms t;
	clock_t    ticks;

	PHP_POSIX_NO_ARGS;

	if((ticks = times(&t)) < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	array_init(return_value);

	add_assoc_long(return_value, "ticks",	ticks);			/* clock ticks */
	add_assoc_long(return_value, "utime",	t.tms_utime);	/* user time */
	add_assoc_long(return_value, "stime",	t.tms_stime);	/* system time */
	add_assoc_long(return_value, "cutime",	t.tms_cutime);	/* user time of children */
	add_assoc_long(return_value, "cstime",	t.tms_cstime);	/* system time of children */
}
/* }}} */

/* POSIX.1, 4.6.1 getenv() - Environment Access
							already covered by PHP
*/

/* {{{ proto string posix_ctermid(void)
   Generate terminal path name (POSIX.1, 4.7.1) */
#ifdef HAVE_CTERMID
PHP_FUNCTION(posix_ctermid)
{
	char  buffer[L_ctermid];
	
	PHP_POSIX_NO_ARGS;

	if (NULL == ctermid(buffer)) {
		/* Found no documentation how the defined behaviour is when this
		 * function fails
		 */
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}
	
	RETURN_STRING(buffer, 1);
}
#endif
/* }}} */

/* Checks if the provides resource is a stream and if it provides a file descriptor */
static int php_posix_stream_get_fd(zval *zfp, int *fd TSRMLS_DC)
{
	php_stream *stream;

	php_stream_from_zval_no_verify(stream, &zfp);

	if (stream == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "expects argument 1 to be a valid stream resource");
		return 0;
	}
	if (php_stream_can_cast(stream, PHP_STREAM_AS_FD) == SUCCESS) {
		php_stream_cast(stream, PHP_STREAM_AS_FD, (void*)fd, 0);
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not use stream of type '%s'", 
				stream->ops->label);
		return 0;
	}
	return 1;
}

/* {{{ proto string posix_ttyname(int fd)
   Determine terminal device name (POSIX.1, 4.7.2) */
PHP_FUNCTION(posix_ttyname)
{
	zval *z_fd;
	char *p;
	int fd;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z_fd) == FAILURE) {
		RETURN_FALSE;
	}

	switch (Z_TYPE_P(z_fd)) {
		case IS_RESOURCE:
			if (!php_posix_stream_get_fd(z_fd, &fd TSRMLS_CC)) {
				RETURN_FALSE;
			}
			break;
		default:
			convert_to_long(z_fd);
			fd = Z_LVAL_P(z_fd);
	}

	if (NULL == (p = ttyname(fd))) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}
	
	RETURN_STRING(p, 1);
}
/* }}} */

/* {{{ proto bool posix_isatty(int fd)
   Determine if filedesc is a tty (POSIX.1, 4.7.1) */
PHP_FUNCTION(posix_isatty)
{
	zval *z_fd;
	int fd;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z_fd) == FAILURE) {
		RETURN_FALSE;
	}
	
	switch (Z_TYPE_P(z_fd)) {
		case IS_RESOURCE:
			if (!php_posix_stream_get_fd(z_fd, &fd TSRMLS_CC)) {
				RETURN_FALSE;
			}
			break;
		default:
			convert_to_long(z_fd);
			fd = Z_LVAL_P(z_fd);
	}

	if (isatty(fd)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/*
	POSIX.1, 4.8.1 sysconf() - TODO
	POSIX.1, 5.7.1 pathconf(), fpathconf() - TODO

	POSIX.1, 5.1.2 opendir(), readdir(), rewinddir(), closedir()
	POSIX.1, 5.2.1 chdir()
				already supported by PHP
 */

/* {{{ proto string posix_getcwd(void)
   Get working directory pathname (POSIX.1, 5.2.2) */
PHP_FUNCTION(posix_getcwd)
{
	char  buffer[MAXPATHLEN];
	char *p;

	PHP_POSIX_NO_ARGS;

	p = VCWD_GETCWD(buffer, MAXPATHLEN);
	if (!p) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	RETURN_STRING(buffer, 1);
}
/* }}} */

/*
	POSIX.1, 5.3.x open(), creat(), umask()
	POSIX.1, 5.4.1 link()
		already supported by PHP.
 */

/* {{{ proto bool posix_mkfifo(string pathname, int mode)
   Make a FIFO special file (POSIX.1, 5.4.2) */
#ifdef HAVE_MKFIFO
PHP_FUNCTION(posix_mkfifo)
{
	char *path;
	int path_len;
	long mode;
	int     result;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &path, &path_len, &mode) == FAILURE) {
		RETURN_FALSE;
	}

	if (PG(safe_mode) && (!php_checkuid(path, NULL, CHECKUID_ALLOW_ONLY_DIR))) {
		RETURN_FALSE;
	}

	result = mkfifo(path, mode);
	if (result < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
#endif
/* }}} */

/* {{{ proto bool posix_mknod(string pathname, int mode [, int major [, int minor]])
   Make a special or ordinary file (POSIX.1) */
#ifdef HAVE_MKNOD
PHP_FUNCTION(posix_mknod)
{
	char *path;
	int path_len;
	long mode;
	long major = 0, minor = 0;
	int result;
	dev_t php_dev;

	php_dev = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|ll", &path, &path_len,
			&mode, &major, &minor) == FAILURE) {
		RETURN_FALSE;
	}

	if (php_check_open_basedir_ex(path, 0 TSRMLS_CC) ||
			(PG(safe_mode) && (!php_checkuid(path, NULL, CHECKUID_ALLOW_ONLY_DIR)))) {
		RETURN_FALSE;
	}

	if ((mode & S_IFCHR) || (mode & S_IFBLK)) {
		if (ZEND_NUM_ARGS() == 2) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "For S_IFCHR and S_IFBLK you need to pass a major device kernel identifier");
			RETURN_FALSE;
		}
		if (major == 0) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Expects argument 3 to be non-zero for POSIX_S_IFCHR and POSIX_S_IFBLK");
			RETURN_FALSE;
		} else {
#if defined(HAVE_MAKEDEV) || defined(makedev)
			php_dev = makedev(major, minor);
#else
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Can not create a block or character device, creating a normal file instead");
#endif
		}
	}

	result = mknod(path, mode, php_dev);
	if (result < 0) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
#endif
/* }}} */

/* Takes a pointer to posix group and a pointer to an already initialized ZVAL
 * array container and fills the array with the posix group member data. */
int php_posix_group_to_array(struct group *g, zval *array_group) {
	zval *array_members;
	int count;

	if (NULL == g)
		return 0;

	if (array_group == NULL || Z_TYPE_P(array_group) != IS_ARRAY)
		return 0;

	MAKE_STD_ZVAL(array_members);
	array_init(array_members);
	
	add_assoc_string(array_group, "name", g->gr_name, 1);
	add_assoc_string(array_group, "passwd", g->gr_passwd, 1);
	for (count=0; g->gr_mem[count] != NULL; count++) {
		add_next_index_string(array_members, g->gr_mem[count], 1);
	}
	zend_hash_update(Z_ARRVAL_P(array_group), "members", sizeof("members"), (void*)&array_members, sizeof(zval*), NULL);
	add_assoc_long(array_group, "gid", g->gr_gid);
	return 1;
}

/*
	POSIX.1, 5.5.1 unlink()
	POSIX.1, 5.5.2 rmdir()
	POSIX.1, 5.5.3 rename()
	POSIX.1, 5.6.x stat(), chmod(), utime() already supported by PHP.
*/

/* {{{ proto bool posix_access(string file [, int mode])
   Determine accessibility of a file (POSIX.1 5.6.3) */
PHP_FUNCTION(posix_access)
{
	long mode = 0;
	int filename_len, ret;
	char *filename, *path;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &filename, &filename_len, &mode) == FAILURE) {
		RETURN_FALSE;
	}

	path = expand_filepath(filename, NULL TSRMLS_CC);

	if (php_check_open_basedir_ex(path, 0 TSRMLS_CC) ||
			(PG(safe_mode) && (!php_checkuid_ex(filename, NULL, CHECKUID_CHECK_FILE_AND_DIR, CHECKUID_NO_ERRORS)))) {
		efree(path);
		POSIX_G(last_error) = EPERM;
		RETURN_FALSE;
	}

	ret = access(path, mode);
	efree(path);

	if (ret) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/*
	POSIX.1, 6.x most I/O functions already supported by PHP.
	POSIX.1, 7.x tty functions, TODO
	POSIX.1, 8.x interactions with other C language functions
	POSIX.1, 9.x system database access
*/

/* {{{ proto array posix_getgrnam(string groupname)
   Group database access (POSIX.1, 9.2.1) */
PHP_FUNCTION(posix_getgrnam)
{
	char *name;
	struct group *g;
	int name_len;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (NULL == (g = getgrnam(name))) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}
	
	array_init(return_value);

	if (!php_posix_group_to_array(g, return_value)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to convert posix group to array");
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto array posix_getgrgid(long gid) 
   Group database access (POSIX.1, 9.2.1) */
PHP_FUNCTION(posix_getgrgid)
{
	long gid;
	struct group *g;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &gid) == FAILURE) {
		RETURN_FALSE;
	}

	if (NULL == (g = getgrgid(gid))) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	array_init(return_value);

	if (!php_posix_group_to_array(g, return_value)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to convert posix group struct to array");
		RETURN_FALSE;
	}
}
/* }}} */

int php_posix_passwd_to_array(struct passwd *pw, zval *return_value) {
	if (NULL == pw)
		return 0;
	if (NULL == return_value || Z_TYPE_P(return_value) != IS_ARRAY)
		return 0;

	add_assoc_string(return_value, "name",      pw->pw_name, 1);
	add_assoc_string(return_value, "passwd",    pw->pw_passwd, 1);
	add_assoc_long  (return_value, "uid",       pw->pw_uid);
	add_assoc_long  (return_value, "gid",		pw->pw_gid);
	add_assoc_string(return_value, "gecos",     pw->pw_gecos, 1);
	add_assoc_string(return_value, "dir",       pw->pw_dir, 1);
	add_assoc_string(return_value, "shell",     pw->pw_shell, 1);
	return 1;
}

/* {{{ proto array posix_getpwnam(string groupname) 
   User database access (POSIX.1, 9.2.2) */
PHP_FUNCTION(posix_getpwnam)
{
	struct passwd *pw;
	char *name;
	int name_len;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
		RETURN_FALSE;
	}

	if (NULL == (pw = getpwnam(name))) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}
	
	array_init(return_value);

	if (!php_posix_passwd_to_array(pw, return_value)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to convert posix passwd struct to array");
		RETURN_FALSE;
	}

}
/* }}} */

/* {{{ proto array posix_getpwuid(long uid) 
   User database access (POSIX.1, 9.2.2) */
PHP_FUNCTION(posix_getpwuid)
{
	long uid;
	struct passwd *pw;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &uid) == FAILURE) {
		RETURN_FALSE;
	}

	if (NULL == (pw = getpwuid(uid))) {
		POSIX_G(last_error) = errno;
		RETURN_FALSE;
	}

	array_init(return_value);

	if (!php_posix_passwd_to_array(pw, return_value)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to convert posix passwd struct to array");
		RETURN_FALSE;
	}
}
/* }}} */


#ifdef HAVE_GETRLIMIT

#define UNLIMITED_STRING "unlimited"

/* {{{ posix_addlimit
 */
static int posix_addlimit(int limit, char *name, zval *return_value TSRMLS_DC) {
	int result;
	struct rlimit rl;
	char hard[80];
	char soft[80];

	snprintf(hard, 80, "hard %s", name);
	snprintf(soft, 80, "soft %s", name);

	result = getrlimit(limit, &rl);
	if (result < 0) {
		POSIX_G(last_error) = errno;
		return FAILURE;
	}

	if (rl.rlim_cur == RLIM_INFINITY) {
		add_assoc_stringl(return_value, soft, UNLIMITED_STRING, sizeof(UNLIMITED_STRING)-1, 1);
	} else {
		add_assoc_long(return_value, soft, rl.rlim_cur);
	}

	if (rl.rlim_max == RLIM_INFINITY) {
		add_assoc_stringl(return_value, hard, UNLIMITED_STRING, sizeof(UNLIMITED_STRING)-1, 1);
	} else {
		add_assoc_long(return_value, hard, rl.rlim_max);
	}

	return SUCCESS;
}
/* }}} */

/* {{{ limits[]
 */
struct limitlist {
	int limit;
	char *name;
} limits[] = {
#ifdef RLIMIT_CORE
	{ RLIMIT_CORE,	"core" },
#endif

#ifdef RLIMIT_DATA
	{ RLIMIT_DATA,	"data" },
#endif

#ifdef RLIMIT_STACK
	{ RLIMIT_STACK,	"stack" },
#endif

#ifdef RLIMIT_VMEM
	{ RLIMIT_VMEM, "virtualmem" },
#endif

#ifdef RLIMIT_AS
	{ RLIMIT_AS, "totalmem" },
#endif

#ifdef RLIMIT_RSS
	{ RLIMIT_RSS, "rss" },
#endif

#ifdef RLIMIT_NPROC
	{ RLIMIT_NPROC, "maxproc" },
#endif

#ifdef RLIMIT_MEMLOCK
	{ RLIMIT_MEMLOCK, "memlock" },
#endif

#ifdef RLIMIT_CPU
	{ RLIMIT_CPU,	"cpu" },
#endif

#ifdef RLIMIT_FSIZE
	{ RLIMIT_FSIZE, "filesize" },
#endif

#ifdef RLIMIT_NOFILE
	{ RLIMIT_NOFILE, "openfiles" },
#endif

#ifdef RLIMIT_OFILE
	{ RLIMIT_OFILE, "openfiles" },
#endif

	{ 0, NULL }
};
/* }}} */


/* {{{ proto array posix_getrlimit(void)
   Get system resource consumption limits (This is not a POSIX function, but a BSDism and a SVR4ism. We compile conditionally) */
PHP_FUNCTION(posix_getrlimit)
{
	struct limitlist *l = NULL;

	PHP_POSIX_NO_ARGS;

	array_init(return_value);

	for (l=limits; l->name; l++) {
		if (posix_addlimit(l->limit, l->name, return_value TSRMLS_CC) == FAILURE)
			RETURN_FALSE;
	}
}
/* }}} */

#endif /* HAVE_GETRLIMIT */

/* {{{ proto int posix_get_last_error(void)
   Retrieve the error number set by the last posix function which failed. */
PHP_FUNCTION(posix_get_last_error)
{
	PHP_POSIX_NO_ARGS;	

	RETURN_LONG(POSIX_G(last_error));
}
/* }}} */

/* {{{ proto string posix_strerror(int errno)
   Retrieve the system error message associated with the given errno. */
PHP_FUNCTION(posix_strerror)
{
	long error;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &error) == FAILURE) {
		RETURN_FALSE;
	}

	RETURN_STRING(strerror(error), 1);
}
/* }}} */

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
