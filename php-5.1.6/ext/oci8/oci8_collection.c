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
   | Authors: Stig S�ther Bakken <ssb@php.net>                            |
   |          Thies C. Arntzen <thies@thieso.net>                         |
   |                                                                      |
   | Collection support by Andy Sautins <asautins@veripost.net>           |
   | Temporary LOB support by David Benson <dbenson@mancala.com>          |
   | ZTS per process OCIPLogon by Harald Radi <harald.radi@nme.at>        |
   |                                                                      |
   | Redesigned by: Antony Dovgal <antony@zend.com>                       |
   |                Andi Gutmans <andi@zend.com>                          |
   |                Wez Furlong <wez@omniti.com>                          |
   +----------------------------------------------------------------------+
*/

/* $Id: oci8_collection.c,v 1.5.2.3 2006/01/01 12:50:10 sniper Exp $ */



#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_ini.h"

#if HAVE_OCI8

#include "php_oci8.h"
#include "php_oci8_int.h"

/* {{{ php_oci_collection_create() 
 Create and return connection handle */
php_oci_collection * php_oci_collection_create(php_oci_connection* connection, char *tdo, int tdo_len, char *schema, int schema_len TSRMLS_DC)
{	
	dvoid *dschp1;
	dvoid *parmp1;
	dvoid *parmp2;
	php_oci_collection *collection;
	
	collection = emalloc(sizeof(php_oci_collection));

	collection->connection = connection;
	collection->collection = NULL;
	
	/* get type handle by name */
	connection->errcode = PHP_OCI_CALL(OCITypeByName,	(connection->env, connection->err, connection->svc, (text *) schema, (ub4) schema_len, (text *) tdo, (ub4) tdo_len, (CONST text *) 0, (ub4) 0, OCI_DURATION_SESSION, OCI_TYPEGET_ALL, &(collection->tdo)));

	if (connection->errcode) {
		goto CLEANUP;
	}

	/* allocate describe handle */
	connection->errcode = PHP_OCI_CALL(OCIHandleAlloc, (connection->env, (dvoid **) &dschp1, (ub4) OCI_HTYPE_DESCRIBE, (size_t) 0, (dvoid **) 0));

	if (connection->errcode) {
		goto CLEANUP;
	}

	/* describe TDO */
	connection->errcode = PHP_OCI_CALL(OCIDescribeAny, (connection->svc, connection->err, (dvoid *) collection->tdo, (ub4) 0, OCI_OTYPE_PTR, (ub1) OCI_DEFAULT, (ub1) OCI_PTYPE_TYPE, dschp1));

	if (connection->errcode) {
		goto CLEANUP;
	}

	/* get first parameter handle */
	connection->errcode = PHP_OCI_CALL(OCIAttrGet, ((dvoid *) dschp1, (ub4) OCI_HTYPE_DESCRIBE, (dvoid *)&parmp1, (ub4 *)0, (ub4)OCI_ATTR_PARAM,	connection->err));

	if (connection->errcode) {
		goto CLEANUP;
	}

	/* get the collection type code of the attribute */
	connection->errcode = PHP_OCI_CALL(OCIAttrGet, ((dvoid*) parmp1, (ub4) OCI_DTYPE_PARAM, (dvoid*) &(collection->coll_typecode), (ub4 *) 0, (ub4) OCI_ATTR_COLLECTION_TYPECODE, connection->err));

	if (connection->errcode) {
		goto CLEANUP;
	}

	switch(collection->coll_typecode) {
		case OCI_TYPECODE_TABLE:
		case OCI_TYPECODE_VARRAY:
			/* get collection element handle */
			connection->errcode = PHP_OCI_CALL(OCIAttrGet, ((dvoid*) parmp1, (ub4) OCI_DTYPE_PARAM, (dvoid*) &parmp2, (ub4 *) 0, (ub4) OCI_ATTR_COLLECTION_ELEMENT, connection->err));

			if (connection->errcode) {
				goto CLEANUP;
			}

			/* get REF of the TDO for the type */
			connection->errcode = PHP_OCI_CALL(OCIAttrGet, ((dvoid*) parmp2, (ub4) OCI_DTYPE_PARAM, (dvoid*) &(collection->elem_ref), (ub4 *) 0, (ub4) OCI_ATTR_REF_TDO, connection->err));

			if (connection->errcode) {
				goto CLEANUP;
			}

			/* get the TDO (only header) */
			connection->errcode = PHP_OCI_CALL(OCITypeByRef, (connection->env, connection->err, collection->elem_ref, OCI_DURATION_SESSION, OCI_TYPEGET_HEADER, &(collection->element_type)));

			if (connection->errcode) {
				goto CLEANUP;
			}

			/* get typecode */
			connection->errcode = PHP_OCI_CALL(OCIAttrGet, ((dvoid*) parmp2, (ub4) OCI_DTYPE_PARAM, (dvoid*) &(collection->element_typecode), (ub4 *) 0, (ub4) OCI_ATTR_TYPECODE, connection->err));

			if (connection->errcode) {
				goto CLEANUP;
			}
			break;
			/* we only support VARRAYs and TABLEs */
		default:
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "OCINewCollection - Unknown Type %d", collection->coll_typecode);
			break;
	}	

	/* Create object to hold return table */
	connection->errcode = PHP_OCI_CALL(OCIObjectNew, 
		(
			connection->env, 
			connection->err, 
			connection->svc, 
			OCI_TYPECODE_TABLE, 
			collection->tdo, 
			(dvoid *)0, 
			OCI_DURATION_DEFAULT, 
			TRUE, 
			(dvoid **) &(collection->collection)
		)
	);

	if (connection->errcode) {
		goto CLEANUP;
	}

	PHP_OCI_REGISTER_RESOURCE(collection, le_collection);
	return collection;
	
