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
  | Author: Wez Furlong <wez@php.net>                                    |
  |         Marcus Boerger <helly@php.net>                               |
  |         Sterling Hughes <sterling@php.net>                           |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

/* The PDO Statement Handle Class */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_var.h"
#include "php_pdo.h"
#include "php_pdo_driver.h"
#include "php_pdo_int.h"
#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "php_memory_streams.h"

#if COMPILE_DL_PDO
/* {{{ content from zend_arg_defs.c:
 * since it is a .c file, it won't be installed for use by PECL extensions, so we include it here. */
ZEND_BEGIN_ARG_INFO(first_arg_force_ref, 0)
	ZEND_ARG_PASS_INFO(1)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO(second_arg_force_ref, 0)
	ZEND_ARG_PASS_INFO(0)
	ZEND_ARG_PASS_INFO(1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(third_arg_force_ref, 0)
	ZEND_ARG_PASS_INFO(0)
	ZEND_ARG_PASS_INFO(0)
	ZEND_ARG_PASS_INFO(1)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO(fourth_arg_force_ref, 0)
	ZEND_ARG_PASS_INFO(0)
	ZEND_ARG_PASS_INFO(0)
	ZEND_ARG_PASS_INFO(0)
	ZEND_ARG_PASS_INFO(1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(all_args_by_ref, 1)
ZEND_END_ARG_INFO()
/* }}} */
#endif

#define PHP_STMT_GET_OBJ	\
  pdo_stmt_t *stmt = (pdo_stmt_t*)zend_object_store_get_object(getThis() TSRMLS_CC);	\
  if (!stmt->dbh) {	\
    RETURN_FALSE;	\
  }	\

static PHP_FUNCTION(dbstmt_constructor) /* {{{ */
{
	php_error_docref(NULL TSRMLS_CC, E_ERROR, "You should not create a PDOStatement manually");
}
/* }}} */

static inline int rewrite_name_to_position(pdo_stmt_t *stmt, struct pdo_bound_param_data *param TSRMLS_DC) /* {{{ */
{
	if (stmt->bound_param_map) {
		/* rewriting :name to ? style.
		 * We need to fixup the parameter numbers on the parameters.
		 * If we find that a given named parameter has been used twice,
		 * we will raise an error, as we can't be sure that it is safe
		 * to bind multiple parameters onto the same zval in the underlying
		 * driver */
		char *name;
		int position = 0;

		if (stmt->named_rewrite_template) {
			/* this is not an error here */
			return 1;
		}
		if (!param->name) {
			/* do the reverse; map the parameter number to the name */
			if (SUCCESS == zend_hash_index_find(stmt->bound_param_map, param->paramno, (void**)&name)) {
				param->name = estrdup(name);
				param->namelen = strlen(param->name);
				return 1;
			}
			pdo_raise_impl_error(stmt->dbh, stmt, "HY093", "parameter was not defined" TSRMLS_CC);
			return 0;
		}
    
		zend_hash_internal_pointer_reset(stmt->bound_param_map);
		while (SUCCESS == zend_hash_get_current_data(stmt->bound_param_map, (void**)&name)) {
			if (strcmp(name, param->name)) {
				position++;
				zend_hash_move_forward(stmt->bound_param_map);
				continue;
			}
			if (param->paramno >= 0) {
				pdo_raise_impl_error(stmt->dbh, stmt, "IM001", "PDO refuses to handle repeating the same :named parameter for multiple positions with this driver, as it might be unsafe to do so.  Consider using a separate name for each parameter instead" TSRMLS_CC);
				return -1;
			}
			param->paramno = position;
			return 1;
		}
		pdo_raise_impl_error(stmt->dbh, stmt, "HY093", "parameter was not defined" TSRMLS_CC);
		return 0;
	}
	return 1;	
}
/* }}} */

/* trigger callback hook for parameters */
static int dispatch_param_event(pdo_stmt_t *stmt, enum pdo_param_event event_type TSRMLS_DC) /* {{{ */
{
	int ret = 1, is_param = 1;
	struct pdo_bound_param_data *param;
	HashTable *ht;

	if (!stmt->methods->param_hook) {
		return 1;
	}

	ht = stmt->bound_params;

iterate:
	if (ht) {
		zend_hash_internal_pointer_reset(ht);
		while (SUCCESS == zend_hash_get_current_data(ht, (void**)&param)) {
			if (!stmt->methods->param_hook(stmt, param, event_type TSRMLS_CC)) {
				ret = 0;
				break;
			}
			
			zend_hash_move_forward(ht);
		}
	}
	if (ret && is_param) {
		ht = stmt->bound_columns;
		is_param = 0;
		goto iterate;
	}

	return ret;
}
/* }}} */

int pdo_stmt_describe_columns(pdo_stmt_t *stmt TSRMLS_DC) /* {{{ */
{
	int col;

	stmt->columns = ecalloc(stmt->column_count, sizeof(struct pdo_column_data));

	for (col = 0; col < stmt->column_count; col++) {
		if (!stmt->methods->describer(stmt, col TSRMLS_CC)) {
			return 0;
		}

		/* if we are applying case conversions on column names, do so now */
		if (stmt->dbh->native_case != stmt->dbh->desired_case && stmt->dbh->desired_case != PDO_CASE_NATURAL) {
			char *s = stmt->columns[col].name;

			switch (stmt->dbh->desired_case) {
				case PDO_CASE_UPPER:
					while (*s != '\0') {
						*s = toupper(*s);
						s++;
					}
					break;
				case PDO_CASE_LOWER:
					while (*s != '\0') {
						*s = tolower(*s);
						s++;
					}
					break;
				default:
					;
			}
		}

#if 0
		/* update the column index on named bound parameters */
		if (stmt->bound_params) {
			struct pdo_bound_param_data *param;

			if (SUCCESS == zend_hash_find(stmt->bound_params, stmt->columns[col].name,
						stmt->columns[col].namelen, (void**)&param)) {
				param->paramno = col;
			}
		}
#endif
		if (stmt->bound_columns) {
			struct pdo_bound_param_data *param;

			if (SUCCESS == zend_hash_find(stmt->bound_columns, stmt->columns[col].name,
						stmt->columns[col].namelen, (void**)&param)) {
				param->paramno = col;
			}
		}

	}
	return 1;
}
/* }}} */

static void get_lazy_object(pdo_stmt_t *stmt, zval *return_value TSRMLS_DC) /* {{{ */
{
	if (Z_TYPE(stmt->lazy_object_ref) == IS_NULL) {
		Z_TYPE(stmt->lazy_object_ref) = IS_OBJECT;
		Z_OBJ_HANDLE(stmt->lazy_object_ref) = zend_objects_store_put(stmt, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t)pdo_row_free_storage, NULL TSRMLS_CC);
		Z_OBJ_HT(stmt->lazy_object_ref) = &pdo_row_object_handlers;
		stmt->refcount++;
	}
	Z_TYPE_P(return_value) = IS_OBJECT;
	Z_OBJ_HANDLE_P(return_value) = Z_OBJ_HANDLE(stmt->lazy_object_ref);
	Z_OBJ_HT_P(return_value) = Z_OBJ_HT(stmt->lazy_object_ref);
	zend_objects_store_add_ref(return_value TSRMLS_CC);
}
/* }}} */

static void param_dtor(void *data) /* {{{ */
{
	struct pdo_bound_param_data *param = (struct pdo_bound_param_data *)data;
	TSRMLS_FETCH();

	/* tell the driver that it is going away */
	if (param->stmt->methods->param_hook) {
		param->stmt->methods->param_hook(param->stmt, param, PDO_PARAM_EVT_FREE TSRMLS_CC);
	}

	if (param->name) {
		efree(param->name);
	}

	zval_ptr_dtor(&(param->parameter));
	if (param->driver_params) {
		zval_ptr_dtor(&(param->driver_params));
	}
}
/* }}} */

static int really_register_bound_param(struct pdo_bound_param_data *param, pdo_stmt_t *stmt, int is_param TSRMLS_DC) /* {{{ */
{
	HashTable *hash;
	struct pdo_bound_param_data *pparam = NULL;

	hash = is_param ? stmt->bound_params : stmt->bound_columns;

	if (!hash) {
		ALLOC_HASHTABLE(hash);
		zend_hash_init(hash, 13, NULL, param_dtor, 0);

		if (is_param) {
			stmt->bound_params = hash;
		} else {
			stmt->bound_columns = hash;
		}
	}

	if (PDO_PARAM_TYPE(param->param_type) == PDO_PARAM_STR && param->max_value_len <= 0 && ! ZVAL_IS_NULL(param->parameter)) {
		convert_to_string(param->parameter);
	}

	param->stmt = stmt;
	param->is_param = is_param;

	if (param->driver_params) {
		ZVAL_ADDREF(param->driver_params);
	}

	if (!is_param && param->name && stmt->columns) {
		/* try to map the name to the column */
		int i;

		for (i = 0; i < stmt->column_count; i++) {
			if (strcmp(stmt->columns[i].name, param->name) == 0) {
				param->paramno = i;
				break;
			}
		}

		/* if you prepare and then execute passing an array of params keyed by names,
		 * then this will trigger, and we don't want that */
		if (param->paramno == -1) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Did not found column name '%s' in the defined columns; it will not be bound", param->name);
		}
	}

	if (param->name) {
		if (is_param && param->name[0] != ':') {
			char *temp = emalloc(++param->namelen + 1);
			temp[0] = ':';
			memmove(temp+1, param->name, param->namelen);
			param->name = temp;
		} else {
			param->name = estrndup(param->name, param->namelen);
		}
	}

	if (is_param && !rewrite_name_to_position(stmt, param TSRMLS_CC)) {
		if (param->name) {
			efree(param->name);
			param->name = NULL;
		}
		return 0;
	}

	/* ask the driver to perform any normalization it needs on the
	 * parameter name.  Note that it is illegal for the driver to take
	 * a reference to param, as it resides in transient storage only
	 * at this time. */
	if (stmt->methods->param_hook) {
		if (!stmt->methods->param_hook(stmt, param, PDO_PARAM_EVT_NORMALIZE
				TSRMLS_CC)) {
			if (param->name) {
				efree(param->name);
				param->name = NULL;
			}
			return 0;
		}
	}

	/* delete any other parameter registered with this number.
	 * If the parameter is named, it will be removed and correctly
	 * disposed of by the hash_update call that follows */
	if (param->paramno >= 0) {
		zend_hash_index_del(hash, param->paramno);
	}

	/* allocate storage for the parameter, keyed by its "canonical" name */
	if (param->name) {
		zend_hash_update(hash, param->name, param->namelen, param,
			sizeof(*param), (void**)&pparam);
	} else {
		zend_hash_index_update(hash, param->paramno, param, sizeof(*param),
			(void**)&pparam);
	}

	/* tell the driver we just created a parameter */
	if (stmt->methods->param_hook) {
		if (!stmt->methods->param_hook(stmt, pparam, PDO_PARAM_EVT_ALLOC
					TSRMLS_CC)) {
			/* undo storage allocation; the hash will free the parameter
			 * name if required */
			if (pparam->name) {
				zend_hash_del(hash, pparam->name, pparam->namelen);
			} else {
				zend_hash_index_del(hash, pparam->paramno);
			}
			return 0;
		}
	}
	return 1;
}
/* }}} */

