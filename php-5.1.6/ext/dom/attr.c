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

/* $Id: attr.c,v 1.18.2.2 2006/05/03 08:43:04 rrichards Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"

#if HAVE_LIBXML && HAVE_DOM

#include "php_dom.h"


/*
* class DOMAttr extends DOMNode 
*
* URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#ID-637646024
* Since: 
*/

zend_function_entry php_dom_attr_class_functions[] = {
	PHP_FALIAS(isId, dom_attr_is_id, NULL)
	PHP_ME(domattr, __construct, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

/* {{{ proto void DOMAttr::__construct(string name, [string value]); */
PHP_METHOD(domattr, __construct)
{

	zval *id;
	xmlAttrPtr nodep = NULL;
	xmlNodePtr oldnode = NULL;
	dom_object *intern;
	char *name, *value = NULL;
	int name_len, value_len, name_valid;

	php_set_error_handling(EH_THROW, dom_domexception_class_entry TSRMLS_CC);
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|s", &id, dom_attr_class_entry, &name, &name_len, &value, &value_len) == FAILURE) {
		php_std_error_handling();
		return;
	}

	php_std_error_handling();
	intern = (dom_object *)zend_object_store_get_object(id TSRMLS_CC);

	name_valid = xmlValidateName((xmlChar *) name, 0);
	if (name_valid != 0) {
		php_dom_throw_error(INVALID_CHARACTER_ERR, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	nodep = xmlNewProp(NULL, (xmlChar *) name, value);

	if (!nodep) {
		php_dom_throw_error(INVALID_STATE_ERR, 1 TSRMLS_CC);
		RETURN_FALSE;
	}

	if (intern != NULL) {
		oldnode = dom_object_get_node(intern);
		if (oldnode != NULL) {
			php_libxml_node_free_resource(oldnode  TSRMLS_CC);
		}
		php_libxml_increment_node_ptr((php_libxml_node_object *)intern, (xmlNodePtr)nodep, (void *)intern TSRMLS_CC);
	}
}

/* }}} end DOMAttr::__construct */


/* {{{ name	string	
readonly=yes 
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#ID-1112119403
Since: 
*/
int dom_attr_name_read(dom_object *obj, zval **retval TSRMLS_DC)
{
	xmlAttrPtr attrp;

	attrp = (xmlAttrPtr) dom_object_get_node(obj);

	if (attrp == NULL) {
		php_dom_throw_error(INVALID_STATE_ERR, 0 TSRMLS_CC);
		return FAILURE;
	}

	ALLOC_ZVAL(*retval);
	ZVAL_STRING(*retval, (char *) (attrp->name), 1);

	return SUCCESS;
}

/* }}} */



/* {{{ specified	boolean	
readonly=yes 
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#ID-862529273
Since: 
*/
int dom_attr_specified_read(dom_object *obj, zval **retval TSRMLS_DC)
{
	/* TODO */
	ALLOC_ZVAL(*retval);
	ZVAL_TRUE(*retval);
	return SUCCESS;
}

/* }}} */



/* {{{ value	string	
readonly=no 
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#ID-221662474
Since: 
*/
int dom_attr_value_read(dom_object *obj, zval **retval TSRMLS_DC)
{
	xmlAttrPtr attrp;
	xmlChar *content;

	attrp = (xmlAttrPtr) dom_object_get_node(obj);

	if (attrp == NULL) {
		php_dom_throw_error(INVALID_STATE_ERR, 0 TSRMLS_CC);
		return FAILURE;
	}

	ALLOC_ZVAL(*retval);

	
	if ((content = xmlNodeGetContent((xmlNodePtr) attrp)) != NULL) {
		ZVAL_STRING(*retval, content, 1);
		xmlFree(content);
	} else {
		ZVAL_EMPTY_STRING(*retval);
	}

	return SUCCESS;

}

int dom_attr_value_write(dom_object *obj, zval *newval TSRMLS_DC)
{
	zval value_copy;
	xmlAttrPtr attrp;

	attrp = (xmlAttrPtr) dom_object_get_node(obj);

	if (attrp == NULL) {
		php_dom_throw_error(INVALID_STATE_ERR, 0 TSRMLS_CC);
		return FAILURE;
	}

	if (attrp->children) {
		node_list_unlink(attrp->children TSRMLS_CC);
	}

	if (newval->type != IS_STRING) {
		if(newval->refcount > 1) {
			value_copy = *newval;
			zval_copy_ctor(&value_copy);
			newval = &value_copy;
		}
		convert_to_string(newval);
	}

	xmlNodeSetContentLen((xmlNodePtr) attrp, Z_STRVAL_P(newval), Z_STRLEN_P(newval) + 1);

	if (newval == &value_copy) {
		zval_dtor(newval);
	}

	return SUCCESS;
}

/* }}} */



/* {{{ ownerElement	DOMElement	
readonly=yes 
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#Attr-ownerElement
Since: DOM Level 2
*/
int dom_attr_owner_element_read(dom_object *obj, zval **retval TSRMLS_DC)
{
	xmlNodePtr nodep, nodeparent;
	int ret;

	nodep = dom_object_get_node(obj);

	if (nodep == NULL) {
		php_dom_throw_error(INVALID_STATE_ERR, 0 TSRMLS_CC);
		return FAILURE;
	}

	nodeparent = nodep->parent;
	if (!nodeparent) {
		return FAILURE;
	}

	ALLOC_ZVAL(*retval);

	if (NULL == (*retval = php_dom_create_object(nodeparent, &ret, NULL, *retval, obj TSRMLS_CC))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING,  "Cannot create required DOM object");
		return FAILURE;
	}
	return SUCCESS;

}

/* }}} */



/* {{{ schemaTypeInfo	DOMTypeInfo	
readonly=yes 
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#Attr-schemaTypeInfo
Since: DOM Level 3
*/
int dom_attr_schema_type_info_read(dom_object *obj, zval **retval TSRMLS_DC)
{
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Not yet implemented");
	ALLOC_ZVAL(*retval);
	ZVAL_NULL(*retval);
	return SUCCESS;
}

/* }}} */



/* {{{ proto boolean dom_attr_is_id();
URL: http://www.w3.org/TR/2003/WD-DOM-Level-3-Core-20030226/DOM3-Core.html#Attr-isId
Since: DOM Level 3
*/
PHP_FUNCTION(dom_attr_is_id)
{
	zval *id;
	dom_object *intern;
	xmlAttrPtr attrp;
	xmlNodePtr nodep;

	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, dom_attr_class_entry) == FAILURE) {
		return;
	}

	DOM_GET_OBJ(attrp, id, xmlAttrPtr, intern);

	nodep = attrp->parent;

	if (xmlIsID(attrp->doc, nodep, attrp)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} end dom_attr_is_id */

#endif
