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
   | Author: Andrei Zmievski <andrei@php.net>                             |
   +----------------------------------------------------------------------+
 */
 
/* $Id$ */

#ifndef PHP_PCRE_H
#define PHP_PCRE_H

#if HAVE_PCRE || HAVE_BUNDLED_PCRE

#if HAVE_BUNDLED_PCRE
#include "pcrelib/pcre.h"
#else
#include "pcre.h"
#endif

#if HAVE_LOCALE_H
#include <locale.h>
#endif

PHP_FUNCTION(preg_match);
PHP_FUNCTION(preg_match_all);
PHP_FUNCTION(preg_replace);
PHP_FUNCTION(preg_replace_callback);
PHP_FUNCTION(preg_split);
PHP_FUNCTION(preg_quote);
PHP_FUNCTION(preg_grep);

PHPAPI char *php_pcre_replace(char *regex,   int regex_len, char *subject, int subject_len, zval *replace_val, int is_callable_replace, int *result_len, int limit, int *replace_count TSRMLS_DC);
PHPAPI pcre* pcre_get_compiled_regex(char *regex, pcre_extra **extra, int *options TSRMLS_DC);
PHPAPI pcre* pcre_get_compiled_regex_ex(char *regex, pcre_extra **extra, int *preg_options, int *coptions TSRMLS_DC);

extern zend_module_entry pcre_module_entry;
#define pcre_module_ptr &pcre_module_entry

typedef struct {
	pcre *re;
	pcre_extra *extra;
	int preg_options;
#if HAVE_SETLOCALE
	char *locale;
	unsigned const char *tables;
#endif
	int compile_options;
} pcre_cache_entry;

ZEND_BEGIN_MODULE_GLOBALS(pcre)
	HashTable pcre_cache;
ZEND_END_MODULE_GLOBALS(pcre)

#ifdef ZTS
# define PCRE_G(v) TSRMG(pcre_globals_id, zend_pcre_globals *, v)
#else
# define PCRE_G(v)	(pcre_globals.v)
#endif

#else

#define pcre_module_ptr NULL

#endif /* HAVE_PCRE || HAVE_BUNDLED_PCRE */

#define phpext_pcre_ptr pcre_module_ptr

#endif /* PHP_PCRE_H */
