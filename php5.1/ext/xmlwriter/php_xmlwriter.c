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
  | Author: Rob Richards <rrichards@php.net>                             |
  |         Pierre-A. Joye <pajoye@php.net>                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_xmlwriter.h"

zend_class_entry *xmlwriter_class_entry;

static void xmlwriter_free_resource_ptr(xmlwriter_object *intern TSRMLS_DC);
static void xmlwriter_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);

/* {{{ xmlwriter_object_free_storage */
static void xmlwriter_free_resource_ptr(xmlwriter_object *intern TSRMLS_DC) 
{
	if (intern) {
		if (intern->ptr) {
			xmlFreeTextWriter(intern->ptr);
			intern->ptr = NULL;
		}
		if (intern->output) {
			xmlBufferFree(intern->output);
			intern->output = NULL;
		}
		efree(intern);
	}
}
/* }}} */

#ifdef ZEND_ENGINE_2
/* {{{ XMLWRITER_FROM_OBJECT */
#define XMLWRITER_FROM_OBJECT(intern, object) \
	{ \
		ze_xmlwriter_object *obj = (ze_xmlwriter_object*) zend_object_store_get_object(object TSRMLS_CC); \
		intern = obj->xmlwriter_ptr; \
		if (!intern) { \
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid or unitialized XMLWriter object"); \
			RETURN_FALSE; \
		} \
	}
/* }}} */

static zend_object_handlers xmlwriter_object_handlers;

/* {{{ xmlwriter_object_free_storage */
static void xmlwriter_object_free_storage(void *object TSRMLS_DC)
{
	ze_xmlwriter_object * intern = (ze_xmlwriter_object *) object;
	if (!intern) {
		return;
	}
	if (intern->xmlwriter_ptr) {
		xmlwriter_free_resource_ptr(intern->xmlwriter_ptr TSRMLS_CC);
	}
	intern->xmlwriter_ptr = NULL;

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 1 && PHP_RELEASE_VERSION > 2) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 1) || (PHP_MAJOR_VERSION > 5)
	zend_object_std_dtor(&intern->zo TSRMLS_CC);
#else
	if (intern->zo.guards) {
		zend_hash_destroy(intern->zo.guards);
		FREE_HASHTABLE(intern->zo.guards);
	}

	if (intern->zo.properties) {
		zend_hash_destroy(intern->zo.properties);
		FREE_HASHTABLE(intern->zo.properties);
	}
#endif

	efree(intern);
}
/* }}} */


/* {{{ xmlwriter_object_new */
PHP_XMLWRITER_API zend_object_value xmlwriter_object_new(zend_class_entry *class_type TSRMLS_DC)
{
	ze_xmlwriter_object *intern;
	zval *tmp;
	zend_object_value retval;

	intern = emalloc(sizeof(ze_xmlwriter_object));
	memset(&intern->zo, 0, sizeof(zend_object));
	intern->xmlwriter_ptr = NULL;

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION == 1 && PHP_RELEASE_VERSION > 2) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 1) || (PHP_MAJOR_VERSION > 5)
	zend_object_std_init(&intern->zo, class_type TSRMLS_CC);
#else
	ALLOC_HASHTABLE(intern->zo.properties);
	zend_hash_init(intern->zo.properties, 0, NULL, ZVAL_PTR_DTOR, 0);

	intern->zo.ce = class_type;
	intern->zo.guards = NULL;
#endif

	zend_hash_copy(intern->zo.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref,
					(void *) &tmp, sizeof(zval *));

	retval.handle = zend_objects_store_put(intern,
						NULL,
						(zend_objects_free_object_storage_t) xmlwriter_object_free_storage,
						NULL TSRMLS_CC);
	
	retval.handlers = (zend_object_handlers *) & xmlwriter_object_handlers;
	
	return retval;
}
/* }}} */
#endif