CLEANUP:

	php_oci_error(connection->err, connection->errcode TSRMLS_CC);
	php_oci_collection_close(collection TSRMLS_CC);	
	return NULL;
} /* }}} */

/* {{{ php_oci_collection_size() 
 Return size of the collection */
int php_oci_collection_size(php_oci_collection *collection, sb4 *size TSRMLS_DC)
{
	php_oci_connection *connection = collection->connection;
	
	connection->errcode = PHP_OCI_CALL(OCICollSize, (connection->env, connection->err, collection->collection, (sb4 *)size));

	if (connection->errcode != OCI_SUCCESS) {
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}
	return 0;
} /* }}} */

/* {{{ php_oci_collection_max() 
 Return max number of elements in the collection */
int php_oci_collection_max(php_oci_collection *collection, long *max TSRMLS_DC)
{
	php_oci_connection *connection = collection->connection;
	
	*max = PHP_OCI_CALL(OCICollMax, (connection->env, collection->collection));

	/* error handling is not necessary here? */
	return 0;
} /* }}} */

/* {{{ php_oci_collection_trim() 
 Trim collection to the given number of elements */
int php_oci_collection_trim(php_oci_collection *collection, long trim_size TSRMLS_DC)
{
	php_oci_connection *connection = collection->connection;
	
	connection->errcode = PHP_OCI_CALL(OCICollTrim, (connection->env, connection->err, trim_size, collection->collection));

	if (connection->errcode != OCI_SUCCESS) {
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}
	return 0;
} /* }}} */

/* {{{ php_oci_collection_append_null() 
 Append NULL element to the end of the collection */
int php_oci_collection_append_null(php_oci_collection *collection TSRMLS_DC)
{
	OCIInd null_index = OCI_IND_NULL;
	php_oci_connection *connection = collection->connection;

	/* append NULL element */
	connection->errcode = PHP_OCI_CALL(OCICollAppend, (connection->env, connection->err, (dvoid *)0, &null_index, collection->collection));
	
	if (connection->errcode != OCI_SUCCESS) {
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}
	return 0;
} /* }}} */

/* {{{ php_oci_collection_append_date() 
 Append DATE element to the end of the collection (use "DD-MON-YY" format) */
int php_oci_collection_append_date(php_oci_collection *collection, char *date, int date_len TSRMLS_DC)
{
	OCIInd new_index = OCI_IND_NOTNULL;
	OCIDate oci_date;
	php_oci_connection *connection = collection->connection;

	/* format and language are NULLs, so format is "DD-MON-YY" and language is the default language of the session */
	connection->errcode = PHP_OCI_CALL(OCIDateFromText, (connection->err, date, date_len, NULL, 0, NULL, 0, &oci_date));

	if (connection->errcode != OCI_SUCCESS) {
		/* failed to convert string to date */
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}

	connection->errcode = PHP_OCI_CALL(OCICollAppend, (connection->env, connection->err, (dvoid *) &oci_date, (dvoid *) &new_index, (OCIColl *) collection->collection));

	if (connection->errcode != OCI_SUCCESS) {
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}
			
	return 0;
} /* }}} */

/* {{{ php_oci_collection_append_number()
 Append NUMBER to the end of the collection */
