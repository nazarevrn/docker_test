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
   | Author: Chad Robinson <chadr@brttech.com>                            |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

/*
  filePro 4.x support developed by Chad Robinson, chadr@brttech.com
  Contact Chad Robinson at BRT Technical Services Corp. for details.
  filePro is a registered trademark by Fiserv, Inc.  This file contains
  no code or information that is not freely available from the filePro
  web site at http://www.fileproplus.com/
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "safe_mode.h"
#include "fopen_wrappers.h"
#include <string.h>
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <errno.h>
#include "php_globals.h"

#include "php_filepro.h"
#if HAVE_FILEPRO

typedef struct fp_field {
	char *name;
	char *format;
	int width;
	struct fp_field *next;
} FP_FIELD;

#ifdef THREAD_SAFE
DWORD FPTls;
static int numthreads=0;

typedef struct fp_global_struct{
	char *fp_database;
	signed int fp_fcount;
	signed int fp_keysize;
	FP_FIELD *fp_fieldlist;
}fp_global_struct;

#define FP_GLOBAL(a) fp_globals->a

#define FP_TLS_VARS \
	fp_global_struct *fp_globals; \
	fp_globals=TlsGetValue(FPTls); 

#else
#define FP_GLOBAL(a) a
#define FP_TLS_VARS
static char *fp_database = NULL;			/* Database directory */
static signed int fp_fcount = -1;			/* Column count */
static signed int fp_keysize = -1;			/* Size of key records */
static FP_FIELD *fp_fieldlist = NULL;		/* List of fields */
#endif

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(filepro)
{
#ifdef THREAD_SAFE
	fp_global_struct *fp_globals;
#ifdef COMPILE_DL_FILEPRO
	CREATE_MUTEX(fp_mutex,"FP_TLS");
	SET_MUTEX(fp_mutex);
	numthreads++;
	if (numthreads==1){
	if ((FPTls=TlsAlloc())==0xFFFFFFFF){
		FREE_MUTEX(fp_mutex);
		return 0;
	}}
	FREE_MUTEX(fp_mutex);
#endif
	fp_globals = (fp_global_struct *) LocalAlloc(LPTR, sizeof(fp_global_struct)); 
	TlsSetValue(FPTls, (void *) fp_globals);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(filepro)
{
	FP_GLOBAL(fp_database)=NULL;
	FP_GLOBAL(fp_fcount)=-1;
	FP_GLOBAL(fp_keysize)=-1;
	FP_GLOBAL(fp_fieldlist)=NULL;
 
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(filepro)
{
	FP_FIELD *tmp, *next;

	if (FP_GLOBAL(fp_database)) {
		efree(FP_GLOBAL(fp_database));
	}
	
	if (FP_GLOBAL(fp_fieldlist)) {
		for (tmp = FP_GLOBAL(fp_fieldlist); tmp;) {
			efree(tmp->name);
			efree(tmp->format);
			next = tmp->next;
			efree(tmp);
			tmp=next;
		}	
	}
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(filepro)
{
#ifdef THREAD_SAFE
	fp_global_struct *fp_globals;
	fp_globals = TlsGetValue(FPTls); 
	if (fp_globals != 0) 
		LocalFree((HLOCAL) fp_globals); 
#ifdef COMPILE_DL_FILEPRO
	SET_MUTEX(fp_mutex);
	numthreads--;
	if (!numthreads){
		if (!TlsFree(FPTls)){
			FREE_MUTEX(fp_mutex);
			return 0;
		}
	}
	FREE_MUTEX(fp_mutex);
#endif
#endif
	return SUCCESS;
}
/* }}} */

zend_function_entry filepro_functions[] = {
	PHP_FE(filepro,									NULL)
	PHP_FE(filepro_rowcount,						NULL)
	PHP_FE(filepro_fieldname,						NULL)
	PHP_FE(filepro_fieldtype,						NULL)
	PHP_FE(filepro_fieldwidth,						NULL)
	PHP_FE(filepro_fieldcount,						NULL)
	PHP_FE(filepro_retrieve,						NULL)
	{NULL, NULL, NULL}
};

zend_module_entry filepro_module_entry = {
	STANDARD_MODULE_HEADER,
	"filepro", 
	filepro_functions, 
	PHP_MINIT(filepro), 
	PHP_MSHUTDOWN(filepro), 
	PHP_RINIT(filepro), 
	PHP_RSHUTDOWN(filepro), 
	NULL, 
	NO_VERSION_YET, 
	STANDARD_MODULE_PROPERTIES
};


#ifdef COMPILE_DL_FILEPRO
ZEND_GET_MODULE(filepro)
#if defined(PHP_WIN32) && defined(THREAD_SAFE)

/*NOTE: You should have an odbc.def file where you
export DllMain*/
BOOL WINAPI DllMain(HANDLE hModule, 
                      DWORD  ul_reason_for_call, 
                      LPVOID lpReserved)
{
	switch( ul_reason_for_call ) {
		case DLL_PROCESS_ATTACH:
			if ((FPTls=TlsAlloc())==0xFFFFFFFF) {
				return 0;
			}
			break;    
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			if (!TlsFree(FPTls)) {
				return 0;
			}
			break;
	}
	return 1;
}
#endif
#endif

/*
 * LONG filePro(STRING directory)
 * 
 * Read and verify the map file.  We store the field count and field info
 * internally, which means we become unstable if you modify the table while
 * a user is using it!  We cannot lock anything since Web connections don't
 * provide the ability to later unlock what we locked.  Be smart, be safe.
 */
/* {{{ proto bool filepro(string directory)
   Read and verify the map file */
PHP_FUNCTION(filepro)
{
	zval **dir;
	FILE *fp;
	char workbuf[MAXPATHLEN];
	char readbuf[256];
	char *strtok_buf = NULL;
	int i;
	FP_FIELD *new_field, *tmp, *next;
	FP_TLS_VARS;

	if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &dir) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_string_ex(dir);

	/* free memory */
	if (FP_GLOBAL(fp_database) != NULL) {
		efree (FP_GLOBAL(fp_database));
	}
	
	/* free linked list of fields */
	tmp = FP_GLOBAL(fp_fieldlist);
	while (tmp != NULL) {
		next = tmp->next;
		efree(tmp->name);
		efree(tmp->format);
		efree(tmp);
		tmp = next;
	} 
	
	/* init the global vars */
	FP_GLOBAL(fp_database) = NULL;
	FP_GLOBAL(fp_fieldlist) = NULL;
	FP_GLOBAL(fp_fcount) = -1;
	FP_GLOBAL(fp_keysize) = -1;
	
	snprintf(workbuf, sizeof(workbuf), "%s/map", Z_STRVAL_PP(dir));

	if (PG(safe_mode) && (!php_checkuid(workbuf, NULL, CHECKUID_CHECK_FILE_AND_DIR))) {
		RETURN_FALSE;
	}
	
	if (php_check_open_basedir(workbuf TSRMLS_CC)) {
		RETURN_FALSE;
	}

	if (!(fp = VCWD_FOPEN(workbuf, "r"))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot open map: [%d] %s", errno, strerror(errno));
		RETURN_FALSE;
	}
	if (!fgets(readbuf, sizeof(readbuf), fp)) {
		fclose(fp);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot read map: [%d] %s", errno, strerror(errno));
		RETURN_FALSE;
	}
	
	/* Get the field count, assume the file is readable! */
	if (strcmp(php_strtok_r(readbuf, ":", &strtok_buf), "map")) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Map file corrupt or encrypted");
		RETURN_FALSE;
	}
	FP_GLOBAL(fp_keysize) = atoi(php_strtok_r(NULL, ":", &strtok_buf));
	php_strtok_r(NULL, ":", &strtok_buf);
	FP_GLOBAL(fp_fcount) = atoi(php_strtok_r(NULL, ":", &strtok_buf));

	/* Read in the fields themselves */
	for (i = 0; i < FP_GLOBAL(fp_fcount); i++) {
		if (!fgets(readbuf, sizeof(readbuf), fp)) {
			fclose(fp);
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot read map: [%d] %s", errno, strerror(errno));
			RETURN_FALSE;
		}
		new_field = emalloc(sizeof(FP_FIELD));
		new_field->next = NULL;
		new_field->name = estrdup(php_strtok_r(readbuf, ":", &strtok_buf));
		new_field->width = atoi(php_strtok_r(NULL, ":", &strtok_buf));
		new_field->format = estrdup(php_strtok_r(NULL, ":", &strtok_buf));
        
		/* Store in forward-order to save time later */
		if (!FP_GLOBAL(fp_fieldlist)) {
			FP_GLOBAL(fp_fieldlist) = new_field;
		} else {
			for (tmp = FP_GLOBAL(fp_fieldlist); tmp; tmp = tmp->next) {
				if (!tmp->next) {
					tmp->next = new_field;
					tmp = new_field;
				}
			}
		}
	}
	fclose(fp);
		
	FP_GLOBAL(fp_database) = estrndup(Z_STRVAL_PP(dir), Z_STRLEN_PP(dir));

	RETVAL_TRUE;
}
/* }}} */


/*
 * LONG filePro_rowcount(void)
 * 
 * Count the used rows in the database.  filePro just marks deleted records
 * as deleted; they are not removed.  Since no counts are maintained we need
 * to go in and count records ourselves.  <sigh>
 * 
 * Errors return false, success returns the row count.
 */
/* {{{ proto int filepro_rowcount(void)
   Find out how many rows are in a filePro database */
PHP_FUNCTION(filepro_rowcount)
{
	FILE *fp;
	char workbuf[MAXPATHLEN];
	char readbuf[256];
	int recsize = 0, records = 0;
	FP_TLS_VARS;

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if (!FP_GLOBAL(fp_database)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Must set database directory first!");
		RETURN_FALSE;
	}
	
	recsize = FP_GLOBAL(fp_keysize) + 19; /* 20 bytes system info -1 to save time later */
	
	/* Now read the records in, moving forward recsize-1 bytes each time */
	snprintf(workbuf, sizeof(workbuf), "%s/key", FP_GLOBAL(fp_database));

	if (PG(safe_mode) && (!php_checkuid(workbuf, NULL, CHECKUID_CHECK_FILE_AND_DIR))) {
		RETURN_FALSE;
	}
	
	if (php_check_open_basedir(workbuf TSRMLS_CC)) {
		RETURN_FALSE;
	}

	if (!(fp = VCWD_FOPEN(workbuf, "r"))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot open key: [%d] %s", errno, strerror(errno));
		RETURN_FALSE;
	}
	while (!feof(fp)) {
		if (fread(readbuf, 1, 1, fp) == 1) {
			if (readbuf[0])
				records++;
			fseek(fp, recsize, SEEK_CUR);
		}
	}
	fclose(fp);
	
	RETVAL_LONG(records);
}
/* }}} */


/*
 * STRING filePro_fieldname(LONG field_number)
 * 
 * Errors return false, success returns the name of the field.
 */
/* {{{ proto string filepro_fieldname(int fieldnumber)
   Gets the name of a field */
PHP_FUNCTION(filepro_fieldname)
{
	zval **fno;
	FP_FIELD *lp;
	int i;
	FP_TLS_VARS;

	if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &fno) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(fno);

	if (!FP_GLOBAL(fp_database)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Must set database directory first!");
		RETURN_FALSE;
	}
	
	for (i = 0, lp = FP_GLOBAL(fp_fieldlist); lp; lp = lp->next, i++) {
		if (i == Z_LVAL_PP(fno)) {
			RETURN_STRING(lp->name, 1);
		}
	}

	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to locate field number %ld.", Z_LVAL_PP(fno));

	RETVAL_FALSE;
}
/* }}} */


