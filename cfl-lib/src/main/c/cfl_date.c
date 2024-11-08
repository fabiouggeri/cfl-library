/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <stdlib.h>
#include <time.h>

#include "cfl_date.h"

#define GREGORIAN_BASE 1582

#define SET_YEAR(d, v)   (d)->year = (v)
#define SET_MONTH(d, v)  (d)->month = (v)
#define SET_DAY(d, v)    (d)->day = (v)
#define SET_HOUR(d, v)   (d)->hour = (v)
#define SET_MIN(d, v)    (d)->min = (v)
#define SET_SEC(d, v)    (d)->sec = (v)
#define SET_MILLIS(d, v) (d)->millis = (v)

#define GET_YEAR(d)   (d)->year
#define GET_MONTH(d)  (d)->month
#define GET_DAY(d)    (d)->day
#define GET_HOUR(d)   (d)->hour
#define GET_MIN(d)    (d)->min
#define GET_SEC(d)    (d)->sec
#define GET_MILLIS(d) (d)->millis

#define SEC_TO_MILLIS(sec) ((sec) * CFL_MILLIS_PER_SEC)
#define MIN_TO_SEC(min)    ((min) * CFL_SEC_PER_MIN)
#define HOUR_TO_MIN(hour)  ((hour) * CFL_MIN_PER_HOUR)
#define DAY_TO_HOUR(days)  ((days) * CFL_HOUR_PER_DAY)

static CFL_BOOL isLeapYear(CFL_INT16 year) {
   if (year > GREGORIAN_BASE) { 
      return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0 ? CFL_TRUE : CFL_FALSE;
   } else if (year >= 8) { 
      return year % 4 == 0 ? CFL_TRUE : CFL_FALSE;
   }
   switch(year) {
      case -45:
      case -42:
      case -39:
      case -36:
      case -33:
      case -30:
      case -27:
      case -24:
      case -21:
      case -18:
      case -15:
      case -12:
      case -9:
        return CFL_TRUE;
      default:
        return CFL_FALSE;
   }
}

static CFL_DATEP newDateInit(CFL_INT16 year, CFL_UINT8 month, CFL_UINT8 day, CFL_UINT8 hour, CFL_UINT8 min, CFL_UINT8 sec, CFL_UINT16 millis) {
   CFL_DATEP date;
   date = (CFL_DATEP) CFL_MEM_ALLOC(sizeof(CFL_DATE));
   SET_YEAR(date, year);
   SET_MONTH(date, month);
   SET_DAY(date, day);
   SET_HOUR(date, hour);
   SET_MIN(date, min);
   SET_SEC(date, sec);
   SET_MILLIS(date, millis);
   return date;
}

void cfl_date_init(CFL_DATEP date) {
   cfl_date_getCurrent(date);
   date->allocated = CFL_FALSE;
}

CFL_DATEP cfl_date_new(void) {
   CFL_DATEP date;
   date = (CFL_DATEP) CFL_MEM_ALLOC(sizeof(CFL_DATE));
   cfl_date_init(date);
   date->allocated = CFL_TRUE;
   return date;
}

void cfl_date_free(CFL_DATEP date) {
   if (date->allocated) {
      CFL_MEM_FREE(date);
   }
}

CFL_DATEP cfl_date_newDate(CFL_INT16 year, CFL_UINT8 month, CFL_UINT8 day) {
   return newDateInit(year, month, day, 0, 0, 0, 0);
}

CFL_DATEP cfl_date_newDateTime(CFL_INT16 year, CFL_UINT8 month, CFL_UINT8 day, CFL_UINT8 hour, CFL_UINT8 min, CFL_UINT8 sec, CFL_UINT16 millis) {
   return newDateInit(year, month, day, hour, min, sec, millis);
}