/* {{{ proto bool PDOStatement::execute([array $bound_input_params])
   Execute a prepared statement, optionally binding parameters */
static PHP_METHOD(PDOStatement, execute)
{
	zval *input_params = NULL;
	int ret = 1;
	PHP_STMT_GET_OBJ;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a!", &input_params)) {
		RETURN_FALSE;
	}

	PDO_STMT_CLEAR_ERR();
	
	if (input_params) {
		struct pdo_bound_param_data param;
		zval **tmp;
		uint str_length;
		ulong num_index;

		zend_hash_internal_pointer_reset(Z_ARRVAL_P(input_params));
		while (SUCCESS == zend_hash_get_current_data(Z_ARRVAL_P(input_params), (void*)&tmp)) {
			memset(&param, 0, sizeof(param));

			if (HASH_KEY_IS_STRING == zend_hash_get_current_key_ex(Z_ARRVAL_P(input_params),
						&param.name, &str_length, &num_index, 0, NULL)) {
				/* yes this is correct.  we don't want to count the null byte.  ask wez */
				param.namelen = str_length - 1;
				param.paramno = -1;
			} else {
				/* we're okay to be zero based here */
				if (num_index < 0) {
					pdo_raise_impl_error(stmt->dbh, stmt, "HY093", NULL TSRMLS_CC);
					RETURN_FALSE;
				}
				param.paramno = num_index;
			}

			param.param_type = PDO_PARAM_STR;
			MAKE_STD_ZVAL(param.parameter);
			*param.parameter = **tmp;
			zval_copy_ctor(param.parameter);
			INIT_PZVAL(param.parameter);

			if (!really_register_bound_param(&param, stmt, 1 TSRMLS_CC)) {
				zval_ptr_dtor(&param.parameter);
				RETURN_FALSE;
			}

			zend_hash_move_forward(Z_ARRVAL_P(input_params));
		}
	}

	if (PDO_PLACEHOLDER_NONE == stmt->supports_placeholders) {
		/* handle the emulated parameter binding,
         * stmt->active_query_string holds the query with binds expanded and 
		 * quoted.
         */

		ret = pdo_parse_params(stmt, stmt->query_string, stmt->query_stringlen,
			&stmt->active_query_string, &stmt->active_query_stringlen TSRMLS_CC);

		if (ret == 0) {
			/* no changes were made */
			stmt->active_query_string = stmt->query_string;
			stmt->active_query_stringlen = stmt->query_stringlen;
		} else if (ret == -1) {
			/* something broke */
			PDO_HANDLE_STMT_ERR();
			RETURN_FALSE;
		}
	} else if (!dispatch_param_event(stmt, PDO_PARAM_EVT_EXEC_PRE TSRMLS_CC)) {
		PDO_HANDLE_STMT_ERR();
		RETURN_FALSE;
	}
	if (stmt->methods->executer(stmt TSRMLS_CC)) {
		if (stmt->active_query_string && stmt->active_query_string != stmt->query_string) {
			efree(stmt->active_query_string);
		}
		stmt->active_query_string = NULL;
		if (!stmt->executed) {
			/* this is the first execute */

			if (stmt->dbh->alloc_own_columns) {
				/* for "big boy" drivers, we need to allocate memory to fetch
				 * the results into, so lets do that now */
				ret = pdo_stmt_describe_columns(stmt TSRMLS_CC);
			}

			stmt->executed = 1;
		}

		if (ret && !dispatch_param_event(stmt, PDO_PARAM_EVT_EXEC_POST TSRMLS_CC)) {
			RETURN_FALSE;
		}
			
		RETURN_BOOL(ret);
	}
	if (stmt->active_query_string && stmt->active_query_string != stmt->query_string) {
		efree(stmt->active_query_string);
	}
	stmt->active_query_string = NULL;
	PDO_HANDLE_STMT_ERR();
	RETURN_FALSE;
}
/* }}} */

static inline void fetch_value(pdo_stmt_t *stmt, zval *dest, int colno, int *type_override TSRMLS_DC) /* {{{ */
{
	struct pdo_column_data *col;
	char *value = NULL;
	unsigned long value_len = 0;
	int caller_frees = 0;
	int type, new_type;

	col = &stmt->columns[colno];
	type = PDO_PARAM_TYPE(col->param_type);
	new_type =  type_override ? PDO_PARAM_TYPE(*type_override) : type;

	value = NULL;
	value_len = 0;

	stmt->methods->get_col(stmt, colno, &value, &value_len, &caller_frees TSRMLS_CC);

	switch (type) {
		case PDO_PARAM_INT:
			if (value && value_len == sizeof(long)) {
				ZVAL_LONG(dest, *(long*)value);
				break;
			}
			ZVAL_NULL(dest);
			break;

		case PDO_PARAM_BOOL:
			if (value && value_len == sizeof(zend_bool)) {
				ZVAL_BOOL(dest, *(zend_bool*)value);
				break;
			}
			ZVAL_NULL(dest);
			break;

		case PDO_PARAM_LOB:
			if (value == NULL) {
				ZVAL_NULL(dest);
			} else if (value_len == 0) {
				if (stmt->dbh->stringify || new_type == PDO_PARAM_STR) {
					char *buf = NULL;
					size_t len;
					len = php_stream_copy_to_mem((php_stream*)value, &buf, PHP_STREAM_COPY_ALL, 0);
					ZVAL_STRINGL(dest, buf, len, 0);
					php_stream_close((php_stream*)value);
				} else {
					php_stream_to_zval((php_stream*)value, dest);
				}
			} else if (!stmt->dbh->stringify && new_type != PDO_PARAM_STR) {
				/* they gave us a string, but LOBs are represented as streams in PDO */
				php_stream *stm;
#ifdef TEMP_STREAM_TAKE_BUFFER
				if (caller_frees) {
					stm = php_stream_memory_open(TEMP_STREAM_TAKE_BUFFER, value, value_len);
					if (stm) {
						caller_frees = 0;
					}
				} else
#endif
				{
					stm = php_stream_memory_open(TEMP_STREAM_READONLY, value, value_len);
				}
				if (stm) {
					php_stream_to_zval(stm, dest);
				} else {
					ZVAL_NULL(dest);
				}
			} else {
				ZVAL_STRINGL(dest, value, value_len, !caller_frees);
				if (caller_frees) {
					caller_frees = 0;
				}
			}
			break;
		
		case PDO_PARAM_STR:
			if (value && !(value_len == 0 && stmt->dbh->oracle_nulls == PDO_NULL_EMPTY_STRING)) {
				ZVAL_STRINGL(dest, value, value_len, !caller_frees);
				if (caller_frees) {
					caller_frees = 0;
				}
				break;
			}
		default:
			ZVAL_NULL(dest);
	}

	if (type != new_type) {
		switch (new_type) {
			case PDO_PARAM_INT:
				convert_to_long_ex(&dest);
				break;
			case PDO_PARAM_BOOL:
				convert_to_boolean_ex(&dest);
				break;
			case PDO_PARAM_STR:
				convert_to_string_ex(&dest);
				break;
			case PDO_PARAM_NULL:
				convert_to_null_ex(&dest);
				break;
			default:
				;
		}
	}
	
	if (caller_frees && value) {
		efree(value);
	}

	if (stmt->dbh->stringify) {
		switch (Z_TYPE_P(dest)) {
			case IS_LONG:
			case IS_DOUBLE:
				convert_to_string(dest);
				break;
		}
	}

	if (Z_TYPE_P(dest) == IS_NULL && stmt->dbh->oracle_nulls == PDO_NULL_TO_STRING) {
		ZVAL_EMPTY_STRING(dest);
	}
}
/* }}} */

static int do_fetch_common(pdo_stmt_t *stmt, enum pdo_fetch_orientation ori,
	long offset, int do_bind TSRMLS_DC) /* {{{ */
{
	if (!dispatch_param_event(stmt, PDO_PARAM_EVT_FETCH_PRE TSRMLS_CC)) {
		return 0;
	}

	if (!stmt->methods->fetcher(stmt, ori, offset TSRMLS_CC)) {
		return 0;
	}

	/* some drivers might need to describe the columns now */
	if (!stmt->columns && !pdo_stmt_describe_columns(stmt TSRMLS_CC)) {
		return 0;
	}
	
	if (!dispatch_param_event(stmt, PDO_PARAM_EVT_FETCH_POST TSRMLS_CC)) {
		return 0;
	}

	if (do_bind && stmt->bound_columns) {
		/* update those bound column variables now */
		struct pdo_bound_param_data *param;

		zend_hash_internal_pointer_reset(stmt->bound_columns);
		while (SUCCESS == zend_hash_get_current_data(stmt->bound_columns, (void**)&param)) {
			if (param->paramno >= 0) {
				convert_to_string(param->parameter);

				/* delete old value */
				zval_dtor(param->parameter);

				/* set new value */
				fetch_value(stmt, param->parameter, param->paramno, (int *)&param->param_type TSRMLS_CC);

				/* TODO: some smart thing that avoids duplicating the value in the
				 * general loop below.  For now, if you're binding output columns,
				 * it's better to use LAZY or BOUND fetches if you want to shave
				 * off those cycles */
			}

			zend_hash_move_forward(stmt->bound_columns);
		}
	}

	return 1;
}
/* }}} */

static int do_fetch_class_prepare(pdo_stmt_t *stmt TSRMLS_DC) /* {{{ */
{
	zend_class_entry * ce = stmt->fetch.cls.ce;
	zend_fcall_info * fci = &stmt->fetch.cls.fci;
	zend_fcall_info_cache * fcc = &stmt->fetch.cls.fcc;

	fci->size = sizeof(zend_fcall_info);

	if (!ce) {
		stmt->fetch.cls.ce = ZEND_STANDARD_CLASS_DEF_PTR;
		ce = ZEND_STANDARD_CLASS_DEF_PTR;
	}
	
	if (ce->constructor) {
		fci->function_table = &ce->function_table;
		fci->function_name = NULL;
		fci->symbol_table = NULL;
		fci->retval_ptr_ptr = &stmt->fetch.cls.retval_ptr;
		if (stmt->fetch.cls.ctor_args) {
			HashTable *ht = Z_ARRVAL_P(stmt->fetch.cls.ctor_args);
			Bucket *p;

			fci->param_count = 0;
			fci->params = safe_emalloc(sizeof(zval**), ht->nNumOfElements, 0);
			p = ht->pListHead;
			while (p != NULL) {
				fci->params[fci->param_count++] = (zval**)p->pData;
				p = p->pListNext;
			}
		} else {
			fci->param_count = 0;
			fci->params = NULL;
		}
		fci->no_separation = 1;

		fcc->initialized = 1;
		fcc->function_handler = ce->constructor;
		fcc->calling_scope = EG(scope);
		return 1;
	} else if (stmt->fetch.cls.ctor_args) {
		pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "user-supplied class does not have a constructor, use NULL for the ctor_params parameter, or simply omit it" TSRMLS_CC);
		return 0;
	} else {
		return 1; /* no ctor no args is also ok */
	}
}
/* }}} */

