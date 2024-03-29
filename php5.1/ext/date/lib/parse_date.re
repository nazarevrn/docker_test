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

/* $Id$ */

#include "timelib.h"

#include <stdio.h>
#include <ctype.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#if defined(_MSC_VER)
# define strtoll(s, f, b) _atoi64(s)
#elif !defined(HAVE_STRTOLL)
# if defined(HAVE_ATOLL)
#  define strtoll(s, f, b) atoll(s)
# else
#  define strtoll(s, f, b) strtol(s, f, b)
# endif
#endif

#define TIMELIB_SECOND  1
#define TIMELIB_MINUTE  2
#define TIMELIB_HOUR    3
#define TIMELIB_DAY     4
#define TIMELIB_MONTH   5
#define TIMELIB_YEAR    6
#define TIMELIB_WEEKDAY 7

#define EOI      257
#define TIME     258
#define DATE     259

#define TIMELIB_XMLRPC_SOAP    260
#define TIMELIB_TIME12         261
#define TIMELIB_TIME24         262
#define TIMELIB_GNU_NOCOLON    263
#define TIMELIB_GNU_NOCOLON_TZ 264
#define TIMELIB_ISO_NOCOLON    265

#define TIMELIB_AMERICAN       266
#define TIMELIB_ISO_DATE       267
#define TIMELIB_DATE_FULL      268
#define TIMELIB_DATE_TEXT      269
#define TIMELIB_DATE_NOCOLON   270
#define TIMELIB_PG_YEARDAY     271
#define TIMELIB_PG_TEXT        272
#define TIMELIB_PG_REVERSE     273
#define TIMELIB_CLF            274
#define TIMELIB_DATE_NO_DAY    275
#define TIMELIB_SHORTDATE_WITH_TIME 276
#define TIMELIB_DATE_FULL_POINTED 277
#define TIMELIB_TIME24_WITH_ZONE 278
#define TIMELIB_ISO_WEEK       279

#define TIMELIB_TIMEZONE       300
#define TIMELIB_AGO            301

#define TIMELIB_RELATIVE       310

#define TIMELIB_ERROR          999

typedef unsigned char uchar;

#define   BSIZE	   8192

#define   YYCTYPE      uchar
#define   YYCURSOR     cursor
#define   YYLIMIT      s->lim
#define   YYMARKER     s->ptr
#define   YYFILL(n)    return EOI;

#define   RET(i)       {s->cur = cursor; return i;}

#define timelib_string_free free

#define TIMELIB_HAVE_TIME() { if (s->time->have_time) { add_error(s, "Double time specification"); timelib_string_free(str); return TIMELIB_ERROR; } else { s->time->have_time = 1; s->time->h = 0; s->time->i = 0; s->time->s = 0; s->time->f = 0; } }
#define TIMELIB_UNHAVE_TIME() { s->time->have_time = 0; s->time->h = 0; s->time->i = 0; s->time->s = 0; s->time->f = 0; }
#define TIMELIB_HAVE_DATE() { if (s->time->have_date) { add_error(s, "Double date specification"); timelib_string_free(str); return TIMELIB_ERROR; } else { s->time->have_date = 1; } }
#define TIMELIB_UNHAVE_DATE() { s->time->have_date = 0; s->time->d = 0; s->time->m = 0; s->time->y = 0; }
#define TIMELIB_HAVE_RELATIVE() { s->time->have_relative = 1; s->time->relative.weekday_behavior = 0; }
#define TIMELIB_HAVE_WEEKDAY_RELATIVE() { s->time->have_weekday_relative = 1; }
#define TIMELIB_HAVE_TZ() { s->cur = cursor; if (s->time->have_zone) { add_warning(s, "Double timezone specification"); timelib_string_free(str); return TIMELIB_ERROR; } else { s->time->have_zone = 1; } }

#define TIMELIB_INIT  s->cur = cursor; str = timelib_string(s); ptr = str
#define TIMELIB_DEINIT timelib_string_free(str)
#define TIMELIB_ADJUST_RELATIVE_WEEKDAY() if (in->time.have_weekday_relative && (in.rel.d > 0)) { in.rel.d -= 7; }

#define TIMELIB_PROCESS_YEAR(x) { \
	if ((x) == -1) {         \
	/*	(x) = 0; */          \
	} else if ((x) < 100) {  \
		if ((x) < 70) {      \
			(x) += 2000;     \
		} else {             \
			(x) += 1900;     \
		}                    \
	}                        \
}

#ifdef DEBUG_PARSER
#define DEBUG_OUTPUT(s) printf("%s\n", s);
#define YYDEBUG(s,c) { if (s != -1) { printf("state: %d ", s); printf("[%c]\n", c); } }
#else
#define DEBUG_OUTPUT(s)
#define YYDEBUG(s,c)
#endif

#include "timelib_structs.h"

typedef struct timelib_elems {
	unsigned int   c; /* Number of elements */
	char         **v; /* Values */
} timelib_elems;

typedef struct Scanner {
	int           fd;
	uchar        *lim, *str, *ptr, *cur, *tok, *pos;
	unsigned int  line, len;
	struct timelib_error_container *errors;

	struct timelib_time *time;
	timelib_tzdb        *tzdb;
} Scanner;

typedef struct _timelib_lookup_table {
    const char *name;
    int         type;
    int         value;
} timelib_lookup_table;

typedef struct _timelib_relunit {
	const char *name;
	int         unit;
	int         multiplier;
} timelib_relunit;

#define HOUR(a) (int)(a * 60)

/* The timezone table. */
static timelib_tz_lookup_table timelib_timezone_lookup[] = {
#include "timezonemap.h"
	{ NULL, 0, 0, NULL },
};

static timelib_tz_lookup_table timelib_timezone_fallbackmap[] = {
#include "fallbackmap.h"
	{ NULL, 0, 0, NULL },
};

static timelib_tz_lookup_table timelib_timezone_utc[] = {
	{ "utc", 0, 0, "UTC" },
};

