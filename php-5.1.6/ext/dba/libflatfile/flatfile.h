/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2006 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Marcus Boerger <helly@php.net>                               |
   +----------------------------------------------------------------------+
 */

/* $Id: flatfile.h,v 1.11.2.1 2006/01/01 12:50:05 sniper Exp $ */

#ifndef PHP_LIB_FLATFILE_H
#define PHP_LIB_FLATFILE_H

typedef struct {
	char *dptr;
	size_t dsize;
} datum;

typedef struct {
	char *lockfn;
	int lockfd;
	php_stream *fp;
	size_t CurrentFlatFilePos;
	datum nextkey;
} flatfile;

#define FLATFILE_INSERT 1
#define FLATFILE_REPLACE 0

int flatfile_store(flatfile *dba, datum key_datum, datum value_datum, int mode TSRMLS_DC);
datum flatfile_fetch(flatfile *dba, datum key_datum TSRMLS_DC);
int flatfile_delete(flatfile *dba, datum key_datum TSRMLS_DC);
int flatfile_findkey(flatfile *dba, datum key_datum TSRMLS_DC);
datum flatfile_firstkey(flatfile *dba TSRMLS_DC);
datum flatfile_nextkey(flatfile *dba TSRMLS_DC);
char *flatfile_version();

#endif