static int make_callable_ex(pdo_stmt_t *stmt, zval *callable, zend_fcall_info * fci, zend_fcall_info_cache * fcc, int num_args TSRMLS_DC) /* {{{ */
{
	zval **object = NULL, **method;
	char *fname, *cname;
	zend_class_entry * ce = NULL, **pce;
	zend_function *function_handler;
	
	if (Z_TYPE_P(callable) == IS_ARRAY) {
		if (Z_ARRVAL_P(callable)->nNumOfElements < 2) {
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "user-supplied function must be a valid callback" TSRMLS_CC);
			return 0;
		}
		object = (zval**)Z_ARRVAL_P(callable)->pListHead->pData;
		method = (zval**)Z_ARRVAL_P(callable)->pListHead->pListNext->pData;

		if (Z_TYPE_PP(object) == IS_STRING) { /* static call */
			object = NULL;
		} else if (Z_TYPE_PP(object) == IS_OBJECT) { /* object call */
			ce = Z_OBJCE_PP(object);
		} else {
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "user-supplied function must be a valid callback; bogus object/class name" TSRMLS_CC);
			return 0;
		}
		
		if (Z_TYPE_PP(method) != IS_STRING) {
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "user-supplied function must be a valid callback; bogus method name" TSRMLS_CC);
			return 0;
		}
	}
	
	if (!zend_is_callable(callable, 0, &fname)) {
		pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "user-supplied function must be a valid callback" TSRMLS_CC);
		return 0;
	}
	
	/* ATM we do not support array($obj, "CLASS::FUNC") or "CLASS_FUNC" */
	cname = fname;
	if ((fname = strstr(fname, "::")) == NULL) {
		fname = cname;
		cname = NULL;
	} else {
		*fname = '\0';
		fname += 2;
	}
	if (cname) {
		if (zend_lookup_class(cname, strlen(cname), &pce TSRMLS_CC) == FAILURE) {
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "user-supplied class does not exist" TSRMLS_CC);
			return 0;
		} else {
			if (ce) {
				/* pce must be base of ce or ce itself */
				if (ce != *pce && !instanceof_function(ce, *pce TSRMLS_CC)) {
					pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "user-supplied class has bogus lineage" TSRMLS_CC);
					return 0;
				}
			}
			ce = *pce;
		}
	}

	zend_str_tolower_copy(fname, fname, strlen(fname));
	fci->function_table = ce ? &ce->function_table : EG(function_table);
	if (zend_hash_find(fci->function_table, fname, strlen(fname)+1, (void **)&function_handler) == FAILURE) {
		pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "user-supplied function does not exist" TSRMLS_CC);
		return 0;
	}
	efree(cname ? cname : fname);

	fci->size = sizeof(zend_fcall_info);
	fci->function_name = NULL;
	fci->symbol_table = NULL;
	fci->param_count = num_args; /* probably less */
	fci->params = safe_emalloc(sizeof(zval**), num_args, 0);
	fci->object_pp = object;

	fcc->initialized = 1;
	fcc->function_handler = function_handler;
	fcc->calling_scope = EG(scope);
	fcc->object_pp = object;
	
	return 1;
}
/* }}} */

static int do_fetch_func_prepare(pdo_stmt_t *stmt TSRMLS_DC) /* {{{ */
{
	zend_fcall_info * fci = &stmt->fetch.cls.fci;
	zend_fcall_info_cache * fcc = &stmt->fetch.cls.fcc;

	if (!make_callable_ex(stmt, stmt->fetch.func.function, fci, fcc, stmt->column_count TSRMLS_CC)) {
		return 0;
	} else {
		stmt->fetch.func.values = safe_emalloc(sizeof(zval*), stmt->column_count, 0);
		return 1;
	}
}
/* }}} */

static int do_fetch_opt_finish(pdo_stmt_t *stmt, int free_ctor_agrs TSRMLS_DC) /* {{{ */
{
	/* fci.size is used to check if it is valid */
	if (stmt->fetch.cls.fci.size && stmt->fetch.cls.fci.params) {
		efree(stmt->fetch.cls.fci.params);
		stmt->fetch.cls.fci.params = NULL;
	}
	stmt->fetch.cls.fci.size = 0;
	if (stmt->fetch.cls.ctor_args && free_ctor_agrs) {
		zval_ptr_dtor(&stmt->fetch.cls.ctor_args);
		stmt->fetch.cls.ctor_args = NULL;
		stmt->fetch.cls.fci.param_count = 0;
	}
	if (stmt->fetch.func.values) {
		FREE_ZVAL(stmt->fetch.func.values);
		stmt->fetch.func.values = NULL;
	}
	return 1;
}
/* }}} */

/* perform a fetch.  If do_bind is true, update any bound columns.
 * If return_value is not null, store values into it according to HOW. */
static int do_fetch(pdo_stmt_t *stmt, int do_bind, zval *return_value,
	enum pdo_fetch_type how, enum pdo_fetch_orientation ori, long offset, zval *return_all TSRMLS_DC) /* {{{ */
{
	int flags = how & PDO_FETCH_FLAGS, idx, old_arg_count;
	zend_class_entry * ce, * old_ce;
	zval grp_val, *grp, **pgrp, *retval, *old_ctor_args;

	how = how & ~PDO_FETCH_FLAGS;
	if (how == PDO_FETCH_USE_DEFAULT) {
		how = stmt->default_fetch_type;
	}

	if (!do_fetch_common(stmt, ori, offset, do_bind TSRMLS_CC)) {
		return 0;
	}

	if (how == PDO_FETCH_BOUND) {
		RETVAL_TRUE;
		return 1;
	}

	if (return_value) {
		int i = 0;

		if (how == PDO_FETCH_LAZY) {
			get_lazy_object(stmt, return_value TSRMLS_CC);
			return 1;
		}

		RETVAL_FALSE;

		switch (how) {
			case PDO_FETCH_ASSOC:
			case PDO_FETCH_BOTH:
			case PDO_FETCH_NUM:
			case PDO_FETCH_NAMED:
				if (!return_all) {
					ALLOC_HASHTABLE(return_value->value.ht);
					zend_hash_init(return_value->value.ht, stmt->column_count, NULL, ZVAL_PTR_DTOR, 0);			
					Z_TYPE_P(return_value) = IS_ARRAY;
				} else {
					array_init(return_value);
				}
				break;

			case PDO_FETCH_COLUMN:
				if (stmt->fetch.column >= 0 && stmt->fetch.column < stmt->column_count) {
					fetch_value(stmt, return_value, stmt->fetch.column, NULL TSRMLS_CC);
					if (!return_all) {
						return 1;
					} else {
						break;
					}
				}
				return 0;

			case PDO_FETCH_OBJ:
				object_init_ex(return_value, ZEND_STANDARD_CLASS_DEF_PTR);
				break;

			case PDO_FETCH_CLASS:
				if (flags & PDO_FETCH_CLASSTYPE) {
					zval val;
					zend_class_entry **cep;

					old_ce = stmt->fetch.cls.ce;
					old_ctor_args = stmt->fetch.cls.ctor_args;
					old_arg_count = stmt->fetch.cls.fci.param_count;
					do_fetch_opt_finish(stmt, 0 TSRMLS_CC);

					INIT_PZVAL(&val);
					fetch_value(stmt, &val, i++, NULL TSRMLS_CC);
					if (Z_TYPE(val) != IS_NULL) {
						convert_to_string(&val);
						if (zend_lookup_class(Z_STRVAL(val), Z_STRLEN(val), &cep TSRMLS_CC) == FAILURE) {
							stmt->fetch.cls.ce = ZEND_STANDARD_CLASS_DEF_PTR;
						} else {
							stmt->fetch.cls.ce = *cep;
						}
					}

					do_fetch_class_prepare(stmt TSRMLS_CC);
					zval_dtor(&val);
				}
				ce = stmt->fetch.cls.ce;
				if ((flags & PDO_FETCH_SERIALIZE) == 0) {
					object_init_ex(return_value, ce);
					if (!stmt->fetch.cls.fci.size) {
						if (!do_fetch_class_prepare(stmt TSRMLS_CC))
						{
							return 0;
						}
					}
				}
				break;
			
			case PDO_FETCH_INTO:
				if (!stmt->fetch.into) {
					pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "No fetch-into object specified." TSRMLS_CC);
					return 0;
					break;
				}

				Z_TYPE_P(return_value) = IS_OBJECT;
				Z_OBJ_HANDLE_P(return_value) = Z_OBJ_HANDLE_P(stmt->fetch.into);
				Z_OBJ_HT_P(return_value) = Z_OBJ_HT_P(stmt->fetch.into);
				zend_objects_store_add_ref(stmt->fetch.into TSRMLS_CC);

				if (zend_get_class_entry(return_value TSRMLS_CC) == ZEND_STANDARD_CLASS_DEF_PTR) {
					how = PDO_FETCH_OBJ;
				}
				break;

			case PDO_FETCH_FUNC:
				if (!stmt->fetch.func.fci.size) {
					if (!do_fetch_func_prepare(stmt TSRMLS_CC))
					{
						return 0;
					}
				}
				break;
				

			default:
				/* shouldn't happen */
				return 0;
		}
		
		if (return_all) {
			INIT_PZVAL(&grp_val);
			fetch_value(stmt, &grp_val, i, NULL TSRMLS_CC);
			convert_to_string(&grp_val);
			if (how == PDO_FETCH_COLUMN) {
				i = stmt->column_count; /* no more data to fetch */
			} else {
				i++;
			}
		}