static timelib_relunit const timelib_relunit_lookup[] = {
	{ "sec",         TIMELIB_SECOND,  1 },
	{ "secs",        TIMELIB_SECOND,  1 },
	{ "second",      TIMELIB_SECOND,  1 },
	{ "seconds",     TIMELIB_SECOND,  1 },
	{ "min",         TIMELIB_MINUTE,  1 },
	{ "mins",        TIMELIB_MINUTE,  1 },
	{ "minute",      TIMELIB_MINUTE,  1 },
	{ "minutes",     TIMELIB_MINUTE,  1 },
	{ "hour",        TIMELIB_HOUR,    1 },
	{ "hours",       TIMELIB_HOUR,    1 },
	{ "day",         TIMELIB_DAY,     1 },
	{ "days",        TIMELIB_DAY,     1 },
	{ "week",        TIMELIB_DAY,     7 },
	{ "weeks",       TIMELIB_DAY,     7 },
	{ "fortnight",   TIMELIB_DAY,    14 },
	{ "fortnights",  TIMELIB_DAY,    14 },
	{ "forthnight",  TIMELIB_DAY,    14 },
	{ "forthnights", TIMELIB_DAY,    14 },
	{ "month",       TIMELIB_MONTH,   1 },
	{ "months",      TIMELIB_MONTH,   1 },
	{ "year",        TIMELIB_YEAR,    1 },
	{ "years",       TIMELIB_YEAR,    1 },

	{ "monday",      TIMELIB_WEEKDAY, 1 },
	{ "mon",         TIMELIB_WEEKDAY, 1 },
	{ "tuesday",     TIMELIB_WEEKDAY, 2 },
	{ "tue",         TIMELIB_WEEKDAY, 2 },
	{ "wednesday",   TIMELIB_WEEKDAY, 3 },
	{ "wed",         TIMELIB_WEEKDAY, 3 },
	{ "thursday",    TIMELIB_WEEKDAY, 4 },
	{ "thu",         TIMELIB_WEEKDAY, 4 },
	{ "friday",      TIMELIB_WEEKDAY, 5 },
	{ "fri",         TIMELIB_WEEKDAY, 5 },
	{ "saturday",    TIMELIB_WEEKDAY, 6 },
	{ "sat",         TIMELIB_WEEKDAY, 6 },
	{ "sunday",      TIMELIB_WEEKDAY, 0 },
	{ "sun",         TIMELIB_WEEKDAY, 0 },

	{ NULL,          0,          0 }
};

/* The relative text table. */
static timelib_lookup_table const timelib_reltext_lookup[] = {
	{ "first",    0,  1 },
	{ "next",     0,  1 },
	{ "second",   0,  2 },
	{ "third",    0,  3 },
	{ "fourth",   0,  4 },
	{ "fifth",    0,  5 },
	{ "sixth",    0,  6 },
	{ "seventh",  0,  7 },
	{ "eight",    0,  8 },
	{ "ninth",    0,  9 },
	{ "tenth",    0, 10 },
	{ "eleventh", 0, 11 },
	{ "twelfth",  0, 12 },
	{ "last",     0, -1 },
	{ "previous", 0, -1 },
	{ "this",     1,  0 },
	{ NULL,       1,  0 }
};

/* The month table. */
static timelib_lookup_table const timelib_month_lookup[] = {
	{ "jan",  0,  1 },
	{ "feb",  0,  2 },
	{ "mar",  0,  3 },
	{ "apr",  0,  4 },
	{ "may",  0,  5 },
	{ "jun",  0,  6 },
	{ "jul",  0,  7 },
	{ "aug",  0,  8 },
	{ "sep",  0,  9 },
	{ "sept", 0,  9 },
	{ "oct",  0, 10 },
	{ "nov",  0, 11 },
	{ "dec",  0, 12 },
	{ "i",    0,  1 },
	{ "ii",   0,  2 },
	{ "iii",  0,  3 },
	{ "iv",   0,  4 },
	{ "v",    0,  5 },
	{ "vi",   0,  6 },
	{ "vii",  0,  7 },
	{ "viii", 0,  8 },
	{ "ix",   0,  9 },
	{ "x",    0, 10 },
	{ "xi",   0, 11 },
	{ "xii",  0, 12 },

	{ "january",   0,  1 },
	{ "february",  0,  2 },
	{ "march",     0,  3 },
	{ "april",     0,  4 },
	{ "may",       0,  5 },
	{ "june",      0,  6 },
	{ "july",      0,  7 },
	{ "august",    0,  8 },
	{ "september", 0,  9 },
	{ "october",   0, 10 },
	{ "november",  0, 11 },
	{ "december",  0, 12 },
	{  NULL,       0,  0 }
};

#if 0
static char* timelib_ltrim(char *s)
{
	char *ptr = s;
	while (ptr[0] == ' ' || ptr[0] == '\t') {
		ptr++;
	}
	return ptr;
}
#endif

#if 0
uchar *fill(Scanner *s, uchar *cursor){
	if(!s->eof){
		unsigned int cnt = s->tok - s->bot;
		if(cnt){
			memcpy(s->bot, s->tok, s->lim - s->tok);
			s->tok = s->bot;
			s->ptr -= cnt;
			cursor -= cnt;
			s->pos -= cnt;
			s->lim -= cnt;
		}
		if((s->top - s->lim) < BSIZE){
			uchar *buf = (uchar*) malloc(((s->lim - s->bot) + BSIZE)*sizeof(uchar));
			memcpy(buf, s->tok, s->lim - s->tok);
			s->tok = buf;
			s->ptr = &buf[s->ptr - s->bot];
			cursor = &buf[cursor - s->bot];
			s->pos = &buf[s->pos - s->bot];
			s->lim = &buf[s->lim - s->bot];
			s->top = &s->lim[BSIZE];
			free(s->bot);
			s->bot = buf;
		}
		if((cnt = read(s->fd, (char*) s->lim, BSIZE)) != BSIZE){
			s->eof = &s->lim[cnt]; *(s->eof)++ = '\n';
		}
		s->lim += cnt;
	}
	return cursor;
}
#endif

static void add_warning(Scanner *s, char *error)
{
	s->errors->warning_count++;
	s->errors->warning_messages = realloc(s->errors->warning_messages, s->errors->warning_count * sizeof(timelib_error_message));
	s->errors->warning_messages[s->errors->warning_count - 1].position = s->tok ? s->tok - s->str : 0;
	s->errors->warning_messages[s->errors->warning_count - 1].character = s->tok ? *s->tok : 0;
	s->errors->warning_messages[s->errors->warning_count - 1].message = strdup(error);
}

static void add_error(Scanner *s, char *error)
{
	s->errors->error_count++;
	s->errors->error_messages = realloc(s->errors->error_messages, s->errors->error_count * sizeof(timelib_error_message));
	s->errors->error_messages[s->errors->error_count - 1].position = s->tok ? s->tok - s->str : 0;
	s->errors->error_messages[s->errors->error_count - 1].character = s->tok ? *s->tok : 0;
	s->errors->error_messages[s->errors->error_count - 1].message = strdup(error);
}

static timelib_sll timelib_meridian(char **ptr, timelib_sll h)
{
	timelib_sll retval = 0;

	while (!strchr("AaPp", **ptr)) {
		++*ptr;
	}
	if (**ptr == 'a' || **ptr == 'A') {
		if (h == 12) {
			retval = -12;
		}
	} else if (h != 12) {
		retval = 12;
	}
	++*ptr;
	if (**ptr == '.') {
		*ptr += 3;
	} else {
		++*ptr;
	}
	return retval;
}

static char *timelib_string(Scanner *s)
{
	char *tmp = calloc(1, s->cur - s->tok + 1);
	memcpy(tmp, s->tok, s->cur - s->tok);

	return tmp;
}

static timelib_sll timelib_get_nr(char **ptr, int max_length)
{
	char *begin, *end, *str;
	timelib_sll tmp_nr = -1;
	int len = 0;

	while ((**ptr < '0') || (**ptr > '9')) {
		if (**ptr == '\0') {
			return -1;
		}
		++*ptr;
	}
	begin = *ptr;
	while ((**ptr >= '0') && (**ptr <= '9') && len < max_length) {
		++*ptr;
		++len;
	}
	end = *ptr;
	str = calloc(1, end - begin + 1);
	memcpy(str, begin, end - begin);
	tmp_nr = strtoll(str, NULL, 10);
	free(str);
	return tmp_nr;
}

