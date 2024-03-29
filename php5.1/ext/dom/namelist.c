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
   | Authors: Christian Stocker <chregu@php.net>                          |
   |          Rob Richards <rrichards@php.net>                            |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#if HAVE_LIBXML && HAVE_DOM
#include "php_dom.h"


/*
* class DOMNameList 
*
* URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#NameList
* Since: DOM Level 3
*/

zend_function_entry php_dom_namelist_class_functions[] = {
	PHP_FALIAS(getName, dom_namelist_get_name, NULL)
	PHP_FALIAS(getNamespaceURI, dom_namelist_get_namespace_uri, NULL)
	{NULL, NULL, NULL}
};

/* {{{ length	int	
readonly=yes 
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#NameList-length
Since: 
*/
int dom_namelist_length_read(dom_object *obj, zval **retval TSRMLS_DC)
{
	ALLOC_ZVAL(*retval);
	ZVAL_STRING(*retval, "TEST", 1);
	return SUCCESS;
}

/* }}} */




/* {{{ proto string dom_namelist_get_name(int index);
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#NameList-getName
Since: 
*/
PHP_FUNCTION(dom_namelist_get_name)
{
 DOM_NOT_IMPLEMENTED();
}
/* }}} end dom_namelist_get_name */


/* {{{ proto string dom_namelist_get_namespace_uri(int index);
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#NameList-getNamespaceURI
Since: 
*/
PHP_FUNCTION(dom_namelist_get_namespace_uri)
{
 DOM_NOT_IMPLEMENTED();
}
/* }}} end dom_namelist_get_namespace_uri */
#endif