		for (idx = 0; i < stmt->column_count; i++, idx++) {
			zval *val;
			MAKE_STD_ZVAL(val);
			fetch_value(stmt, val, i, NULL TSRMLS_CC);

			switch (how) {
				case PDO_FETCH_ASSOC:
					add_assoc_zval(return_value, stmt->columns[i].name, val);
					break;

				case PDO_FETCH_BOTH:
					add_assoc_zval(return_value, stmt->columns[i].name, val);
					ZVAL_ADDREF(val);
					add_next_index_zval(return_value, val);
					break;

				case PDO_FETCH_NAMED:
					/* already have an item with this name? */
					{
						zval **curr_val = NULL;
						if (zend_hash_find(Z_ARRVAL_P(return_value), stmt->columns[i].name,
									strlen(stmt->columns[i].name)+1,
									(void**)&curr_val) == SUCCESS) {
							zval *arr;
							if (Z_TYPE_PP(curr_val) != IS_ARRAY) {
								/* a little bit of black magic here:
								 * we're creating a new array and swapping it for the
								 * zval that's already stored in the hash under the name
								 * we want.  We then add that zval to the array.
								 * This is effectively the same thing as:
								 * if (!is_array($hash[$name])) {
								 *   $hash[$name] = array($hash[$name]);
								 * }
								 * */
								zval *cur;

								MAKE_STD_ZVAL(arr);
								array_init(arr);

								cur = *curr_val;
								*curr_val = arr;

								add_next_index_zval(arr, cur);
							} else {
								arr = *curr_val;
							}
							add_next_index_zval(arr, val);
						} else {
							add_assoc_zval(return_value, stmt->columns[i].name, val);
						}
					}
					break;

				case PDO_FETCH_NUM:
					add_next_index_zval(return_value, val);
					break;

				case PDO_FETCH_OBJ:
				case PDO_FETCH_INTO:
					zend_update_property(NULL, return_value,
						stmt->columns[i].name, stmt->columns[i].namelen,
						val TSRMLS_CC);
					zval_ptr_dtor(&val);
					break;

				case PDO_FETCH_CLASS:
					if ((flags & PDO_FETCH_SERIALIZE) == 0 || idx) {
						zend_update_property(ce, return_value,
							stmt->columns[i].name, stmt->columns[i].namelen,
							val TSRMLS_CC);
						zval_ptr_dtor(&val);
					} else {
#ifdef MBO_0
						php_unserialize_data_t var_hash;

						PHP_VAR_UNSERIALIZE_INIT(var_hash);
						if (php_var_unserialize(&return_value, (const unsigned char**)&Z_STRVAL_P(val), Z_STRVAL_P(val)+Z_STRLEN_P(val), NULL TSRMLS_CC) == FAILURE) {
							pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "cannot unserialize data" TSRMLS_CC);
							PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
							return 0;
						}
						PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
#endif
#if PHP_MAJOR_VERSION > 5 || PHP_MINOR_VERSION >= 1
						if (!ce->unserialize) {
							zval_ptr_dtor(&val);
							pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "cannot unserialize class" TSRMLS_CC);
							return 0;
						} else if (ce->unserialize(&return_value, ce, Z_TYPE_P(val) == IS_STRING ? Z_STRVAL_P(val) : "", Z_TYPE_P(val) == IS_STRING ? Z_STRLEN_P(val) : 0, NULL TSRMLS_CC) == FAILURE) {
							zval_ptr_dtor(&val);
							pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "cannot unserialize class" TSRMLS_CC);
							zval_dtor(return_value);
							ZVAL_NULL(return_value);
							return 0;
						} else {
							zval_ptr_dtor(&val);
						}
#endif
					}
					break;
				
				case PDO_FETCH_FUNC:
					stmt->fetch.func.values[idx] = val;
					stmt->fetch.cls.fci.params[idx] = &stmt->fetch.func.values[idx];
					break;
				
				default:
					zval_ptr_dtor(&val);
					pdo_raise_impl_error(stmt->dbh, stmt, "22003", "mode is out of range" TSRMLS_CC);
					return 0;
					break;
			}
		}
		
		switch (how) {
			case PDO_FETCH_CLASS:
				if (ce->constructor) {
					stmt->fetch.cls.fci.object_pp = &return_value;
					stmt->fetch.cls.fcc.object_pp = &return_value;
					if (zend_call_function(&stmt->fetch.cls.fci, &stmt->fetch.cls.fcc TSRMLS_CC) == FAILURE) {
						pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "could not call class constructor" TSRMLS_CC);
						return 0;
					} else {
						if (stmt->fetch.cls.retval_ptr) {
							zval_ptr_dtor(&stmt->fetch.cls.retval_ptr);
						}
					}
				}
				if (flags & PDO_FETCH_CLASSTYPE) {
					do_fetch_opt_finish(stmt, 0 TSRMLS_CC);
					stmt->fetch.cls.ce = old_ce;
					stmt->fetch.cls.ctor_args = old_ctor_args;
					stmt->fetch.cls.fci.param_count = old_arg_count;
				}
				break;

			case PDO_FETCH_FUNC:
				stmt->fetch.func.fci.param_count = idx;
				stmt->fetch.func.fci.retval_ptr_ptr = &retval;
				if (zend_call_function(&stmt->fetch.func.fci, &stmt->fetch.func.fcc TSRMLS_CC) == FAILURE) {
					pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "could not call user-supplied function" TSRMLS_CC);
					return 0;
				} else {
					if (return_all) {
						zval_ptr_dtor(&return_value); /* we don't need that */
						return_value = retval;
					} else if (retval) {
						*return_value = *retval;
						zval_copy_ctor(return_value);
						INIT_PZVAL(return_value);
						zval_ptr_dtor(&retval);
					}
				}
				while(idx--) {
					zval_ptr_dtor(&stmt->fetch.func.values[idx]);
				}
				break;
			
			default:
				break;
		}

		if (return_all) {
			if ((flags & PDO_FETCH_UNIQUE) == PDO_FETCH_UNIQUE) {
				add_assoc_zval(return_all, Z_STRVAL(grp_val), return_value);
			} else {
				if (zend_symtable_find(Z_ARRVAL_P(return_all), Z_STRVAL(grp_val), Z_STRLEN(grp_val)+1, (void**)&pgrp) == FAILURE) {
					MAKE_STD_ZVAL(grp);
					array_init(grp);
					add_assoc_zval(return_all, Z_STRVAL(grp_val), grp);
				} else {
					grp = *pgrp;
				}
				add_next_index_zval(grp, return_value);
			}
			zval_dtor(&grp_val);
		}

	}

	return 1;
}
/* }}} */

static int pdo_stmt_verify_mode(pdo_stmt_t *stmt, int mode, int fetch_all TSRMLS_DC) /* {{{ */
{
	int flags = mode & PDO_FETCH_FLAGS;

	mode = mode & ~PDO_FETCH_FLAGS;

	if (mode < 0 || mode > PDO_FETCH__MAX) {
		pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "invalid fetch mode" TSRMLS_CC);
		return 0;
	}
	
	if (mode == PDO_FETCH_USE_DEFAULT) {
		flags = stmt->default_fetch_type & PDO_FETCH_FLAGS;
		mode = stmt->default_fetch_type & ~PDO_FETCH_FLAGS;
	}

#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 1
	if ((flags & PDO_FETCH_SERIALIZE) == PDO_FETCH_SERIALIZE) {
		pdo_raise_impl_error(stmt->dbh, stmt, "IM001", "PDO::FETCH_SERIALIZE is not supported in this PHP version" TSRMLS_CC);
		return 0;
	}
#endif

	switch(mode) {
	case PDO_FETCH_FUNC:
		if (!fetch_all) {
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "PDO::FETCH_FUNC is only allowed in PDOStatement::fetchAll()" TSRMLS_CC);
			return 0;
		}
		return 1;

	case PDO_FETCH_LAZY:
		if (fetch_all) {
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "PDO::FETCH_LAZY can't be used with PDOStatement::fetchAll()" TSRMLS_CC);
			return 0;
		}
		/* fall through */
	
	default:
		if ((flags & PDO_FETCH_SERIALIZE) == PDO_FETCH_SERIALIZE) {
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "PDO::FETCH_SERIALIZE can only be used together with PDO::FETCH_CLASS" TSRMLS_CC);
			return 0;
		}
		if ((flags & PDO_FETCH_CLASSTYPE) == PDO_FETCH_CLASSTYPE) {
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "PDO::FETCH_CLASSTYPE can only be used together with PDO::FETCH_CLASS" TSRMLS_CC);
			return 0;
		}
		if (mode >= PDO_FETCH__MAX) {
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "invalid fetch mode" TSRMLS_CC);
			return 0;
		}
		/* no break; */

	case PDO_FETCH_CLASS:
		return 1;
	}
}
/* }}} */

/* {{{ proto mixed PDOStatement::fetch([int $how = PDO_FETCH_BOTH [, int $orientation [, int $offset]]])
   Fetches the next row and returns it, or false if there are no more rows */
static PHP_METHOD(PDOStatement, fetch)
{
	long how = PDO_FETCH_USE_DEFAULT;
	long ori = PDO_FETCH_ORI_NEXT;
	long off = 0;
        PHP_STMT_GET_OBJ;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|lll", &how,
			&ori, &off)) {
		RETURN_FALSE;
	}

	PDO_STMT_CLEAR_ERR();

	if (!pdo_stmt_verify_mode(stmt, how, 0 TSRMLS_CC)) {
		RETURN_FALSE;
	}

	if (!do_fetch(stmt, TRUE, return_value, how, ori, off, 0 TSRMLS_CC)) {
		PDO_HANDLE_STMT_ERR();
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto mixed PDOStatement::fetchObject(string class_name [, NULL|array ctor_args])
   Fetches the next row and returns it as an object. */
static PHP_METHOD(PDOStatement, fetchObject)
{
	long how = PDO_FETCH_CLASS;
	long ori = PDO_FETCH_ORI_NEXT;
	long off = 0;
	char *class_name;
	int class_name_len;
	zend_class_entry *old_ce;
	zval *old_ctor_args, *ctor_args;
	int error = 0, old_arg_count;

	PHP_STMT_GET_OBJ;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sz", 
		&class_name, &class_name_len, &ctor_args)) {
		RETURN_FALSE;
	}

	PDO_STMT_CLEAR_ERR();

	if (!pdo_stmt_verify_mode(stmt, how, 0 TSRMLS_CC)) {
		RETURN_FALSE;
	}

	old_ce = stmt->fetch.cls.ce;
	old_ctor_args = stmt->fetch.cls.ctor_args;
	old_arg_count = stmt->fetch.cls.fci.param_count;
	
	do_fetch_opt_finish(stmt, 0 TSRMLS_CC);
	
	switch(ZEND_NUM_ARGS()) {
	case 0:
		stmt->fetch.cls.ce = zend_standard_class_def;
		break;
	case 2:
		if (Z_TYPE_P(ctor_args) != IS_NULL && Z_TYPE_P(ctor_args) != IS_ARRAY) {
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "ctor_args must be either NULL or an array" TSRMLS_CC);
			error = 1;
			break;
		}
		if (Z_TYPE_P(ctor_args) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(ctor_args))) {
			ALLOC_ZVAL(stmt->fetch.cls.ctor_args);
			*stmt->fetch.cls.ctor_args = *ctor_args;
			zval_copy_ctor(stmt->fetch.cls.ctor_args);
		} else {
			stmt->fetch.cls.ctor_args = NULL;
		}
		/* no break */
	case 1:
		stmt->fetch.cls.ce = zend_fetch_class(class_name, class_name_len, ZEND_FETCH_CLASS_AUTO TSRMLS_CC);

		if (!stmt->fetch.cls.ce) {
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "Could not find user-supplied class" TSRMLS_CC);
			error = 1;
			break;
		}
	}

	if (!error && !do_fetch(stmt, TRUE, return_value, how, ori, off, 0 TSRMLS_CC)) {
		error = 1;
	}
	if (error) {
		PDO_HANDLE_STMT_ERR();
	}
	do_fetch_opt_finish(stmt, 1 TSRMLS_CC);

	stmt->fetch.cls.ce = old_ce;
	stmt->fetch.cls.ctor_args = old_ctor_args;
	stmt->fetch.cls.fci.param_count = old_arg_count;
	if (error) {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto string PDOStatement::fetchColumn([int column_number])
   Returns a data of the specified column in the result set. */
static PHP_METHOD(PDOStatement, fetchColumn)
{
	long col_n = 0;
	PHP_STMT_GET_OBJ;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &col_n)) {
		RETURN_FALSE;
	}

	PDO_STMT_CLEAR_ERR();

	if (!do_fetch_common(stmt, PDO_FETCH_ORI_NEXT, 0, TRUE TSRMLS_CC)) {
		PDO_HANDLE_STMT_ERR();
		RETURN_FALSE;
	}

	fetch_value(stmt, return_value, col_n, NULL TSRMLS_CC);
}
/* }}} */

