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
/* $Id: php_http.h,v 1.16.2.1 2006/01/01 12:50:13 sniper Exp $ */

#ifndef PHP_HTTP_H
#define PHP_HTTP_H

int make_http_soap_request(zval  *this_ptr, 
                           char  *request, 
                           int    request_size, 
                           char  *location, 
                           char  *soapaction, 
                           int    soap_version,
                           char **response, 
                           int   *response_len TSRMLS_DC);

void proxy_authentication(zval* this_ptr, smart_str* soap_headers TSRMLS_DC);
void basic_authentication(zval* this_ptr, smart_str* soap_headers TSRMLS_DC);
#endif
