/**
 * @file cfl_date.h
 * @brief Date and time handling utilities.
 *
 * This module provides a date/time structure and functions for creating,
 * manipulating, and querying date and time values.
 */

#ifndef CFL_DATE_H_

#define CFL_DATE_H_

#include "cfl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Field index for year component */
#define CFL_DATE_YEAR 0
/** @brief Field index for month component */
#define CFL_DATE_MONTH 1
/** @brief Field index for day component */
#define CFL_DATE_DAY 2
/** @brief Field index for hour component */
#define CFL_DATE_HOUR 3
/** @brief Field index for minute component */
#define CFL_DATE_MIN 4
/** @brief Field index for second component */
#define CFL_DATE_SEC 5
/** @brief Field index for millisecond component */
#define CFL_DATE_MILLIS 6

/** @brief Number of milliseconds per second */
#define CFL_MILLIS_PER_SEC 1000
/** @brief Number of seconds per minute */
#define CFL_SEC_PER_MIN 60
/** @brief Number of minutes per hour */
#define CFL_MIN_PER_HOUR 60
/** @brief Number of hours per day */
#define CFL_HOUR_PER_DAY 60

/**
 * @brief Date and time structure.
 */
typedef struct _CFL_DATE {
  CFL_UINT16 millis;  /**< Milliseconds (0-999) */
  CFL_INT16 year;     /**< Year (can be negative for BC dates) */
  CFL_UINT8 month;    /**< Month (1-12) */
  CFL_UINT8 day;      /**< Day of month (1-31) */
  CFL_UINT8 hour;     /**< Hour (0-23) */
  CFL_UINT8 min;      /**< Minute (0-59) */
  CFL_UINT8 sec;      /**< Second (0-59) */
  CFL_BOOL allocated; /**< Whether structure was dynamically allocated */
} CFL_DATE, *CFL_DATEP;

/**
 * @brief Initializes a date structure with default values (all zeros).
 * @param date Pointer to the date structure to initialize.
 */
extern void cfl_date_init(CFL_DATEP date);

/**
 * @brief Creates a new date structure with default values.
 * @return Pointer to the new date structure, or NULL if allocation fails.
 */
extern CFL_DATEP cfl_date_new(void);

/**
 * @brief Frees a date structure.
 * @param date Pointer to the date structure to free.
 */
extern void cfl_date_free(CFL_DATEP date);

/**
 * @brief Creates a new date with the specified year, month, and day.
 * @param year Year value.
 * @param month Month value (1-12).
 * @param day Day of month (1-31).
 * @return Pointer to the new date structure, or NULL if allocation fails.
 */
extern CFL_DATEP cfl_date_newDate(CFL_INT16 year, CFL_UINT8 month,
                                  CFL_UINT8 day);

/**
 * @brief Creates a new date/time with all components specified.
 * @param year Year value.
 * @param month Month value (1-12).
 * @param day Day of month (1-31).
 * @param hour Hour (0-23).
 * @param min Minute (0-59).
 * @param sec Second (0-59).
 * @param millis Milliseconds (0-999).
 * @return Pointer to the new date structure, or NULL if allocation fails.
 */
extern CFL_DATEP cfl_date_newDateTime(CFL_INT16 year, CFL_UINT8 month,
                                      CFL_UINT8 day, CFL_UINT8 hour,
                                      CFL_UINT8 min, CFL_UINT8 sec,
                                      CFL_UINT16 millis);

/**
 * @brief Gets the current system date and time.
 * @param date Pointer to the date structure to fill with current date/time.
 *             If NULL, a new structure is allocated.
 * @return Pointer to the date structure with current date/time.
 */
extern CFL_DATEP cfl_date_getCurrent(CFL_DATEP date);

/**
 * @brief Gets the date components from a date structure.
 * @param date Pointer to the source date structure.
 * @param year Pointer to store the year (can be NULL).
 * @param month Pointer to store the month (can be NULL).
 * @param day Pointer to store the day (can be NULL).
 */
extern void cfl_date_getDate(const CFL_DATEP date, CFL_INT16 *year,
                             CFL_UINT8 *month, CFL_UINT8 *day);

/**
 * @brief Gets the time components from a date structure.
 * @param date Pointer to the source date structure.
 * @param hour Pointer to store the hour (can be NULL).
 * @param min Pointer to store the minute (can be NULL).
 * @param sec Pointer to store the second (can be NULL).
 * @param millis Pointer to store the milliseconds (can be NULL).
 */
extern void cfl_date_getTime(const CFL_DATEP date, CFL_UINT8 *hour,
                             CFL_UINT8 *min, CFL_UINT8 *sec,
                             CFL_UINT16 *millis);

/**
 * @brief Gets all date and time components from a date structure.
 * @param date Pointer to the source date structure.
 * @param year Pointer to store the year (can be NULL).
 * @param month Pointer to store the month (can be NULL).
 * @param day Pointer to store the day (can be NULL).
 * @param hour Pointer to store the hour (can be NULL).
 * @param min Pointer to store the minute (can be NULL).
 * @param sec Pointer to store the second (can be NULL).
 * @param millis Pointer to store the milliseconds (can be NULL).
 */