/* {{{ proto array PDOStatement::fetchAll([int $how = PDO_FETCH_BOTH [, string class_name [, NULL|array ctor_args]]])
   Returns an array of all of the results. */
static PHP_METHOD(PDOStatement, fetchAll)
{
	long how = PDO_FETCH_USE_DEFAULT;
	zval *data, *return_all;
	zval *arg2;
	zend_class_entry *old_ce;
	zval *old_ctor_args, *ctor_args = NULL;
	int error = 0, old_arg_count;
	PHP_STMT_GET_OBJ;    	  

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|lzz", &how, &arg2, &ctor_args)) {
		RETURN_FALSE;
	}

	if (!pdo_stmt_verify_mode(stmt, how, 1 TSRMLS_CC)) {
		RETURN_FALSE;
	}

	old_ce = stmt->fetch.cls.ce;
	old_ctor_args = stmt->fetch.cls.ctor_args;
	old_arg_count = stmt->fetch.cls.fci.param_count;

	do_fetch_opt_finish(stmt, 0 TSRMLS_CC);

	switch(how & ~PDO_FETCH_FLAGS) {
	case PDO_FETCH_CLASS:
		switch(ZEND_NUM_ARGS()) {
		case 0:
		case 1:
			stmt->fetch.cls.ce = zend_standard_class_def;
			break;
		case 3:
			if (Z_TYPE_P(ctor_args) != IS_NULL && Z_TYPE_P(ctor_args) != IS_ARRAY) {
				pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "ctor_args must be either NULL or an array" TSRMLS_CC);
				error = 1;
				break;
			}
			if (Z_TYPE_P(ctor_args) != IS_ARRAY || !zend_hash_num_elements(Z_ARRVAL_P(ctor_args))) {
				ctor_args = NULL;
			}
			/* no break */
		case 2:
			stmt->fetch.cls.ctor_args = ctor_args; /* we're not going to free these */
			if (Z_TYPE_P(arg2) != IS_STRING) {
				pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "Invalid class name (should be a string)" TSRMLS_CC);
				error = 1;
				break;
			} else {
				stmt->fetch.cls.ce = zend_fetch_class(Z_STRVAL_P(arg2), Z_STRLEN_P(arg2), ZEND_FETCH_CLASS_AUTO TSRMLS_CC);
				if (!stmt->fetch.cls.ce) {
					pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "could not find user-specified class" TSRMLS_CC);
					error = 1;
					break;
				}
			}
		}
		if (!error) {
			do_fetch_class_prepare(stmt TSRMLS_CC);
		}
		break;

	case PDO_FETCH_FUNC:
		switch(ZEND_NUM_ARGS()) {
		case 0:
		case 1:
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "no fetch function specified" TSRMLS_CC);
			error = 1;
			break;
		case 3:
		case 2:
			stmt->fetch.func.function = arg2;
			do_fetch_func_prepare(stmt TSRMLS_CC);
			break;
		}
		break;
	
	case PDO_FETCH_COLUMN:
		switch(ZEND_NUM_ARGS()) {
		case 0:
		case 1:
			stmt->fetch.column = how & PDO_FETCH_GROUP ? 1 : 0;
			break;
		case 2:
			convert_to_long(arg2);
			stmt->fetch.column = Z_LVAL_P(arg2);
			break;
		case 3:
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "Third parameter not allowed for PDO::FETCH_COLUMN" TSRMLS_CC);
			error = 1;
		}
		break;

	default:
		if (ZEND_NUM_ARGS() > 1) {
			pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "Extraneous additional parameters" TSRMLS_CC);
			error = 1;
		}
	}

	if (!error)	{
		PDO_STMT_CLEAR_ERR();
		MAKE_STD_ZVAL(data);
		if (how & PDO_FETCH_GROUP) {
			array_init(return_value);
			return_all = return_value;
		} else {
			return_all = 0;
		}
		if (!do_fetch(stmt, TRUE, data, how, PDO_FETCH_ORI_NEXT, 0, return_all TSRMLS_CC)) {
			FREE_ZVAL(data);
			error = 2;
		}
	}
	if (!error) {
		if ((how & PDO_FETCH_GROUP)) {
			do {
				MAKE_STD_ZVAL(data);
			} while (do_fetch(stmt, TRUE, data, how, PDO_FETCH_ORI_NEXT, 0, return_all TSRMLS_CC));
		} else {
			array_init(return_value);
			do {
				add_next_index_zval(return_value, data);
				MAKE_STD_ZVAL(data);
			} while (do_fetch(stmt, TRUE, data, how, PDO_FETCH_ORI_NEXT, 0, 0 TSRMLS_CC));
		}
		FREE_ZVAL(data);
	}
	
	do_fetch_opt_finish(stmt, 0 TSRMLS_CC);

	stmt->fetch.cls.ce = old_ce;
	stmt->fetch.cls.ctor_args = old_ctor_args;
	stmt->fetch.cls.fci.param_count = old_arg_count;
	
	if (error) {
		PDO_HANDLE_STMT_ERR();
		if (error != 2) {
			RETURN_FALSE;
		} else { /* on no results, return an empty array */
			if (Z_TYPE_P(return_value) != IS_ARRAY) {
				array_init(return_value);
			}
			return;
		}
	}
}
/* }}} */

static int register_bound_param(INTERNAL_FUNCTION_PARAMETERS, pdo_stmt_t *stmt, int is_param) /* {{{ */
{
	struct pdo_bound_param_data param = {0};

	param.paramno = -1;
	param.param_type = PDO_PARAM_STR;

	if (FAILURE == zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC,
			"lz|llz!", &param.paramno, &param.parameter, &param.param_type, &param.max_value_len,
			&param.driver_params)) {
		if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|llz!", &param.name,
				&param.namelen, &param.parameter, &param.param_type, &param.max_value_len, 
				&param.driver_params)) {
			return 0;
		}	
	}

	if (param.paramno > 0) {
		--param.paramno; /* make it zero-based internally */
	} else if (!param.name) {
		pdo_raise_impl_error(stmt->dbh, stmt, "HY093", "Columns/Parameters are 1-based" TSRMLS_CC);
		return 0;
	}

	ZVAL_ADDREF(param.parameter);
	return really_register_bound_param(&param, stmt, is_param TSRMLS_CC);
} /* }}} */

/* {{{ proto bool PDOStatement::bindValue(mixed $paramno, mixed $param [, int $type ])
   bind an input parameter to the value of a PHP variable.  $paramno is the 1-based position of the placeholder in the SQL statement (but can be the parameter name for drivers that support named placeholders).  It should be called prior to execute(). */
static PHP_METHOD(PDOStatement, bindValue)
{
	struct pdo_bound_param_data param = {0};
	PHP_STMT_GET_OBJ;

	param.paramno = -1;
	param.param_type = PDO_PARAM_STR;

	if (FAILURE == zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS() TSRMLS_CC,
			"lz/|l", &param.paramno, &param.parameter, &param.param_type)) {
		if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz/|l", &param.name,
				&param.namelen, &param.parameter, &param.param_type)) {
			RETURN_FALSE;
		}
	}
	
	if (param.paramno > 0) {
		--param.paramno; /* make it zero-based internally */
	} else if (!param.name) {
		pdo_raise_impl_error(stmt->dbh, stmt, "HY093", "Columns/Parameters are 1-based" TSRMLS_CC);
		RETURN_FALSE;
	}
	
	ZVAL_ADDREF(param.parameter);
	RETURN_BOOL(really_register_bound_param(&param, stmt, TRUE TSRMLS_CC));
}
/* }}} */


/* {{{ proto bool PDOStatement::bindParam(mixed $paramno, mixed &$param [, int $type [, int $maxlen [, mixed $driverdata]]])
   bind a parameter to a PHP variable.  $paramno is the 1-based position of the placeholder in the SQL statement (but can be the parameter name for drivers that support named placeholders).  This isn't supported by all drivers.  It should be called prior to execute(). */
static PHP_METHOD(PDOStatement, bindParam)
{
	PHP_STMT_GET_OBJ;
	RETURN_BOOL(register_bound_param(INTERNAL_FUNCTION_PARAM_PASSTHRU, stmt, TRUE));
}
/* }}} */

/* {{{ proto bool PDOStatement::bindColumn(mixed $column, mixed &$param [, int $type [, int $maxlen [, mixed $driverdata]]])
   bind a column to a PHP variable.  On each row fetch $param will contain the value of the corresponding column.  $column is the 1-based offset of the column, or the column name.  For portability, don't call this before execute(). */
static PHP_METHOD(PDOStatement, bindColumn)
{
	PHP_STMT_GET_OBJ;
	RETURN_BOOL(register_bound_param(INTERNAL_FUNCTION_PARAM_PASSTHRU, stmt, FALSE));
}
/* }}} */

/* {{{ proto int PDOStatement::rowCount()
   Returns the number of rows in a result set, or the number of rows affected by the last execute().  It is not always meaningful. */
static PHP_METHOD(PDOStatement, rowCount)
{
	PHP_STMT_GET_OBJ;

	RETURN_LONG(stmt->row_count);
}
/* }}} */

/* {{{ proto string PDOStatement::errorCode()
   Fetch the error code associated with the last operation on the statement handle */
static PHP_METHOD(PDOStatement, errorCode)
{
	PHP_STMT_GET_OBJ;

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	RETURN_STRING(stmt->error_code, 1);
}
/* }}} */

/* {{{ proto array PDOStatement::errorInfo()
   Fetch extended error information associated with the last operation on the statement handle */
static PHP_METHOD(PDOStatement, errorInfo)
{
	PHP_STMT_GET_OBJ;

	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}

	array_init(return_value);
	add_next_index_string(return_value, stmt->error_code, 1);

	if (stmt->dbh->methods->fetch_err) {
		stmt->dbh->methods->fetch_err(stmt->dbh, stmt, return_value TSRMLS_CC);
	}
}
/* }}} */

