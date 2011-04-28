#ifndef __calby_h__
#define __calby_h__

#include <sys/types.h>

/*
 * XXX Taken from libwheel
 *
 * Use this macros to control symbol visibility with recent GCC compilers
 * when target system uses the ELF binary format. This improves load times
 * and forbids access to library-private symbols. Consider also enabling
 * the "-fvisibility=hidden" flag for GCC >= 4.1.
 */
#if defined(__ELF__) && defined(__GNUC__) && \
      (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 1)
# define W_EXPORT __attribute__((visibility("default")))
# define W_HIDDEN __attribute__((visibility("hidden")))
#else
# define W_EXPORT
# define W_HIDDEN
#endif

typedef unsigned long long uint64_t;

/* === Miscelaneus utilities === */
/*!
 * \defgroup misc Miscelaneus utilities
 * \addtogroup misc
 * \{
 */

/*!
 * Models a date time internally, using usual notation in Gregorian
 * Calendar
 */
typedef struct _cal_datetime_s {
    long year;  /*!< year in A.D. notation using Gregorian Calendar */
    long mon;   /*!< month of the year 1..12 */
    long day;   /*!< day of the month 1..31 */
    long hour;  /*!< hour of the day 0..24, 24 = 0 */
    long min;   /*!< minute of the hour 0..60 */
    long sec;   /*!< second of the minute 0..60 */
    long off;   /*!< hour offset in minutes using for TZ */
} cal_datetime_t;

/*!
 * Parse a date time string in ISO format.
 * \param str a string which contains a datetime in ISO format
 *        (yyyy-mm-dd hh:mm +oooo).
 * \param dt a pointer to cal_datetime_t type where parsed data will be
 *        stored in.
 * \return a pointer to dt when success, or a NULL if failed.
 */
W_EXPORT cal_datetime_t *cal_isoparse(const char *str, cal_datetime_t *dt);

/*!
 * Return the current data in datetime struct.
 * \param t a cal_datetime_t struct where data will be stored.
 * \return a pointer to t when sucess, or a NULL if failed.
 */
W_EXPORT cal_datetime_t *cal_now(cal_datetime_t *t);

/*!
 * Return the day of the week for an specified datetime struct, using the
 * Doomsday rule.
 * \param t a cal_datetime_t with the data to be evaluated.
 * \return an integer between 0..6, 0 = Monday and 6 = Sunday.
 */
W_EXPORT int cal_weekday(const cal_datetime_t *t);

/*! Definitions for the days of the week, useful for
 * cal_weekday function.
 */
#define CAL_MON 0
#define CAL_TUE 1
#define CAL_WED 2
#define CAL_THU 3
#define CAL_FRI 4
#define CAL_SAT 5
#define CAL_SUN 6

/*!
 * Save the datetime provided as argument into a character buffer in
 * an human-readable form (ISO).
 * \param t a pointer to cal_datetime_t which will be save.
 * \param buf a pointer to char array where date will be saved.
 *            The array must be almost CAL_ISOFMT_LEN length.
 */
W_EXPORT char *cal_format(const cal_datetime_t *t, char *buf);

/*! Length for a buffer which saves a ISO format datetime */
#define CAL_ISOFMT_LEN 25
/*\}*/ /* end of group misc */

/* === Leap seconds utilities === */
/*!
 * \defgroup leapsecs Leap seconds utilities
 * \addtogroup leapsecs
 * \{
 */

/*!
 * Default location for leapsecs file. This file is compatible with
 * Berstein's leapsecs file, and new versions can be located in
 * http://cr.yp.to/libtai/leapsecs.dat
 */
#define CAL_LEAPSECS_FILE "/etc/leapsecs.dat"

/*!
 * Load leap seconds for an opened file descriptor.
 * \param fd an opened for reading file descriptor to get leap
 *        seconds from.
 * \return the number of leapseconds read or -1 when error.
 */
W_EXPORT int cal_leapsecs_load(int fd);

/*!
 * Load leap seconds from the system, using file defined statically
 * in code (CAL_LEAPSECS_FILE). If any leap second is loaded, then
 * init do nothing, and previous values will be kept.
 * \return the number of leapseconds read or -1 when error.
 */
