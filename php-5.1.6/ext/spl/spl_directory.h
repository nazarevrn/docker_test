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
   | Authors: Marcus Boerger <helly@php.net>                              |
   +----------------------------------------------------------------------+
 */

/* $Id: spl_directory.h,v 1.12.2.5 2006/03/08 21:54:48 helly Exp $ */

#ifndef SPL_DIRECTORY_H
#define SPL_DIRECTORY_H

#include "php.h"
#include "php_spl.h"

extern PHPAPI zend_class_entry *spl_ce_SplFileInfo;
extern PHPAPI zend_class_entry *spl_ce_DirectoryIterator;
extern PHPAPI zend_class_entry *spl_ce_RecursiveDirectoryIterator;
extern PHPAPI zend_class_entry *spl_ce_SplFileObject;
extern PHPAPI zend_class_entry *spl_ce_SplTempFileObject;

PHP_MINIT_FUNCTION(spl_directory);

typedef enum {
	SPL_FS_INFO, /* must be 0 */
	SPL_FS_DIR,
	SPL_FS_FILE,
} SPL_FS_OBJ_TYPE;

typedef struct _spl_filesystem_object  spl_filesystem_object;

typedef void (*spl_foreign_dtor_t)(spl_filesystem_object *object TSRMLS_DC);
typedef void (*spl_foreign_clone_t)(spl_filesystem_object *src, spl_filesystem_object *dst TSRMLS_DC);

typedef struct _spl_other_handler {
	spl_foreign_dtor_t     dtor;
	spl_foreign_clone_t    clone;
} spl_other_handler;

struct _spl_filesystem_object {
	zend_object        std;
	void               *oth;
	spl_other_handler  *oth_handler;
	char               *path;
	int                path_len;
	char               *file_name;
	int                file_name_len; 
	SPL_FS_OBJ_TYPE    type;
	long               flags;
	zend_class_entry   *file_class;
	zend_class_entry   *info_class;
	union {
		struct {
			php_stream         *dirp;
			php_stream_dirent  entry;
			char               *sub_path;
			int                sub_path_len;
			int                index;
			int                is_recursive;
		} dir;
		struct {
			php_stream         *stream;
			php_stream_context *context;
			zval               *zcontext;
			char               *open_mode;
			int                open_mode_len;
			zval               *current_zval;
			char               *current_line;
			size_t             current_line_len;
			size_t             max_line_len;
			long               current_line_num;
			zval               zresource;
			zend_function      *func_getCurr;
		} file;
	} u;
};

#define SPL_FILE_OBJECT_DROP_NEW_LINE      0x00000001 /* drop new lines */

#define SPL_FILE_DIR_CURRENT_AS_FILEINFO   0x00000010 /* make RecursiveDirectoryTree::current() return SplFileInfo */
#define SPL_FILE_DIR_CURRENT_AS_PATHNAME   0x00000020 /* make RecursiveDirectoryTree::current() return getPathname() */
#define SPL_FILE_DIR_CURRENT_MODE_MASK     0x000000F0 /* make RecursiveDirectoryTree::key() return getFilename() */

#define SPL_FILE_DIR_KEY_AS_FILENAME       0x00000100 /* make RecursiveDirectoryTree::key() return getFilename() */
#define SPL_FILE_DIR_KEY_MODE_MASK         0x00000F00 /* make RecursiveDirectoryTree::key() return getFilename() */

#endif /* SPL_DIRECTORY_H */

/*
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
