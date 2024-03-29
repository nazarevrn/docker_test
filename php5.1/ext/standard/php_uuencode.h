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
   | Author: Ilia Alshanetsky <ilia@php.net>                              |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_UUENCODE_H
#define PHP_UUENCODE_H

PHP_FUNCTION(convert_uudecode);
PHP_FUNCTION(convert_uuencode);

PHPAPI int php_uudecode(char *src, int src_len, char **dest);
PHPAPI int php_uuencode(char *src, int src_len, char **dest);

#endif /* PHP_UUENCODE_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