static void timelib_skip_day_suffix(char **ptr)
{
	if (isspace(**ptr)) {
		return;
	}
	if (!strncasecmp(*ptr, "nd", 2) || !strncasecmp(*ptr, "rd", 2) ||!strncasecmp(*ptr, "st", 2) || !strncasecmp(*ptr, "th", 2)) {
		*ptr += 2;
	}
}

static double timelib_get_frac_nr(char **ptr, int max_length)
{
	char *begin, *end, *str;
	double tmp_nr = -1;
	int len = 0;

	while ((**ptr != '.') && ((**ptr < '0') || (**ptr > '9'))) {
		if (**ptr == '\0') {
			return -1;
		}
		++*ptr;
	}
	begin = *ptr;
	while (((**ptr == '.') || ((**ptr >= '0') && (**ptr <= '9'))) && len < max_length) {
		++*ptr;
		++len;
	}
	end = *ptr;
	str = calloc(1, end - begin + 1);
	memcpy(str, begin, end - begin);
	tmp_nr = strtod(str, NULL);
	free(str);
	return tmp_nr;
}

static timelib_ull timelib_get_unsigned_nr(char **ptr, int max_length)
{
	timelib_ull dir = 1;

	while (((**ptr < '0') || (**ptr > '9')) && (**ptr != '+') && (**ptr != '-')) {
		if (**ptr == '\0') {
			return -1;
		}
		++*ptr;
	}
	if (**ptr == '+') {
		++*ptr;
	} else if (**ptr == '-') {
		dir = -1;
		++*ptr;
	}
	return dir * timelib_get_nr(ptr, max_length);
}

static long timelib_parse_tz_cor(char **ptr)
{
	char *begin = *ptr, *end;
	long  tmp;

	while (**ptr != '\0') {
		++*ptr;
	}
	end = *ptr;
	switch (end - begin) {
		case 1:
		case 2:
			return HOUR(strtol(begin, NULL, 10));
			break;
		case 3:
		case 4:
			if (begin[1] == ':') {
				tmp = HOUR(strtol(begin, NULL, 10)) + strtol(begin + 2, NULL, 10);
				return tmp;
			} else if (begin[2] == ':') {
				tmp = HOUR(strtol(begin, NULL, 10)) + strtol(begin + 3, NULL, 10);
				return tmp;
			} else {
				tmp = strtol(begin, NULL, 10);
				return HOUR(tmp / 100) + tmp % 100;
			}
		case 5:
			tmp = HOUR(strtol(begin, NULL, 10)) + strtol(begin + 3, NULL, 10);
			return tmp;
	}
	return 0;
}

static timelib_sll timelib_lookup_relative_text(char **ptr, int *behavior)
{
	char *word;
	char *begin = *ptr, *end;
	timelib_sll  value = 0;
	const timelib_lookup_table *tp;

	while ((**ptr >= 'A' && **ptr <= 'Z') || (**ptr >= 'a' && **ptr <= 'z')) {
		++*ptr;
	}
	end = *ptr;
	word = calloc(1, end - begin + 1);
	memcpy(word, begin, end - begin);

	for (tp = timelib_reltext_lookup; tp->name; tp++) {
		if (strcasecmp(word, tp->name) == 0) {
			value = tp->value;
			*behavior = tp->type;
		}
	}

	free(word);
	return value;
}

static timelib_sll timelib_get_relative_text(char **ptr, int *behavior)
{
	while (**ptr == ' ' || **ptr == '\t' || **ptr == '-' || **ptr == '/') {
		++*ptr;
	}
	return timelib_lookup_relative_text(ptr, behavior);
}

static long timelib_lookup_month(char **ptr)
{
	char *word;
	char *begin = *ptr, *end;
	long  value = 0;
	const timelib_lookup_table *tp;

	while ((**ptr >= 'A' && **ptr <= 'Z') || (**ptr >= 'a' && **ptr <= 'z')) {
		++*ptr;
	}
	end = *ptr;
	word = calloc(1, end - begin + 1);
	memcpy(word, begin, end - begin);

	for (tp = timelib_month_lookup; tp->name; tp++) {
		if (strcasecmp(word, tp->name) == 0) {
			value = tp->value;
		}
	}

	free(word);
	return value;
}

static long timelib_get_month(char **ptr)
{
	while (**ptr == ' ' || **ptr == '\t' || **ptr == '-' || **ptr == '.' || **ptr == '/') {
		++*ptr;
	}
	return timelib_lookup_month(ptr);
}

static void timelib_eat_spaces(char **ptr)
{
	while (**ptr == ' ' || **ptr == '\t') {
		++*ptr;
	}
}

static const timelib_relunit* timelib_lookup_relunit(char **ptr)
{
	char *word;
	char *begin = *ptr, *end;
	const timelib_relunit *tp, *value = NULL;

	while (**ptr != '\0' && **ptr != ' ' && **ptr != '\t') {
		++*ptr;
	}
	end = *ptr;
	word = calloc(1, end - begin + 1);
	memcpy(word, begin, end - begin);

	for (tp = timelib_relunit_lookup; tp->name; tp++) {
		if (strcasecmp(word, tp->name) == 0) {
			value = tp;
			break;
		}
	}

	free(word);
	return value;
}

static void timelib_set_relative(char **ptr, timelib_sll amount, int behavior, Scanner *s)
{
	const timelib_relunit* relunit;

	relunit = timelib_lookup_relunit(ptr);
	switch (relunit->unit) {
		case TIMELIB_SECOND: s->time->relative.s += amount * relunit->multiplier; break;
		case TIMELIB_MINUTE: s->time->relative.i += amount * relunit->multiplier; break;
		case TIMELIB_HOUR:   s->time->relative.h += amount * relunit->multiplier; break;
		case TIMELIB_DAY:    s->time->relative.d += amount * relunit->multiplier; break;
		case TIMELIB_MONTH:  s->time->relative.m += amount * relunit->multiplier; break;
		case TIMELIB_YEAR:   s->time->relative.y += amount * relunit->multiplier; break;

		case TIMELIB_WEEKDAY:
			TIMELIB_HAVE_WEEKDAY_RELATIVE();
			TIMELIB_UNHAVE_TIME();
			s->time->relative.d += (amount > 0 ? amount - 1 : amount) * 7;
			s->time->relative.weekday = relunit->multiplier;
			s->time->relative.weekday_behavior = behavior;
			break;
	}
}

static timelib_tz_lookup_table* zone_search(const char *word, long gmtoffset, int isdst)
{
	int first_found = 0;
	timelib_tz_lookup_table  *tp, *first_found_elem = NULL;
	timelib_tz_lookup_table  *fmp;

	if (strcasecmp("utc", word) == 0 || strcasecmp("gmt", word) == 0) {
		return timelib_timezone_utc;
	}
	
	for (tp = timelib_timezone_lookup; tp->name; tp++) {
		if (strcasecmp(word, tp->name) == 0) {
			if (!first_found) {
				first_found = 1;
				first_found_elem = tp;
				if (gmtoffset == -1) {
					return tp;
				}
			}
			if (tp->gmtoffset == gmtoffset) {
				return tp;
			}
		}
	}
	if (first_found) {
		return first_found_elem;
	}

	/* Still didn't find anything, let's find the zone solely based on
	 * offset/isdst then */
	for (fmp = timelib_timezone_fallbackmap; fmp->name; fmp++) {
		if ((fmp->gmtoffset * 3600) == gmtoffset && fmp->type == isdst) {
			return fmp;
		}
	}
	return NULL;
}