int php_oci_collection_append_number(php_oci_collection *collection, char *number, int number_len TSRMLS_DC)
{
	OCIInd new_index = OCI_IND_NOTNULL;
	double element_double;
	OCINumber oci_number;
	php_oci_connection *connection = collection->connection;

	element_double = zend_strtod(number, NULL);
			
	connection->errcode = PHP_OCI_CALL(OCINumberFromReal, (connection->err, &element_double, sizeof(double), &oci_number));

	if (connection->errcode != OCI_SUCCESS) {
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}

	connection->errcode = PHP_OCI_CALL(OCICollAppend, (connection->env, connection->err, (dvoid *) &oci_number, (dvoid *) &new_index, (OCIColl *) collection->collection));

	if (connection->errcode != OCI_SUCCESS) {
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}

	return 0;
} /* }}} */

/* {{{ php_oci_collection_append_string() 
 Append STRING to the end of the collection */
int php_oci_collection_append_string(php_oci_collection *collection, char *element, int element_len TSRMLS_DC)
{
	OCIInd new_index = OCI_IND_NOTNULL;
	OCIString *ocistr = (OCIString *)0;
	php_oci_connection *connection = collection->connection;
			
	connection->errcode = PHP_OCI_CALL(OCIStringAssignText, (connection->env, connection->err, element, element_len, &ocistr));

	if (connection->errcode != OCI_SUCCESS) {
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}

	connection->errcode = PHP_OCI_CALL(OCICollAppend, (connection->env, connection->err, (dvoid *) ocistr, (dvoid *) &new_index,	(OCIColl *) collection->collection));

	if (connection->errcode != OCI_SUCCESS) {
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}

	return 0;
} /* }}} */

/* {{{ php_oci_collection_append() 
 Append wrapper. Appends any supported element to the end of the collection */
int php_oci_collection_append(php_oci_collection *collection, char *element, int element_len TSRMLS_DC)
{
	if (element_len == 0) {
		return php_oci_collection_append_null(collection TSRMLS_CC);
	}
	
	switch(collection->element_typecode) {
		case OCI_TYPECODE_DATE:
			return php_oci_collection_append_date(collection, element, element_len TSRMLS_CC);
			break;
			
		case OCI_TYPECODE_VARCHAR2 :
			return php_oci_collection_append_string(collection, element, element_len TSRMLS_CC);
			break;

		case OCI_TYPECODE_UNSIGNED16 :                       /* UNSIGNED SHORT  */
		case OCI_TYPECODE_UNSIGNED32 :                        /* UNSIGNED LONG  */
		case OCI_TYPECODE_REAL :                                     /* REAL    */
		case OCI_TYPECODE_DOUBLE :                                   /* DOUBLE  */
		case OCI_TYPECODE_INTEGER :                                     /* INT  */
		case OCI_TYPECODE_SIGNED16 :                                  /* SHORT  */
		case OCI_TYPECODE_SIGNED32 :                                   /* LONG  */
		case OCI_TYPECODE_DECIMAL :                                 /* DECIMAL  */
		case OCI_TYPECODE_FLOAT :                                   /* FLOAT    */
		case OCI_TYPECODE_NUMBER :                                  /* NUMBER   */
		case OCI_TYPECODE_SMALLINT :                                /* SMALLINT */
			return php_oci_collection_append_number(collection, element, element_len TSRMLS_CC);
			break;

		default:
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Unknown or unsupported type of element: %d", collection->element_typecode);
			return 1;
			break;
	}
	/* never reached */
	return 1;
} /* }}} */

/* {{{ php_oci_collection_element_get() 
 Get the element with the given index */