#define XMLW_NAME_CHK(__err) \
	if (xmlValidateName((xmlChar *) name, 0) != 0) {	\
		php_error_docref(NULL TSRMLS_CC, E_WARNING, #__err);	\
		RETURN_FALSE;	\
	}	\

/* {{{ xmlwriter_functions */
static zend_function_entry xmlwriter_functions[] = {
	PHP_FE(xmlwriter_open_uri,			NULL)
	PHP_FE(xmlwriter_open_memory,		NULL)
#if LIBXML_VERSION >= 20605
	PHP_FE(xmlwriter_set_indent,		NULL)
	PHP_FE(xmlwriter_set_indent_string, NULL)
#endif
#if LIBXML_VERSION >= 20607
	PHP_FE(xmlwriter_start_comment,		NULL)
	PHP_FE(xmlwriter_end_comment,		NULL)
#endif
	PHP_FE(xmlwriter_start_attribute,	NULL)
	PHP_FE(xmlwriter_end_attribute,		NULL)
	PHP_FE(xmlwriter_write_attribute,	NULL)
#if LIBXML_VERSION > 20617
	PHP_FE(xmlwriter_start_attribute_ns,NULL)
	PHP_FE(xmlwriter_write_attribute_ns,NULL)
#endif
	PHP_FE(xmlwriter_start_element,		NULL)
	PHP_FE(xmlwriter_end_element,		NULL)
	PHP_FE(xmlwriter_start_element_ns,	NULL)
	PHP_FE(xmlwriter_write_element,		NULL)
	PHP_FE(xmlwriter_write_element_ns,	NULL)
	PHP_FE(xmlwriter_start_pi,			NULL)
	PHP_FE(xmlwriter_end_pi,			NULL)
	PHP_FE(xmlwriter_write_pi,			NULL)
	PHP_FE(xmlwriter_start_cdata,		NULL)
	PHP_FE(xmlwriter_end_cdata,			NULL)
	PHP_FE(xmlwriter_write_cdata,		NULL)
	PHP_FE(xmlwriter_text,				NULL)
	PHP_FE(xmlwriter_start_document,	NULL)
	PHP_FE(xmlwriter_end_document,		NULL)
	PHP_FE(xmlwriter_write_comment,		NULL)
	PHP_FE(xmlwriter_start_dtd,			NULL)
	PHP_FE(xmlwriter_end_dtd,			NULL)
	PHP_FE(xmlwriter_write_dtd,			NULL)
	PHP_FE(xmlwriter_start_dtd_element,	NULL)
	PHP_FE(xmlwriter_end_dtd_element,	NULL)
	PHP_FE(xmlwriter_write_dtd_element,	NULL)
#if LIBXML_VERSION > 20608
	PHP_FE(xmlwriter_start_dtd_attlist,	NULL)
	PHP_FE(xmlwriter_end_dtd_attlist,	NULL)
	PHP_FE(xmlwriter_write_dtd_attlist,	NULL)
#endif
	PHP_FE(xmlwriter_output_memory,		NULL)
	PHP_FE(xmlwriter_flush,				NULL)
	{NULL, NULL, NULL}
};
/* }}} */

#ifdef ZEND_ENGINE_2
/* {{{ xmlwriter_class_functions */
static zend_function_entry xmlwriter_class_functions[] = {
	PHP_ME_MAPPING(openUri,		xmlwriter_open_uri,		NULL)
	PHP_ME_MAPPING(openMemory,	xmlwriter_open_memory, 	NULL)
#if LIBXML_VERSION >= 20605
	PHP_ME_MAPPING(setIndent,	xmlwriter_set_indent,	NULL)
	PHP_ME_MAPPING(setIndentString,	xmlwriter_set_indent_string, NULL)
#endif
#if LIBXML_VERSION >= 20607
	PHP_ME_MAPPING(startComment,	xmlwriter_start_comment,	NULL)
	PHP_ME_MAPPING(endComment,		xmlwriter_end_comment,		NULL)
#endif
	PHP_ME_MAPPING(startAttribute,	xmlwriter_start_attribute,	NULL)
	PHP_ME_MAPPING(endAttribute,	xmlwriter_end_attribute,	NULL)
	PHP_ME_MAPPING(writeAttribute,	xmlwriter_write_attribute,	NULL)
#if LIBXML_VERSION > 20617
	PHP_ME_MAPPING(startAttributeNs,	xmlwriter_start_attribute_ns,NULL)
	PHP_ME_MAPPING(writeAttributeNs,	xmlwriter_write_attribute_ns,NULL)
#endif
	PHP_ME_MAPPING(startElement,	xmlwriter_start_element,	NULL)
	PHP_ME_MAPPING(endElement,		xmlwriter_end_element,		NULL)
	PHP_ME_MAPPING(startElementNs,	xmlwriter_start_element_ns,	NULL)
	PHP_ME_MAPPING(writeElement,	xmlwriter_write_element,	NULL)
	PHP_ME_MAPPING(writeElementNs,	xmlwriter_write_element_ns,	NULL)
	PHP_ME_MAPPING(startPi,			xmlwriter_start_pi,			NULL)
	PHP_ME_MAPPING(endPi,			xmlwriter_end_pi,			NULL)
	PHP_ME_MAPPING(writePi,			xmlwriter_write_pi,			NULL)
	PHP_ME_MAPPING(startCdata,		xmlwriter_start_cdata,		NULL)
	PHP_ME_MAPPING(endCdata,		xmlwriter_end_cdata,		NULL)
	PHP_ME_MAPPING(writeCdata,		xmlwriter_write_cdata,		NULL)
	PHP_ME_MAPPING(text,			xmlwriter_text,				NULL)
	PHP_ME_MAPPING(startDocument,	xmlwriter_start_document,	NULL)
	PHP_ME_MAPPING(endDocument,		xmlwriter_end_document,		NULL)
	PHP_ME_MAPPING(writeComment,	xmlwriter_write_comment,	NULL)
	PHP_ME_MAPPING(startDtd,		xmlwriter_start_dtd,		NULL)
	PHP_ME_MAPPING(endDtd,			xmlwriter_end_dtd,			NULL)
	PHP_ME_MAPPING(writeDtd,		xmlwriter_write_dtd,		NULL)
	PHP_ME_MAPPING(startDtdElement,	xmlwriter_start_dtd_element,	NULL)
	PHP_ME_MAPPING(endDtdElement,	xmlwriter_end_dtd_element,	NULL)
	PHP_ME_MAPPING(writeDtdElement,	xmlwriter_write_dtd_element,	NULL)
#if LIBXML_VERSION > 20608
	PHP_ME_MAPPING(startDtdAttlist,	xmlwriter_start_dtd_attlist,	NULL)
	PHP_ME_MAPPING(endDtdAttlist,	xmlwriter_end_dtd_attlist,	NULL)
	PHP_ME_MAPPING(writeDtdAttlist,	xmlwriter_write_dtd_attlist,	NULL)
#endif
	PHP_ME_MAPPING(outputMemory,	xmlwriter_output_memory,	NULL)
	PHP_ME_MAPPING(flush,			xmlwriter_flush,			NULL)
	{NULL, NULL, NULL}
};
/* }}} */
#endif

/* {{{ function prototypes */
PHP_MINIT_FUNCTION(xmlwriter);
PHP_MSHUTDOWN_FUNCTION(xmlwriter);
PHP_MINFO_FUNCTION(xmlwriter);

static int le_xmlwriter;
/* }}} */

/* _xmlwriter_get_valid_file_path should be made a 
	common function in libxml extension as code is common to a few xml extensions */
/* {{{ _xmlwriter_get_valid_file_path */
char *_xmlwriter_get_valid_file_path(char *source, char *resolved_path, int resolved_path_len  TSRMLS_DC) {
	xmlURI *uri;
	xmlChar *escsource;
	char *file_dest;
	int isFileUri = 0;

	uri = xmlCreateURI();
	escsource = xmlURIEscapeStr(source, ":");
	xmlParseURIReference(uri, escsource);
	xmlFree(escsource);

	if (uri->scheme != NULL) {
		/* absolute file uris - libxml only supports localhost or empty host */
		if (strncasecmp(source, "file:///",8) == 0) {
			isFileUri = 1;
#ifdef PHP_WIN32
			source += 8;
#else
			source += 7;
#endif
		} else if (strncasecmp(source, "file://localhost/",17) == 0) {
			isFileUri = 1;
#ifdef PHP_WIN32
			source += 17;
#else
			source += 16;
#endif
		}
	}

	file_dest = source;

	if ((uri->scheme == NULL || isFileUri)) {
		/* XXX possible buffer overflow if VCWD_REALPATH does not know size of resolved_path */
		if (! VCWD_REALPATH(source, resolved_path)) {
			expand_filepath(source, resolved_path TSRMLS_CC);
		}
		file_dest = resolved_path;
	}

	xmlFreeURI(uri);

	return file_dest;
}
/* }}} */

#ifndef ZEND_ENGINE_2
/* Channel libxml file io layer through the PHP streams subsystem.
 * This allows use of ftps:// and https:// urls */

/* {{{ php_xmlwriter_streams_IO_open_write_wrapper */
static void *php_xmlwriter_streams_IO_open_write_wrapper(const char *filename TSRMLS_DC)
{
	php_stream_wrapper *wrapper = NULL;
	void *ret_val = NULL;

	ret_val = php_stream_open_wrapper_ex((char *)filename, "wb", ENFORCE_SAFE_MODE|REPORT_ERRORS, NULL, NULL);
	return ret_val;
}
/* }}} */

/* {{{ php_xmlwriter_streams_IO_write */
int php_xmlwriter_streams_IO_write(void *context, const char *buffer, int len)
{
	TSRMLS_FETCH();
	return php_stream_write((php_stream*)context, buffer, len);
}
/* }}} */

/* {{{ xmlwriter_objects_clone */
int php_xmlwriter_streams_IO_close(void *context)
{
	TSRMLS_FETCH();
	return php_stream_close((php_stream*)context);
}
/* }}} */
#endif

/* {{{ xmlwriter_module_entry
 */
zend_module_entry xmlwriter_module_entry = {
	STANDARD_MODULE_HEADER,
	"xmlwriter",
	xmlwriter_functions,
	PHP_MINIT(xmlwriter),
	PHP_MSHUTDOWN(xmlwriter),
	NULL,
	NULL,
	PHP_MINFO(xmlwriter),
	"0.1",
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_XMLWRITER
ZEND_GET_MODULE(xmlwriter)
#endif

/* {{{ xmlwriter_objects_clone */
void xmlwriter_objects_clone(void *object, void **object_clone TSRMLS_DC)
{
	/* TODO */
}
/* }}} */

/* {{{ xmlwriter_dtor */
static void xmlwriter_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) {
	xmlwriter_object *intern;

	intern = (xmlwriter_object *) rsrc->ptr;
	xmlwriter_free_resource_ptr(intern TSRMLS_CC);
}
/* }}} */

#if LIBXML_VERSION >= 20605
/* {{{ proto bool xmlwriter_set_indent(resource xmlwriter, bool indent)
Toggle indentation on/off - returns FALSE on error */
PHP_FUNCTION(xmlwriter_set_indent)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	int retval;
	zend_bool indent;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &indent) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rb", &pind, &indent) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}


	ptr = intern->ptr;
	if (ptr) {
		retval = xmlTextWriterSetIndent(ptr, indent);
		if (retval == 0) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_set_indent_string(resource xmlwriter, string indentString)
Set string used for indenting - returns FALSE on error */
PHP_FUNCTION(xmlwriter_set_indent_string)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *indent;
	int indent_len, retval;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &indent, &indent_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &pind, &indent, &indent_len) == FAILURE) {
			return;
		}
	
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}


	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterSetIndentString(ptr, indent);
		if (retval == 0) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