extern void cfl_date_getDateTime(const CFL_DATEP date, CFL_INT16 *year,
                                 CFL_UINT8 *month, CFL_UINT8 *day,
                                 CFL_UINT8 *hour, CFL_UINT8 *min,
                                 CFL_UINT8 *sec, CFL_UINT16 *millis);

/**
 * @brief Sets the date components of a date structure.
 * @param date Pointer to the date structure to modify.
 * @param year Year value.
 * @param month Month value (1-12).
 * @param day Day of month (1-31).
 */
extern void cfl_date_setDate(CFL_DATEP date, CFL_INT16 year, CFL_UINT8 month,
                             CFL_UINT8 day);

/**
 * @brief Sets the time components of a date structure.
 * @param date Pointer to the date structure to modify.
 * @param hour Hour (0-23).
 * @param min Minute (0-59).
 * @param sec Second (0-59).
 * @param millis Milliseconds (0-999).
 */
extern void cfl_date_setTime(CFL_DATEP date, CFL_UINT8 hour, CFL_UINT8 min,
                             CFL_UINT8 sec, CFL_UINT16 millis);

/**
 * @brief Sets all date and time components of a date structure.
 * @param date Pointer to the date structure to modify.
 * @param year Year value.
 * @param month Month value (1-12).
 * @param day Day of month (1-31).
 * @param hour Hour (0-23).
 * @param min Minute (0-59).
 * @param sec Second (0-59).
 * @param millis Milliseconds (0-999).
 */
extern void cfl_date_setDateTime(CFL_DATEP date, CFL_INT16 year,
                                 CFL_UINT8 month, CFL_UINT8 day, CFL_UINT8 hour,
                                 CFL_UINT8 min, CFL_UINT8 sec,
                                 CFL_UINT16 millis);

/**
 * @brief Gets the year component.
 * @param date Pointer to the date structure.
 * @return The year value.
 */
extern CFL_INT16 cfl_date_getYear(const CFL_DATEP date);

/**
 * @brief Gets the month component.
 * @param date Pointer to the date structure.
 * @return The month value (1-12).
 */
extern CFL_UINT8 cfl_date_getMonth(const CFL_DATEP date);

/**
 * @brief Gets the day component.
 * @param date Pointer to the date structure.
 * @return The day of month (1-31).
 */
extern CFL_UINT8 cfl_date_getDay(const CFL_DATEP date);

/**
 * @brief Gets the hour component.
 * @param date Pointer to the date structure.
 * @return The hour (0-23).
 */
extern CFL_UINT8 cfl_date_getHour(const CFL_DATEP date);

/**
 * @brief Gets the minute component.
 * @param date Pointer to the date structure.
 * @return The minute (0-59).
 */
extern CFL_UINT8 cfl_date_getMin(const CFL_DATEP date);

/**
 * @brief Gets the second component.
 * @param date Pointer to the date structure.
 * @return The second (0-59).
 */
extern CFL_UINT8 cfl_date_getSec(const CFL_DATEP date);

/**
 * @brief Gets the millisecond component.
 * @param date Pointer to the date structure.
 * @return The milliseconds (0-999).
 */
extern CFL_UINT16 cfl_date_getMillis(const CFL_DATEP date);

/**
 * @brief Sets the year component.
 * @param date Pointer to the date structure.
 * @param year Year value.
 */
extern void cfl_date_setYear(CFL_DATEP date, CFL_INT16 year);

/**
 * @brief Sets the month component.
 * @param date Pointer to the date structure.
 * @param month Month value (1-12).
 */
extern void cfl_date_setMonth(CFL_DATEP date, CFL_UINT8 month);

/**
 * @brief Sets the day component.
 * @param date Pointer to the date structure.
 * @param day Day of month (1-31).
 */
extern void cfl_date_setDay(CFL_DATEP date, CFL_UINT8 day);

/**
 * @brief Sets the hour component.
 * @param date Pointer to the date structure.
 * @param hour Hour (0-23).
 */
extern void cfl_date_setHour(CFL_DATEP date, CFL_UINT8 hour);

/**
 * @brief Sets the minute component.
 * @param date Pointer to the date structure.
 * @param min Minute (0-59).
 */
extern void cfl_date_setMin(CFL_DATEP date, CFL_UINT8 min);

/**
 * @brief Sets the second component.
 * @param date Pointer to the date structure.
 * @param sec Second (0-59).
 */
extern void cfl_date_setSec(CFL_DATEP date, CFL_UINT8 sec);

/**
 * @brief Sets the millisecond component.
 * @param date Pointer to the date structure.
 * @param millis Milliseconds (0-999).
 */
extern void cfl_date_setMillis(CFL_DATEP date, CFL_UINT16 millis);

/**
 * @brief Copies a date structure to another.
 * @param fromDate Pointer to the source date structure.
 * @param toDate Pointer to the destination date structure.
 */
extern void cfl_date_copy(const CFL_DATEP fromDate, CFL_DATEP toDate);

#ifdef __cplusplus
}
#endif

#endif