/*
 * STRING filePro_fieldtype(LONG field_number)
 * 
 * Errors return false, success returns the type (edit) of the field
 */
/* {{{ proto string filepro_fieldtype(int field_number)
   Gets the type of a field */
PHP_FUNCTION(filepro_fieldtype)
{
	zval **fno;
	FP_FIELD *lp;
	int i;
	FP_TLS_VARS;

	if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &fno) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(fno);

	if (!FP_GLOBAL(fp_database)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Must set database directory first!");
		RETURN_FALSE;
	}
	
	for (i = 0, lp = FP_GLOBAL(fp_fieldlist); lp; lp = lp->next, i++) {
		if (i == Z_LVAL_PP(fno)) {
			RETURN_STRING(lp->format, 1);
		}
	}
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to locate field number %ld.", Z_LVAL_PP(fno));
	RETVAL_FALSE;
}
/* }}} */


/*
 * STRING filePro_fieldwidth(int field_number)
 * 
 * Errors return false, success returns the character width of the field.
 */
/* {{{ proto int filepro_fieldwidth(int field_number)
   Gets the width of a field */
PHP_FUNCTION(filepro_fieldwidth)
{
	zval **fno;
	FP_FIELD *lp;
	int i;
	FP_TLS_VARS;

	if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &fno) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long_ex(fno);

	if (!FP_GLOBAL(fp_database)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Must set database directory first!");
		RETURN_FALSE;
	}
	
	for (i = 0, lp = FP_GLOBAL(fp_fieldlist); lp; lp = lp->next, i++) {
		if (i == Z_LVAL_PP(fno)) {
			RETURN_LONG(lp->width);
		}
	}
	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to locate field number %ld.", Z_LVAL_PP(fno));
	RETVAL_FALSE;
}
/* }}} */