#endif

/* {{{ proto bool xmlwriter_start_attribute(resource xmlwriter, string name)
Create start attribute - returns FALSE on error */
PHP_FUNCTION(xmlwriter_start_attribute)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name;
	int name_len, retval;
#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &pind, &name, &name_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}


	XMLW_NAME_CHK("Invalid Element Name");

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterStartAttribute(ptr, name);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}

	RETURN_FALSE;
}
/* }}} */

static void php_xmlwriter_end(INTERNAL_FUNCTION_PARAMETERS, int type)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	int retval;
#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		XMLWRITER_FROM_OBJECT(intern, this);
	} else 
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &pind) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	ptr = intern->ptr;

	if (ptr) {
		switch (type) {
			case 0:
				retval = xmlTextWriterEndAttribute(ptr);
				break;
			case 1:
				retval = xmlTextWriterEndElement(ptr);
				break;
			case 2:
				retval = xmlTextWriterEndPI(ptr);
				break;
			case 3:
				retval = xmlTextWriterEndCDATA(ptr);
				break;
			case 4:
				retval = xmlTextWriterEndComment(ptr);
				break;
			case 5:
				retval = xmlTextWriterEndDocument(ptr);
				break;
			case 6:
				retval = xmlTextWriterEndDTD(ptr);
				break;
			case 7:
				retval = xmlTextWriterEndDTDElement(ptr);
				break;
			case 8:
				retval = xmlTextWriterEndDTDAttlist(ptr);
				break;
			case 9:
				retval = xmlTextWriterEndDTDEntity(ptr);
				break;
			default:
				retval = -1;
				break;
		}
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}


