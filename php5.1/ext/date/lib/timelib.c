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
   | Authors: Derick Rethans <derick@derickrethans.nl>                    |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#include "timelib.h"
#include <ctype.h>
#include <math.h>

#define TIMELIB_TIME_FREE(m) 	\
	if (m) {		\
		free(m);	\
		m = NULL;	\
	}			\

timelib_time* timelib_time_ctor()
{
	timelib_time *t;
	t = calloc(1, sizeof(timelib_time));

	return t;
}

void timelib_time_tz_abbr_update(timelib_time* tm, char* tz_abbr)
{
	unsigned int i;
	
	TIMELIB_TIME_FREE(tm->tz_abbr);
	tm->tz_abbr = strdup(tz_abbr);
	for (i = 0; i < strlen(tz_abbr); i++) {
		tm->tz_abbr[i] = toupper(tz_abbr[i]);
	}
}

void timelib_time_dtor(timelib_time* t)
{
	TIMELIB_TIME_FREE(t->tz_abbr);
	TIMELIB_TIME_FREE(t);
}

timelib_time_offset* timelib_time_offset_ctor()
{
	timelib_time_offset *t;
	t = calloc(1, sizeof(timelib_time_offset));

	return t;
}

void timelib_time_offset_dtor(timelib_time_offset* t)
{
	TIMELIB_TIME_FREE(t->abbr);
	TIMELIB_TIME_FREE(t);
}

timelib_tzinfo* timelib_tzinfo_ctor(char *name)
{
	timelib_tzinfo *t;
	t = calloc(1, sizeof(timelib_tzinfo));
	t->name = strdup(name);

	return t;
}

timelib_tzinfo *timelib_tzinfo_clone(timelib_tzinfo *tz)
{
	timelib_tzinfo *tmp = timelib_tzinfo_ctor(tz->name);
	tmp->ttisgmtcnt = tz->ttisgmtcnt;
	tmp->ttisstdcnt = tz->ttisstdcnt;
	tmp->leapcnt = tz->leapcnt;
	tmp->timecnt = tz->timecnt;
	tmp->typecnt = tz->typecnt;
	tmp->charcnt = tz->charcnt;
	
	tmp->trans = (int32_t *) malloc(tz->timecnt * sizeof(int32_t));
	tmp->trans_idx = (unsigned char*) malloc(tz->timecnt * sizeof(unsigned char));
	memcpy(tmp->trans, tz->trans, tz->timecnt * sizeof(int32_t));
	memcpy(tmp->trans_idx, tz->trans_idx, tz->timecnt * sizeof(unsigned char));

	tmp->type = (ttinfo*) malloc(tz->typecnt * sizeof(struct ttinfo));
	memcpy(tmp->type, tz->type, tz->typecnt * sizeof(struct ttinfo));

	tmp->timezone_abbr = (char*) malloc(tz->charcnt);
	memcpy(tmp->timezone_abbr, tz->timezone_abbr, tz->charcnt);

	tmp->leap_times = (tlinfo*) malloc(tz->leapcnt * sizeof(tlinfo));
	memcpy(tmp->leap_times, tz->leap_times, tz->leapcnt * sizeof(tlinfo));

	return tmp;
}

void timelib_tzinfo_dtor(timelib_tzinfo *tz)
{
	TIMELIB_TIME_FREE(tz->name);
	TIMELIB_TIME_FREE(tz->trans);
	TIMELIB_TIME_FREE(tz->trans_idx);
	TIMELIB_TIME_FREE(tz->type);
	TIMELIB_TIME_FREE(tz->timezone_abbr);
	TIMELIB_TIME_FREE(tz->leap_times);
	TIMELIB_TIME_FREE(tz);
}

char *timelib_get_tz_abbr_ptr(timelib_time *t)
{
	if (!t->sse_uptodate) {
		timelib_update_ts(t, NULL);
	};
	return t->tz_abbr;
}

void timelib_error_container_dtor(timelib_error_container *errors)
{
	int i;

	for (i = 0; i < errors->warning_count; i++) {
		free(errors->warning_messages[i].message);
	}
	free(errors->warning_messages);
	for (i = 0; i < errors->error_count; i++) {
		free(errors->error_messages[i].message);
	}
	free(errors->error_messages);
	free(errors);
}

signed long timelib_date_to_int(timelib_time *d, int *error)
{
	timelib_sll ts;

	ts = d->sse;

	if (ts < LONG_MIN || ts > LONG_MAX) {
		if (error) {
			*error = 1;
		}
		return 0;
	}
	if (error) {
		*error = 0;
	}
	return (signed long) d->sse;
}

void timelib_decimal_hour_to_hms(double h, int *hour, int *min, int *sec)
{
	*hour = floor(h);
	*min =  floor((h - *hour) * 60);
	*sec =  (h - *hour - ((float) *min / 60)) * 3600;
}

void timelib_dump_date(timelib_time *d, int options)
{
	if ((options & 2) == 2) {
		printf("TYPE: %d ", d->zone_type);
	}
	printf("TS: %lld | %04lld-%02lld-%02lld %02lld:%02lld:%02lld",
		d->sse, d->y, d->m, d->d, d->h, d->i, d->s);
	if (d->f > +0.0) {
		printf(" %.5f", d->f);
	}

	if (d->is_localtime) {
		switch (d->zone_type) {
			case TIMELIB_ZONETYPE_OFFSET: /* Only offset */
				printf(" GMT %05d%s", d->z, d->dst == 1 ? " (DST)" : "");
				break;
			case TIMELIB_ZONETYPE_ID: /* Timezone struct */
				/* Show abbreviation if wanted */
				if (d->tz_abbr) {
					printf(" %s", d->tz_abbr);
				}
				/* Do we have a TimeZone struct? */
				if (d->tz_info) {
					printf(" %s", d->tz_info->name);
				}
				break;
			case TIMELIB_ZONETYPE_ABBR:
				printf(" %s", d->tz_abbr);
				printf(" %05d%s", d->z, d->dst == 1 ? " (DST)" : "");
				break;
		}
	} else {
		printf(" GMT 00000");
	}

	if ((options & 1) == 1) {
		if (d->have_relative) {
			printf("%3lldY %3lldM %3lldD / %3lldH %3lldM %3lldS", 
				d->relative.y, d->relative.m, d->relative.d, d->relative.h, d->relative.i, d->relative.s);
		}
		if (d->have_weekday_relative) {
			printf(" / %d.%d", d->relative.weekday, d->relative.weekday_behavior);
		}
	}
	printf("\n");
}