/*
 * LONG filePro_fieldcount(void)
 * 
 * Errors return false, success returns the field count.
 */
/* {{{ proto int filepro_fieldcount(void)
   Find out how many fields are in a filePro database */
PHP_FUNCTION(filepro_fieldcount)
{
	FP_TLS_VARS;

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if (!FP_GLOBAL(fp_database)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Must set database directory first!");
		RETURN_FALSE;
	}
	
	/* Read in the first line from the map file */
	RETVAL_LONG(FP_GLOBAL(fp_fcount));
}
/* }}} */


/*
 * STRING filePro_retrieve(int row_number, int field_number)
 * 
 * Errors return false, success returns the datum.
 */
/* {{{ proto string filepro_retrieve(int row_number, int field_number)
   Retrieves data from a filePro database */
PHP_FUNCTION(filepro_retrieve)
{
	zval **rno, **fno;
	FP_FIELD *lp;
	FILE *fp;
	char workbuf[MAXPATHLEN];
	char *readbuf;
	int i, fnum, rnum;
	long offset;
	FP_TLS_VARS;

	if (ZEND_NUM_ARGS() != 2 || zend_get_parameters_ex(2, &rno, &fno) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	if (!FP_GLOBAL(fp_database)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Must set database directory first!");
		RETURN_FALSE;
	}
	
	convert_to_long_ex(rno);
	convert_to_long_ex(fno);

	fnum = Z_LVAL_PP(fno);
	rnum = Z_LVAL_PP(rno);
    
	if (rnum < 0 || fnum < 0 || fnum >= FP_GLOBAL(fp_fcount)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Parameters out of range");
		RETURN_FALSE;
	}
    
	offset = (rnum + 1) * (FP_GLOBAL(fp_keysize) + 20) + 20; /* Record location */
	for (i = 0, lp = FP_GLOBAL(fp_fieldlist); lp && i < fnum; lp = lp->next, i++) {
		offset += lp->width;
	}
	if (!lp) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot locate field");
		RETURN_FALSE;
	}
    
	/* Now read the record in */
	snprintf(workbuf, sizeof(workbuf), "%s/key", FP_GLOBAL(fp_database));

	if (PG(safe_mode) && (!php_checkuid(workbuf, NULL, CHECKUID_CHECK_FILE_AND_DIR))) {
		RETURN_FALSE;
	}
	
	if (php_check_open_basedir(workbuf TSRMLS_CC)) {
		RETURN_FALSE;
	}

	if (!(fp = VCWD_FOPEN(workbuf, "r"))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot open key: [%d] %s", errno, strerror(errno));
		fclose(fp);
		RETURN_FALSE;
	}
	fseek(fp, offset, SEEK_SET);
	
	readbuf = emalloc (lp->width+1);
	if (fread(readbuf, lp->width, 1, fp) != 1) {
        	php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot read data: [%d] %s", errno, strerror(errno));
		efree(readbuf);
		fclose(fp);
		RETURN_FALSE;
	}
	readbuf[lp->width] = '\0';
	fclose(fp);
	RETURN_STRING(readbuf, 0);
}
/* }}} */

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