static long timelib_lookup_zone(char **ptr, int *dst, char **tz_abbr, int *found)
{
	char *word;
	char *begin = *ptr, *end;
	long  value = 0;
	timelib_tz_lookup_table *tp;

	while (**ptr != '\0' && **ptr != ')') {
		++*ptr;
	}
	end = *ptr;
	word = calloc(1, end - begin + 1);
	memcpy(word, begin, end - begin);

	if ((tp = zone_search(word, -1, 0))) {
		value = -tp->gmtoffset / 60;
		*dst = tp->type;
		value += tp->type * 60;
		*found = 1;
	} else {
		*found = 0;
	}

	*tz_abbr = word;
	return value;
}

static long timelib_get_zone(char **ptr, int *dst, timelib_time *t, int *tz_not_found, timelib_tzdb *tzdb)
{
	timelib_tzinfo *res;
	long            retval = 0;

	*tz_not_found = 0;

	while (**ptr == ' ' || **ptr == '\t' || **ptr == '(') {
		++*ptr;
	}
	if (**ptr == '+') {
		++*ptr;
		t->is_localtime = 1;
		t->zone_type = TIMELIB_ZONETYPE_OFFSET;
		*tz_not_found = 0;
		t->dst = 0;

		retval = -1 * timelib_parse_tz_cor(ptr);
	} else if (**ptr == '-') {
		++*ptr;
		t->is_localtime = 1;
		t->zone_type = TIMELIB_ZONETYPE_OFFSET;
		*tz_not_found = 0;
		t->dst = 0;

		retval = timelib_parse_tz_cor(ptr);
	} else {
		int found = 0;
		long offset;
		char *tz_abbr;

		t->is_localtime = 1;

		offset = timelib_lookup_zone(ptr, dst, &tz_abbr, &found);
		if (found) {
			t->zone_type = TIMELIB_ZONETYPE_ABBR;
		}
#if 0
		/* If we found a TimeZone identifier, use it */
		if (tz_name) {
			t->tz_info = timelib_parse_tzfile(tz_name);
			t->zone_type = TIMELIB_ZONETYPE_ID;
		}
#endif
		/* If we have a TimeZone identifier to start with, use it */
		if (strstr(tz_abbr, "/")) {
			if ((res = timelib_parse_tzfile(tz_abbr, tzdb)) != NULL) {
				t->tz_info = res;
				t->zone_type = TIMELIB_ZONETYPE_ID;
				found++;
			}
		}
		if (found && t->zone_type != TIMELIB_ZONETYPE_ID) {
			timelib_time_tz_abbr_update(t, tz_abbr);
		}
		free(tz_abbr);
		*tz_not_found = (found == 0);
		retval = offset;
	}
	while (**ptr == ')') {
		++*ptr;
	}
	return retval;
}

#define timelib_split_free(arg) {       \
	int i;                         \
	for (i = 0; i < arg.c; i++) {  \
		free(arg.v[i]);            \
	}                              \
	if (arg.v) {                   \
		free(arg.v);               \
	}                              \
}

