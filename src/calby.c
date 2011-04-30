/* libcalby: tiny calendar library.
 * Copyright (C) 2011  Andrés J. Díaz <ajdiaz@connectical.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; only version 2 of the License is applicable.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "calby.h"

/* === Miscelaneus utilties === */
cal_datetime_t *
cal_parse(const char *s, cal_datetime_t *dt)
{
  char *t = (char *)s;
  unsigned long z;
  unsigned long c;
  int sign = 1;

  if (*t == '-') { ++t; sign = -1; }
  z = 0; while ((c = (unsigned char) (*t - '0')) <= 9) { z = z * 10 + c; ++t; }
  dt->year = z * sign;

  if (*t++ != '-') return NULL;
  z = 0; while ((c = (unsigned char) (*t - '0')) <= 9) { z = z * 10 + c; ++t; }
  dt->mon = z;

  if (*t++ != '-') return NULL;
  z = 0; while ((c = (unsigned char) (*t - '0')) <= 9) { z = z * 10 + c; ++t; }
  dt->day = z;

  while ((*t == ' ') || (*t == '\t') || (*t == 'T')) ++t;
  z = 0; while ((c = (unsigned char) (*t - '0')) <= 9) { z = z * 10 + c; ++t; }
  dt->hour = z;

  if (*t++ != ':') return NULL;
  z = 0; while ((c = (unsigned char) (*t - '0')) <= 9) { z = z * 10 + c; ++t; }
  dt->min = z;

  if (*t != ':')
    dt->sec = 0;
  else {
    ++t;
    z = 0; while ((c = (unsigned char) (*t - '0')) <= 9) { z = z * 10 + c; ++t; }
    dt->sec = z;
  }

  while ((*t == ' ') || (*t == '\t')) ++t;
  if (*t == '+') sign = 1; else if (*t == '-') sign = -1; else return NULL;
  ++t;
  c = (unsigned char) (*t++ - '0'); if (c > 9) return NULL; z = c;
  c = (unsigned char) (*t++ - '0'); if (c > 9) return NULL; z = z * 10 + c;
  c = (unsigned char) (*t++ - '0'); if (c > 9) return NULL; z = z * 6 + c;
  c = (unsigned char) (*t++ - '0'); if (c > 9) return NULL; z = z * 10 + c;
  dt->off = z * sign;

  return dt;
}

char *
cal_format(const cal_datetime_t *t, char *buf)
{
    long h,m,o;

    o = (t->off < 0) ? (-1)*t->off : t->off;
    h = o / 60;
    m = o - h * 60;

    buf[0]  = (t->year / 1000) + '0';
    buf[1]  = (t->year /  100) % 10 + '0';
    buf[2]  = (t->year /   10) % 10 + '0';
    buf[3]  = (t->year %   10) + '0';
    buf[4]  = '-';
    buf[5]  = (t->mon / 10) + '0';
    buf[6]  = (t->mon % 10) + '0';
    buf[7]  = '-';
    buf[8]  = (t->day / 10) + '0';
    buf[9]  = (t->day % 10) + '0';
    buf[10] = ' ';
    buf[11] = (t->hour / 10) + '0';
    buf[12] = (t->hour % 10) + '0';
    buf[13] = ':';
    buf[14] = (t->min / 10) + '0';
    buf[15] = (t->min % 10) + '0';
    buf[16] = ':';
    buf[17] = (t->sec / 10) + '0';
    buf[18] = (t->sec % 10) + '0';
    buf[19] = ' ';
    buf[20] = (t->off < 0) ? '-' : '+';
    buf[21] = (h / 10) + '0';
    buf[22] = (h % 10) + '0';
    buf[23] = (m / 10) + '0';
    buf[24] = (m % 10) + '0';
    buf[25] = 0;

    return buf;
}

int
cal_weekday(const cal_datetime_t *t)
{
    long m = t->mon;
    long y = t->year;
    long d = t->day;

    if (y < 100)
        y += 2000;

    if (m < 3)
    {
        m += 13;
        y -= 1;
    }
    else
        m += 1;

    return (d + ((26*m)/10) + y + (y/4) - (y/100) + (y/400) + 5) % 7;
}

cal_datetime_t *
cal_now(cal_datetime_t *t)
{
    struct tm l;
    time_t    s;

    s = time((long *)0);

    if (localtime_r(&s,&l) == NULL)
        return NULL;

    tzset();

    t->year = l.tm_year + 1900;
    t->day  = l.tm_mday;
    t->hour = l.tm_hour;
    t->mon  = l.tm_mon + 1;
    t->min  = l.tm_min;
    t->sec  = l.tm_sec;
    t->off  = (-1)*(long)(timezone / 60);

    if (l.tm_isdst > 0)
        t->off += 60;

    return t;
}

/* === TAI utilities === */
void
cal_tai_unpack(const char *s, uint64_t *u)
{
    *u = (unsigned char) s[0];
    *u <<= 8; *u += (unsigned char) s[1];
    *u <<= 8; *u += (unsigned char) s[2];
    *u <<= 8; *u += (unsigned char) s[3];
    *u <<= 8; *u += (unsigned char) s[4];
    *u <<= 8; *u += (unsigned char) s[5];
    *u <<= 8; *u += (unsigned char) s[6];
    *u <<= 8; *u += (unsigned char) s[7];
}

