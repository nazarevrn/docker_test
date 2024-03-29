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
  | Author: Wez Furlong <wez@thebrainroom.com>                           |
  +----------------------------------------------------------------------+
*/

/* $Id: php_streams_int.h,v 1.7.2.2 2006/04/22 17:17:40 wez Exp $ */


#if ZEND_DEBUG

#if USE_ZEND_ALLOC
# define emalloc_rel_orig(size)	\
		( __php_stream_call_depth == 0 \
		? _emalloc((size) ZEND_FILE_LINE_CC ZEND_FILE_LINE_RELAY_CC) \
		: _emalloc((size) ZEND_FILE_LINE_CC ZEND_FILE_LINE_ORIG_RELAY_CC) )

# define erealloc_rel_orig(ptr, size)	\
		( __php_stream_call_depth == 0 \
		? _erealloc((ptr), (size), 0 ZEND_FILE_LINE_CC ZEND_FILE_LINE_RELAY_CC) \
		: _erealloc((ptr), (size), 0 ZEND_FILE_LINE_CC ZEND_FILE_LINE_ORIG_RELAY_CC) )
#else
# define emalloc_rel_orig(size)			emalloc(size)
# define erealloc_rel_orig(ptr, size)	erealloc(ptr, size)
#endif

#define pemalloc_rel_orig(size, persistent)	((persistent) ? malloc((size)) : emalloc_rel_orig((size)))
#define perealloc_rel_orig(ptr, size, persistent)	((persistent) ? realloc((ptr), (size)) : erealloc_rel_orig((ptr), (size)))
#else
# define pemalloc_rel_orig(size, persistent)				pemalloc((size), (persistent))
# define perealloc_rel_orig(ptr, size, persistent)			perealloc((ptr), (size), (persistent))
# define emalloc_rel_orig(size)								emalloc((size))
#endif

#define STREAM_DEBUG 0
#define STREAM_WRAPPER_PLAIN_FILES	((php_stream_wrapper*)-1)

#ifndef MAP_FAILED
#define MAP_FAILED ((void *) -1)
#endif

#define CHUNK_SIZE	8192

#ifdef PHP_WIN32
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif

#ifndef S_ISREG
#define S_ISREG(mode)	(((mode)&S_IFMT) == S_IFREG)
#endif

void php_stream_tidy_wrapper_error_log(php_stream_wrapper *wrapper TSRMLS_DC);
void php_stream_display_wrapper_errors(php_stream_wrapper *wrapper, const char *path, const char *caption TSRMLS_DC);

