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
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#include <oci.h>

typedef struct {
	const char *file;
	int line;
	sb4 errcode;
	char *errmsg;
} pdo_oci_error_info;

/* stuff we use in an OCI database handle */
typedef struct {
	OCIServer	*server;
	OCISession	*session;
	OCIEnv 		*env;
	OCIError	*err;
	OCISvcCtx 	*svc;
	/* OCI9; 0 == use NLS_LANG */
	ub2			charset;
	sword		last_err;

	unsigned attached:1;
	unsigned _reserved:31;

	pdo_oci_error_info einfo;
} pdo_oci_db_handle;

typedef struct {
	OCIDefine	*def;
	ub2			fetched_len;
	ub2			retcode;
	sb2			indicator;

	char *data;
	ub4 datalen;

	ub2 dtype;

} pdo_oci_column;

typedef struct {
	pdo_oci_db_handle *H;
	OCIStmt		*stmt;
	OCIError	*err;
	sword		last_err;
	ub2		stmt_type;
	ub4		exec_type;
	pdo_oci_column *cols;
	pdo_oci_error_info einfo;
	unsigned int have_blobs:1;
} pdo_oci_stmt;

typedef struct {
	OCIBind 	*bind;	/* allocated by OCI */
	sb2			oci_type;
	sb2			indicator;
	ub2			retcode;

	ub4			actual_len;

	dvoid		*thing;	/* for LOBS, REFCURSORS etc. */

	unsigned used_for_output;
} pdo_oci_bound_param;

extern const ub4 PDO_OCI_INIT_MODE;
extern pdo_driver_t pdo_oci_driver;
extern OCIEnv *pdo_oci_Env;

ub4 _oci_error(OCIError *err, pdo_dbh_t *dbh, pdo_stmt_t *stmt, char *what, sword status, const char *file, int line TSRMLS_DC);
#define oci_drv_error(w)	_oci_error(H->err, dbh, NULL, w, H->last_err, __FILE__, __LINE__ TSRMLS_CC)
#define oci_stmt_error(w)	_oci_error(S->err, stmt->dbh, stmt, w, S->last_err, __FILE__, __LINE__ TSRMLS_CC)

extern struct pdo_stmt_methods oci_stmt_methods;