CFL_DATEP cfl_date_getCurrent(CFL_DATEP date) {
   time_t curTime;
   const struct tm *tm;

   time(&curTime);
   tm = localtime(&curTime);
   SET_YEAR(date, (CFL_UINT16) tm->tm_year);
   SET_MONTH(date, (CFL_UINT8) tm->tm_mon);
   SET_DAY(date, (CFL_UINT8) tm->tm_mday);
   SET_HOUR(date, (CFL_UINT8) tm->tm_hour);
   SET_MIN(date, (CFL_UINT8) tm->tm_min);
   SET_SEC(date, (CFL_UINT8) tm->tm_sec);
   SET_MILLIS(date, 0);
   return date;
}

void cfl_date_getDate(const CFL_DATEP date, CFL_INT16 *year, CFL_UINT8 *month, CFL_UINT8 *day) {
   *year = GET_YEAR(date);
   *month = GET_MONTH(date);
   *day = GET_DAY(date);
}

void cfl_date_getTime(const CFL_DATEP date, CFL_UINT8 *hour, CFL_UINT8 *min, CFL_UINT8 *sec, CFL_UINT16 *millis) {
   *hour = GET_HOUR(date);
   *min = GET_MIN(date);
   *sec = GET_SEC(date);
   *millis = GET_MILLIS(date);
}

void cfl_date_getDateTime(const CFL_DATEP date, CFL_INT16 *year, CFL_UINT8 *month, CFL_UINT8 *day, CFL_UINT8 *hour, CFL_UINT8 *min, CFL_UINT8 *sec, CFL_UINT16 *millis) {
   *year = GET_YEAR(date);
   *month = GET_MONTH(date);
   *day = GET_DAY(date);
   *hour = GET_HOUR(date);
   *min = GET_MIN(date);
   *sec = GET_SEC(date);
   *millis = GET_MILLIS(date);
}

void cfl_date_setDate(CFL_DATEP date, CFL_INT16 year, CFL_UINT8 month, CFL_UINT8 day) {
   SET_YEAR(date, year);
   SET_MONTH(date, month);
   SET_DAY(date, day);
}

void cfl_date_setTime(CFL_DATEP date, CFL_UINT8 hour, CFL_UINT8 min, CFL_UINT8 sec, CFL_UINT16 millis) {
   SET_HOUR(date, hour);
   SET_MIN(date, min);
   SET_SEC(date, sec);
   SET_MILLIS(date, millis);
}

void cfl_date_setDateTime(CFL_DATEP date, CFL_INT16 year, CFL_UINT8 month, CFL_UINT8 day, CFL_UINT8 hour, CFL_UINT8 min, CFL_UINT8 sec, CFL_UINT16 millis) {
   SET_YEAR(date, year);
   SET_MONTH(date, month);
   SET_DAY(date, day);
   SET_HOUR(date, hour);
   SET_MIN(date, min);
   SET_SEC(date, sec);
   SET_MILLIS(date, millis);
}

CFL_INT16 cfl_date_getYear(const CFL_DATEP date) {
   return GET_YEAR(date);
}

CFL_UINT8 cfl_date_getMonth(const CFL_DATEP date) {
   return GET_MONTH(date);
}

CFL_UINT8 cfl_date_getDay(const CFL_DATEP date) {
   return GET_DAY(date);
}

CFL_UINT8 cfl_date_getHour(const CFL_DATEP date) {
   return GET_HOUR(date);
}

CFL_UINT8 cfl_date_getMin(const CFL_DATEP date) {
   return GET_MIN(date);
}

CFL_UINT8 cfl_date_getSec(const CFL_DATEP date) {
   return GET_SEC(date);
}

CFL_UINT16 cfl_date_getMillis(const CFL_DATEP date) {
   return GET_MILLIS(date);
}

void cfl_date_setYear(CFL_DATEP date, CFL_INT16 year) {
   SET_YEAR(date, year);
}

void cfl_date_setMonth(CFL_DATEP date, CFL_UINT8 month) {
   if (month < 13) {
      SET_MONTH(date, month);
   }
}

