/* __header_here__ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_extname.h"

/* If you declare any globals in php_extname.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(extname)
*/

/* True global resources - no need for thread safety here */
static int le_extname;

/* {{{ extname_functions[]
 *
 * Every user visible function must have an entry in extname_functions[].
 */
zend_function_entry extname_functions[] = {
	PHP_FE(confirm_extname_compiled,	NULL)		/* For testing, remove later. */
	/* __function_entries_here__ */
	{NULL, NULL, NULL}	/* Must be the last line in extname_functions[] */
};
/* }}} */

/* {{{ extname_module_entry
 */
zend_module_entry extname_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"extname",
	extname_functions,
	PHP_MINIT(extname),
	PHP_MSHUTDOWN(extname),
	PHP_RINIT(extname),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(extname),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(extname),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_EXTNAME
ZEND_GET_MODULE(extname)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("extname.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_extname_globals, extname_globals)
    STD_PHP_INI_ENTRY("extname.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_extname_globals, extname_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_extname_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_extname_init_globals(zend_extname_globals *extname_globals)
{
	extname_globals->global_value = 0;
	extname_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(extname)
{
	/* If you have INI entries, uncomment these lines 
	ZEND_INIT_MODULE_GLOBALS(extname, php_extname_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(extname)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(extname)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(extname)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(extname)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "extname support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_extname_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_extname_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char string[256];

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = sprintf(string, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "extname", arg);
	RETURN_STRINGL(string, len, 1);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/

/* __function_stubs_here__ */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
