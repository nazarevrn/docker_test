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
  | Author: Sterling Hughes <sterling@php.net>                           |
  +----------------------------------------------------------------------+
*/

/* $Id: php_simplexml.h,v 1.20.2.2 2006/02/26 23:14:45 helly Exp $ */

#ifndef PHP_SIMPLEXML_H
#define PHP_SIMPLEXML_H

extern zend_module_entry simplexml_module_entry;
#define phpext_simplexml_ptr &simplexml_module_entry

#ifdef PHP_WIN32
#define PHP_SIMPLEXML_API __declspec(dllexport)
#else
#define PHP_SIMPLEXML_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#include "ext/libxml/php_libxml.h"
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/tree.h>
#include <libxml/uri.h>
#include <libxml/xmlerror.h>
#include <libxml/xinclude.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xpointer.h>
#include <libxml/xmlschemas.h>

PHP_MINIT_FUNCTION(simplexml);
PHP_MSHUTDOWN_FUNCTION(simplexml);
#ifdef HAVE_SPL
PHP_RINIT_FUNCTION(simplexml);
#endif
PHP_MINFO_FUNCTION(simplexml);

typedef enum {
	SXE_ITER_NONE     = 0,
	SXE_ITER_ELEMENT  = 1,
	SXE_ITER_CHILD    = 2,
	SXE_ITER_ATTRLIST = 3
} SXE_ITER;

typedef struct {
	zend_object zo;
	php_libxml_node_ptr *node;
	php_libxml_ref_obj *document;
	HashTable *properties;
	xmlXPathContextPtr xpath;
	struct {
		int                   itertype;
		char                  *name;
		char                  *nsprefix;
		SXE_ITER              type;
		zval                  *data;
	} iter;
	zval *tmp;
} php_sxe_object;

#ifdef ZTS
#define SIMPLEXML_G(v) TSRMG(simplexml_globals_id, zend_simplexml_globals *, v)
#else
#define SIMPLEXML_G(v) (simplexml_globals.v)
#endif

ZEND_API zend_class_entry *sxe_get_element_class_entry();

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
