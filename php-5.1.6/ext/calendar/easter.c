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
   | Authors: Shane Caraveo             <shane@caraveo.com>               | 
   |          Colin Viebrock            <colin@easydns.com>               |
   |          Hartmut Holzgraefe        <hholzgra@php.net>                |
   +----------------------------------------------------------------------+
 */
/* $Id: */

#include "php.h"
#include "php_calendar.h"
#include "sdncal.h"
#include <time.h>

static void _cal_easter(INTERNAL_FUNCTION_PARAMETERS, int gm)
{

	/* based on code by Simon Kershaw, <webmaster@ely.anglican.org> */

	struct tm te;
	long year, golden, solar, lunar, pfm, dom, tmp, easter;
	long method = CAL_EASTER_DEFAULT;

	/* Default to the current year if year parameter is not given */
	{
		time_t a;
		struct tm b;
		time(&a);
		php_localtime_r(&a, &b);
		year = 1900 + b.tm_year;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		"|ll", &year, &method) == FAILURE) {
			return;
	}
 
	if (gm && (year<1970 || year>2037)) {				/* out of range for timestamps */
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "This function is only valid for years between 1970 and 2037 inclusive");
		RETURN_FALSE;
	}

	golden = (year % 19) + 1;					/* the Golden number */

	if ((year <= 1582 && method != CAL_EASTER_ALWAYS_GREGORIAN) ||
	    (year >= 1583 && year <= 1752 && method != CAL_EASTER_ROMAN && method != CAL_EASTER_ALWAYS_GREGORIAN) ||
	     method == CAL_EASTER_ALWAYS_JULIAN) {		/* JULIAN CALENDAR */
	     
		dom = (year + (year/4) + 5) % 7;			/* the "Dominical number" - finding a Sunday */
		if (dom < 0) {
			dom += 7;
		}

		pfm = (3 - (11*golden) - 7) % 30;			/* uncorrected date of the Paschal full moon */
		if (pfm < 0) {
			pfm += 30;
		}
	} else {							/* GREGORIAN CALENDAR */
		dom = (year + (year/4) - (year/100) + (year/400)) % 7;	/* the "Domincal number" */
		if (dom < 0) {
			dom += 7;
		}

		solar = (year-1600)/100 - (year-1600)/400;		/* the solar and lunar corrections */
		lunar = (((year-1400) / 100) * 8) / 25;

		pfm = (3 - (11*golden) + solar - lunar) % 30;		/* uncorrected date of the Paschal full moon */
		if (pfm < 0) {
			pfm += 30;
		}
	}

	if ((pfm == 29) || (pfm == 28 && golden > 11)) {		/* corrected date of the Paschal full moon */
		pfm--;							/* - days after 21st March                 */
	}

	tmp = (4-pfm-dom) % 7;
	if (tmp < 0) {
		tmp += 7;
	}

	easter = pfm + tmp + 1;	    					/* Easter as the number of days after 21st March */

	if (gm) {							/* return a timestamp */
		te.tm_isdst = -1;
		te.tm_year = year-1900;
		te.tm_sec = 0;
		te.tm_min = 0;
		te.tm_hour = 0;

		if (easter < 11) {
			te.tm_mon = 2;			/* March */
			te.tm_mday = easter+21;
		} else {
			te.tm_mon = 3;			/* April */
			te.tm_mday = easter-10;
		}

	        Z_LVAL_P(return_value) = mktime(&te);
	} else {							/* return the days after March 21 */	
	        Z_LVAL_P(return_value) = easter;
	}

        Z_TYPE_P(return_value) = IS_LONG;

}

/* {{{ proto int easter_date([int year])
   Return the timestamp of midnight on Easter of a given year (defaults to current year) */
PHP_FUNCTION(easter_date)
{
	_cal_easter(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto int easter_days([int year, [int method]])
   Return the number of days after March 21 that Easter falls on for a given year (defaults to current year) */
PHP_FUNCTION(easter_days)
{
	_cal_easter(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