/* {{{ proto bool xmlwriter_end_attribute(resource xmlwriter)
End attribute - returns FALSE on error */
PHP_FUNCTION(xmlwriter_end_attribute)
{
	php_xmlwriter_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

#if LIBXML_VERSION > 20617
/* {{{ proto bool xmlwriter_start_attribute_ns(resource xmlwriter, string prefix, string name, string uri)
Create start namespaced attribute - returns FALSE on error */
PHP_FUNCTION(xmlwriter_start_attribute_ns)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name, *prefix, *uri;
	int name_len, prefix_len, uri_len, retval;
#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", 
			&prefix, &prefix_len, &name, &name_len, &uri, &uri_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsss", &pind, 
			&prefix, &prefix_len, &name, &name_len, &uri, &uri_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	XMLW_NAME_CHK("Invalid Attribute Name");

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterStartAttributeNS(ptr, prefix, name, uri);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */
#endif

/* {{{ proto bool xmlwriter_write_attribute(resource xmlwriter, string name, string content)
Write full attribute - returns FALSE on error */
PHP_FUNCTION(xmlwriter_write_attribute)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name, *content;
	int name_len, content_len, retval;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", 
			&name, &name_len, &content, &content_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &pind, 
			&name, &name_len, &content, &content_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	XMLW_NAME_CHK("Invalid Attribute Name");

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterWriteAttribute(ptr, name, content);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

#if LIBXML_VERSION > 20617
/* {{{ proto bool xmlwriter_write_attribute_ns(resource xmlwriter, string prefix, string name, string uri, string content)
Write full namespaced attribute - returns FALSE on error */
PHP_FUNCTION(xmlwriter_write_attribute_ns)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name, *prefix, *uri, *content;
	int name_len, prefix_len, uri_len, content_len, retval;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssss", 
			&prefix, &prefix_len, &name, &name_len, &uri, &uri_len, &content, &content_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rssss", &pind, 
			&prefix, &prefix_len, &name, &name_len, &uri, &uri_len, &content, &content_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	XMLW_NAME_CHK("Invalid Attribute Name");

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterWriteAttributeNS(ptr, prefix, name, uri, content);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */
#endif

/* {{{ proto bool xmlwriter_start_element(resource xmlwriter, string name)
Create start element tag - returns FALSE on error */
PHP_FUNCTION(xmlwriter_start_element)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name;
	int name_len, retval;
#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &pind, &name, &name_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	XMLW_NAME_CHK("Invalid Element Name");

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterStartElement(ptr, name);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */


/* {{{ proto bool xmlwriter_start_element_ns(resource xmlwriter, string prefix, string name, string uri)
Create start namespaced element tag - returns FALSE on error */
PHP_FUNCTION(xmlwriter_start_element_ns)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name, *prefix, *uri;
	int name_len, prefix_len, uri_len, retval;
#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s!ss",
			&prefix, &prefix_len, &name, &name_len, &uri, &uri_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs!ss", &pind, 
			&prefix, &prefix_len, &name, &name_len, &uri, &uri_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	XMLW_NAME_CHK("Invalid Element Name");

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterStartElementNS(ptr, prefix, name, uri);
		if (retval != -1) {
			RETURN_TRUE;
		}
		
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_end_element(resource xmlwriter)
End current element - returns FALSE on error */
PHP_FUNCTION(xmlwriter_end_element)
{
	php_xmlwriter_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto bool xmlwriter_write_element(resource xmlwriter, string name, string content)
Write full element tag - returns FALSE on error */
PHP_FUNCTION(xmlwriter_write_element)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name, *content;
	int name_len, content_len, retval;
#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
			&name, &name_len, &content, &content_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &pind, 
			&name, &name_len, &content, &content_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	XMLW_NAME_CHK("Invalid Element Name");

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterWriteElement(ptr, name, content);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_write_element_ns(resource xmlwriter, string prefix, string name, string uri, string content)
Write full namesapced element tag - returns FALSE on error */
PHP_FUNCTION(xmlwriter_write_element_ns)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name, *prefix, *uri, *content;
	int name_len, prefix_len, uri_len, content_len, retval;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s!sss", 
			&prefix, &prefix_len, &name, &name_len, &uri, &uri_len, &content, &content_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs!sss", &pind, 
			&prefix, &prefix_len, &name, &name_len, &uri, &uri_len, &content, &content_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	XMLW_NAME_CHK("Invalid Element Name");

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterWriteElementNS(ptr, prefix, name, uri, content);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_start_pi(resource xmlwriter, string target)
Create start PI tag - returns FALSE on error */
PHP_FUNCTION(xmlwriter_start_pi)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name;
	int name_len, retval;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &pind, &name, &name_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	XMLW_NAME_CHK("Invalid PI Target");

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterStartPI(ptr, name);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_end_pi(resource xmlwriter)
End current PI - returns FALSE on error */
PHP_FUNCTION(xmlwriter_end_pi)
{
	php_xmlwriter_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, 2);
}
/* }}} */

/* {{{ proto bool xmlwriter_write_pi(resource xmlwriter, string target, string content)
Write full PI tag - returns FALSE on error */
PHP_FUNCTION(xmlwriter_write_pi)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name, *content;
	int name_len, content_len, retval;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
			&name, &name_len, &content, &content_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &pind, 
			&name, &name_len, &content, &content_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	XMLW_NAME_CHK("Invalid PI Target");

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterWritePI(ptr, name, content);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_start_cdata(resource xmlwriter)
Create start CDATA tag - returns FALSE on error */
PHP_FUNCTION(xmlwriter_start_cdata)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	int retval;
#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &pind) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterStartCDATA(ptr);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_end_cdata(resource xmlwriter)
End current CDATA - returns FALSE on error */
PHP_FUNCTION(xmlwriter_end_cdata)
{
	php_xmlwriter_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, 3);
}
/* }}} */