static int scan(Scanner *s)
{
	uchar *cursor = s->cur;
	char *str, *ptr = NULL;
		
std:
	s->tok = cursor;
	s->len = 0;
/*!re2c
any = [\000-\377];

space = [ \t]+;
frac = "."[0-9]+;

ago = 'ago';

hour24 = [01]?[0-9] | "2"[0-3];
hour24lz = [01][0-9] | "2"[0-3];
hour12 = "0"?[1-9] | "1"[0-2];
minute = [0-5]?[0-9];
minutelz = [0-5][0-9];
second = minute | "60";
secondlz = minutelz | "60";
meridian = ([AaPp] "."? [Mm] "."?) [\000\t ];
tz = "("? [A-Za-z]{1,6} ")"? | [A-Z][a-z]+([_/][A-Z][a-z]+)+;
tzcorrection = [+-] hour24 ":"? minute?;

daysuf = "st" | "nd" | "rd" | "th";

month = "0"? [0-9] | "1"[0-2];
day   = ([0-2]?[0-9] | "3"[01]) daysuf?;
year  = [0-9]{1,4};
year2 = [0-9]{2};
year4 = [0-9]{4};

dayofyear = "00"[1-9] | "0"[1-9][0-9] | [1-2][0-9][0-9] | "3"[0-5][0-9] | "36"[0-6];
weekofyear = "0"[1-9] | [1-4][0-9] | "5"[0-3];

monthlz = "0" [1-9] | "1" [0-2];
daylz   = "0" [1-9] | [1-2][0-9] | "3" [01];

dayfull = 'sunday' | 'monday' | 'tuesday' | 'wednesday' | 'thursday' | 'friday' | 'saturday';
dayabbr = 'sun' | 'mon' | 'tue' | 'wed' | 'thu' | 'fri' | 'sat' | 'sun';
daytext = dayfull | dayabbr;

monthfull = 'january' | 'february' | 'march' | 'april' | 'may' | 'june' | 'july' | 'august' | 'september' | 'october' | 'november' | 'december';
monthabbr = 'jan' | 'feb' | 'mar' | 'apr' | 'may' | 'jun' | 'jul' | 'aug' | 'sep' | 'sept' | 'oct' | 'nov' | 'dec';
monthroman = "I" | "II" | "III" | "IV" | "V" | "VI" | "VII" | "VIII" | "IX" | "X" | "XI" | "XII";
monthtext = monthfull | monthabbr | monthroman;

/* Time formats */
timetiny12 = hour12 space? meridian;
timeshort12 = hour12[:.]minutelz space? meridian;
timelong12 = hour12[:.]minute[:.]secondlz space? meridian;

timeshort24 = 't'? hour24[:.]minute;
timelong24 =  't'? hour24[:.]minute[:.]second;
iso8601long =  't'? hour24 [:.] minute [:.] second frac;

/* iso8601shorttz = hour24 [:] minutelz space? (tzcorrection | tz); */
iso8601normtz =  't'? hour24 [:.] minute [:.] secondlz space? (tzcorrection | tz);
/* iso8601longtz =  hour24 [:] minute [:] secondlz frac space? (tzcorrection | tz); */

gnunocolon       = 't'? hour24lz minutelz;
/* gnunocolontz     = hour24lz minutelz space? (tzcorrection | tz); */
iso8601nocolon   = 't'? hour24lz minutelz secondlz; 
/* iso8601nocolontz = hour24lz minutelz secondlz space? (tzcorrection | tz); */

/* Date formats */
americanshort    = month "/" day;
american         = month "/" day "/" year;
iso8601dateslash = year4 "/" monthlz "/" daylz "/"?;
dateslash        = year4 "/" month "/" day;
gnudateshort     = year "-" month "-" day;
iso8601date      = year4 "-" monthlz "-" daylz;
pointeddate      = day [.\t-] month [.-] year;
datefull         = day ([ \t.-])* monthtext ([ \t.-])* year;
datenoday        = monthtext ([ .\t-])* year4;
datenodayrev     = year4 ([ .\t-])* monthtext;
datetextual      = monthtext ([ .\t-])* day [,.stndrh\t ]* year;
datenoyear       = monthtext ([ .\t-])* day [,.stndrh\t ]*;
datenoyearrev    = day ([ .\t-])* monthtext;
datenocolon      = year4 monthlz daylz;

/* Special formats */
soap             = year4 "-" monthlz "-" daylz "T" hour24lz ":" minutelz ":" secondlz frac tzcorrection?;
xmlrpc           = year4 monthlz daylz "T" hour24 ":" minutelz ":" secondlz;
xmlrpcnocolon    = year4 monthlz daylz 't' hour24 minutelz secondlz;
wddx             = year4 "-" month "-" day "T" hour24 ":" minute ":" second;
pgydotd          = year4 "."? dayofyear;
pgtextshort      = monthabbr "-" daylz "-" year;
pgtextreverse    = year "-" monthabbr "-" daylz;
isoweekday       = year4 "W" weekofyear [0-7];
isoweek          = year4 "W" weekofyear;
exif             = year4 ":" monthlz ":" daylz " " hour24lz ":" minutelz ":" secondlz;

/* Common Log Format: 10/Oct/2000:13:55:36 -0700 */
clf              = day "/" monthabbr "/" year4 ":" hour24lz ":" minutelz ":" secondlz space tzcorrection;

/* Timestamp format: @1126396800 */
timestamp        = "@" "-"? [0-9]+;

/* To fix some ambiguities */
dateshortwithtimeshort  = datenoyear timeshort24;
dateshortwithtimelong   = datenoyear timelong24;
dateshortwithtimelongtz = datenoyear iso8601normtz;

/*
 * Relative regexps
 */
reltextnumber = 'first'|'next'|'second'|'third'|'fourth'|'fifth'|'sixth'|'seventh'|'eight'|'ninth'|'tenth'|'eleventh'|'twelfth'|'last'|'previous'|'this';
reltextunit = (('sec'|'second'|'min'|'minute'|'hour'|'day'|'week'|'fortnight'|'forthnight'|'month'|'year') 's'?) | daytext;

relnumber = ([+-]?[ \t]*[0-9]+);
relative = relnumber space? reltextunit;
relativetext = reltextnumber space? reltextunit;

*/

/*!re2c
	/* so that vim highlights correctly */
	'yesterday'
	{
		DEBUG_OUTPUT("yesterday");
		TIMELIB_INIT;
		TIMELIB_HAVE_RELATIVE();
		TIMELIB_UNHAVE_TIME();

		s->time->relative.d = -1;
		TIMELIB_DEINIT;
		return TIMELIB_RELATIVE;
	}

	'now'
	{
		DEBUG_OUTPUT("now");
		TIMELIB_INIT;

		TIMELIB_DEINIT;
		return TIMELIB_RELATIVE;
	}

	'noon'
	{
		DEBUG_OUTPUT("noon");
		TIMELIB_INIT;
		TIMELIB_UNHAVE_TIME();
		TIMELIB_HAVE_TIME();
		s->time->h = 12;

		TIMELIB_DEINIT;
		return TIMELIB_RELATIVE;
	}

	'midnight' | 'today'
	{
		DEBUG_OUTPUT("midnight | today");
		TIMELIB_INIT;
		TIMELIB_UNHAVE_TIME();

		TIMELIB_DEINIT;
		return TIMELIB_RELATIVE;
	}

	'tomorrow'
	{
		DEBUG_OUTPUT("tomorrow");
		TIMELIB_INIT;
		TIMELIB_HAVE_RELATIVE();
		TIMELIB_UNHAVE_TIME();

		s->time->relative.d = 1;
		TIMELIB_DEINIT;
		return TIMELIB_RELATIVE;
	}

	timestamp
	{
		timelib_ull i;

		TIMELIB_INIT;
		TIMELIB_HAVE_RELATIVE();
		TIMELIB_UNHAVE_DATE();
		TIMELIB_UNHAVE_TIME();

		i = timelib_get_unsigned_nr((char **) &ptr, 24);
		s->time->y = 1970;
		s->time->m = 1;
		s->time->d = 1;
		s->time->h = s->time->i = s->time->s = 0;
		s->time->f = 0.0;
		s->time->relative.s += i;
		s->time->is_localtime = 1;
		s->time->zone_type = TIMELIB_ZONETYPE_OFFSET;
		s->time->z = 0;

		TIMELIB_DEINIT;
		return TIMELIB_RELATIVE;
	}

	timetiny12 | timeshort12 | timelong12
	{
		DEBUG_OUTPUT("timetiny12 | timeshort12 | timelong12");
		TIMELIB_INIT;
		TIMELIB_HAVE_TIME();
		s->time->h = timelib_get_nr((char **) &ptr, 2);
		if (*ptr == ':' || *ptr == '.') {
			s->time->i = timelib_get_nr((char **) &ptr, 2);
			if (*ptr == ':' || *ptr == '.') {
				s->time->s = timelib_get_nr((char **) &ptr, 2);
			}
		}
		s->time->h += timelib_meridian((char **) &ptr, s->time->h);
		TIMELIB_DEINIT;
		return TIMELIB_TIME12;
	}

	timeshort24 | timelong24 /* | iso8601short | iso8601norm */ | iso8601long /*| iso8601shorttz | iso8601normtz | iso8601longtz*/
	{
		int tz_not_found;
		DEBUG_OUTPUT("timeshort24 | timelong24 | iso8601long");
		TIMELIB_INIT;
		TIMELIB_HAVE_TIME();
		s->time->h = timelib_get_nr((char **) &ptr, 2);
		s->time->i = timelib_get_nr((char **) &ptr, 2);
		if (*ptr == ':' || *ptr == '.') {
			s->time->s = timelib_get_nr((char **) &ptr, 2);

			if (*ptr == '.') {
				s->time->f = timelib_get_frac_nr((char **) &ptr, 8);
			}
		}

		if (*ptr != '\0') {
			s->time->z = timelib_get_zone((char **) &ptr, &s->time->dst, s->time, &tz_not_found, s->tzdb);
			if (tz_not_found) {
				add_error(s, "The timezone could not be found in the database");
			}
		}
		TIMELIB_DEINIT;
		return TIMELIB_TIME24_WITH_ZONE;
	}

	gnunocolon
	{
		DEBUG_OUTPUT("gnunocolon");
		TIMELIB_INIT;
		switch (s->time->have_time) {
			case 0:
				s->time->h = timelib_get_nr((char **) &ptr, 2);
				s->time->i = timelib_get_nr((char **) &ptr, 2);
				s->time->s = 0;
				break;
			case 1:
				s->time->y = timelib_get_nr((char **) &ptr, 4);
				break;
			default:
				TIMELIB_DEINIT;
				add_error(s, "Double time specification");
				return TIMELIB_ERROR;
		}
		s->time->have_time++;
		TIMELIB_DEINIT;
		return TIMELIB_GNU_NOCOLON;
	}
/*
	gnunocolontz
	{
		DEBUG_OUTPUT("gnunocolontz");
		TIMELIB_INIT;
		switch (s->time->have_time) {
			case 0:
				s->time->h = timelib_get_nr((char **) &ptr, 2);
				s->time->i = timelib_get_nr((char **) &ptr, 2);
				s->time->s = 0;
				s->time->z = timelib_get_zone((char **) &ptr, &s->time->dst, s->time, s->tzdb);
				break;
			case 1:
				s->time->y = timelib_get_nr((char **) &ptr, 4);
				break;
			default:
				TIMELIB_DEINIT;
				return TIMELIB_ERROR;
		}
		s->time->have_time++;
		TIMELIB_DEINIT;
		return TIMELIB_GNU_NOCOLON_TZ;
	}
*/
	iso8601nocolon /*| iso8601nocolontz*/
	{
		int tz_not_found;
		DEBUG_OUTPUT("iso8601nocolon");
		TIMELIB_INIT;
		TIMELIB_HAVE_TIME();
		s->time->h = timelib_get_nr((char **) &ptr, 2);
		s->time->i = timelib_get_nr((char **) &ptr, 2);
		s->time->s = timelib_get_nr((char **) &ptr, 2);

		if (*ptr != '\0') {
			s->time->z = timelib_get_zone((char **) &ptr, &s->time->dst, s->time, &tz_not_found, s->tzdb);
			if (tz_not_found) {
				add_error(s, "The timezone could not be found in the database");
			}
		}
		TIMELIB_DEINIT;
		return TIMELIB_ISO_NOCOLON;
	}

	americanshort | american
	{
		DEBUG_OUTPUT("americanshort | american");
		TIMELIB_INIT;
		TIMELIB_HAVE_DATE();
		s->time->m = timelib_get_nr((char **) &ptr, 2);
		s->time->d = timelib_get_nr((char **) &ptr, 2);
		if (*ptr == '/') {
			s->time->y = timelib_get_nr((char **) &ptr, 4);
			TIMELIB_PROCESS_YEAR(s->time->y);
		}
		TIMELIB_DEINIT;
		return TIMELIB_AMERICAN;
	}

	iso8601date | iso8601dateslash | dateslash
	{
		DEBUG_OUTPUT("iso8601date | iso8601dateslash | dateslash");
		TIMELIB_INIT;
		TIMELIB_HAVE_DATE();
		s->time->y = timelib_get_nr((char **) &ptr, 4);
		s->time->m = timelib_get_nr((char **) &ptr, 2);
		s->time->d = timelib_get_nr((char **) &ptr, 2);
		TIMELIB_DEINIT;
		return TIMELIB_ISO_DATE;
	}

	gnudateshort
	{
		DEBUG_OUTPUT("gnudateshort");
		TIMELIB_INIT;
		TIMELIB_HAVE_DATE();
		s->time->y = timelib_get_nr((char **) &ptr, 4);
		s->time->m = timelib_get_nr((char **) &ptr, 2);
		s->time->d = timelib_get_nr((char **) &ptr, 2);
		TIMELIB_PROCESS_YEAR(s->time->y);
		TIMELIB_DEINIT;
		return TIMELIB_ISO_DATE;
	}

	datefull
	{
		DEBUG_OUTPUT("datefull");
		TIMELIB_INIT;
		TIMELIB_HAVE_DATE();
		s->time->d = timelib_get_nr((char **) &ptr, 2);
		timelib_skip_day_suffix((char **) &ptr);
		s->time->m = timelib_get_month((char **) &ptr);
		s->time->y = timelib_get_nr((char **) &ptr, 4);
		TIMELIB_PROCESS_YEAR(s->time->y);
		TIMELIB_DEINIT;
		return TIMELIB_DATE_FULL;
	}

	pointeddate
	{
		DEBUG_OUTPUT("pointed date");
		TIMELIB_INIT;
		TIMELIB_HAVE_DATE();
		s->time->d = timelib_get_nr((char **) &ptr, 2);
		s->time->m = timelib_get_nr((char **) &ptr, 2);
		s->time->y = timelib_get_nr((char **) &ptr, 4);
		TIMELIB_PROCESS_YEAR(s->time->y);
		TIMELIB_DEINIT;
		return TIMELIB_DATE_FULL_POINTED;
	}

	datenoday
	{
		DEBUG_OUTPUT("datenoday");
		TIMELIB_INIT;
		TIMELIB_HAVE_DATE();
		s->time->m = timelib_get_month((char **) &ptr);
		s->time->y = timelib_get_nr((char **) &ptr, 4);
		s->time->d = 1;
		TIMELIB_PROCESS_YEAR(s->time->y);
		TIMELIB_DEINIT;
		return TIMELIB_DATE_NO_DAY;
	}

	datenodayrev
	{
		DEBUG_OUTPUT("datenodayrev");
		TIMELIB_INIT;
		TIMELIB_HAVE_DATE();
		s->time->y = timelib_get_nr((char **) &ptr, 4);
		s->time->m = timelib_get_month((char **) &ptr);
		s->time->d = 1;
		TIMELIB_PROCESS_YEAR(s->time->y);
		TIMELIB_DEINIT;
		return TIMELIB_DATE_NO_DAY;
	}

	datetextual | datenoyear
	{
		DEBUG_OUTPUT("datetextual | datenoyear");
		TIMELIB_INIT;
		TIMELIB_HAVE_DATE();
		s->time->m = timelib_get_month((char **) &ptr);
		s->time->d = timelib_get_nr((char **) &ptr, 2);
		s->time->y = timelib_get_nr((char **) &ptr, 4);
		TIMELIB_PROCESS_YEAR(s->time->y);
		TIMELIB_DEINIT;
		return TIMELIB_DATE_TEXT;
	}

	datenoyearrev
	{
		DEBUG_OUTPUT("datenoyearrev");
		TIMELIB_INIT;
		TIMELIB_HAVE_DATE();
		s->time->d = timelib_get_nr((char **) &ptr, 2);
		timelib_skip_day_suffix((char **) &ptr);
		s->time->m = timelib_get_month((char **) &ptr);
		TIMELIB_DEINIT;
		return TIMELIB_DATE_TEXT;
	}

	datenocolon
	{
		DEBUG_OUTPUT("datenocolon");
		TIMELIB_INIT;
		TIMELIB_HAVE_DATE();
		s->time->y = timelib_get_nr((char **) &ptr, 4);
		s->time->m = timelib_get_nr((char **) &ptr, 2);
		s->time->d = timelib_get_nr((char **) &ptr, 2);
		TIMELIB_DEINIT;
		return TIMELIB_DATE_NOCOLON;
	}

	xmlrpc | xmlrpcnocolon | soap | wddx | exif
	{
		int tz_not_found;
		DEBUG_OUTPUT("xmlrpc | xmlrpcnocolon | soap | wddx | exif");
		TIMELIB_INIT;
		TIMELIB_HAVE_TIME();
		TIMELIB_HAVE_DATE();
		s->time->y = timelib_get_nr((char **) &ptr, 4);
		s->time->m = timelib_get_nr((char **) &ptr, 2);
		s->time->d = timelib_get_nr((char **) &ptr, 2);
		s->time->h = timelib_get_nr((char **) &ptr, 2);
		s->time->i = timelib_get_nr((char **) &ptr, 2);
		s->time->s = timelib_get_nr((char **) &ptr, 2);
		if (*ptr == '.') {
			s->time->f = timelib_get_frac_nr((char **) &ptr, 9);
			if (*ptr) { /* timezone is optional */
				s->time->z = timelib_get_zone((char **) &ptr, &s->time->dst, s->time, &tz_not_found, s->tzdb);
				if (tz_not_found) {
					add_error(s, "The timezone could not be found in the database");
				}
			}
		}
		TIMELIB_DEINIT;
		return TIMELIB_XMLRPC_SOAP;
	}

	pgydotd
	{
		DEBUG_OUTPUT("pgydotd");
		TIMELIB_INIT;
		TIMELIB_HAVE_DATE();
		s->time->y = timelib_get_nr((char **) &ptr, 4);
		s->time->d = timelib_get_nr((char **) &ptr, 3);
		s->time->m = 1;
		TIMELIB_PROCESS_YEAR(s->time->y);
		TIMELIB_DEINIT;
		return TIMELIB_PG_YEARDAY;
	}

	isoweekday
	{
		timelib_sll w, d;
		DEBUG_OUTPUT("isoweekday");
		TIMELIB_INIT;
		TIMELIB_HAVE_DATE();
		TIMELIB_HAVE_RELATIVE();
		
		s->time->y = timelib_get_nr((char **) &ptr, 4);
		w = timelib_get_nr((char **) &ptr, 2);
		d = timelib_get_nr((char **) &ptr, 1);
		s->time->m = 1;
		s->time->d = 1;
		s->time->relative.d = timelib_daynr_from_weeknr(s->time->y, w, d);

		TIMELIB_DEINIT;
		return TIMELIB_ISO_WEEK;
	}

	isoweek
	{
		timelib_sll w, d;
		DEBUG_OUTPUT("isoweek");
		TIMELIB_INIT;
		TIMELIB_HAVE_DATE();
		TIMELIB_HAVE_RELATIVE();
		
		s->time->y = timelib_get_nr((char **) &ptr, 4);
		w = timelib_get_nr((char **) &ptr, 2);
		d = 1;
		s->time->m = 1;
		s->time->d = 1;
		s->time->relative.d = timelib_daynr_from_weeknr(s->time->y, w, d);

		TIMELIB_DEINIT;
		return TIMELIB_ISO_WEEK;
	}

	pgtextshort
	{
		DEBUG_OUTPUT("pgtextshort");
		TIMELIB_INIT;
		TIMELIB_HAVE_DATE();
		s->time->m = timelib_get_month((char **) &ptr);
		s->time->d = timelib_get_nr((char **) &ptr, 2);
		s->time->y = timelib_get_nr((char **) &ptr, 4);
		TIMELIB_PROCESS_YEAR(s->time->y);
		TIMELIB_DEINIT;
		return TIMELIB_PG_TEXT;
	}

	pgtextreverse
	{
		DEBUG_OUTPUT("pgtextreverse");
		TIMELIB_INIT;
		TIMELIB_HAVE_DATE();
		s->time->y = timelib_get_nr((char **) &ptr, 4);
		s->time->m = timelib_get_month((char **) &ptr);
		s->time->d = timelib_get_nr((char **) &ptr, 2);
		TIMELIB_PROCESS_YEAR(s->time->y);
		TIMELIB_DEINIT;
		return TIMELIB_PG_TEXT;
	}

	clf
	{
		int tz_not_found;
		DEBUG_OUTPUT("clf");
		TIMELIB_INIT;
		TIMELIB_HAVE_TIME();
		TIMELIB_HAVE_DATE();
		s->time->d = timelib_get_nr((char **) &ptr, 2);
		s->time->m = timelib_get_month((char **) &ptr);
		s->time->y = timelib_get_nr((char **) &ptr, 4);
		s->time->h = timelib_get_nr((char **) &ptr, 2);
		s->time->i = timelib_get_nr((char **) &ptr, 2);
		s->time->s = timelib_get_nr((char **) &ptr, 2);
		s->time->z = timelib_get_zone((char **) &ptr, &s->time->dst, s->time, &tz_not_found, s->tzdb);
		if (tz_not_found) {
			add_error(s, "The timezone could not be found in the database");
		}
		TIMELIB_DEINIT;
		return TIMELIB_CLF;
	}

	year4
	{
		DEBUG_OUTPUT("year4");
		TIMELIB_INIT;
		s->time->y = timelib_get_nr((char **) &ptr, 4);
		TIMELIB_DEINIT;
		return TIMELIB_CLF;
	}

	ago
	{
		DEBUG_OUTPUT("ago");
		TIMELIB_INIT;
		s->time->relative.y = 0 - s->time->relative.y;
		s->time->relative.m = 0 - s->time->relative.m;
		s->time->relative.d = 0 - s->time->relative.d;
		s->time->relative.h = 0 - s->time->relative.h;
		s->time->relative.i = 0 - s->time->relative.i;
		s->time->relative.s = 0 - s->time->relative.s;
		s->time->relative.weekday = 0 - s->time->relative.weekday;
		TIMELIB_DEINIT;
		return TIMELIB_AGO;
	}

	daytext
	{
		const timelib_relunit* relunit;
		DEBUG_OUTPUT("daytext");
		TIMELIB_INIT;
		TIMELIB_HAVE_RELATIVE();
		TIMELIB_HAVE_WEEKDAY_RELATIVE();
		TIMELIB_UNHAVE_TIME();
		relunit = timelib_lookup_relunit((char**) &ptr);
		s->time->relative.weekday = relunit->multiplier;
		s->time->relative.weekday_behavior = 1;
		
		TIMELIB_DEINIT;
		return TIMELIB_WEEKDAY;
	}

	relativetext
	{
		timelib_sll i;
		int         behavior = 0;
		DEBUG_OUTPUT("relativetext");
		TIMELIB_INIT;
		TIMELIB_HAVE_RELATIVE();

		while(*ptr) {
			i = timelib_get_relative_text((char **) &ptr, &behavior);
			timelib_eat_spaces((char **) &ptr);
			timelib_set_relative((char **) &ptr, i, behavior, s);
		}
		TIMELIB_DEINIT;
		return TIMELIB_RELATIVE;
	}

	tzcorrection | tz
	{
		int tz_not_found;
		DEBUG_OUTPUT("tzcorrection | tz");
		TIMELIB_INIT;
		TIMELIB_HAVE_TZ();
		s->time->z = timelib_get_zone((char **) &ptr, &s->time->dst, s->time, &tz_not_found, s->tzdb);
		if (tz_not_found) {
			add_error(s, "The timezone could not be found in the database");
		}
		TIMELIB_DEINIT;
		return TIMELIB_TIMEZONE;
	}

	dateshortwithtimeshort | dateshortwithtimelong | dateshortwithtimelongtz
	{
		int tz_not_found;
		DEBUG_OUTPUT("dateshortwithtimeshort | dateshortwithtimelong | dateshortwithtimelongtz");
		TIMELIB_INIT;
		TIMELIB_HAVE_DATE();
		s->time->m = timelib_get_month((char **) &ptr);
		s->time->d = timelib_get_nr((char **) &ptr, 2);

		TIMELIB_HAVE_TIME();
		s->time->h = timelib_get_nr((char **) &ptr, 2);
		s->time->i = timelib_get_nr((char **) &ptr, 2);
		if (*ptr == ':') {
			s->time->s = timelib_get_nr((char **) &ptr, 2);

			if (*ptr == '.') {
				s->time->f = timelib_get_frac_nr((char **) &ptr, 8);
			}
		}

		if (*ptr != '\0') {
			s->time->z = timelib_get_zone((char **) &ptr, &s->time->dst, s->time, &tz_not_found, s->tzdb);
			if (tz_not_found) {
				add_error(s, "The timezone could not be found in the database");
			}
		}
		TIMELIB_DEINIT;
		return TIMELIB_SHORTDATE_WITH_TIME;
	}

	relative
	{
		timelib_ull i;
		DEBUG_OUTPUT("relative");
		TIMELIB_INIT;
		TIMELIB_HAVE_RELATIVE();

		while(*ptr) {
			i = timelib_get_unsigned_nr((char **) &ptr, 24);
			timelib_eat_spaces((char **) &ptr);
			timelib_set_relative((char **) &ptr, i, 0, s);
		}
		TIMELIB_DEINIT;
		return TIMELIB_RELATIVE;
	}

	[ .,\t]
	{
		goto std;
	}

	"\000"|"\n"
	{
		s->pos = cursor; s->line++;
		goto std;
	}

	any
	{
		add_error(s, "Unexpected character");
		goto std;
	}
*/
}