/* {{{ proto bool PDOStatement::setAttribute(long attribute, mixed value)
   Set an attribute */
static PHP_METHOD(PDOStatement, setAttribute)
{
	long attr;
	zval *value = NULL;
	PHP_STMT_GET_OBJ;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lz!", &attr, &value)) {
		RETURN_FALSE;
	}

	if (!stmt->methods->set_attribute) {
		goto fail;
	}

	PDO_STMT_CLEAR_ERR();
	if (stmt->methods->set_attribute(stmt, attr, value TSRMLS_CC)) {
		RETURN_TRUE;
	}

fail:
	if (!stmt->methods->set_attribute) {
		pdo_raise_impl_error(stmt->dbh, stmt, "IM001", "This driver doesn't support setting attributes" TSRMLS_CC);
	} else {
		PDO_HANDLE_STMT_ERR();
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto mixed PDOStatement::getAttribute(long attribute)
   Get an attribute */

static int generic_stmt_attr_get(pdo_stmt_t *stmt, zval *return_value, long attr)
{
	switch (attr) {
		case PDO_ATTR_EMULATE_PREPARES:
			RETVAL_BOOL(stmt->supports_placeholders == PDO_PLACEHOLDER_NONE);
			return 1;
	}
	return 0;
}
   
static PHP_METHOD(PDOStatement, getAttribute)
{
	long attr;
	PHP_STMT_GET_OBJ;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &attr)) {
		RETURN_FALSE;
	}

	if (!stmt->methods->get_attribute) {
		if (!generic_stmt_attr_get(stmt, return_value, attr)) {
			pdo_raise_impl_error(stmt->dbh, stmt, "IM001",
				"This driver doesn't support getting attributes" TSRMLS_CC);
			RETURN_FALSE;
		}
		return;
	}

	PDO_STMT_CLEAR_ERR();
	switch (stmt->methods->get_attribute(stmt, attr, return_value TSRMLS_CC)) {
		case -1:
			PDO_HANDLE_STMT_ERR();
			RETURN_FALSE;

		case 0:
			if (!generic_stmt_attr_get(stmt, return_value, attr)) {
				/* XXX: should do something better here */
				pdo_raise_impl_error(stmt->dbh, stmt, "IM001",
					"driver doesn't support getting that attribute" TSRMLS_CC);
				RETURN_FALSE;
			}
			return;

		default:
			return;
	}
}
/* }}} */

/* {{{ proto int PDOStatement::columnCount()
   Returns the number of columns in the result set */
static PHP_METHOD(PDOStatement, columnCount)
{
	PHP_STMT_GET_OBJ;
	if (ZEND_NUM_ARGS()) {
		RETURN_FALSE;
	}
	RETURN_LONG(stmt->column_count);
}
/* }}} */

/* {{{ proto array PDOStatement::getColumnMeta(int $column)
   Returns meta data for a numbered column */
static PHP_METHOD(PDOStatement, getColumnMeta)
{
	long colno;
	struct pdo_column_data *col;
	PHP_STMT_GET_OBJ;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &colno)) {
		RETURN_FALSE;
	}
	if(colno < 0) {
		pdo_raise_impl_error(stmt->dbh, stmt, "42P10", "column number must be non-negative" TSRMLS_CC);
		RETURN_FALSE;
	}

	if (!stmt->methods->get_column_meta) {
		pdo_raise_impl_error(stmt->dbh, stmt, "IM001", "driver doesn't support meta data" TSRMLS_CC);
		RETURN_FALSE;
	}

	PDO_STMT_CLEAR_ERR();
	if (FAILURE == stmt->methods->get_column_meta(stmt, colno, return_value TSRMLS_CC)) {
		PDO_HANDLE_STMT_ERR();
		RETURN_FALSE;
	}

	/* add stock items */
	col = &stmt->columns[colno];
	add_assoc_string(return_value, "name", col->name, 1);
	add_assoc_long(return_value, "len", col->maxlen); /* FIXME: unsigned ? */
	add_assoc_long(return_value, "precision", col->precision);
	add_assoc_long(return_value, "pdo_type", col->param_type);
}
/* }}} */

/* {{{ proto bool PDOStatement::setFetchMode(int mode [mixed* params])
   Changes the default fetch mode for subsequent fetches (params have different meaning for different fetch modes) */

int pdo_stmt_setup_fetch_mode(INTERNAL_FUNCTION_PARAMETERS, pdo_stmt_t *stmt, int skip)
{
	long mode = PDO_FETCH_BOTH;
	int argc = ZEND_NUM_ARGS() - skip;
	zval ***args;
	zend_class_entry **cep;
	
	do_fetch_opt_finish(stmt, 1 TSRMLS_CC);

	switch (stmt->default_fetch_type) {
		case PDO_FETCH_INTO:
			if (stmt->fetch.into) {
				ZVAL_DELREF(stmt->fetch.into);
				stmt->fetch.into = NULL;
			}
			break;
		default:
			;
	}
	
	stmt->default_fetch_type = PDO_FETCH_BOTH;

	if (argc == 0) {
		return SUCCESS;
	}

	args = safe_emalloc(ZEND_NUM_ARGS(), sizeof(zval*), 0);

	if (FAILURE == zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args)) {
fail_out:
		efree(args);
		return FAILURE;
	}
	
	convert_to_long_ex(args[skip]);
	mode = Z_LVAL_PP(args[skip]);
	
	if (!pdo_stmt_verify_mode(stmt, mode, 0 TSRMLS_CC)) {
		efree(args);
		return FAILURE;
	}

	switch (mode & ~PDO_FETCH_FLAGS) {
		case PDO_FETCH_USE_DEFAULT:
		case PDO_FETCH_LAZY:
		case PDO_FETCH_ASSOC:
		case PDO_FETCH_NUM:
		case PDO_FETCH_BOTH:
		case PDO_FETCH_OBJ:
		case PDO_FETCH_BOUND:
		case PDO_FETCH_NAMED:
			break;

		case PDO_FETCH_COLUMN:
			if (argc != 2) {
				goto fail_out;
			}
			convert_to_long_ex(args[skip+1]);
			stmt->fetch.column = Z_LVAL_PP(args[skip+1]);
			break;

		case PDO_FETCH_CLASS:
			if (argc < 2 || argc > 3) {
				goto fail_out;
			}
			convert_to_string_ex(args[skip+1]);

			if (FAILURE == zend_lookup_class(Z_STRVAL_PP(args[skip+1]),
					Z_STRLEN_PP(args[skip+1]), &cep TSRMLS_CC)) {
				goto fail_out;
			}
				
			if (!cep || !*cep) {
				goto fail_out;
			}
			
			stmt->fetch.cls.ce = *cep;
			stmt->fetch.cls.ctor_args = NULL;

			if (stmt->dbh->is_persistent) {
				/* TODO: CRITICAL for final release */
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "PHP might crash if you don't call $stmt->setFetchMode() to reset to defaults on this persistent statement.  This will be fixed in a later release");
			}
			
			if (argc == 3) {
				if (Z_TYPE_PP(args[skip+2]) != IS_NULL && Z_TYPE_PP(args[skip+2]) != IS_ARRAY) {
					pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "ctor_args must be either NULL or an array" TSRMLS_CC);
				} else if (Z_TYPE_PP(args[skip+2]) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_PP(args[skip+2]))) {
					ALLOC_ZVAL(stmt->fetch.cls.ctor_args);
					*stmt->fetch.cls.ctor_args = **args[skip+2];
					zval_copy_ctor(stmt->fetch.cls.ctor_args);
				}
			}
			
			do_fetch_class_prepare(stmt TSRMLS_CC);
			break;

		case PDO_FETCH_INTO:
			if (argc != 2) {
				goto fail_out;
			}
			if (Z_TYPE_PP(args[skip+1]) != IS_OBJECT) {
				goto fail_out;
			}

			if (stmt->dbh->is_persistent) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "PHP might crash if you don't call $stmt->setFetchMode() to reset to defaults on this persistent statement.  This will be fixed in a later release");
			}
	
			MAKE_STD_ZVAL(stmt->fetch.into);

			Z_TYPE_P(stmt->fetch.into) = IS_OBJECT;
			Z_OBJ_HANDLE_P(stmt->fetch.into) = Z_OBJ_HANDLE_PP(args[skip+1]);
			Z_OBJ_HT_P(stmt->fetch.into) = Z_OBJ_HT_PP(args[skip+1]);
			zend_objects_store_add_ref(stmt->fetch.into TSRMLS_CC);
			break;
		
		default:
			pdo_raise_impl_error(stmt->dbh, stmt, "22003", "Invalid fetch mode specified" TSRMLS_CC);
			goto fail_out;
	}

	stmt->default_fetch_type = mode;
	efree(args);

	return SUCCESS;
}
   
static PHP_METHOD(PDOStatement, setFetchMode)
{
	PHP_STMT_GET_OBJ;

	RETVAL_BOOL(
		pdo_stmt_setup_fetch_mode(INTERNAL_FUNCTION_PARAM_PASSTHRU,
			stmt, 0) == SUCCESS ? 1 : 0
		);
}
/* }}} */

/* {{{ proto bool PDOStatement::nextRowset()
   Advances to the next rowset in a multi-rowset statement handle. Returns true if it succeded, false otherwise */

static int pdo_stmt_do_next_rowset(pdo_stmt_t *stmt TSRMLS_DC)
{
	if (!stmt->methods->next_rowset(stmt TSRMLS_CC)) {
		return 0;
	}

	/* un-describe */
	if (stmt->columns) {
		int i;
		struct pdo_column_data *cols = stmt->columns;
		
		for (i = 0; i < stmt->column_count; i++) {
			efree(cols[i].name);
		}
		efree(stmt->columns);
		stmt->columns = NULL;
		stmt->column_count = 0;
	}

	pdo_stmt_describe_columns(stmt TSRMLS_CC);

	return 1;
}