/* {{{ proto bool xmlwriter_write_cdata(resource xmlwriter, string content)
Write full CDATA tag - returns FALSE on error */
PHP_FUNCTION(xmlwriter_write_cdata)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *content;
	int content_len, retval;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &content, &content_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &pind, 
			&content, &content_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterWriteCDATA(ptr, content);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_text(resource xmlwriter, string content)
Write text - returns FALSE on error */
PHP_FUNCTION(xmlwriter_text)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *content;
	int content_len, retval;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &content, &content_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &pind, &content, &content_len) == FAILURE) {
			return;
		}
	
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterWriteString(ptr, content);
		if (retval) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

#if LIBXML_VERSION >= 20607
/* {{{ proto bool xmlwriter_start_comment(resource xmlwriter)
Create start comment - returns FALSE on error */
PHP_FUNCTION(xmlwriter_start_comment)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	int retval;
#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	
	if (this) {
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &pind) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterStartComment(ptr);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_end_comment(resource xmlwriter)
Create end comment - returns FALSE on error */
PHP_FUNCTION(xmlwriter_end_comment)
{
	php_xmlwriter_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, 4);
}
/* }}} */
#endif  /* LIBXML_VERSION >= 20607 */


/* {{{ proto bool xmlwriter_write_comment(resource xmlwriter, string content)
Write full comment tag - returns FALSE on error */
PHP_FUNCTION(xmlwriter_write_comment)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *content;
	int content_len, retval;

	
