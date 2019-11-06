/* -*- C -*-
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
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
 */

/* $Id: internal_functions.c.in,v 1.30.2.1 2006/01/01 12:50:17 sniper Exp $ */

#include "php.h"
#include "php_main.h"
#include "zend_modules.h"
#include "zend_compile.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "ext/libxml/php_libxml.h"
#include "ext/pcre/php_pcre.h"
#include "ext/ctype/php_ctype.h"
#include "ext/date/php_date.h"
#include "ext/dom/php_dom.h"
#include "ext/hash/php_hash.h"
#include "ext/iconv/php_iconv.h"
#include "ext/pdo/php_pdo.h"
#include "ext/posix/php_posix.h"
#include "ext/reflection/php_reflection.h"
#include "ext/session/php_session.h"
#include "ext/simplexml/php_simplexml.h"
#include "ext/spl/php_spl.h"
#include "ext/sqlite/php_sqlite.h"
#include "ext/standard/php_standard.h"
#include "ext/tokenizer/php_tokenizer.h"
#include "ext/xml/php_xml.h"
#include "ext/xmlreader/php_xmlreader.h"
#include "ext/xmlwriter/php_xmlwriter.h"


zend_module_entry *php_builtin_extensions[] = {
	phpext_xmlwriter_ptr,
	phpext_xmlreader_ptr,
	phpext_xml_ptr,
	phpext_tokenizer_ptr,
	phpext_standard_ptr,
	phpext_sqlite_ptr,
	phpext_spl_ptr,
	phpext_simplexml_ptr,
	phpext_session_ptr,
	phpext_reflection_ptr,
	phpext_posix_ptr,
	phpext_pdo_ptr,
	phpext_iconv_ptr,
	phpext_hash_ptr,
	phpext_dom_ptr,
	phpext_date_ptr,
	phpext_ctype_ptr,
	phpext_pcre_ptr,
	phpext_libxml_ptr,

};

#define EXTCOUNT (sizeof(php_builtin_extensions)/sizeof(zend_module_entry *))
	

int php_register_internal_extensions(TSRMLS_D)
{
	return php_register_extensions(php_builtin_extensions, EXTCOUNT TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