int php_oci_collection_element_get(php_oci_collection *collection, long index, zval **result_element TSRMLS_DC)
{
	php_oci_connection *connection = collection->connection;
	dvoid *element;
	OCIInd *element_index;
	boolean exists;
	char buff[1024];
	int buff_len = 1024;
	
	MAKE_STD_ZVAL(*result_element);
	ZVAL_NULL(*result_element);

	connection->errcode = PHP_OCI_CALL(OCICollGetElem, (connection->env, connection->err, collection->collection, (ub4)index, &exists, &element,	(dvoid **)&element_index));

	if (connection->errcode != OCI_SUCCESS) {
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		FREE_ZVAL(*result_element);
		return 1;
	}
	
	if (exists == 0) {
		/* element doesn't exist */
		FREE_ZVAL(*result_element);
		return 1;
	}

	if (*element_index == OCI_IND_NULL) {
		/* this is not an error, we're returning NULL here */
		return 0;
	}

	switch (collection->element_typecode) {
		case OCI_TYPECODE_DATE:
			connection->errcode = PHP_OCI_CALL(OCIDateToText, (connection->err, element, 0, 0, 0, 0, &buff_len, buff));
	
			if (connection->errcode != OCI_SUCCESS) {
				php_oci_error(connection->err, connection->errcode TSRMLS_CC);
				FREE_ZVAL(*result_element);
				return 1;
			}

			ZVAL_STRINGL(*result_element, buff, buff_len, 1);
			Z_STRVAL_P(*result_element)[buff_len] = '\0';
			
			return 0;
			break;

		case OCI_TYPECODE_VARCHAR2:
		{
			OCIString *oci_string = *(OCIString **)element;
			text *str;
			
			str = (text *)PHP_OCI_CALL(OCIStringPtr, (connection->env, oci_string));
			
			if (str) {
				ZVAL_STRING(*result_element, str, 1);
			}
			return 0;
		}
			break;

		case OCI_TYPECODE_UNSIGNED16:                       /* UNSIGNED SHORT  */
		case OCI_TYPECODE_UNSIGNED32:                       /* UNSIGNED LONG  */
		case OCI_TYPECODE_REAL:                             /* REAL    */
		case OCI_TYPECODE_DOUBLE:                           /* DOUBLE  */
		case OCI_TYPECODE_INTEGER:                          /* INT  */
		case OCI_TYPECODE_SIGNED16:                         /* SHORT  */
		case OCI_TYPECODE_SIGNED32:                         /* LONG  */
		case OCI_TYPECODE_DECIMAL:                          /* DECIMAL  */
		case OCI_TYPECODE_FLOAT:                            /* FLOAT    */
		case OCI_TYPECODE_NUMBER:                           /* NUMBER   */
		case OCI_TYPECODE_SMALLINT:                         /* SMALLINT */
		{
			double double_number;
			
			connection->errcode = PHP_OCI_CALL(OCINumberToReal, (connection->err, (CONST OCINumber *) element, (uword) sizeof(double), (dvoid *) &double_number));

			if (connection->errcode != OCI_SUCCESS) {
				php_oci_error(connection->err, connection->errcode TSRMLS_CC);
				FREE_ZVAL(*result_element);
				return 1;
			}
			
			ZVAL_DOUBLE(*result_element, double_number);

			return 0;
		}
			break;
		default:
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Unknown or unsupported type of element: %d", collection->element_typecode);
			FREE_ZVAL(*result_element);
			return 1;
			break;
	}
	/* never reached */
	return 1;
} /* }}} */

/* {{{ php_oci_collection_element_set_null() 
 Set the element with the given index to NULL */
int php_oci_collection_element_set_null(php_oci_collection *collection, long index TSRMLS_DC)
{
	OCIInd null_index = OCI_IND_NULL;
	php_oci_connection *connection = collection->connection;

	/* set NULL element */
	connection->errcode = PHP_OCI_CALL(OCICollAssignElem, (connection->env, connection->err, (ub4) index, (dvoid *)"", &null_index, collection->collection));
	
	if (connection->errcode != OCI_SUCCESS) {
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}
	return 0;
} /* }}} */

/* {{{ php_oci_collection_element_set_date() 
 Change element's value to the given DATE */
int php_oci_collection_element_set_date(php_oci_collection *collection, long index, char *date, int date_len TSRMLS_DC)
{
	OCIInd new_index = OCI_IND_NOTNULL;
	OCIDate oci_date;
	php_oci_connection *connection = collection->connection;

	/* format and language are NULLs, so format is "DD-MON-YY" and language is the default language of the session */
	connection->errcode = PHP_OCI_CALL(OCIDateFromText, (connection->err, date, date_len, NULL, 0, NULL, 0, &oci_date));

	if (connection->errcode != OCI_SUCCESS) {
		/* failed to convert string to date */
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}

	connection->errcode = PHP_OCI_CALL(OCICollAssignElem, (connection->env, connection->err, (ub4)index, (dvoid *) &oci_date, (dvoid *) &new_index, (OCIColl *) collection->collection));

	if (connection->errcode != OCI_SUCCESS) {
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}
			
	return 0;
} /* }}} */

/* {{{ php_oci_collection_element_set_number()
 Change element's value to the given NUMBER */