#ifdef ZEND_ENGINE_2
	zval *this = getThis();

	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
			&content, &content_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &pind, 
			&content, &content_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterWriteComment(ptr, content);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_start_document(resource xmlwriter, string version, string encoding, string standalone)
Create document tag - returns FALSE on error */
PHP_FUNCTION(xmlwriter_start_document)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *version = NULL, *enc = NULL, *alone = NULL;
	int version_len, enc_len, alone_len, retval;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();

	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s!s!s!", &version, &version_len, &enc, &enc_len, &alone, &alone_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|s!s!s!", &pind, &version, &version_len, &enc, &enc_len, &alone, &alone_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterStartDocument(ptr, version, enc, alone);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_end_document(resource xmlwriter)
End current document - returns FALSE on error */
PHP_FUNCTION(xmlwriter_end_document)
{
	php_xmlwriter_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, 5);
}
/* }}} */

/* {{{ proto bool xmlwriter_start_dtd(resource xmlwriter, string name, string pubid, string sysid)
Create start DTD tag - returns FALSE on error */
PHP_FUNCTION(xmlwriter_start_dtd)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name, *pubid = NULL, *sysid = NULL;
	int name_len, pubid_len, sysid_len, retval;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();

	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s!s!", &name, &name_len, &pubid, &pubid_len, &sysid, &sysid_len) == FAILURE) {
			return;
		}

		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|s!s!", &pind, &name, &name_len, &pubid, &pubid_len, &sysid, &sysid_len) == FAILURE) {
			return;
		}
	
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}
	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterStartDTD(ptr, name, pubid, sysid);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_end_dtd(resource xmlwriter)
End current DTD - returns FALSE on error */
PHP_FUNCTION(xmlwriter_end_dtd)
{
	php_xmlwriter_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, 6);
}
/* }}} */

/* {{{ proto bool xmlwriter_write_dtd(resource xmlwriter, string name, string pubid, string sysid, string subset)
Write full DTD tag - returns FALSE on error */
PHP_FUNCTION(xmlwriter_write_dtd)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name, *pubid = NULL, *sysid = NULL, *subset = NULL;
	int name_len, pubid_len, sysid_len, subset_len, retval;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();

	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s!s!s!", &name, &name_len, &pubid, &pubid_len, &sysid, &sysid_len, &subset, &subset_len) == FAILURE) {
			return;
		}

		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|s!s!s!", &pind, &name, &name_len, &pubid, &pubid_len, &sysid, &sysid_len, &subset, &subset_len) == FAILURE) {
			return;
		}
	
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterWriteDTD(ptr, name, pubid, sysid, subset);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_start_dtd_element(resource xmlwriter, string name)
Create start DTD element - returns FALSE on error */
PHP_FUNCTION(xmlwriter_start_dtd_element)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name;
	int name_len, retval;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();

	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &pind, &name, &name_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	XMLW_NAME_CHK("Invalid Attribute Name");

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterStartDTDElement(ptr, name);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_end_dtd_element(resource xmlwriter)
End current DTD element - returns FALSE on error */
PHP_FUNCTION(xmlwriter_end_dtd_element)
{
	php_xmlwriter_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, 7);
}
/* }}} */

/* {{{ proto bool xmlwriter_write_dtd_element(resource xmlwriter, string name, string content)
Write full DTD element tag - returns FALSE on error */
PHP_FUNCTION(xmlwriter_write_dtd_element)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name, *content;
	int name_len, content_len, retval;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();

	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &name, &name_len, &content, &content_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &pind, 
			&name, &name_len, &content, &content_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	XMLW_NAME_CHK("Invalid Element Name");

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterWriteDTDElement(ptr, name, content);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

