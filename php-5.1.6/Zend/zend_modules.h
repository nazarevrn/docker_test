/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2006 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        | 
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id: zend_modules.h,v 1.67.2.3 2006/04/06 21:10:45 andrei Exp $ */

#ifndef MODULES_H
#define MODULES_H

#include "zend.h"
#include "zend_compile.h"

#define INIT_FUNC_ARGS		int type, int module_number TSRMLS_DC
#define INIT_FUNC_ARGS_PASSTHRU	type, module_number TSRMLS_CC
#define SHUTDOWN_FUNC_ARGS	int type, int module_number TSRMLS_DC
#define SHUTDOWN_FUNC_ARGS_PASSTHRU type, module_number TSRMLS_CC
#define ZEND_MODULE_INFO_FUNC_ARGS zend_module_entry *zend_module TSRMLS_DC
#define ZEND_MODULE_INFO_FUNC_ARGS_PASSTHRU zend_module TSRMLS_CC

extern struct _zend_arg_info first_arg_force_ref[2];
extern struct _zend_arg_info second_arg_force_ref[3];
extern struct _zend_arg_info third_arg_force_ref[4];
extern struct _zend_arg_info fourth_arg_force_ref[5];
extern struct _zend_arg_info fifth_arg_force_ref[6];
extern struct _zend_arg_info all_args_by_ref[1];

#define ZEND_MODULE_API_NO 20050922
#ifdef ZTS
#define USING_ZTS 1
#else
#define USING_ZTS 0
#endif

#define STANDARD_MODULE_HEADER_EX sizeof(zend_module_entry), ZEND_MODULE_API_NO, ZEND_DEBUG, USING_ZTS
#define STANDARD_MODULE_HEADER \
	STANDARD_MODULE_HEADER_EX, NULL, NULL
#define ZE2_STANDARD_MODULE_HEADER \
	STANDARD_MODULE_HEADER_EX, ini_entries, NULL

#define STANDARD_MODULE_PROPERTIES_EX 0, 0, 0, NULL, 0

#define STANDARD_MODULE_PROPERTIES \
	NULL, STANDARD_MODULE_PROPERTIES_EX

#define NO_VERSION_YET NULL

#define MODULE_PERSISTENT 1
#define MODULE_TEMPORARY 2

struct _zend_ini_entry;
typedef struct _zend_module_entry zend_module_entry;
typedef struct _zend_module_dep zend_module_dep;

struct _zend_module_entry {
	unsigned short size;
	unsigned int zend_api;
	unsigned char zend_debug;
	unsigned char zts;
	struct _zend_ini_entry *ini_entry;
	struct _zend_module_dep *deps;
	char *name;
	struct _zend_function_entry *functions;
	int (*module_startup_func)(INIT_FUNC_ARGS);
	int (*module_shutdown_func)(SHUTDOWN_FUNC_ARGS);
	int (*request_startup_func)(INIT_FUNC_ARGS);
	int (*request_shutdown_func)(SHUTDOWN_FUNC_ARGS);
	void (*info_func)(ZEND_MODULE_INFO_FUNC_ARGS);
	char *version;
	int (*post_deactivate_func)(void);
	int globals_id;
	int module_started;
	unsigned char type;
	void *handle;
	int module_number;
};

#define MODULE_DEP_REQUIRED		1
#define MODULE_DEP_CONFLICTS	2
#define MODULE_DEP_OPTIONAL		3

#define ZEND_MOD_REQUIRED_EX(name, rel, ver)	{ name, rel, ver, MODULE_DEP_REQUIRED  },
#define ZEND_MOD_CONFLICTS_EX(name, rel, ver)	{ name, rel, ver, MODULE_DEP_CONFLICTS },
#define ZEND_MOD_OPTIONAL_EX(name, rel, ver)	{ name, rel, ver, MODULE_DEP_OPTIONAL  },

#define ZEND_MOD_REQUIRED(name)		ZEND_MOD_REQUIRED_EX(name, NULL, NULL)
#define ZEND_MOD_CONFLICTS(name)	ZEND_MOD_CONFLICTS_EX(name, NULL, NULL)
#define ZEND_MOD_OPTIONAL(name)		ZEND_MOD_OPTIONAL_EX(name, NULL, NULL)

struct _zend_module_dep {
	char *name;			/* module name */
	char *rel;			/* version relationship: NULL (exists), lt|le|eq|ge|gt (to given version) */
	char *version;		/* version */
	unsigned char type;	/* dependency type */
};

extern ZEND_API HashTable module_registry;

void module_destructor(zend_module_entry *module);
int module_registry_cleanup(zend_module_entry *module TSRMLS_DC);
int module_registry_request_startup(zend_module_entry *module TSRMLS_DC);
int module_registry_unload_temp(zend_module_entry *module TSRMLS_DC);

#define ZEND_MODULE_DTOR (void (*)(void *)) module_destructor
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
