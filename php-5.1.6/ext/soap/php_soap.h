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
  | Authors: Brad Lafountain <rodif_bl@yahoo.com>                        |
  |          Shane Caraveo <shane@caraveo.com>                           |
  |          Dmitry Stogov <dmitry@zend.com>                             |
  +----------------------------------------------------------------------+
*/
/* $Id: php_soap.h,v 1.38.2.6 2006/04/19 10:48:54 dmitry Exp $ */

#ifndef PHP_SOAP_H
#define PHP_SOAP_H

#include "php.h"
#include "php_globals.h"
#include "ext/standard/info.h"
#include "ext/standard/php_standard.h"
#if HAVE_PHP_SESSION && !defined(COMPILE_DL_SESSION)
#include "ext/session/php_session.h"
#endif
#include "ext/standard/php_smart_str.h"
#include "php_ini.h"
#include "SAPI.h"
#include <libxml/parser.h>
#include <libxml/xpath.h>

#ifdef HAVE_PHP_DOMXML
# include "ext/domxml/php_domxml.h"
#endif

#ifndef PHP_HAVE_STREAMS
# error You lose - must be compiled against PHP 4.3.0 or later
#endif

#ifndef PHP_WIN32
# define TRUE 1
# define FALSE 0
# define stricmp strcasecmp
#endif

extern int le_url;

typedef struct _encodeType encodeType, *encodeTypePtr;
typedef struct _encode encode, *encodePtr;

typedef struct _sdl sdl, *sdlPtr;
typedef struct _sdlRestrictionInt sdlRestrictionInt, *sdlRestrictionIntPtr;
typedef struct _sdlRestrictionChar sdlRestrictionChar, *sdlRestrictionCharPtr;
typedef struct _sdlRestrictions sdlRestrictions, *sdlRestrictionsPtr;
typedef struct _sdlType sdlType, *sdlTypePtr;
typedef struct _sdlParam sdlParam, *sdlParamPtr;
typedef struct _sdlFunction sdlFunction, *sdlFunctionPtr;
typedef struct _sdlAttribute sdlAttribute, *sdlAttributePtr;
typedef struct _sdlBinding sdlBinding, *sdlBindingPtr;
typedef struct _sdlSoapBinding sdlSoapBinding, *sdlSoapBindingPtr;
typedef struct _sdlSoapBindingFunction sdlSoapBindingFunction, *sdlSoapBindingFunctionPtr;
typedef struct _sdlSoapBindingFunctionBody sdlSoapBindingFunctionBody, *sdlSoapBindingFunctionBodyPtr;

typedef struct _soapMapping soapMapping, *soapMappingPtr;
typedef struct _soapService soapService, *soapServicePtr;

#include "php_xml.h"
#include "php_encoding.h"
#include "php_sdl.h"
#include "php_schema.h"
#include "php_http.h"
#include "php_packet_soap.h"

struct _soapMapping {
	char *ns;
	char *ctype;
	int type;

	struct _map_functions {
		zval *to_xml_before;
		zval *to_xml;
		zval *to_xml_after;
		zval *to_zval_before;
		zval *to_zval;
		zval *to_zval_after;
	} map_functions;

	struct _map_class {
		int type;
		zend_class_entry *ce;
	} map_class;
};

struct _soapHeader;

struct _soapService {
	sdlPtr sdl;

	struct _soap_functions {
		HashTable *ft;
		int functions_all;
	} soap_functions;

	struct _soap_class {
		zend_class_entry *ce;
		zval **argv;
		int argc;
		int persistance;
	} soap_class;

	HashTable *mapping;
	int        version;
	int        type;
	char      *actor;
	char      *uri;
	xmlCharEncodingHandlerPtr encoding;
	HashTable *class_map;
	int        features;
	struct _soapHeader **soap_headers_ptr;
};

#define SOAP_CLASS 1
#define SOAP_FUNCTIONS 2
#define SOAP_FUNCTIONS_ALL 999

#define SOAP_MAP_FUNCTION 1
#define SOAP_MAP_CLASS 2

#define SOAP_PERSISTENCE_SESSION 1
#define SOAP_PERSISTENCE_REQUEST 2

#define SOAP_1_1 1
#define SOAP_1_2 2

#define SOAP_ACTOR_NEXT             1
#define SOAP_ACTOR_NONE             2
#define SOAP_ACTOR_UNLIMATERECEIVER 3

#define SOAP_1_1_ACTOR_NEXT             "http://schemas.xmlsoap.org/soap/actor/next"

#define SOAP_1_2_ACTOR_NEXT             "http://www.w3.org/2003/05/soap-envelope/role/next"
#define SOAP_1_2_ACTOR_NONE             "http://www.w3.org/2003/05/soap-envelope/role/none"
#define SOAP_1_2_ACTOR_UNLIMATERECEIVER "http://www.w3.org/2003/05/soap-envelope/role/ultimateReceiver"

#define SOAP_COMPRESSION_ACCEPT  0x20
#define SOAP_COMPRESSION_GZIP    0x00
#define SOAP_COMPRESSION_DEFLATE 0x10

#define SOAP_AUTHENTICATION_BASIC   0
#define SOAP_AUTHENTICATION_DIGEST  1

#define SOAP_SINGLE_ELEMENT_ARRAYS  (1<<0)
#define SOAP_WAIT_ONE_WAY_CALLS     (2<<0)

#define WSDL_CACHE_NONE     0x0
#define WSDL_CACHE_DISK     0x1
#define WSDL_CACHE_MEMORY   0x2
#define WSDL_CACHE_BOTH     0x3

ZEND_BEGIN_MODULE_GLOBALS(soap)
	HashTable  defEncNs;     /* mapping of default namespaces to prefixes */
	HashTable  defEnc;
	HashTable  defEncIndex;
	HashTable *overrides;
	int        cur_uniq_ns;
	int        soap_version;
	sdlPtr     sdl;
	zend_bool  use_soap_error_handler;
	char*      error_code;
	zval*      error_object;
	long       cache;
	char*      cache_dir;
	long       cache_ttl;
	long       cache_limit;
	HashTable *mem_cache;
	xmlCharEncodingHandlerPtr encoding;
	HashTable *class_map;
	int        features;
	HashTable  wsdl_cache;
ZEND_END_MODULE_GLOBALS(soap)

#ifdef PHP_WIN32
#define PHP_SOAP_API __declspec(dllexport)
#else
#define PHP_SOAP_API
#endif
#ifdef ZTS
#include "TSRM.h"
#endif

extern zend_module_entry soap_module_entry;
#define soap_module_ptr &soap_module_entry
#define phpext_soap_ptr soap_module_ptr

ZEND_EXTERN_MODULE_GLOBALS(soap)

#ifdef ZTS
# define SOAP_GLOBAL(v) TSRMG(soap_globals_id, zend_soap_globals *, v)
#else
# define SOAP_GLOBAL(v) (soap_globals.v)
#endif

extern zend_class_entry* soap_var_class_entry;

zval* add_soap_fault(zval *obj, char *fault_code, char *fault_string, char *fault_actor, zval *fault_detail TSRMLS_DC);

#define soap_error0(severity, format) \
	php_error(severity, "SOAP-ERROR: " format)

#define soap_error1(severity, format, param1) \
	php_error(severity, "SOAP-ERROR: " format, param1)

#define soap_error2(severity, format, param1, param2) \
	php_error(severity, "SOAP-ERROR: " format, param1, param2)

#define soap_error3(severity, format, param1, param2, param3) \
	php_error(severity, "SOAP-ERROR: " format, param1, param2, param3)

#endif