#if LIBXML_VERSION > 20608
/* {{{ proto bool xmlwriter_start_dtd_attlist(resource xmlwriter, string name)
Create start DTD AttList - returns FALSE on error */
PHP_FUNCTION(xmlwriter_start_dtd_attlist)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name;
	int name_len, retval;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();

	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &pind, &name, &name_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	XMLW_NAME_CHK("Invalid Attribute Name");

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterStartDTDAttlist(ptr, name);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_end_dtd_attlist(resource xmlwriter)
End current DTD AttList - returns FALSE on error */
PHP_FUNCTION(xmlwriter_end_dtd_attlist)
{
	php_xmlwriter_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, 8);
}
/* }}} */

/* {{{ proto bool xmlwriter_write_dtd_attlist(resource xmlwriter, string name, string content)
Write full DTD AttList tag - returns FALSE on error */
PHP_FUNCTION(xmlwriter_write_dtd_attlist)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name, *content;
	int name_len, content_len, retval;

	
#ifdef ZEND_ENGINE_2
	zval *this = getThis();

	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
			&name, &name_len, &content, &content_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &pind, 
			&name, &name_len, &content, &content_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	XMLW_NAME_CHK("Invalid Element Name");

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterWriteDTDAttlist(ptr, name, content);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_start_dtd_entity(resource xmlwriter, string name, bool isparam)
Create start DTD Entity - returns FALSE on error */
PHP_FUNCTION(xmlwriter_start_dtd_entity)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name;
	int name_len, retval;
	zend_bool isparm;

	
#ifdef ZEND_ENGINE_2
	zval *this = getThis();

	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sb", &name, &name_len, &isparm) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsb", &pind, &name, &name_len, &isparm) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	XMLW_NAME_CHK("Invalid Attribute Name");

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterStartDTDEntity(ptr, isparm, name);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool xmlwriter_end_dtd_entity(resource xmlwriter)
End current DTD Entity - returns FALSE on error */
PHP_FUNCTION(xmlwriter_end_dtd_entity)
{
	php_xmlwriter_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, 9);
}
/* }}} */

/* {{{ proto bool xmlwriter_write_dtd_entity(resource xmlwriter, string name, string content)
Write full DTD Entity tag - returns FALSE on error */
PHP_FUNCTION(xmlwriter_write_dtd_entity)
{
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *name, *content;
	int name_len, content_len, retval;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();

	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",
			&name, &name_len, &content, &content_len) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rss", &pind, 
			&name, &name_len, &content, &content_len) == FAILURE) {
			return;
		}
		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}

	XMLW_NAME_CHK("Invalid Element Name");

	ptr = intern->ptr;

	if (ptr) {
		retval = xmlTextWriterWriteDTDAttlist(ptr, name, content);
		if (retval != -1) {
			RETURN_TRUE;
		}
	}
	
	RETURN_FALSE;
}
/* }}} */
#endif

/* {{{ proto resource xmlwriter_open_uri(resource xmlwriter, string source)
Create new xmlwriter using source uri for output */
PHP_FUNCTION(xmlwriter_open_uri)
{
	char *valid_file = NULL;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	char *source;
	char resolved_path[MAXPATHLEN + 1];
	int source_len;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	ze_xmlwriter_object *ze_obj = NULL;
#endif

#ifndef ZEND_ENGINE_2
	xmlOutputBufferPtr out_buffer;
	void *ioctx;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &source, &source_len) == FAILURE) {
		WRONG_PARAM_COUNT;
		return;
	}
	
#ifdef ZEND_ENGINE_2
	if (this) {
		/* We do not use XMLWRITER_FROM_OBJECT, xmlwriter init function here */
		ze_obj = (ze_xmlwriter_object*) zend_object_store_get_object(this TSRMLS_CC); 
	}
#endif

	if (source_len == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty string as source");
		RETURN_FALSE;
	}

	valid_file = _xmlwriter_get_valid_file_path(source, resolved_path, MAXPATHLEN TSRMLS_CC);
	if (!valid_file) {
		RETURN_FALSE;
	}

#ifndef ZEND_ENGINE_2
	ioctx = php_xmlwriter_streams_IO_open_write_wrapper(valid_file TSRMLS_CC);
	if (ioctx == NULL) {
		RETURN_FALSE;
	}

	out_buffer = xmlOutputBufferCreateIO(php_xmlwriter_streams_IO_write, 
		php_xmlwriter_streams_IO_close, ioctx, NULL);

	if (out_buffer == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to create output buffer");
		RETURN_FALSE;
	}
	ptr = xmlNewTextWriter(out_buffer);