static PHP_METHOD(PDOStatement, nextRowset)
{
	PHP_STMT_GET_OBJ;

	if (!stmt->methods->next_rowset) {
		pdo_raise_impl_error(stmt->dbh, stmt, "IM001", "driver does not support multiple rowsets" TSRMLS_CC);
		RETURN_FALSE;
	}

	PDO_STMT_CLEAR_ERR();
	
	if (!pdo_stmt_do_next_rowset(stmt TSRMLS_CC)) {
		PDO_HANDLE_STMT_ERR();
		RETURN_FALSE;
	}
	
	pdo_stmt_describe_columns(stmt TSRMLS_CC);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool PDOStatement::closeCursor()
   Closes the cursor, leaving the statement ready for re-execution. */
static PHP_METHOD(PDOStatement, closeCursor)
{
	PHP_STMT_GET_OBJ;

	if (!stmt->methods->cursor_closer) {
		/* emulate it by fetching and discarding rows */
		do {
			while (stmt->methods->fetcher(stmt, PDO_FETCH_ORI_NEXT, 0 TSRMLS_CC))
				;
			if (!stmt->methods->next_rowset) {
				break;
			}

			if (!pdo_stmt_do_next_rowset(stmt TSRMLS_CC)) {
				break;
			}
				
		} while (1);
		RETURN_TRUE;
	}

	PDO_STMT_CLEAR_ERR();

	if (!stmt->methods->cursor_closer(stmt TSRMLS_CC)) {
		PDO_HANDLE_STMT_ERR();
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void PDOStatement::debugDumpParams()
   A utility for internals hackers to debug parameter internals */
static PHP_METHOD(PDOStatement, debugDumpParams)
{
	php_stream *out = php_stream_open_wrapper("php://output", "w", 0, NULL);
	HashPosition pos;
	struct pdo_bound_param_data *param;
	PHP_STMT_GET_OBJ;

	if (out == NULL) {
		RETURN_FALSE;
	}
	
	php_stream_printf(out TSRMLS_CC, "SQL: [%d] %.*s\n",
		stmt->query_stringlen,
		stmt->query_stringlen, stmt->query_string);

	php_stream_printf(out TSRMLS_CC, "Params:  %d\n",
		stmt->bound_params ? zend_hash_num_elements(stmt->bound_params) : 0);
	
	if (stmt->bound_params) {
		zend_hash_internal_pointer_reset_ex(stmt->bound_params, &pos);
		while (SUCCESS == zend_hash_get_current_data_ex(stmt->bound_params,
				(void**)&param, &pos)) {
			char *str;
			uint len;
			ulong num;

			if (zend_hash_get_current_key_ex(stmt->bound_params, &str, &len, &num, 0, &pos) == HASH_KEY_IS_STRING) {
				php_stream_printf(out TSRMLS_CC, "Key: Position #%d:\n", num);
			} else {
				php_stream_printf(out TSRMLS_CC, "Key: Name: [%d] %.*s\n", len, len, str);
			}

			php_stream_printf(out TSRMLS_CC, "paramno=%d\nname=[%d] %.*s\nis_param=%d\nparam_type=%d\n",
				param->paramno, param->namelen, param->namelen, param->name,
				param->is_param,
				param->param_type);
			
		}
	}

	php_stream_close(out);
}
/* }}} */

/* {{{ proto int PDOStatement::__wakeup()
   Prevents use of a PDOStatement instance that has been unserialized */
static PHP_METHOD(PDOStatement, __wakeup)
{
	zend_throw_exception_ex(php_pdo_get_exception(), 0 TSRMLS_CC, "You cannot serialize or unserialize PDOStatement instances");
}
/* }}} */

/* {{{ proto int PDOStatement::__sleep()
   Prevents serialization of a PDOStatement instance */
static PHP_METHOD(PDOStatement, __sleep)
{
	zend_throw_exception_ex(php_pdo_get_exception(), 0 TSRMLS_CC, "You cannot serialize or unserialize PDOStatement instances");
}
/* }}} */

zend_function_entry pdo_dbstmt_functions[] = {
	PHP_ME(PDOStatement, execute,		NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, fetch,			NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, bindParam,		second_arg_force_ref,	ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, bindColumn,	second_arg_force_ref,	ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, bindValue,		NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, rowCount,		NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, fetchColumn,	NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, fetchAll,		NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, fetchObject,	NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, errorCode,		NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, errorInfo,		NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, setAttribute,	NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, getAttribute,	NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, columnCount,	NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, getColumnMeta,	NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, setFetchMode,	NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, nextRowset,	NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, closeCursor,	NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, debugDumpParams, NULL,					ZEND_ACC_PUBLIC)
	PHP_ME(PDOStatement, __wakeup,		NULL,					ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(PDOStatement, __sleep,		NULL,					ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	{NULL, NULL, NULL}
};

/* {{{ overloaded handlers for PDOStatement class */
static void dbstmt_prop_write(zval *object, zval *member, zval *value TSRMLS_DC)
{
	pdo_stmt_t * stmt = (pdo_stmt_t *) zend_object_store_get_object(object TSRMLS_CC);

	convert_to_string(member);

	if(strcmp(Z_STRVAL_P(member), "queryString") == 0) {
		pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "property queryString is read only" TSRMLS_CC);
	} else {
		std_object_handlers.write_property(object, member, value TSRMLS_CC);
	}
}

static void dbstmt_prop_delete(zval *object, zval *member TSRMLS_DC)
{
	pdo_stmt_t * stmt = (pdo_stmt_t *) zend_object_store_get_object(object TSRMLS_CC);

	convert_to_string(member);

	if(strcmp(Z_STRVAL_P(member), "queryString") == 0) {
		pdo_raise_impl_error(stmt->dbh, stmt, "HY000", "property queryString is read only" TSRMLS_CC);
	} else {
		std_object_handlers.unset_property(object, member TSRMLS_CC);
	}
}

static union _zend_function *dbstmt_method_get(
#if PHP_API_VERSION >= 20041225
	zval **object_pp,
#else
	zval *object,
#endif
   	char *method_name, int method_len TSRMLS_DC)
{
	zend_function *fbc = NULL;
	char *lc_method_name;
#if PHP_API_VERSION >= 20041225
	zval *object = *object_pp;
#endif

	lc_method_name = emalloc(method_len + 1);
	zend_str_tolower_copy(lc_method_name, method_name, method_len);

	if (zend_hash_find(&Z_OBJCE_P(object)->function_table, lc_method_name, 
			method_len+1, (void**)&fbc) == FAILURE) {
		pdo_stmt_t *stmt = (pdo_stmt_t*)zend_object_store_get_object(object TSRMLS_CC);
		/* not a pre-defined method, nor a user-defined method; check
		 * the driver specific methods */
		if (!stmt->dbh->cls_methods[PDO_DBH_DRIVER_METHOD_KIND_STMT]) {
			if (!pdo_hash_methods(stmt->dbh, 
				PDO_DBH_DRIVER_METHOD_KIND_STMT TSRMLS_CC)
				|| !stmt->dbh->cls_methods[PDO_DBH_DRIVER_METHOD_KIND_STMT]) {
				goto out;
			}
		}

		if (zend_hash_find(stmt->dbh->cls_methods[PDO_DBH_DRIVER_METHOD_KIND_STMT],
				lc_method_name, method_len+1, (void**)&fbc) == FAILURE) {
			fbc = NULL;
			goto out;
		}
		/* got it */
	}
	
out:
	efree(lc_method_name);
	return fbc;
}

static int dbstmt_compare(zval *object1, zval *object2 TSRMLS_DC)
{
	return -1;
}

static zend_object_value dbstmt_clone_obj(zval *zobject TSRMLS_DC)
{
	zend_object_value retval;
	zval *tmp;
	pdo_stmt_t *stmt;
	pdo_stmt_t *old_stmt;
	zend_object_handle handle = Z_OBJ_HANDLE_P(zobject);

	stmt = ecalloc(1, sizeof(*stmt));
	stmt->ce = Z_OBJCE_P(zobject);
	stmt->refcount = 1;
	ALLOC_HASHTABLE(stmt->properties);
	zend_hash_init(stmt->properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_copy(stmt->properties, &stmt->ce->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));

	old_stmt = (pdo_stmt_t *)zend_object_store_get_object(zobject TSRMLS_CC);
	
	retval.handle = zend_objects_store_put(stmt, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t)pdo_dbstmt_free_storage, (zend_objects_store_clone_t)dbstmt_clone_obj TSRMLS_CC);
	retval.handlers = Z_OBJ_HT_P(zobject);

	zend_objects_clone_members((zend_object *)stmt, retval, (zend_object *)old_stmt, handle TSRMLS_CC);
	
	zend_objects_store_add_ref(&old_stmt->database_object_handle TSRMLS_CC);
	stmt->database_object_handle = old_stmt->database_object_handle;
			
	return retval;
}

zend_object_handlers pdo_dbstmt_object_handlers;

void pdo_stmt_init(TSRMLS_D)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "PDOStatement", pdo_dbstmt_functions);
	pdo_dbstmt_ce = zend_register_internal_class(&ce TSRMLS_CC);
	pdo_dbstmt_ce->get_iterator = pdo_stmt_iter_get;
	pdo_dbstmt_ce->create_object = pdo_dbstmt_new;
	zend_class_implements(pdo_dbstmt_ce TSRMLS_CC, 1, zend_ce_traversable); 
	zend_declare_property_null(pdo_dbstmt_ce, "queryString", sizeof("queryString")-1, ZEND_ACC_PUBLIC TSRMLS_CC);

	memcpy(&pdo_dbstmt_object_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	pdo_dbstmt_object_handlers.write_property = dbstmt_prop_write;
	pdo_dbstmt_object_handlers.unset_property = dbstmt_prop_delete;
	pdo_dbstmt_object_handlers.get_method = dbstmt_method_get;
	pdo_dbstmt_object_handlers.compare_objects = dbstmt_compare;
	pdo_dbstmt_object_handlers.clone_obj = dbstmt_clone_obj;

	INIT_CLASS_ENTRY(ce, "PDORow", pdo_row_functions);
	pdo_row_ce = zend_register_internal_class(&ce TSRMLS_CC);
	pdo_row_ce->ce_flags |= ZEND_ACC_FINAL_CLASS; /* when removing this a lot of handlers need to be redone */
	pdo_row_ce->create_object = pdo_row_new;
}

static void free_statement(pdo_stmt_t *stmt TSRMLS_DC)
{
	if (stmt->properties) {
		zend_hash_destroy(stmt->properties);
		efree(stmt->properties);
		stmt->properties = NULL;
	}

	if (stmt->bound_params) {
		zend_hash_destroy(stmt->bound_params);
		FREE_HASHTABLE(stmt->bound_params);
		stmt->bound_params = NULL;
	}
	if (stmt->bound_param_map) {
		zend_hash_destroy(stmt->bound_param_map);
		FREE_HASHTABLE(stmt->bound_param_map);
		stmt->bound_param_map = NULL;
	}
	if (stmt->bound_columns) {
		zend_hash_destroy(stmt->bound_columns);
		FREE_HASHTABLE(stmt->bound_columns);
		stmt->bound_columns = NULL;
	}

	if (stmt->methods && stmt->methods->dtor) {
		stmt->methods->dtor(stmt TSRMLS_CC);
	}
	if (stmt->query_string) {
		efree(stmt->query_string);
	}

	if (stmt->columns) {
		int i;
		struct pdo_column_data *cols = stmt->columns;

		for (i = 0; i < stmt->column_count; i++) {
			if (cols[i].name) {
				efree(cols[i].name);
				cols[i].name = NULL;
			}
		}
		efree(stmt->columns);
		stmt->columns = NULL;
	}

	if (stmt->fetch.into && stmt->default_fetch_type == PDO_FETCH_INTO) {
		FREE_ZVAL(stmt->fetch.into);
		stmt->fetch.into = NULL;
	}
	
	do_fetch_opt_finish(stmt, 1 TSRMLS_CC);

	zend_objects_store_del_ref(&stmt->database_object_handle TSRMLS_CC);
	efree(stmt);
}

PDO_API void php_pdo_stmt_addref(pdo_stmt_t *stmt TSRMLS_DC)
{
	stmt->refcount++;
}

PDO_API void php_pdo_stmt_delref(pdo_stmt_t *stmt TSRMLS_DC)
{
	if (--stmt->refcount == 0) {
		free_statement(stmt TSRMLS_CC);
	}
}

void pdo_dbstmt_free_storage(pdo_stmt_t *stmt TSRMLS_DC)
{
	php_pdo_stmt_delref(stmt TSRMLS_CC);
}

zend_object_value pdo_dbstmt_new(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value retval;
	zval *tmp;

	pdo_stmt_t *stmt;
	stmt = emalloc(sizeof(*stmt));
	memset(stmt, 0, sizeof(*stmt));
	stmt->ce = ce;
	stmt->refcount = 1;
	ALLOC_HASHTABLE(stmt->properties);
	zend_hash_init(stmt->properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_copy(stmt->properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));

	retval.handle = zend_objects_store_put(stmt, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t)pdo_dbstmt_free_storage, (zend_objects_store_clone_t)dbstmt_clone_obj TSRMLS_CC);
	retval.handlers = &pdo_dbstmt_object_handlers;

	return retval;
}
/* }}} */

/* {{{ statement iterator */

struct php_pdo_iterator {
	zend_object_iterator iter;
	pdo_stmt_t *stmt;
	ulong key;
	zval *fetch_ahead;
};

static void pdo_stmt_iter_dtor(zend_object_iterator *iter TSRMLS_DC)
{
	struct php_pdo_iterator *I = (struct php_pdo_iterator*)iter->data;

	if (--I->stmt->refcount == 0) {
		free_statement(I->stmt TSRMLS_CC);
	}
		
	if (I->fetch_ahead) {
		zval_ptr_dtor(&I->fetch_ahead);
	}

	efree(I);
}

static int pdo_stmt_iter_valid(zend_object_iterator *iter TSRMLS_DC)
{
	struct php_pdo_iterator *I = (struct php_pdo_iterator*)iter->data;

	return I->fetch_ahead ? SUCCESS : FAILURE;
}

static void pdo_stmt_iter_get_data(zend_object_iterator *iter, zval ***data TSRMLS_DC)
{
	struct php_pdo_iterator *I = (struct php_pdo_iterator*)iter->data;

	/* sanity */
	if (!I->fetch_ahead) {
		*data = NULL;
		return;
	}

	*data = &I->fetch_ahead;
}

static int pdo_stmt_iter_get_key(zend_object_iterator *iter, char **str_key, uint *str_key_len,
	ulong *int_key TSRMLS_DC)
{
	struct php_pdo_iterator *I = (struct php_pdo_iterator*)iter->data;

	if (I->key == (ulong)-1) {
		return HASH_KEY_NON_EXISTANT;
	}
	*int_key = I->key;
	return HASH_KEY_IS_LONG;
}

static void pdo_stmt_iter_move_forwards(zend_object_iterator *iter TSRMLS_DC)
{
	struct php_pdo_iterator *I = (struct php_pdo_iterator*)iter->data;

	if (I->fetch_ahead) {
		zval_ptr_dtor(&I->fetch_ahead);
		I->fetch_ahead = NULL;
	}

	MAKE_STD_ZVAL(I->fetch_ahead);

	if (!do_fetch(I->stmt, TRUE, I->fetch_ahead, PDO_FETCH_USE_DEFAULT,
			PDO_FETCH_ORI_NEXT, 0, 0 TSRMLS_CC)) {
		pdo_stmt_t *stmt = I->stmt; /* for PDO_HANDLE_STMT_ERR() */

		PDO_HANDLE_STMT_ERR();
		I->key = (ulong)-1;
		FREE_ZVAL(I->fetch_ahead);
		I->fetch_ahead = NULL;

		return;
	}

	I->key++;
}

static zend_object_iterator_funcs pdo_stmt_iter_funcs = {
	pdo_stmt_iter_dtor,
	pdo_stmt_iter_valid,
	pdo_stmt_iter_get_data,
	pdo_stmt_iter_get_key,
	pdo_stmt_iter_move_forwards,
	NULL
};

zend_object_iterator *pdo_stmt_iter_get(zend_class_entry *ce, zval *object TSRMLS_DC)
{
	pdo_stmt_t *stmt = (pdo_stmt_t*)zend_object_store_get_object(object TSRMLS_CC);
	struct php_pdo_iterator *I;

	I = ecalloc(1, sizeof(*I));
	I->iter.funcs = &pdo_stmt_iter_funcs;
	I->iter.data = I;
	I->stmt = stmt;
	stmt->refcount++;

	MAKE_STD_ZVAL(I->fetch_ahead);
	if (!do_fetch(I->stmt, TRUE, I->fetch_ahead, PDO_FETCH_USE_DEFAULT,
			PDO_FETCH_ORI_NEXT, 0, 0 TSRMLS_CC)) {
		PDO_HANDLE_STMT_ERR();
		I->key = (ulong)-1;
		FREE_ZVAL(I->fetch_ahead);
		I->fetch_ahead = NULL;
	}

	return &I->iter;
}

/* }}} */

/* {{{ overloaded handlers for PDORow class (used by PDO_FETCH_LAZY) */

zend_function_entry pdo_row_functions[] = {
	{NULL, NULL, NULL}
};

static zval *row_prop_or_dim_read(zval *object, zval *member, int type TSRMLS_DC)
{
	zval *return_value;
	pdo_stmt_t * stmt = (pdo_stmt_t *) zend_object_store_get_object(object TSRMLS_CC);
	int colno = -1;

	MAKE_STD_ZVAL(return_value);
		
	if (Z_TYPE_P(member) == IS_LONG) {
		if (Z_LVAL_P(member) >= 0 && Z_LVAL_P(member) < stmt->column_count) {
			fetch_value(stmt, return_value, Z_LVAL_P(member), NULL TSRMLS_CC);
		}
	} else {
		convert_to_string(member);
		/* TODO: replace this with a hash of available column names to column
		 * numbers */
		for (colno = 0; colno < stmt->column_count; colno++) {
			if (strcmp(stmt->columns[colno].name, Z_STRVAL_P(member)) == 0) {
				fetch_value(stmt, return_value, colno, NULL TSRMLS_CC);
				break;
			}
		}
	}

	return_value->refcount = 0;
	return_value->is_ref = 0;
	
	return return_value;
}

static void row_prop_or_dim_write(zval *object, zval *member, zval *value TSRMLS_DC)
{
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "This PDORow is not from a writable result set");
}

