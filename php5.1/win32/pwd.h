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
   | Author: Sterling Hughes <sterling@php.net>                           |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PWD_H
#define PWD_H

struct passwd {
	char *pw_name;		
	char *pw_passwd;		
	int pw_uid;	
	int pw_gid;	
	char *pw_comment;	
	char *pw_gecos;	
	char *pw_dir;
	char *pw_shell;	
};

extern struct passwd *getpwuid(int);
extern struct passwd *getpwnam(char *name);
extern char *getlogin(void);
#endif
