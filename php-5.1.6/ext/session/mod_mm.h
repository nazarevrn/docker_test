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
   | Author: Sascha Schumann <sascha@schumann.cx>                         |
   +----------------------------------------------------------------------+
 */

/* $Id: mod_mm.h,v 1.12.2.1 2006/01/01 12:50:12 sniper Exp $ */

#ifndef MOD_MM_H
#define MOD_MM_H

#ifdef HAVE_LIBMM

#include "php_session.h"

PHP_MINIT_FUNCTION(ps_mm);
PHP_MSHUTDOWN_FUNCTION(ps_mm);

extern ps_module ps_mod_mm;
#define ps_mm_ptr &ps_mod_mm

PS_FUNCS(mm);

#endif
#endif
