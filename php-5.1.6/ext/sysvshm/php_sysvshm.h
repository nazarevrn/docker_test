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
   | Author: Christian Cartus <cartus@atrior.de>                          |
   +----------------------------------------------------------------------+
*/

/* $Id: php_sysvshm.h,v 1.17.2.1 2006/01/01 12:50:16 sniper Exp $ */

#ifndef PHP_SYSVSHM_H
#define PHP_SYSVSHM_H

#if HAVE_SYSVSHM

extern zend_module_entry sysvshm_module_entry;
#define sysvshm_module_ptr &sysvshm_module_entry

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct {
	int le_shm;
	long init_mem;
} sysvshm_module;

typedef struct {
	long key;
	long length;
	long next;
	char mem;
} sysvshm_chunk;

typedef struct {
	char magic[8];
	long start;
	long end;
	long free;
	long total;
} sysvshm_chunk_head;

typedef struct {
	key_t key;               /* Key set by user */
	long id;                 /* Returned by shmget. */
	sysvshm_chunk_head *ptr; /* memoryaddress of shared memory */ 
} sysvshm_shm;

PHP_MINIT_FUNCTION(sysvshm);
PHP_FUNCTION(shm_attach);
PHP_FUNCTION(shm_detach);
PHP_FUNCTION(shm_remove);
PHP_FUNCTION(shm_put_var);
PHP_FUNCTION(shm_get_var);
PHP_FUNCTION(shm_remove_var);

extern sysvshm_module php_sysvshm;

#else

#define sysvshm_module_ptr NULL

#endif

#define phpext_sysvshm_ptr sysvshm_module_ptr

#endif /* PHP_SYSVSHM_H */
