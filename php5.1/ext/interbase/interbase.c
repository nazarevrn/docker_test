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
   | Authors: Jouni Ahto <jouni.ahto@exdec.fi>                            |
   |          Andrew Avdeev <andy@rsc.mv.ru>                              |
   |          Ard Biesheuvel <a.k.biesheuvel@ewi.tudelft.nl>              |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define _GNU_SOURCE

#include "php.h"

#if HAVE_IBASE

#include "php_ini.h"
#include "ext/standard/php_standard.h"
#include "ext/standard/md5.h"
#include "php_interbase.h"
#include "php_ibase_includes.h"
#include "SAPI.h"

#include <time.h>

#define ROLLBACK		0
#define COMMIT			1
#define RETAIN			2

/* {{{ extension definition structures */
zend_function_entry ibase_functions[] = {
	PHP_FE(ibase_connect, NULL)
	PHP_FE(ibase_pconnect, NULL)
	PHP_FE(ibase_close, NULL)
	PHP_FE(ibase_drop_db, NULL)
	PHP_FE(ibase_query, NULL)
	PHP_FE(ibase_fetch_row, NULL)
	PHP_FE(ibase_fetch_assoc, NULL)
	PHP_FE(ibase_fetch_object, NULL)
	PHP_FE(ibase_free_result, NULL)
	PHP_FE(ibase_name_result, NULL)
	PHP_FE(ibase_prepare, NULL)
	PHP_FE(ibase_execute, NULL)
	PHP_FE(ibase_free_query, NULL)
	PHP_FE(ibase_gen_id, NULL)
	PHP_FE(ibase_num_fields, NULL)
	PHP_FE(ibase_num_params, NULL)
#if abies_0
	PHP_FE(ibase_num_rows, NULL)
#endif
	PHP_FE(ibase_affected_rows, NULL)
	PHP_FE(ibase_field_info, NULL)
	PHP_FE(ibase_param_info, NULL)

	PHP_FE(ibase_trans, NULL)
	PHP_FE(ibase_commit, NULL)
	PHP_FE(ibase_rollback, NULL)
	PHP_FE(ibase_commit_ret, NULL)
	PHP_FE(ibase_rollback_ret, NULL)

	PHP_FE(ibase_blob_info, NULL)
	PHP_FE(ibase_blob_create, NULL)
	PHP_FE(ibase_blob_add, NULL)
	PHP_FE(ibase_blob_cancel, NULL)
	PHP_FE(ibase_blob_close, NULL)
	PHP_FE(ibase_blob_open, NULL)
	PHP_FE(ibase_blob_get, NULL)
	PHP_FE(ibase_blob_echo, NULL)
	PHP_FE(ibase_blob_import, NULL)
	PHP_FE(ibase_errmsg, NULL)
	PHP_FE(ibase_errcode, NULL)

	PHP_FE(ibase_add_user, NULL)
	PHP_FE(ibase_modify_user, NULL)
	PHP_FE(ibase_delete_user, NULL)

	PHP_FE(ibase_service_attach, NULL)
	PHP_FE(ibase_service_detach, NULL)
	PHP_FE(ibase_backup, NULL)
	PHP_FE(ibase_restore, NULL)
	PHP_FE(ibase_maintain_db, NULL)
	PHP_FE(ibase_db_info, NULL)
	PHP_FE(ibase_server_info, NULL)

	PHP_FE(ibase_wait_event, NULL)
	PHP_FE(ibase_set_event_handler, NULL)
	PHP_FE(ibase_free_event_handler, NULL)

	/**
	* These aliases are provided in order to maintain forward compatibility. As Firebird
	* and InterBase are developed independently, functionality might be different between
	* the two branches in future versions.
	* Firebird users should use the aliases, so future InterBase-specific changes will
	* not affect their code
	*/
	PHP_FALIAS(fbird_connect,ibase_connect, NULL)
	PHP_FALIAS(fbird_pconnect,ibase_pconnect, NULL)
	PHP_FALIAS(fbird_close,ibase_close, NULL)
	PHP_FALIAS(fbird_drop_db,ibase_drop_db, NULL)
	PHP_FALIAS(fbird_query,ibase_query, NULL)
	PHP_FALIAS(fbird_fetch_row,ibase_fetch_row, NULL)
	PHP_FALIAS(fbird_fetch_assoc,ibase_fetch_assoc, NULL)
	PHP_FALIAS(fbird_fetch_object,ibase_fetch_object, NULL)
	PHP_FALIAS(fbird_free_result,ibase_free_result, NULL)
	PHP_FALIAS(fbird_name_result,ibase_name_result, NULL)
	PHP_FALIAS(fbird_prepare,ibase_prepare, NULL)
	PHP_FALIAS(fbird_execute,ibase_execute, NULL)
	PHP_FALIAS(fbird_free_query,ibase_free_query, NULL)
	PHP_FALIAS(fbird_gen_id,ibase_gen_id, NULL)
	PHP_FALIAS(fbird_num_fields,ibase_num_fields, NULL)
	PHP_FALIAS(fbird_num_params,ibase_num_params, NULL)
#if abies_0
	PHP_FALIAS(fbird_num_rows,ibase_num_rows, NULL)
#endif
	PHP_FALIAS(fbird_affected_rows,ibase_affected_rows, NULL)
	PHP_FALIAS(fbird_field_info,ibase_field_info, NULL)
	PHP_FALIAS(fbird_param_info,ibase_param_info, NULL)

	PHP_FALIAS(fbird_trans,ibase_trans, NULL)
	PHP_FALIAS(fbird_commit,ibase_commit, NULL)
	PHP_FALIAS(fbird_rollback,ibase_rollback, NULL)
	PHP_FALIAS(fbird_commit_ret,ibase_commit_ret, NULL)
	PHP_FALIAS(fbird_rollback_ret,ibase_rollback_ret, NULL)

	PHP_FALIAS(fbird_blob_info,ibase_blob_info, NULL)
	PHP_FALIAS(fbird_blob_create,ibase_blob_create, NULL)
	PHP_FALIAS(fbird_blob_add,ibase_blob_add, NULL)
	PHP_FALIAS(fbird_blob_cancel,ibase_blob_cancel, NULL)
	PHP_FALIAS(fbird_blob_close,ibase_blob_close, NULL)
	PHP_FALIAS(fbird_blob_open,ibase_blob_open, NULL)
	PHP_FALIAS(fbird_blob_get,ibase_blob_get, NULL)
	PHP_FALIAS(fbird_blob_echo,ibase_blob_echo, NULL)
	PHP_FALIAS(fbird_blob_import,ibase_blob_import, NULL)
	PHP_FALIAS(fbird_errmsg,ibase_errmsg, NULL)
	PHP_FALIAS(fbird_errcode,ibase_errcode, NULL)

	PHP_FALIAS(fbird_add_user,ibase_add_user, NULL)
	PHP_FALIAS(fbird_modify_user,ibase_modify_user, NULL)
	PHP_FALIAS(fbird_delete_user,ibase_delete_user, NULL)

	PHP_FALIAS(fbird_service_attach,ibase_service_attach, NULL)
	PHP_FALIAS(fbird_service_detach,ibase_service_detach, NULL)
	PHP_FALIAS(fbird_backup,ibase_backup, NULL)
	PHP_FALIAS(fbird_restore,ibase_restore, NULL)
	PHP_FALIAS(fbird_maintain_db,ibase_maintain_db, NULL)
	PHP_FALIAS(fbird_db_info,ibase_db_info, NULL)
	PHP_FALIAS(fbird_server_info,ibase_server_info, NULL)

	PHP_FALIAS(fbird_wait_event,ibase_wait_event, NULL)
	PHP_FALIAS(fbird_set_event_handler,ibase_set_event_handler, NULL)
	PHP_FALIAS(fbird_free_event_handler,ibase_free_event_handler, NULL)
	{NULL, NULL, NULL}
};