int php_oci_collection_element_set_number(php_oci_collection *collection, long index, char *number, int number_len TSRMLS_DC)
{
	OCIInd new_index = OCI_IND_NOTNULL;
	double element_double;
	OCINumber oci_number;
	php_oci_connection *connection = collection->connection;

	element_double = zend_strtod(number, NULL);
			
	connection->errcode = PHP_OCI_CALL(OCINumberFromReal, (connection->err, &element_double, sizeof(double), &oci_number));

	if (connection->errcode != OCI_SUCCESS) {
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}

	connection->errcode = PHP_OCI_CALL(OCICollAssignElem, (connection->env, connection->err, (ub4) index, (dvoid *) &oci_number, (dvoid *) &new_index, (OCIColl *) collection->collection));

	if (connection->errcode != OCI_SUCCESS) {
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}

	return 0;
} /* }}} */

/* {{{ php_oci_collection_element_set_string()
 Change element's value to the given string */
int php_oci_collection_element_set_string(php_oci_collection *collection, long index, char *element, int element_len TSRMLS_DC)
{
	OCIInd new_index = OCI_IND_NOTNULL;
	OCIString *ocistr = (OCIString *)0;
	php_oci_connection *connection = collection->connection;
			
	connection->errcode = PHP_OCI_CALL(OCIStringAssignText, (connection->env, connection->err, element, element_len, &ocistr));

	if (connection->errcode != OCI_SUCCESS) {
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}

	connection->errcode = PHP_OCI_CALL(OCICollAssignElem, (connection->env, connection->err, (ub4)index, (dvoid *) ocistr, (dvoid *) &new_index,	(OCIColl *) collection->collection));

	if (connection->errcode != OCI_SUCCESS) {
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}

	return 0;
} /* }}} */

/* {{{ php_oci_collection_element_set()
 Collection element setter */
int php_oci_collection_element_set(php_oci_collection *collection, long index, char *value, int value_len TSRMLS_DC)
{
	if (value_len == 0) {
		return php_oci_collection_element_set_null(collection, index TSRMLS_CC);
	}
	
	switch(collection->element_typecode) {
		case OCI_TYPECODE_DATE:
			return php_oci_collection_element_set_date(collection, index, value, value_len TSRMLS_CC);
			break;
			
		case OCI_TYPECODE_VARCHAR2 :
			return php_oci_collection_element_set_string(collection, index, value, value_len TSRMLS_CC);
			break;

		case OCI_TYPECODE_UNSIGNED16 :                       /* UNSIGNED SHORT  */
		case OCI_TYPECODE_UNSIGNED32 :                        /* UNSIGNED LONG  */
		case OCI_TYPECODE_REAL :                                     /* REAL    */
		case OCI_TYPECODE_DOUBLE :                                   /* DOUBLE  */
		case OCI_TYPECODE_INTEGER :                                     /* INT  */
		case OCI_TYPECODE_SIGNED16 :                                  /* SHORT  */
		case OCI_TYPECODE_SIGNED32 :                                   /* LONG  */
		case OCI_TYPECODE_DECIMAL :                                 /* DECIMAL  */
		case OCI_TYPECODE_FLOAT :                                   /* FLOAT    */
		case OCI_TYPECODE_NUMBER :                                  /* NUMBER   */
		case OCI_TYPECODE_SMALLINT :                                /* SMALLINT */
			return php_oci_collection_element_set_number(collection, index, value, value_len TSRMLS_CC);
			break;

		default:
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Unknown or unsupported type of element: %d", collection->element_typecode);
			return 1;
			break;
	}
	/* never reached */
	return 1;
} /* }}} */

/* {{{ php_oci_collection_assign() 
 Assigns a value to the collection from another collection */
int php_oci_collection_assign(php_oci_collection *collection_dest, php_oci_collection *collection_from TSRMLS_DC)
{
	php_oci_connection *connection = collection_dest->connection;
	
	connection->errcode = PHP_OCI_CALL(OCICollAssign, (connection->env, connection->err, collection_from->collection, collection_dest->collection));

	if (connection->errcode != OCI_SUCCESS) {
		php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		return 1;
	}
	return 0;
} /* }}} */

/* {{{ php_oci_collection_close()
 Destroy collection and all associated resources */
void php_oci_collection_close(php_oci_collection *collection TSRMLS_DC)
{
	php_oci_connection *connection = collection->connection;

	if (collection->collection) {
		connection->errcode = PHP_OCI_CALL(OCIObjectFree, (connection->env, connection->err, (dvoid *)collection->collection, (ub2)OCI_OBJECTFREE_FORCE));

		if (connection->errcode != OCI_SUCCESS) {
			php_oci_error(connection->err, connection->errcode TSRMLS_CC);
		}
	}
	
	zend_list_delete(collection->connection->rsrc_id);
	
	efree(collection);
	return;
} /* }}} */

#endif /* HAVE_OCI8 */