void
cal_tai_pack(char *s, uint64_t x)
{
  s[7] = x & 255; x >>= 8;
  s[6] = x & 255; x >>= 8;
  s[5] = x & 255; x >>= 8;
  s[4] = x & 255; x >>= 8;
  s[3] = x & 255; x >>= 8;
  s[2] = x & 255; x >>= 8;
  s[1] = x & 255; x >>= 8;
  s[0] = x;
}

uint64_t
cal_tai(const cal_datetime_t *t)
{
    long d,s;
    uint64_t x;
    /* XXX: check for overflow? */

    d = (long) cal_mjd(t);

    s = t->hour * 60 + t->min;
    s = (s - t->off) * 60 + t->sec;

    x = d * 86400ULL + 4611686014920671114ULL + (long long) s;

    return x;
}

/* === Leapsecs utilities === */
int
cal_leapsecs_load(int fd)
{
    struct stat st;
    uint64_t *t, u;
    int n;

    if (fstat(fd,&st) == -1) return -1;
    if ((t = (uint64_t *) malloc (st.st_size)) == NULL)
        return -1;

    if ((n = read(fd,(char *)t,st.st_size)) != st.st_size)
    {
        free(t);
        return -1;
    }

    n /= sizeof(uint64_t);
    cal_leapsecs.count = n;

    while (--n >= 0)
    {
        cal_tai_unpack((char *)&t[n],&u);
        t[n] = u;
    }

    if (cal_leapsecs.list != NULL)
        free(cal_leapsecs.list);

    cal_leapsecs.list = t;

    return (int)cal_leapsecs.count;
}

int
cal_leapsecs_init(void)
{
    int ret = -1;
    int fd;

    if (cal_leapsecs.count)
        return cal_leapsecs.count;

    if ((fd = open(CAL_LEAPSECS_FILE,O_RDONLY)) == -1)
        return -1;

    ret = cal_leapsecs_load(fd);
    close(fd);

    return ret;
}

int
cal_leapsecs_sub(uint64_t *tai)
{
    int i,r;

    if (cal_leapsecs_init() == -1)
        return -1;

    for (i=0,r=0;i<cal_leapsecs.count;i++,r++)
        if (*tai < cal_leapsecs.list[i]) break;

    *tai -= r;
    return r;
}

int
cal_leapsecs_add(uint64_t *tai)
{
    int i,r;

    if (cal_leapsecs_init() == -1)
        return -1;

    for (r=0,i=0;i<cal_leapsecs.count;i++,r++)
        if (*tai < cal_leapsecs.list[i]) break;
        else if (*tai == cal_leapsecs.list[i]) r--;

    *tai += r;
    return r;
}

int
cal_isleapsec(uint64_t tai)
{
    int i;

    if (cal_leapsecs_init() == -1)
        return -1;

    for (i=0;i<cal_leapsecs.count;i++)
    {
        if (tai < cal_leapsecs.list[i]) return 0;
        if (tai == cal_leapsecs.list[i])
            return 1;
    }

    return 0;
}

int
cal_leapsecs_get(uint64_t tai)
{
    int i,r=0;

    if (cal_leapsecs_init() == -1)
        return -1;

    for (i=0;i<cal_leapsecs.count;i++,r++)
        if (tai < cal_leapsecs.list[i]) break;

    return r;
}

/* === Modified Julian Date utilities === */
/* Part of this code is taken from libtai */

static unsigned long _times365[]   = { 0, 365, 730, 1095 };
static unsigned long _times36524[] = { 0, 36524UL, 73048UL, 109572UL };
static unsigned long _montab[]     = {   0,  31,  61,  92, 122, 153,
                                       184, 214, 245, 275, 306, 337 };

/* XXX month length after february is (306 * m + 5) / 10 */

double
cal_mjd(const cal_datetime_t *t)
{
    long y;
    long m;
    long d;

    d = t->day - 678882L;
    m = t->mon - 1;
    y = t->year;

    d += 146097L * (y / 400);
    y %= 400;

    if (m >= 2) m -= 2; else { m += 10; --y; }

    y += (m / 12);
    m %= 12;
    if (m < 0) { m += 12; --y; }

    d += _montab[m];

    d += 146097L * (y / 400);
    y %= 400;
    if (y < 0) { y += 400; d -= 146097L; }

    d += _times365[y & 3];
    y >>= 2;

    d += 1461L * (y % 25);
    y /= 25;

    d += _times36524[y & 3];

    return d + (double)(
            ((t->sec == 60) ? t->sec -1: t->sec) +
            t->min * 60 + t->hour * 3600) / 86400.0;
}

/* === Julian calendar utilities === */

cal_datetime_t *
cal_julian(cal_datetime_t *t)
{
    long x, b, c, d, e;

#define _FL(_x) \
    ((_x < 0) ? (((long)(_x)) - 1) : ((long)(_x)))

    x = _FL(cal_jd(t) + 0.5);
    b = (x + 1524);
    c = _FL((double)(b - 122.1) / 365.25);
    d = _FL((double)(365.25 * c));
    e = _FL((double)(b - d) / 30.6001);

    t->mon  = _FL((double)((e < 14) ? (e - 1) : (e - 13)));
    t->year = _FL((double)((t->mon > 2) ? (c - 4716) : (c - 4715)));
    t->day  = b - d - _FL(30.6001 * e);
#undef _FL

    /* If year is less than 1, subtract one to convert from
       a zero based date system to the common era system in
       which the year -1 (1 B.C.E) is followed by year 1 (1 C.E.). */

    if (t->year < 1) {
        t->year--;
    }

    return t;
}

/* vim:set et:set ts=4: */