void cfl_date_setDay(CFL_DATEP date, CFL_UINT8 day) {
   switch(date->month) {
      case 1:
      case 3:
      case 5:
      case 7:
      case 8:
      case 10:
      case 12:
         if (day <= 31) {
            SET_DAY(date, day);
         }
         break;

      case 2:
         if (day <= 28) {
            SET_DAY(date, day);
         } else if (isLeapYear(GET_YEAR(date)) && day <= 29) {
            SET_DAY(date, day);
         }
         break;

      default:
         if (day <= 30) {
            SET_DAY(date, day);
         }
         break;
   }
}

void cfl_date_setHour(CFL_DATEP date, CFL_UINT8 hour) {
   if (hour < 24) {
      SET_HOUR(date, hour);
   }
}

void cfl_date_setMin(CFL_DATEP date, CFL_UINT8 min) {
   if (min < 60) {
      SET_MIN(date, min);
   }
}

void cfl_date_setSec(CFL_DATEP date, CFL_UINT8 sec) {
   if (sec < 60) {
      SET_SEC(date, sec);
   }
}

void cfl_date_setMillis(CFL_DATEP date, CFL_UINT16 millis) {
   if (millis < 1000) {
      SET_MILLIS(date, millis);
   }
}

void cfl_date_copy(const CFL_DATEP fromDate, CFL_DATEP toDate) {
   SET_YEAR(toDate, GET_YEAR(fromDate));
   SET_MONTH(toDate, GET_MONTH(fromDate));
   SET_DAY(toDate, GET_DAY(fromDate));
   SET_HOUR(toDate, GET_HOUR(fromDate));
   SET_MIN(toDate, GET_MIN(fromDate));
   SET_SEC(toDate, GET_SEC(fromDate));
   SET_MILLIS(toDate, GET_MILLIS(fromDate));
}

/*
CFL_UINT64 cfl_date_toMillis(CFL_DATEP date) {
   CFL_UINT64 millis;
   CFL_UINT16 diffYear;

   millis = GET_MILLIS(date) +
            SEC_TO_MILLIS(GET_SEC(date)) +
            SEC_TO_MILLIS(MIN_TO_SEC(GET_MIN(date))) +
            SEC_TO_MILLIS(MIN_TO_SEC(HOUR_TO_MIN(GET_HOUR(date)))) +
            SEC_TO_MILLIS(MIN_TO_SEC(HOUR_TO_MIN(DAY_TO_HOUR(GET_DAY(date)))));
   
   return millis;
}

CFL_UINT64 cfl_date_toSec(CFL_DATEP date) {
   CFL_UINT64 millis;
   CFL_INT16 year;

   millis = GET_MILLIS(date) +
            SEC_TO_MILLIS(GET_SEC(date)) +
            SEC_TO_MILLIS(MIN_TO_SEC(GET_MIN(date))) +
            SEC_TO_MILLIS(MIN_TO_SEC(HOUR_TO_MIN(GET_HOUR(date)))) +
            SEC_TO_MILLIS(MIN_TO_SEC(HOUR_TO_MIN(DAY_TO_HOUR(GET_DAY(date)))));
   return millis;
}

CFL_INT64 cfl_date_diffMillis(CFL_DATEP date1, CFL_DATEP date2) {
   CFL_UINT64 millis1 = cfl_date_toMillis(date1);
   CFL_UINT64 millis2 = cfl_date_toMillis(date2);
   return millis1 > millis2 ? millis1 - millis2 : millis2 > millis1;
}

CFL_UINT64 cfl_date_diffSec(CFL_DATEP date1, CFL_DATEP date2) {
   CFL_UINT64 sec1 = cfl_date_toSec(date1);
   CFL_UINT64 sec2 = cfl_date_toSec(date2);
   return sec1 > sec2 ? sec1 - sec2 : sec2 > sec1;
}
*/