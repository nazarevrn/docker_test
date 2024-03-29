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
   | Authors: Christian Stocker <chregu@php.net>                          |
   |          Rob Richards <rrichards@php.net>                            |
   +----------------------------------------------------------------------+
*/

/* $Id: text.c,v 1.23.2.1 2006/01/01 12:50:06 sniper Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#if HAVE_LIBXML && HAVE_DOM
#include "php_dom.h"
#include "dom_ce.h"

/*
* class DOMText extends DOMCharacterData 
*
* URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#ID-1312295772
* Since: 
*/

zend_function_entry php_dom_text_class_functions[] = {
	PHP_FALIAS(splitText, dom_text_split_text, NULL)
	PHP_FALIAS(isWhitespaceInElementContent, dom_text_is_whitespace_in_element_content, NULL)
	PHP_FALIAS(isElementContentWhitespace, dom_text_is_whitespace_in_element_content, NULL)
	PHP_FALIAS(replaceWholeText, dom_text_replace_whole_text, NULL)
	PHP_ME(domtext, __construct, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

/* {{{ proto void DOMText::__construct([string value]); */
PHP_METHOD(domtext, __construct)
{

	zval *id;
	xmlNodePtr nodep = NULL, oldnode = NULL;
	dom_object *intern;
	char *value = NULL;
	int value_len;

	php_set_error_handling(EH_THROW, dom_domexception_class_entry TSRMLS_CC);
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|s", &id, dom_text_class_entry, &value, &value_len) == FAILURE) {
		php_std_error_handling();
		return;
	}

	php_std_error_handling();
	nodep = xmlNewText((xmlChar *) value);

	if (!nodep) {
		php_dom_throw_error(INVALID_STATE_ERR, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	intern = (dom_object *)zend_object_store_get_object(id TSRMLS_CC);
	if (intern != NULL) {
		oldnode = (xmlNodePtr)intern->ptr;
		if (oldnode != NULL) {
			php_libxml_node_free_resource(oldnode  TSRMLS_CC);
		}
		php_libxml_increment_node_ptr((php_libxml_node_object *)intern, nodep, (void *)intern TSRMLS_CC);
	}
}
/* }}} end DOMText::__construct */

/* {{{ wholeText	string	
readonly=yes 
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#core-Text3-wholeText
Since: DOM Level 3
*/
int dom_text_whole_text_read(dom_object *obj, zval **retval TSRMLS_DC)
{
	xmlNodePtr node;
	xmlChar *wholetext;

	node = dom_object_get_node(obj);

	if (node == NULL) {
		php_dom_throw_error(INVALID_STATE_ERR, 0 TSRMLS_CC);
		return FAILURE;
	}

	ALLOC_ZVAL(*retval);
	wholetext = xmlNodeListGetString(node->doc, node, 1);
	ZVAL_STRING(*retval, wholetext, 1);

	xmlFree(wholetext);

	return SUCCESS;
}

/* }}} */


/* {{{ proto DOMText dom_text_split_text(int offset);
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#core-ID-38853C1D
Since: 
*/
PHP_FUNCTION(dom_text_split_text)
{
	zval       *id;
	xmlChar    *cur;
	xmlChar    *first;
	xmlChar    *second;
	xmlNodePtr  node;
	xmlNodePtr  nnode;
	long        offset;
	int         ret;
	int         length;
	dom_object	*intern;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ol", &id, dom_text_class_entry, &offset) == FAILURE) {
		return;
	}
	DOM_GET_OBJ(node, id, xmlNodePtr, intern);

	if (node->type != XML_TEXT_NODE) {
		RETURN_FALSE;
	}

	cur = xmlNodeGetContent(node);
	if (cur == NULL) {
		RETURN_FALSE;
	}
	length = xmlStrlen(cur);

	if (offset > length || offset < 0) {
		xmlFree(cur);
		RETURN_FALSE;
	}

	first = xmlStrndup(cur, offset);
	second = xmlStrdup(cur + offset);
	
	xmlFree(cur);

	xmlNodeSetContentLen(node, first, offset);
	nnode = xmlNewDocText(node->doc, second);
	
	xmlFree(first);
	xmlFree(second);

	if (node->parent != NULL) {
		nnode->type = XML_ELEMENT_NODE;
		xmlAddNextSibling(node, nnode);
		nnode->type = XML_TEXT_NODE;
	}
	
	return_value = php_dom_create_object(nnode, &ret, NULL, return_value, intern TSRMLS_CC);
}
/* }}} end dom_text_split_text */


/* {{{ proto boolean dom_text_is_whitespace_in_element_content();
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#core-Text3-isWhitespaceInElementContent
Since: DOM Level 3
*/
PHP_FUNCTION(dom_text_is_whitespace_in_element_content)
{
	zval       *id;
	xmlNodePtr  node;
	dom_object	*intern;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, dom_text_class_entry) == FAILURE) {
		return;
	}
	DOM_GET_OBJ(node, id, xmlNodePtr, intern);

	if (xmlIsBlankNode(node)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} end dom_text_is_whitespace_in_element_content */


/* {{{ proto DOMText dom_text_replace_whole_text(string content);
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#core-Text3-replaceWholeText
Since: DOM Level 3
*/
PHP_FUNCTION(dom_text_replace_whole_text)
{
 DOM_NOT_IMPLEMENTED();
}
/* }}} end dom_text_replace_whole_text */
#endif
