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
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef RFC1867_H
#define RFC1867_H

#include "SAPI.h"

#define MULTIPART_CONTENT_TYPE "multipart/form-data"

SAPI_API SAPI_POST_HANDLER_FUNC(rfc1867_post_handler);

void destroy_uploaded_files_hash(TSRMLS_D);
void php_rfc1867_register_constants(TSRMLS_D);

#endif /* RFC1867_H */
