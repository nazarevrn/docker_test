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
   | Original Author: Rasmus Lerdorf <rasmus@lerdorf.on.ca>               |
   | Modified for pHASH by: Sara Golemon <pollita@php.net>
   +----------------------------------------------------------------------+
*/

/* $Id: php_hash_md.h,v 1.2.2.4 2006/01/01 12:50:07 sniper Exp $ */

#ifndef PHP_HASH_MD_H
#define PHP_HASH_MD_H

/* When SHA is removed from Core,
    the ext/standard/sha1.c file can be removed
    and the ext/standard/sha1.h file can be reduced to:
        #define PHP_HASH_SHA1_NOT_IN_CORE
        #include "ext/hash/php_hash_sha.h"
	Don't forget to remove md5() and md5_file() entries from basic_functions.c
 */

#include "ext/standard/md5.h"

#ifdef PHP_HASH_MD5_NOT_IN_CORE
/* MD5.H - header file for MD5C.C
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
   rights reserved.

   License to copy and use this software is granted provided that it
   is identified as the "RSA Data Security, Inc. MD5 Message-Digest
   Algorithm" in all material mentioning or referencing this software
   or this function.

   License is also granted to make and use derivative works provided
   that such works are identified as "derived from the RSA Data
   Security, Inc. MD5 Message-Digest Algorithm" in all material
   mentioning or referencing the derived work.

   RSA Data Security, Inc. makes no representations concerning either
   the merchantability of this software or the suitability of this
   software for any particular purpose. It is provided "as is"
   without express or implied warranty of any kind.

   These notices must be retained in any copies of any part of this
   documentation and/or software.
 */

/* MD5 context. */
typedef struct {
	php_hash_uint32 state[4];				/* state (ABCD) */
	php_hash_uint32 count[2];				/* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64];	/* input buffer */
} PHP_MD5_CTX;

PHP_HASH_API void make_digest(char *md5str, unsigned char *digest);
PHP_HASH_API void PHP_MD5Init(PHP_MD5_CTX *);
PHP_HASH_API void PHP_MD5Update(PHP_MD5_CTX *, const unsigned char *, unsigned int);
PHP_HASH_API void PHP_MD5Final(unsigned char[16], PHP_MD5_CTX *);

PHP_NAMED_FUNCTION(php_if_md5);
PHP_NAMED_FUNCTION(php_if_md5_file);
#endif /* PHP_HASH_MD5_NOT_IN_CORE */

/* MD4 context */
typedef struct {
	php_hash_uint32 state[4];
	php_hash_uint32 count[2];
	unsigned char buffer[64];
} PHP_MD4_CTX;

#define PHP_MD4Init			PHP_MD5Init
PHP_HASH_API void PHP_MD4Update(PHP_MD4_CTX *context, const unsigned char *, unsigned int);
PHP_HASH_API void PHP_MD4Final(unsigned char[16], PHP_MD4_CTX *);

#endif