/*!max:re2c */

timelib_time* timelib_strtotime(char *s, int len, struct timelib_error_container **errors, timelib_tzdb *tzdb)
{
	Scanner in;
	int t;
	char *e = s + len - 1;

	memset(&in, 0, sizeof(in));
	in.errors = malloc(sizeof(struct timelib_error_container));
	in.errors->warning_count = 0;
	in.errors->warning_messages = NULL;
	in.errors->error_count = 0;
	in.errors->error_messages = NULL;

	while (isspace(*s) && s < e) {
		s++;
	}
	while (isspace(*e) && e > s) {
		e--;
	}
	if (e - s < 1) {
		in.time = timelib_time_ctor();
		add_error(&in, "Empty string");
		if (errors) {
			*errors = in.errors;
		} else {
			timelib_error_container_dtor(in.errors);
		}
		in.time->y = in.time->d = in.time->m = in.time->h = in.time->i = in.time->s = in.time->f = in.time->z = in.time->dst = -1;
		in.time->is_localtime = in.time->zone_type = 0;
		return in.time;
	}
	e++;

	in.str = malloc((e - s) + YYMAXFILL);
	memset(in.str, 0, (e - s) + YYMAXFILL);
	memcpy(in.str, s, (e - s));
	in.lim = in.str + (e - s) + YYMAXFILL;
	in.cur = in.str;
	in.time = timelib_time_ctor();
	in.time->y = -1;
	in.time->d = -1;
	in.time->m = -1;
	in.time->h = -1;
	in.time->i = -1;
	in.time->s = -1;
	in.time->f = -1;
	in.time->z = -1;
	in.time->dst = -1;
	in.tzdb = tzdb;
	in.time->is_localtime = 0;
	in.time->zone_type = 0;

	do {
		t = scan(&in);
#ifdef DEBUG_PARSER
		printf("%d\n", t);
#endif
	} while(t != EOI);

	free(in.str);
	if (errors) {
		*errors = in.errors;
	} else {
		timelib_error_container_dtor(in.errors);
	}
	return in.time;
}