#else
	ptr = xmlNewTextWriterFilename(valid_file, 0);
#endif

	if (!ptr) {
		RETURN_FALSE;
	}

	intern = emalloc(sizeof(xmlwriter_object));
	intern->ptr = ptr;
	intern->output = NULL;
#ifndef ZEND_ENGINE_2
	intern->uri_output = out_buffer;
#else
	if (this) {
		ze_obj->xmlwriter_ptr = intern;
		RETURN_TRUE;
	} else
#endif
	{
		ZEND_REGISTER_RESOURCE(return_value,intern,le_xmlwriter);
	}
}
/* }}} */

/* {{{ proto resource xmlwriter_open_memory()
Create new xmlwriter using memory for string output */
PHP_FUNCTION(xmlwriter_open_memory)
{
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	xmlBufferPtr buffer;

#ifdef ZEND_ENGINE_2
	zval *this = getThis();
	ze_xmlwriter_object *ze_obj = NULL;
#endif

#ifdef ZEND_ENGINE_2
	if (this) {
		/* We do not use XMLWRITER_FROM_OBJECT, xmlwriter init function here */
		ze_obj = (ze_xmlwriter_object*) zend_object_store_get_object(this TSRMLS_CC); 
	}
#endif

	buffer = xmlBufferCreate();

	if (buffer == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to create output buffer");
		RETURN_FALSE;
	}

	ptr = xmlNewTextWriterMemory(buffer, 0);
	if (! ptr) {
		xmlBufferFree(buffer);
		RETURN_FALSE;
	}

	intern = emalloc(sizeof(xmlwriter_object));
	intern->ptr = ptr;
	intern->output = buffer;
#ifndef ZEND_ENGINE_2
	intern->uri_output = NULL;
#else
	if (this) {
		ze_obj->xmlwriter_ptr = intern;
		RETURN_TRUE;
	} else
#endif
	{
		ZEND_REGISTER_RESOURCE(return_value,intern,le_xmlwriter);
	}

}
/* }}} */

/* {{{ php_xmlwriter_flush */
static void php_xmlwriter_flush(INTERNAL_FUNCTION_PARAMETERS, int force_string) {
	zval *pind;
	xmlwriter_object *intern;
	xmlTextWriterPtr ptr;
	xmlBufferPtr buffer;
	zend_bool empty = 1;
	int output_bytes;


#ifdef ZEND_ENGINE_2
	zval *this = getThis();

	if (this) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &empty) == FAILURE) {
			return;
		}
		XMLWRITER_FROM_OBJECT(intern, this);
	} else
#endif
	{
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|b", &pind, &empty) == FAILURE) {
			return;
		}

		ZEND_FETCH_RESOURCE(intern,xmlwriter_object *, &pind, -1, "XMLWriter", le_xmlwriter);
	}
	ptr = intern->ptr;

	if (ptr) {
		buffer = intern->output;
		if (force_string == 1 && buffer == NULL) {
			RETURN_EMPTY_STRING();
		}
		output_bytes = xmlTextWriterFlush(ptr);
		if (buffer) {
			RETVAL_STRING(buffer->content, 1);
			if (empty) {
				xmlBufferEmpty(buffer);
			}
		} else {
			RETVAL_LONG(output_bytes);
		}
		return;
	}
	
	RETURN_EMPTY_STRING();
}
/* }}} */

/* {{{ proto string xmlwriter_output_memory(resource xmlwriter [,bool flush])
Output current buffer as string */
PHP_FUNCTION(xmlwriter_output_memory)
{
	php_xmlwriter_flush(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto mixed xmlwriter_flush(resource xmlwriter [,bool empty])
Output current buffer */
PHP_FUNCTION(xmlwriter_flush)
{
	php_xmlwriter_flush(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(xmlwriter)
{
#ifdef ZEND_ENGINE_2
	zend_class_entry ce;
#endif

	le_xmlwriter = zend_register_list_destructors_ex(xmlwriter_dtor, NULL, "xmlwriter", module_number);

#ifdef ZEND_ENGINE_2
	memcpy(&xmlwriter_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	xmlwriter_object_handlers.clone_obj      = NULL;
	INIT_CLASS_ENTRY(ce, "XMLWriter", xmlwriter_class_functions);
	ce.create_object = xmlwriter_object_new;
	xmlwriter_class_entry_ce = zend_register_internal_class(&ce TSRMLS_CC);
#endif
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(xmlwriter)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(xmlwriter)
{
	php_info_print_table_start();
	{
		php_info_print_table_row(2, "XMLWriter", "enabled");
	}
	php_info_print_table_end();
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