W_EXPORT int cal_leapsecs_init(void);

/*!
 * Remove from the TAI stamp the leapsecs, acording to leapsec database.
 * \param tai a pointer to a TAI stamp, this value will be modified to
 *        substract the leapseconds.
 * \return the number of seconds substracted, or -1 if fails.
 */
W_EXPORT int cal_leapsecs_sub(uint64_t *tai);

/*!
 * Add to the TAI stamp passed as argument the number of leapsecons acording
 * to the database.
 * \param tai a pointer to a TAI stamp, this value will be modified by
 *        adition of leapseconds.
 * \return the number of added seconds or -1 if fails.
 */
W_EXPORT int cal_leapsecs_add(uint64_t *tai);

/*!
 * Check if a TAI stamp is a leap second.
 * \param tai a TAI stamp to be evaluated.
 * \return -1 if fails, 0 if its not a leap second or 1 if it's a leap
 *         second.
 */
W_EXPORT int cal_isleapsec(uint64_t tai);

/*!
 * Get the number of leapsecs from the begining of time until the TAI stamp
 * passed as argument.
 * \param tai a TAI stamp until which leap seconds will be counted.
 * \return the number of leapseconds affected or -1 if error
 */
W_EXPORT int cal_leapsecs_get(uint64_t tai);

/*! Leap seconds master structure, which saves readed leapseconds. */
struct _cal_leapsecs_s {
    size_t    count;          /*!< the number of saved leapseconds */
    uint64_t *list;           /*!< list of tai stamps for that seconds */
    int     (*load)(int);     /*!< pointer to cal_leapsecs_load function */
    int     (*init)(void);    /*!< pointer to cal_leapsecs_init function */
} cal_leapsecs = {
    .count = 0,
    .list  = NULL,
    .load  = cal_leapsecs_load,
    .init  = cal_leapsecs_init
};
/*/}*/ /* end of group leapseconds */

/* === TAI utilities === */
/*!
 * \defgroup tai TAI utilities
 * \addtogroup tai
 * \{
 */

/*! Size of a TAI packed datetime */
#define CAL_TAIFMT_LEN 8

/*!
 * Pack a TAI stamp into a portable format.
 * \param s a 8byte string which will contain the binary code
 *        for the TAI stamp (without adding leapsecs).
 * \param x a pointer to a TAI stamp to be packed.
 */
W_EXPORT void cal_tai_pack(char *s, uint64_t x);

/*!
 * Unpack a TAI stamp from a portable byte format.
 * \param s a 8byte string which contains the binary code
 *        of a TAI stamp.
 * \param x a pointer to TAI stamp where it will be unpacked.
 */
W_EXPORT void cal_tai_unpack(const char *s, uint64_t *u);

/*!
 * Get a TAI stamp from a date time.
 * \param t a pointer to a cal_datetime_t struct.
 * \return a TAI stamp (64 bit unsigned int).
 */
W_EXPORT uint64_t cal_tai(const cal_datetime_t *t);
/*/}*/ /* end of group tai */

/* === Modified Julian Date utilities === */
/*!
 * \defgroup mjd Modified Julian Date utilities
 * \addtogroup mjd
 * \{
 */

/*!
 * Return the MJD acording to datetime passed as agument.
 * \param t a cal_datetime_t pointer which contains a datetime.
 * \return a double with the MJD related with datetime in t.
 */
W_EXPORT double cal_mjd(const cal_datetime_t *t);

/*/}*/ /* end of group mjd */

/* === Julian Date utilities === */
/*!
 * \defgroup jd Julian Date utilities
 * \addtogroup jd
 * \{
 */

/*|
 * Return the JD related with a datetime passed as argument.
 * \param _x a cal_datetime_t pointer which contains datetime.
 * \return a double with JD.
 */

#define cal_jd(_x) \
    (cal_mjd(_x) + 2400000.5)

/*/}*/ /* end of group jd */

#endif
/* vim:set et:set ts=4: */