static int row_prop_or_dim_exists(zval *object, zval *member, int check_empty TSRMLS_DC)
{
	pdo_stmt_t * stmt = (pdo_stmt_t *) zend_object_store_get_object(object TSRMLS_CC);
	int colno = -1;

	if (Z_TYPE_P(member) == IS_LONG) {
		return Z_LVAL_P(member) >= 0 && Z_LVAL_P(member) < stmt->column_count;
	} else {
		convert_to_string(member);

		/* TODO: replace this with a hash of available column names to column
		 * numbers */
		for (colno = 0; colno < stmt->column_count; colno++) {
			if (strcmp(stmt->columns[colno].name, Z_STRVAL_P(member)) == 0) {
				return 1;
			}
		}
	}

	return 0;
}

static void row_prop_or_dim_delete(zval *object, zval *offset TSRMLS_DC)
{
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot delete properties from a PDORow");
}

static HashTable *row_get_properties(zval *object TSRMLS_DC)
{
	zval *tmp;
	pdo_stmt_t * stmt = (pdo_stmt_t *) zend_object_store_get_object(object TSRMLS_CC);
	int i;
	HashTable *ht;

	MAKE_STD_ZVAL(tmp);
	array_init(tmp);

	for (i = 0; i < stmt->column_count; i++) {
		zval *val;
		MAKE_STD_ZVAL(val);
		fetch_value(stmt, val, i, NULL TSRMLS_CC);

		add_assoc_zval(tmp, stmt->columns[i].name, val);
	}

	ht = Z_ARRVAL_P(tmp);

	ZVAL_NULL(tmp);
	FREE_ZVAL(tmp);

	return ht;
}

static union _zend_function *row_method_get(
#if PHP_API_VERSION >= 20041225
	zval **object_pp,
#else
	zval *object,
#endif
	char *method_name, int method_len TSRMLS_DC)
{
	zend_function *fbc;
	char *lc_method_name;

	lc_method_name = emalloc(method_len + 1);
	zend_str_tolower_copy(lc_method_name, method_name, method_len);

	if (zend_hash_find(&pdo_row_ce->function_table, lc_method_name, method_len+1, (void**)&fbc) == FAILURE) {
		efree(lc_method_name);
		return NULL;
	}
	
	efree(lc_method_name);
	return fbc;
}

static int row_call_method(char *method, INTERNAL_FUNCTION_PARAMETERS)
{
	return FAILURE;
}

static union _zend_function *row_get_ctor(zval *object TSRMLS_DC)
{
	static zend_internal_function ctor = {0};

	ctor.type = ZEND_INTERNAL_FUNCTION;
	ctor.function_name = "__construct";
	ctor.scope = pdo_row_ce;
	ctor.handler = ZEND_FN(dbstmt_constructor);

	return (union _zend_function*)&ctor;
}

static zend_class_entry *row_get_ce(zval *object TSRMLS_DC)
{
	return pdo_dbstmt_ce;
}

static int row_get_classname(zval *object, char **class_name, zend_uint *class_name_len, int parent TSRMLS_DC)
{
	*class_name = estrndup("PDORow", sizeof("PDORow")-1);
	*class_name_len = sizeof("PDORow")-1;
	return 0;
}

static int row_compare(zval *object1, zval *object2 TSRMLS_DC)
{
	return -1;
}

zend_object_handlers pdo_row_object_handlers = {
	ZEND_OBJECTS_STORE_HANDLERS,
	row_prop_or_dim_read,
	row_prop_or_dim_write,
	row_prop_or_dim_read,
	row_prop_or_dim_write,
	NULL,
	NULL,
	NULL,
	row_prop_or_dim_exists,
	row_prop_or_dim_delete,
	row_prop_or_dim_exists,
	row_prop_or_dim_delete,
	row_get_properties,
	row_method_get,
	row_call_method,
	row_get_ctor,
	row_get_ce,
	row_get_classname,
	row_compare,
	NULL, /* cast */
	NULL
};

void pdo_row_free_storage(pdo_stmt_t *stmt TSRMLS_DC)
{
	ZVAL_NULL(&stmt->lazy_object_ref);
	
	if (--stmt->refcount == 0) {
		free_statement(stmt TSRMLS_CC);
	}
}

zend_object_value pdo_row_new(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value retval;

	retval.handle = zend_objects_store_put(NULL, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t)pdo_row_free_storage, NULL TSRMLS_CC);
	retval.handlers = &pdo_row_object_handlers;

	return retval;
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