zend_module_entry ibase_module_entry = {
	STANDARD_MODULE_HEADER,
	"interbase",
	ibase_functions,
	PHP_MINIT(ibase),
	PHP_MSHUTDOWN(ibase),
	NULL,
	PHP_RSHUTDOWN(ibase),
	PHP_MINFO(ibase),
	NO_VERSION_YET,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_INTERBASE
ZEND_GET_MODULE(ibase)
#endif

/* True globals, no need for thread safety */
int le_link, le_plink, le_trans;

ZEND_DECLARE_MODULE_GLOBALS(ibase)

/* }}} */

/* error handling ---------------------------- */

/* {{{ proto string ibase_errmsg(void) 
   Return error message */
PHP_FUNCTION(ibase_errmsg)
{
	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if (IBG(sql_code) != 0) {
		RETURN_STRING(IBG(errmsg), 1);
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ proto int ibase_errcode(void) 
   Return error code */
PHP_FUNCTION(ibase_errcode)
{
	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if (IBG(sql_code) != 0) {
		RETURN_LONG(IBG(sql_code));
	}
	RETURN_FALSE;
}
/* }}} */

/* print interbase error and save it for ibase_errmsg() */
void _php_ibase_error(TSRMLS_D) /* {{{ */
{
	char *s = IBG(errmsg);
	ISC_STATUS *statusp = IB_STATUS;

	IBG(sql_code) = isc_sqlcode(IB_STATUS);
	
	while ((s - IBG(errmsg)) < MAX_ERRMSG - (IBASE_MSGSIZE + 2) && isc_interprete(s, &statusp)) {
		strcat(IBG(errmsg), " ");
		s = IBG(errmsg) + strlen(IBG(errmsg));
	}

	php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", IBG(errmsg));
}
/* }}} */

/* print php interbase module error and save it for ibase_errmsg() */
void _php_ibase_module_error(char *msg TSRMLS_DC, ...) /* {{{ */
{
	va_list ap;

#ifdef ZTS
	va_start(ap, TSRMLS_C);
#else
	va_start(ap, msg);
#endif

	/* vsnprintf NUL terminates the buf and writes at most n-1 chars+NUL */
	vsnprintf(IBG(errmsg), MAX_ERRMSG, msg, ap);
	va_end(ap);

	IBG(sql_code) = -999; /* no SQL error */

	php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", IBG(errmsg));
}
/* }}} */

/* {{{ internal macros, functions and structures */
typedef struct {
	isc_db_handle *db_ptr;
	long tpb_len;
	char *tpb_ptr;
} ISC_TEB;

/* }}} */

/* Fill ib_link and trans with the correct database link and transaction. */
void _php_ibase_get_link_trans(INTERNAL_FUNCTION_PARAMETERS, /* {{{ */
	zval **link_id, ibase_db_link **ib_link, ibase_trans **trans)
{
	int type;

	IBDEBUG("Transaction or database link?");
	if (zend_list_find(Z_LVAL_PP(link_id), &type)) {
	 	if (type == le_trans) {
			/* Transaction resource: make sure it refers to one link only, then 
			   fetch it; database link is stored in ib_trans->db_link[]. */
			IBDEBUG("Type is le_trans");
			ZEND_FETCH_RESOURCE(*trans, ibase_trans *, link_id, -1, LE_TRANS, le_trans);
			if ((*trans)->link_cnt > 1) {
				_php_ibase_module_error("Link id is ambiguous: transaction spans multiple connections."
					TSRMLS_CC);
				return;
			}				
			*ib_link = (*trans)->db_link[0];
			return;
		}
	} 
	IBDEBUG("Type is le_[p]link or id not found");
	/* Database link resource, use default transaction. */
	*trans = NULL;
	ZEND_FETCH_RESOURCE2(*ib_link, ibase_db_link *, link_id, -1, LE_LINK, le_link, le_plink);
}
/* }}} */	

/* destructors ---------------------- */

static void _php_ibase_commit_link(ibase_db_link *link TSRMLS_DC) /* {{{ */
{
	unsigned short i = 0, j;
	ibase_tr_list *l;
	ibase_event *e;
	IBDEBUG("Checking transactions to close...");

	for (l = link->tr_list; l != NULL; ++i) {
		ibase_tr_list *p = l;
		if (p->trans != NULL) {
			if (i == 0) {
				if (p->trans->handle != NULL) {
					IBDEBUG("Committing default transaction...");
					if (isc_commit_transaction(IB_STATUS, &p->trans->handle)) {
						_php_ibase_error(TSRMLS_C);
					}
				}
				efree(p->trans); /* default transaction is not a registered resource: clean up */
			} else {
				if (p->trans->handle != NULL) { 
					/* non-default trans might have been rolled back by other call of this dtor */
					IBDEBUG("Rolling back other transactions...");
					if (isc_rollback_transaction(IB_STATUS, &p->trans->handle)) {
						_php_ibase_error(TSRMLS_C);
					}
				}
				/* set this link pointer to NULL in the transaction */
				for (j = 0; j < p->trans->link_cnt; ++j) {
					if (p->trans->db_link[j] == link) {
						p->trans->db_link[j] = NULL;
						break;
					}
				}
			}
		}
		l = l->next;
		efree(p);
	}
	link->tr_list = NULL;
	
	for (e = link->event_head; e; e = e->event_next) {
		_php_ibase_free_event(e TSRMLS_CC);
		e->link = NULL;
	}
}

/* }}} */

static void php_ibase_commit_link_rsrc(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */
{
	ibase_db_link *link = (ibase_db_link *) rsrc->ptr;

	_php_ibase_commit_link(link TSRMLS_CC);
}
/* }}} */

static void _php_ibase_close_link(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */
{
	ibase_db_link *link = (ibase_db_link *) rsrc->ptr;

	_php_ibase_commit_link(link TSRMLS_CC);
	if (link->handle != NULL) {
		IBDEBUG("Closing normal link...");
		isc_detach_database(IB_STATUS, &link->handle);
	}
	IBG(num_links)--;
	efree(link);
}
/* }}} */

static void _php_ibase_close_plink(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */
{
	ibase_db_link *link = (ibase_db_link *) rsrc->ptr;

	_php_ibase_commit_link(link TSRMLS_CC);
	IBDEBUG("Closing permanent link...");
	if (link->handle != NULL) {
		isc_detach_database(IB_STATUS, &link->handle);
	}
	IBG(num_persistent)--;
	IBG(num_links)--;
	free(link);
}
/* }}} */

static void _php_ibase_free_trans(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */
{
	ibase_trans *trans = (ibase_trans *)rsrc->ptr;
	unsigned short i;
	
	IBDEBUG("Cleaning up transaction resource...");
	if (trans->handle != NULL) {
		IBDEBUG("Rolling back unhandled transaction...");
		if (isc_rollback_transaction(IB_STATUS, &trans->handle)) {
			_php_ibase_error(TSRMLS_C);
		}
	}

	/* now remove this transaction from all the connection-transaction lists */
	for (i = 0; i < trans->link_cnt; ++i) {
		if (trans->db_link[i] != NULL) {
			ibase_tr_list **l;
			for (l = &trans->db_link[i]->tr_list; *l != NULL; l = &(*l)->next) {
				if ( (*l)->trans == trans) {
					ibase_tr_list *p = *l;
					*l = p->next;
					efree(p);
					break;
				}
			}
		}
	}
	efree(trans);
}
/* }}} */

/* TODO this function should be part of either Zend or PHP API */
static PHP_INI_DISP(php_ibase_password_displayer_cb)
{
	TSRMLS_FETCH();

	if ((type == PHP_INI_DISPLAY_ORIG && ini_entry->orig_value) 
			|| (type == PHP_INI_DISPLAY_ACTIVE && ini_entry->value)) {
		PUTS("********");
	} else if (!sapi_module.phpinfo_as_text) {
		PUTS("<i>no value</i>");
	} else {
		PUTS("no value");
	}
}

/* {{{ startup, shutdown and info functions */
PHP_INI_BEGIN()
	PHP_INI_ENTRY_EX("ibase.allow_persistent", "1", PHP_INI_SYSTEM, NULL, zend_ini_boolean_displayer_cb)
	PHP_INI_ENTRY_EX("ibase.max_persistent", "-1", PHP_INI_SYSTEM, NULL, display_link_numbers)
	PHP_INI_ENTRY_EX("ibase.max_links", "-1", PHP_INI_SYSTEM, NULL, display_link_numbers)
	PHP_INI_ENTRY("ibase.default_db", NULL, PHP_INI_SYSTEM, NULL)
	PHP_INI_ENTRY("ibase.default_user", NULL, PHP_INI_ALL, NULL)
	PHP_INI_ENTRY_EX("ibase.default_password", NULL, PHP_INI_ALL, NULL, php_ibase_password_displayer_cb)
	PHP_INI_ENTRY("ibase.default_charset", NULL, PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("ibase.timestampformat", IB_DEF_DATE_FMT " " IB_DEF_TIME_FMT, PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("ibase.dateformat", IB_DEF_DATE_FMT, PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("ibase.timeformat", IB_DEF_TIME_FMT, PHP_INI_ALL, NULL)
PHP_INI_END()

static void php_ibase_init_globals(zend_ibase_globals *ibase_globals)
{
	ibase_globals->num_persistent = ibase_globals->num_links = 0;
	ibase_globals->sql_code = *ibase_globals->errmsg = 0;
	ibase_globals->default_link = -1;
}

PHP_MINIT_FUNCTION(ibase)
{
	ZEND_INIT_MODULE_GLOBALS(ibase, php_ibase_init_globals, NULL);

	REGISTER_INI_ENTRIES();

	le_link = zend_register_list_destructors_ex(_php_ibase_close_link, NULL, LE_LINK, module_number);
	le_plink = zend_register_list_destructors_ex(php_ibase_commit_link_rsrc, _php_ibase_close_plink, LE_PLINK, module_number);
	le_trans = zend_register_list_destructors_ex(_php_ibase_free_trans, NULL, LE_TRANS, module_number);

	REGISTER_LONG_CONSTANT("IBASE_DEFAULT", PHP_IBASE_DEFAULT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_CREATE", PHP_IBASE_CREATE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_TEXT", PHP_IBASE_FETCH_BLOBS, CONST_PERSISTENT); /* deprecated, for BC only */
	REGISTER_LONG_CONSTANT("IBASE_FETCH_BLOBS", PHP_IBASE_FETCH_BLOBS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_FETCH_ARRAYS", PHP_IBASE_FETCH_ARRAYS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_UNIXTIME", PHP_IBASE_UNIXTIME, CONST_PERSISTENT);

	/* transactions */
	REGISTER_LONG_CONSTANT("IBASE_WRITE", PHP_IBASE_WRITE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_READ", PHP_IBASE_READ, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_COMMITTED", PHP_IBASE_COMMITTED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_CONSISTENCY", PHP_IBASE_CONSISTENCY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_CONCURRENCY", PHP_IBASE_CONCURRENCY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_REC_VERSION", PHP_IBASE_REC_VERSION, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_REC_NO_VERSION", PHP_IBASE_REC_NO_VERSION, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_NOWAIT", PHP_IBASE_NOWAIT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_WAIT", PHP_IBASE_WAIT, CONST_PERSISTENT);

	php_ibase_query_minit(INIT_FUNC_ARGS_PASSTHRU);
	php_ibase_blobs_minit(INIT_FUNC_ARGS_PASSTHRU);
	php_ibase_events_minit(INIT_FUNC_ARGS_PASSTHRU);
	php_ibase_service_minit(INIT_FUNC_ARGS_PASSTHRU);
	
	return SUCCESS;          
}                            
                             
PHP_MSHUTDOWN_FUNCTION(ibase)
{
#ifndef PHP_WIN32
	/**
	 * When the Interbase client API library libgds.so is first loaded, it registers a call to 
	 * gds__cleanup() with atexit(), in order to clean up after itself when the process exits.
	 * This means that the library is called at process shutdown, and cannot be unloaded beforehand.
	 * PHP tries to unload modules after every request [dl()'ed modules], and right before the 
	 * process shuts down [modules loaded from php.ini]. This results in a segfault for this module.
	 * By NULLing the dlopen() handle in the module entry, Zend omits the call to dlclose(),
	 * ensuring that the module will remain present until the process exits. However, the functions
	 * and classes exported by the module will not be available until the module is 'reloaded'. 
	 * When reloaded, dlopen() will return the handle of the already loaded module. The module will
	 * be unloaded automatically when the process exits.
	 */
	zend_module_entry *ibase_entry;
	if (SUCCESS == zend_hash_find(&module_registry, ibase_module_entry.name,
			strlen(ibase_module_entry.name) +1, (void*) &ibase_entry)) {
		ibase_entry->handle = NULL;
	}
#endif
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ibase)
{
	IBG(num_links) = IBG(num_persistent);
	IBG(default_link)= -1;

	RESET_ERRMSG;

	return SUCCESS;
} 
 
PHP_MINFO_FUNCTION(ibase)
{
	char tmp[64], *s;

	php_info_print_table_start();
	php_info_print_table_row(2, "Firebird/InterBase Support", 
#ifdef COMPILE_DL_INTERBASE
		"dynamic");
#else
		"static");
#endif

#ifdef FB_API_VER
	sprintf( (s = tmp), "Firebird API version %d", FB_API_VER);
#elif (SQLDA_CURRENT_VERSION > 1)
	s =  "Interbase 7.0 and up";
#elif !defined(DSC_null)
	s = "Interbase 6";
#else
	s = "Firebird 1.0";
#endif
	php_info_print_table_row(2, "Compile-time Client Library Version", s);

#if defined(__GNUC__) || defined(PHP_WIN32)
	do {
		info_func_t info_func = NULL;
#ifdef __GNUC__
		info_func = (info_func_t)dlsym(RTLD_DEFAULT, "isc_get_client_version");
#else
		HMODULE l = GetModuleHandle("fbclient");

		if (!l && !(l = GetModuleHandle("gds32"))) {
			break;
		}
		info_func = (info_func_t)GetProcAddress(l, "isc_get_client_version");
#endif
		if (info_func) {
			info_func(s = tmp);
		} else {
			s = "Firebird 1.0/Interbase 6";
		}
		php_info_print_table_row(2, "Run-time Client Library Version", s);
	} while (0);
#endif			
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();

}
/* }}} */

enum connect_args { DB = 0, USER = 1, PASS = 2, CSET = 3, ROLE = 4, BUF = 0, DLECT = 1, SYNC = 2 };
	
static char const dpb_args[] = { 
	0, isc_dpb_user_name, isc_dpb_password, isc_dpb_lc_ctype, isc_dpb_sql_role_name, 0
};
	
int _php_ibase_attach_db(char **args, int *len, long *largs, isc_db_handle *db TSRMLS_DC)
{
	short i;
	char dpb_buffer[256] = { isc_dpb_version1 }, *dpb;

	dpb = dpb_buffer + 1;

	for (i = 0; i < sizeof(dpb_args); ++i) {
		if (dpb_args[i] && args[i] && len[i]) {
			dpb += sprintf(dpb, "%c%c%s", dpb_args[i],(unsigned char)len[i],args[i]);
		}
	}
	if (largs[BUF]) {
		dpb += sprintf(dpb, "%c\2%c%c", isc_dpb_num_buffers, 
			(char)(largs[BUF] >> 8), (char)(largs[BUF] & 0xff));
	}
	if (largs[SYNC]) {
		dpb += sprintf(dpb, "%c\1%c", isc_dpb_force_write, largs[SYNC] == isc_spb_prp_wm_sync ? 1 : 0);
	}
	if (isc_attach_database(IB_STATUS, (short)len[DB], args[DB], db, (short)(dpb-dpb_buffer), dpb_buffer)) {
		_php_ibase_error(TSRMLS_C);
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

static void _php_ibase_connect(INTERNAL_FUNCTION_PARAMETERS, int persistent) /* {{{ */
{
	char *c, hash[16], *args[] = { NULL, NULL, NULL, NULL, NULL };
	int i, len[] = { 0, 0, 0, 0, 0 };
	long largs[] = { 0, 0, 0 };
	PHP_MD5_CTX hash_context;
	zend_rsrc_list_entry new_index_ptr, *le;
	isc_db_handle db_handle = NULL;
	ibase_db_link *ib_link;

	RESET_ERRMSG;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ssssllsl",
			&args[DB], &len[DB], &args[USER], &len[USER], &args[PASS], &len[PASS],
			&args[CSET], &len[CSET], &largs[BUF], &largs[DLECT], &args[ROLE], &len[ROLE],
			&largs[SYNC])) {
		RETURN_FALSE;
	}
	
	/* restrict to the server/db in the .ini if in safe mode */
	if ((!len[DB] || PG(sql_safe_mode)) && (c = INI_STR("ibase.default_db"))) { 
		args[DB] = c;
		len[DB] = strlen(c);
	}
	if (!len[USER] && (c = INI_STR("ibase.default_user"))) {
		args[USER] = c;
		len[USER] = strlen(c);
	}
	if (!len[PASS] && (c = INI_STR("ibase.default_password"))) {
		args[PASS] = c;
		len[PASS] = strlen(c);
	}
	if (!len[CSET] && (c = INI_STR("ibase.default_charset"))) {
		args[CSET] = c;
		len[CSET] = strlen(c);
	}
	
	/* don't want usernames and passwords floating around */
	PHP_MD5Init(&hash_context);
	for (i = 0; i < sizeof(args)/sizeof(char*); ++i) {
		PHP_MD5Update(&hash_context,args[i],len[i]);
	}
	for (i = 0; i < sizeof(largs)/sizeof(long); ++i) {
		PHP_MD5Update(&hash_context,(char*)&largs[i],sizeof(long));
	}
	PHP_MD5Final(hash, &hash_context);
	
	/* try to reuse a connection */
	if (SUCCESS == zend_hash_find(&EG(regular_list), hash, sizeof(hash), (void *) &le)) {
		long xlink;
		int type;

		if (Z_TYPE_P(le) != le_index_ptr) {
			RETURN_FALSE;
		}
			
		xlink = (long) le->ptr;
		if (zend_list_find(xlink, &type) && ((!persistent && type == le_link) || type == le_plink)) {
			zend_list_addref(xlink);
			RETURN_RESOURCE(IBG(default_link) = xlink);
		} else {
			zend_hash_del(&EG(regular_list), hash, sizeof(hash));
		}
	}		

	/* ... or a persistent one */
	switch (zend_hash_find(&EG(persistent_list), hash, sizeof(hash), (void *) &le)) {
		long l;
		
		static char info[] = { isc_info_base_level, isc_info_end };
		char result[8];
		ISC_STATUS status[20];

	case SUCCESS:

		if (Z_TYPE_P(le) != le_plink) {
			RETURN_FALSE;
		}
		/* check if connection has timed out */
		ib_link = (ibase_db_link *) le->ptr;
		if (!isc_database_info(status, &ib_link->handle, sizeof(info), info, sizeof(result), result)) {
			ZEND_REGISTER_RESOURCE(return_value, ib_link, le_plink);
			break;
		}
		zend_hash_del(&EG(persistent_list), hash, sizeof(hash));
	
	default:

		/* no link found, so we have to open one */
	
		if ((l = INI_INT("ibase.max_links")) != -1 && IBG(num_links) >= l) {
			_php_ibase_module_error("Too many open links (%ld)" TSRMLS_CC, IBG(num_links));
			RETURN_FALSE;
		}
	
		/* create the ib_link */
		if (FAILURE == _php_ibase_attach_db(args, len, largs, &db_handle TSRMLS_CC)) {
			RETURN_FALSE;
		}
	
		/* use non-persistent if allowed number of persistent links is exceeded */
		if (!persistent || ((l = INI_INT("ibase.max_persistent") != -1) && IBG(num_persistent) >= l)) {
			ib_link = (ibase_db_link *) emalloc(sizeof(ibase_db_link));
			ZEND_REGISTER_RESOURCE(return_value, ib_link, le_link);
		} else {
			zend_rsrc_list_entry new_le;
			
			ib_link = (ibase_db_link *) malloc(sizeof(ibase_db_link));
	
			/* hash it up */
			Z_TYPE(new_le) = le_plink;
			new_le.ptr = ib_link;
			if (FAILURE == zend_hash_update(&EG(persistent_list), hash, sizeof(hash),
					(void *) &new_le, sizeof(zend_rsrc_list_entry), NULL)) {
				free(ib_link);
				RETURN_FALSE;
			}
			ZEND_REGISTER_RESOURCE(return_value, ib_link, le_plink);
			++IBG(num_persistent);
		}
		ib_link->handle = db_handle;
		ib_link->dialect = largs[DLECT] ? (unsigned short)largs[DLECT] : SQL_DIALECT_CURRENT;
		ib_link->tr_list = NULL;
		ib_link->event_head = NULL;
	
		++IBG(num_links);
	}

	/* add it to the hash */
	new_index_ptr.ptr = (void *) Z_LVAL_P(return_value);
	Z_TYPE(new_index_ptr) = le_index_ptr;
	if (FAILURE == zend_hash_update(&EG(regular_list), hash, sizeof(hash),
			(void *) &new_index_ptr, sizeof(zend_rsrc_list_entry), NULL)) {
		RETURN_FALSE;
	}
	zend_list_addref(IBG(default_link) = Z_LVAL_P(return_value));
}
/* }}} */

/* {{{ proto resource ibase_connect(string database [, string username [, string password [, string charset [, int buffers [, int dialect [, string role]]]]]])
   Open a connection to an InterBase database */
PHP_FUNCTION(ibase_connect)
{
	_php_ibase_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto resource ibase_pconnect(string database [, string username [, string password [, string charset [, int buffers [, int dialect [, string role]]]]]])
   Open a persistent connection to an InterBase database */
PHP_FUNCTION(ibase_pconnect)
{
	_php_ibase_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, INI_INT("ibase.allow_persistent"));
}
/* }}} */

/* {{{ proto bool ibase_close([resource link_identifier])
   Close an InterBase connection */
PHP_FUNCTION(ibase_close)
{
	zval **link_arg = NULL;
	ibase_db_link *ib_link;
	int link_id;

	RESET_ERRMSG;
	
	switch (ZEND_NUM_ARGS()) {
		case 0:
			link_id = IBG(default_link);
			IBG(default_link) = -1;
			break;
		case 1:
			if (zend_get_parameters_ex(1, &link_arg) == FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long_ex(link_arg);
			link_id = Z_LVAL_PP(link_arg);
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}

	ZEND_FETCH_RESOURCE2(ib_link, ibase_db_link *, link_arg, link_id, LE_LINK, le_link, le_plink);
	zend_list_delete(link_id);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ibase_drop_db([resource link_identifier])
   Drop an InterBase database */
PHP_FUNCTION(ibase_drop_db)
{
	zval **link_arg = NULL;
	ibase_db_link *ib_link;
	ibase_tr_list *l;
	int link_id;

	RESET_ERRMSG;

	switch (ZEND_NUM_ARGS()) {
		case 0:
			link_id = IBG(default_link);
			IBG(default_link) = -1;
			break;
		case 1:
			if (zend_get_parameters_ex(1, &link_arg) == FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long_ex(link_arg);
			link_id = Z_LVAL_PP(link_arg);
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	
	ZEND_FETCH_RESOURCE2(ib_link, ibase_db_link *, link_arg, link_id, LE_LINK, le_link, le_plink);

	if (isc_drop_database(IB_STATUS, &ib_link->handle)) {
		_php_ibase_error(TSRMLS_C);
		RETURN_FALSE;
	}

	/* isc_drop_database() doesn't invalidate the transaction handles */
	for (l = ib_link->tr_list; l != NULL; l = l->next) {
		if (l->trans != NULL) l->trans->handle = NULL;
	}

	zend_list_delete(link_id);
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto resource ibase_trans([int trans_args [, resource link_identifier [, ... ], int trans_args [, resource link_identifier [, ... ]] [, ...]]])
   Start a transaction over one or several databases */

#define TPB_MAX_SIZE (8*sizeof(char))

PHP_FUNCTION(ibase_trans)
{
	unsigned short i, argn, link_cnt = 0, tpb_len = 0;
	char last_tpb[TPB_MAX_SIZE];
	ibase_db_link **ib_link = NULL;
	ibase_trans *ib_trans;
	isc_tr_handle tr_handle = NULL;
	ISC_STATUS result;
	
	RESET_ERRMSG;

	argn = ZEND_NUM_ARGS();

	/* (1+argn) is an upper bound for the number of links this trans connects to */
	ib_link = (ibase_db_link **) safe_emalloc(sizeof(ibase_db_link *),1+argn,0);
	
	if (argn > 0) {
		long trans_argl = 0;
		char *tpb;
		ISC_TEB *teb;
		zval ***args = (zval ***) safe_emalloc(sizeof(zval **),argn,0);

		if (zend_get_parameters_array_ex(argn, args) == FAILURE) {
			efree(args);
			efree(ib_link);
			RETURN_FALSE;
		}

		teb = (ISC_TEB *) safe_emalloc(sizeof(ISC_TEB),argn,0);
		tpb = (char *) safe_emalloc(TPB_MAX_SIZE,argn,0);

		/* enumerate all the arguments: assume every non-resource argument 
		   specifies modifiers for the link ids that follow it */
		for (i = 0; i < argn; ++i) {
			
			if (Z_TYPE_PP(args[i]) == IS_RESOURCE) {
				
				ZEND_FETCH_RESOURCE2(ib_link[link_cnt], ibase_db_link *, args[i], -1, 
					LE_LINK, le_link, le_plink);
	
				/* copy the most recent modifier string into tbp[] */
				memcpy(&tpb[TPB_MAX_SIZE * link_cnt], last_tpb, TPB_MAX_SIZE);

				/* add a database handle to the TEB with the most recently specified set of modifiers */
				teb[link_cnt].db_ptr = &ib_link[link_cnt]->handle;
				teb[link_cnt].tpb_len = tpb_len;
				teb[link_cnt].tpb_ptr = &tpb[TPB_MAX_SIZE * link_cnt];
				
				++link_cnt;
				
			} else {
				
				tpb_len = 0;

				convert_to_long_ex(args[i]);
				trans_argl = Z_LVAL_PP(args[i]);

				if (trans_argl != PHP_IBASE_DEFAULT) {
					last_tpb[tpb_len++] = isc_tpb_version3;

					/* access mode */
					if (PHP_IBASE_READ == (trans_argl & PHP_IBASE_READ)) {
						last_tpb[tpb_len++] = isc_tpb_read;
					} else if (PHP_IBASE_WRITE == (trans_argl & PHP_IBASE_WRITE)) {
						last_tpb[tpb_len++] = isc_tpb_write;
					}

					/* isolation level */
					if (PHP_IBASE_COMMITTED == (trans_argl & PHP_IBASE_COMMITTED)) {
						last_tpb[tpb_len++] = isc_tpb_read_committed;
						if (PHP_IBASE_REC_VERSION == (trans_argl & PHP_IBASE_REC_VERSION)) {
							last_tpb[tpb_len++] = isc_tpb_rec_version;
						} else if (PHP_IBASE_REC_NO_VERSION == (trans_argl & PHP_IBASE_REC_NO_VERSION)) {
							last_tpb[tpb_len++] = isc_tpb_no_rec_version; 
						}	
					} else if (PHP_IBASE_CONSISTENCY == (trans_argl & PHP_IBASE_CONSISTENCY)) {
						last_tpb[tpb_len++] = isc_tpb_consistency;
					} else if (PHP_IBASE_CONCURRENCY == (trans_argl & PHP_IBASE_CONCURRENCY)) {
						last_tpb[tpb_len++] = isc_tpb_concurrency;
					}
					
					/* lock resolution */
					if (PHP_IBASE_NOWAIT == (trans_argl & PHP_IBASE_NOWAIT)) {
						last_tpb[tpb_len++] = isc_tpb_nowait;
					} else if (PHP_IBASE_WAIT == (trans_argl & PHP_IBASE_WAIT)) {
						last_tpb[tpb_len++] = isc_tpb_wait;
					}
				}
			}
		}	
					
		if (link_cnt > 0) {
			result = isc_start_multiple(IB_STATUS, &tr_handle, link_cnt, teb);
		}

		efree(args);
		efree(tpb);
		efree(teb);
	}

	if (link_cnt == 0) {
		link_cnt = 1;
		ZEND_FETCH_RESOURCE2(ib_link[0], ibase_db_link *, NULL, IBG(default_link), LE_LINK, 
			le_link, le_plink);
		result = isc_start_transaction(IB_STATUS, &tr_handle, 1, &ib_link[0]->handle, tpb_len, last_tpb);
	}
	
	/* start the transaction */
	if (result) {
		_php_ibase_error(TSRMLS_C);
		efree(ib_link);
		RETURN_FALSE;
	}

	/* register the transaction in our own data structures */
	ib_trans = (ibase_trans *) safe_emalloc(link_cnt-1, sizeof(ibase_db_link *), sizeof(ibase_trans));
	ib_trans->handle = tr_handle;
	ib_trans->link_cnt = link_cnt;
	ib_trans->affected_rows = 0;
	for (i = 0; i < link_cnt; ++i) {
		ibase_tr_list **l;
		ib_trans->db_link[i] = ib_link[i];
		
		/* the first item in the connection-transaction list is reserved for the default transaction */
		if (ib_link[i]->tr_list == NULL) {
			ib_link[i]->tr_list = (ibase_tr_list *) emalloc(sizeof(ibase_tr_list));
			ib_link[i]->tr_list->trans = NULL;
			ib_link[i]->tr_list->next = NULL;
		}

		/* link the transaction into the connection-transaction list */
		for (l = &ib_link[i]->tr_list; *l != NULL; l = &(*l)->next);
		*l = (ibase_tr_list *) emalloc(sizeof(ibase_tr_list));
		(*l)->trans = ib_trans;
		(*l)->next = NULL;
	}
	efree(ib_link);
	ZEND_REGISTER_RESOURCE(return_value, ib_trans, le_trans);
}
/* }}} */

int _php_ibase_def_trans(ibase_db_link *ib_link, ibase_trans **trans TSRMLS_DC) /* {{{ */
{
	if (ib_link == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid database link");
		return FAILURE;
	}

	/* the first item in the connection-transaction list is reserved for the default transaction */
	if (ib_link->tr_list == NULL) {
		ib_link->tr_list = (ibase_tr_list *) emalloc(sizeof(ibase_tr_list));
		ib_link->tr_list->trans = NULL;
		ib_link->tr_list->next = NULL;
	}

	if (*trans == NULL) {
		ibase_trans *tr = ib_link->tr_list->trans;

		if (tr == NULL) {
			tr = (ibase_trans *) emalloc(sizeof(ibase_trans));
			tr->handle = NULL;
			tr->link_cnt = 1;
			tr->affected_rows = 0;
			tr->db_link[0] = ib_link;
			ib_link->tr_list->trans = tr;
		}
		if (tr->handle == NULL) {
			if (isc_start_transaction(IB_STATUS, &tr->handle, 1, &ib_link->handle, 0, NULL)) {
				_php_ibase_error(TSRMLS_C);
				return FAILURE;
			}
		}
		*trans = tr;
	}
	return SUCCESS;
}
/* }}} */

static void _php_ibase_trans_end(INTERNAL_FUNCTION_PARAMETERS, int commit) /* {{{ */
{
	ibase_trans *trans = NULL;
	int res_id = 0;
	ISC_STATUS result;

	RESET_ERRMSG;

	switch (ZEND_NUM_ARGS()) {

		ibase_db_link *ib_link;
		zval **arg;
		int type;

		case 0:
			ZEND_FETCH_RESOURCE2(ib_link, ibase_db_link *, NULL, IBG(default_link), LE_LINK, 
				le_link, le_plink);
			if (ib_link->tr_list == NULL || ib_link->tr_list->trans == NULL) {
				/* this link doesn't have a default transaction */
				_php_ibase_module_error("Default link has no default transaction" TSRMLS_CC);
				RETURN_FALSE;
			}
			trans = ib_link->tr_list->trans;
			break;

		case 1: 
			if (zend_get_parameters_ex(1, &arg) == FAILURE) {
				RETURN_FALSE;
			}
			/* one id was passed, could be db or trans id */
			if (zend_list_find(Z_LVAL_PP(arg), &type) && type == le_trans) {			
				ZEND_FETCH_RESOURCE(trans, ibase_trans *, arg, -1, LE_TRANS, le_trans);

				convert_to_long_ex(arg);
				res_id = Z_LVAL_PP(arg);
			} else {
				ZEND_FETCH_RESOURCE2(ib_link, ibase_db_link *, arg, -1, LE_LINK, le_link, le_plink);

				if (ib_link->tr_list == NULL || ib_link->tr_list->trans == NULL) {
					/* this link doesn't have a default transaction */
					_php_ibase_module_error("Link has no default transaction" TSRMLS_CC);
					RETURN_FALSE;
				}
				trans = ib_link->tr_list->trans;
			}
			break;

		default:
			WRONG_PARAM_COUNT;
			break;
	}

	switch (commit) {
		
		default: /* == case ROLLBACK: */
			result = isc_rollback_transaction(IB_STATUS, &trans->handle);
			break;
		case COMMIT:
			result = isc_commit_transaction(IB_STATUS, &trans->handle);
			break;
		case (ROLLBACK | RETAIN):
			result = isc_rollback_retaining(IB_STATUS, &trans->handle);
			break;
		case (COMMIT | RETAIN):
			result = isc_commit_retaining(IB_STATUS, &trans->handle);
			break;
	}
	
	if (result) {
		_php_ibase_error(TSRMLS_C);
		RETURN_FALSE;
	}

	/* Don't try to destroy implicitly opened transaction from list... */
	if ( (commit & RETAIN) == 0 && res_id != 0) {
		zend_list_delete(res_id);
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ibase_commit( resource link_identifier )
   Commit transaction */
PHP_FUNCTION(ibase_commit)
{
	_php_ibase_trans_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, COMMIT);
}
/* }}} */

/* {{{ proto bool ibase_rollback( resource link_identifier )
   Rollback transaction */
PHP_FUNCTION(ibase_rollback)
{
	_php_ibase_trans_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, ROLLBACK);
}
/* }}} */

/* {{{ proto bool ibase_commit_ret( resource link_identifier )
   Commit transaction and retain the transaction context */
PHP_FUNCTION(ibase_commit_ret)
{
	_php_ibase_trans_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, COMMIT | RETAIN);
}
/* }}} */

/* {{{ proto bool ibase_rollback_ret( resource link_identifier )
   Rollback transaction and retain the transaction context */
PHP_FUNCTION(ibase_rollback_ret)
{
	_php_ibase_trans_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, ROLLBACK | RETAIN);
}
/* }}} */

/* {{{ proto int ibase_gen_id(string generator [, int increment [, resource link_identifier ]])
   Increments the named generator and returns its new value */
PHP_FUNCTION(ibase_gen_id)
{
	zval *link = NULL;
	char query[128], *generator;
	int gen_len;
	long inc = 1;
	ibase_db_link *ib_link;
	ibase_trans *trans = NULL;
	XSQLDA out_sqlda;
	ISC_INT64 result;

	RESET_ERRMSG;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lr", &generator, &gen_len,
			&inc, &link)) {
		RETURN_FALSE;
	}

	PHP_IBASE_LINK_TRANS(link, ib_link, trans);
	
	sprintf(query, "SELECT GEN_ID(%s,%ld) FROM rdb$database", generator, inc);

	/* allocate a minimal descriptor area */
	out_sqlda.sqln = out_sqlda.sqld = 1;
	out_sqlda.version = SQLDA_CURRENT_VERSION;
	
	/* allocate the field for the result */
	out_sqlda.sqlvar[0].sqltype = SQL_INT64;
	out_sqlda.sqlvar[0].sqlscale = 0;
	out_sqlda.sqlvar[0].sqllen = sizeof(result);
	out_sqlda.sqlvar[0].sqldata = (void*) &result;

	/* execute the query */
	if (isc_dsql_exec_immed2(IB_STATUS, &ib_link->handle, &trans->handle, 0, query,
			SQL_DIALECT_CURRENT, NULL, &out_sqlda)) {
		_php_ibase_error(TSRMLS_C);
		RETURN_FALSE;
	}

	/* don't return the generator value as a string unless it doesn't fit in a long */
#if SIZEOF_LONG < 8
	if (result < LONG_MIN || result > LONG_MAX) {
		char res[24];

		sprintf(res, "%" LL_MASK "d", result);
		RETURN_STRING(res,1);
	}
#endif
	RETURN_LONG((long)result);
}

/* }}} */
    
#endif /* HAVE_IBASE */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
