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
   | Authors: Derick Rethans <derick@derickrethans.nl>                    |
   +----------------------------------------------------------------------+
 */

/* $Id: timelib_structs.h,v 1.13.2.6 2006/04/11 18:03:52 derick Exp $ */

#ifndef __TIMELIB_STRUCTS_H__
#define __TIMELIB_STRUCTS_H__

#include <timelib_config.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#ifndef HAVE_INT32_T
# if SIZEOF_INT == 4
typedef int int32_t;
# elif SIZEOF_LONG == 4
typedef long int int32_t;
# endif
#endif

#ifndef HAVE_UINT32_T
# if SIZEOF_INT == 4
typedef unsigned int uint32_t;
# elif SIZEOF_LONG == 4
typedef unsigned long int uint32_t;
# endif
#endif

#include <stdio.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#if defined(_MSC_VER)
typedef unsigned __int64 timelib_ull;
typedef __int64 timelib_sll;
#else
typedef unsigned long long timelib_ull;
typedef signed long long timelib_sll;
#endif

#if defined(_MSC_VER)
#define int32_t __int32
#define uint32_t unsigned __int32
#endif

typedef struct ttinfo
{
	int32_t      offset;
	int          isdst;
	unsigned int abbr_idx;

	unsigned int isstdcnt;
	unsigned int isgmtcnt;
} ttinfo;

typedef struct tlinfo
{
	int32_t  trans;
	int32_t  offset;
} tlinfo;

typedef struct timelib_tzinfo
{
	char    *name;
	uint32_t ttisgmtcnt;
	uint32_t ttisstdcnt;
	uint32_t leapcnt;
	uint32_t timecnt;
	uint32_t typecnt;
	uint32_t charcnt;

	int32_t *trans;
	unsigned char *trans_idx;

	ttinfo  *type;
	char    *timezone_abbr;

	tlinfo  *leap_times;
} timelib_tzinfo;

typedef struct timelib_rel_time {
	timelib_sll y, m, d; /* Years, Months and Days */
	timelib_sll h, i, s; /* Hours, mInutes and Seconds */

	int weekday; /* Stores the day in 'next monday' */
	int weekday_behavior; /* 0: the current day should *not* be counted when advancing forwards; 1: the current day *should* be counted */
} timelib_rel_time;

typedef struct timelib_time_offset {
	int32_t      offset;
	unsigned int leap_secs;
	unsigned int is_dst;
	char        *abbr;
	timelib_sll  transistion_time;
} timelib_time_offset;

typedef struct timelib_time {
	timelib_sll      y, m, d;     /* Year, Month, Day */
	timelib_sll      h, i, s;     /* Hour, mInute, Second */
	double           f;           /* Fraction */
	int              z;           /* GMT offset in minutes */
	char            *tz_abbr;     /* Timezone abbreviation (display only) */
	timelib_tzinfo  *tz_info;     /* Timezone structure */
	signed int       dst;         /* Flag if we were parsing a DST zone */
	timelib_rel_time relative;

	timelib_sll      sse;         /* Seconds since epoch */

	unsigned int   have_time, have_date, have_zone, have_relative, have_weekday_relative, have_weeknr_day;

	unsigned int   sse_uptodate; /* !0 if the sse member is up to date with the date/time members */
	unsigned int   tim_uptodate; /* !0 if the date/time members are up to date with the sse member */
	unsigned int   is_localtime; /*  1 if the current struct represents localtime, 0 if it is in GMT */
	unsigned int   zone_type;    /*  1 time offset,
	                              *  3 TimeZone identifier,
	                              *  2 TimeZone abbreviation */
} timelib_time;

typedef struct timelib_error_message {
	int         position;
	char        character;
	char       *message;
} timelib_error_message;

typedef struct timelib_error_container {
	int                           warning_count;
	struct timelib_error_message *warning_messages;
	int                           error_count;
	struct timelib_error_message *error_messages;
} timelib_error_container;

typedef struct _timelib_tz_lookup_table {
	char       *name;
	int         type;
	int         gmtoffset;
	char       *full_tz_name;
} timelib_tz_lookup_table;

typedef struct _timelib_tzdb_index_entry {
	char *id;
	unsigned int pos;
} timelib_tzdb_index_entry;

typedef struct _timelib_tzdb {
	char                     *version;
	int                       index_size;
	timelib_tzdb_index_entry *index;
	char                     *data;
} timelib_tzdb;

#define TIMELIB_ZONETYPE_OFFSET 1
#define TIMELIB_ZONETYPE_ABBR   2
#define TIMELIB_ZONETYPE_ID     3

#define SECS_PER_ERA 12622780800L
#define SECS_PER_DAY       86400
#define DAYS_PER_YEAR        365
#define DAYS_PER_LYEAR       366

#define timelib_is_leap(y) ((y) % 4 == 0 && ((y) % 100 != 0 || (y) % 400 == 0))

#define DEBUG(s)  if (0) { s }

#endif