void timelib_fill_holes(timelib_time *parsed, timelib_time *now, int options)
{
	if (!(options && TIMELIB_OVERRIDE_TIME) && parsed->have_date && !parsed->have_time) {
		parsed->h = 0;
		parsed->i = 0;
		parsed->s = 0;
		parsed->f = 0;
	}
	if (parsed->y == -1) parsed->y = now->y != -1 ? now->y : 0;
	if (parsed->d == -1) parsed->d = now->d != -1 ? now->d : 0;
	if (parsed->m == -1) parsed->m = now->m != -1 ? now->m : 0;
	if (parsed->h == -1) parsed->h = now->h != -1 ? now->h : 0;
	if (parsed->i == -1) parsed->i = now->i != -1 ? now->i : 0;
	if (parsed->s == -1) parsed->s = now->s != -1 ? now->s : 0;
	if (parsed->f == -1) parsed->f = now->f != -1 ? now->f : 0;
	if (parsed->z == -1) parsed->z = now->z != -1 ? now->z : 0;
	if (parsed->dst == -1) parsed->dst = now->dst != -1 ? now->dst : 0;

	if (!parsed->tz_abbr) {
		parsed->tz_abbr = now->tz_abbr ? strdup(now->tz_abbr) : NULL;
	}
	if (!parsed->tz_info) {
		parsed->tz_info = now->tz_info ? timelib_tzinfo_clone(now->tz_info) : NULL;
	}
	if (parsed->zone_type == 0 && now->zone_type != 0) {
		parsed->zone_type = now->zone_type;
/*		parsed->tz_abbr = now->tz_abbr ? strdup(now->tz_abbr) : NULL;
		parsed->tz_info = now->tz_info ? timelib_tzinfo_clone(now->tz_info) : NULL;
*/		parsed->is_localtime = 1;
	}
/*	timelib_dump_date(parsed, 2);
	timelib_dump_date(now, 2);
*/
}

