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
   | Author: Sara Golemon <pollita@php.net>                               |
   +----------------------------------------------------------------------+
*/

/* $Id: php_hash_ripemd.h,v 1.2.2.3 2006/01/01 12:50:07 sniper Exp $ */

#ifndef PHP_HASH_RIPEMD_H
#define PHP_HASH_RIPEMD_H
#include "ext/standard/basic_functions.h"

/* RIPEMD context. */
typedef struct {
	php_hash_uint32 state[4];		/* state (ABCD) */
	php_hash_uint32 count[2];		/* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64];	/* input buffer */
} PHP_RIPEMD128_CTX;

typedef struct {
	php_hash_uint32 state[5];		/* state (ABCD) */
	php_hash_uint32 count[2];		/* number of bits, modulo 2^64 (lsb first) */
	unsigned char buffer[64];	/* input buffer */
} PHP_RIPEMD160_CTX;

PHP_HASH_API void PHP_RIPEMD128Init(PHP_RIPEMD128_CTX *);
PHP_HASH_API void PHP_RIPEMD128Update(PHP_RIPEMD128_CTX *, const unsigned char *, unsigned int);
PHP_HASH_API void PHP_RIPEMD128Final(unsigned char[16], PHP_RIPEMD128_CTX *);

PHP_HASH_API void PHP_RIPEMD160Init(PHP_RIPEMD160_CTX *);
PHP_HASH_API void PHP_RIPEMD160Update(PHP_RIPEMD160_CTX *, const unsigned char *, unsigned int);
PHP_HASH_API void PHP_RIPEMD160Final(unsigned char[20], PHP_RIPEMD160_CTX *);

#endif /* PHP_HASH_RIPEMD_H */