char *timelib_timezone_id_from_abbr(const char *abbr, long gmtoffset, int isdst)
{
	timelib_tz_lookup_table *tp;

	tp = zone_search(abbr, gmtoffset, isdst);
	if (tp) {
		return (tp->full_tz_name);
	} else {
		return NULL;
	}
}

timelib_tz_lookup_table *timelib_timezone_abbreviations_list(void)
{
	return timelib_timezone_lookup;
}

#ifdef DEBUG_PARSER_STUB
int main(void)
{
	timelib_time time = timelib_strtotime("May 12");

	printf ("%04d-%02d-%02d %02d:%02d:%02d.%-5d %+04d %1d",
		time.y, time.m, time.d, time.h, time.i, time.s, time.f, time.z, time.dst);
	if (time.have_relative) {
		printf ("%3dY %3dM %3dD / %3dH %3dM %3dS", 
			time.relative.y, time.relative.m, time.relative.d, time.relative.h, time.relative.i, time.relative.s);
	}
	if (time.have_weekday_relative) {
		printf (" / %d", time.relative.weekday);
	}
	if (time.have_weeknr_day) {
		printf(" / %dW%d", time.relative.weeknr_day.weeknr, time.relative.weeknr_day.dayofweek);
	}
	return 0;				
}
#endif

/*
 * vim: syntax=c
 */
