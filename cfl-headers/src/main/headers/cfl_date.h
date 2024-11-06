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

#ifndef CFL_DATE_H_

#define CFL_DATE_H_

#include "cfl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CFL_DATE_YEAR   0
#define CFL_DATE_MONTH  1
#define CFL_DATE_DAY    2
#define CFL_DATE_HOUR   3
#define CFL_DATE_MIN    4
#define CFL_DATE_SEC    5
#define CFL_DATE_MILLIS 6

#define CFL_MILLIS_PER_SEC 1000
#define CFL_SEC_PER_MIN    60
#define CFL_MIN_PER_HOUR   60
#define CFL_HOUR_PER_DAY   60

typedef struct _CFL_DATE {
   CFL_UINT16 millis;
   CFL_INT16  year;
   CFL_UINT8  month;
   CFL_UINT8  day;
   CFL_UINT8  hour;
   CFL_UINT8  min;
   CFL_UINT8  sec;
   CFL_BOOL   allocated;
} CFL_DATE, *CFL_DATEP;

extern void cfl_date_init(CFL_DATEP date);
extern CFL_DATEP cfl_date_new(void);
extern void cfl_date_free(CFL_DATEP date);
extern CFL_DATEP cfl_date_newDate(CFL_INT16 year, CFL_UINT8 month, CFL_UINT8 day);
extern CFL_DATEP cfl_date_newDateTime(CFL_INT16 year, CFL_UINT8 month, CFL_UINT8 day, CFL_UINT8 hour, CFL_UINT8 min, CFL_UINT8 sec, CFL_UINT16 millis);
extern CFL_DATEP cfl_date_getCurrent(CFL_DATEP date);
extern void cfl_date_getDate(CFL_DATEP date, CFL_INT16 *year, CFL_UINT8 *month, CFL_UINT8 *day);
extern void cfl_date_getTime(CFL_DATEP date, CFL_UINT8 *hour, CFL_UINT8 *min, CFL_UINT8 *sec, CFL_UINT16 *millis);
extern void cfl_date_getDateTime(CFL_DATEP date, CFL_INT16 *year, CFL_UINT8 *month, CFL_UINT8 *day, CFL_UINT8 *hour, CFL_UINT8 *min, CFL_UINT8 *sec, CFL_UINT16 *millis);
extern void cfl_date_setDate(CFL_DATEP date, CFL_INT16 year, CFL_UINT8 month, CFL_UINT8 day);
extern void cfl_date_setTime(CFL_DATEP date, CFL_UINT8 hour, CFL_UINT8 min, CFL_UINT8 sec, CFL_UINT16 millis);
extern void cfl_date_setDateTime(CFL_DATEP date, CFL_INT16 year, CFL_UINT8 month, CFL_UINT8 day, CFL_UINT8 hour, CFL_UINT8 min, CFL_UINT8 sec, CFL_UINT16 millis);
extern CFL_INT16 cfl_date_getYear(CFL_DATEP date);
extern CFL_UINT8 cfl_date_getMonth(CFL_DATEP date);
extern CFL_UINT8 cfl_date_getDay(CFL_DATEP date);
extern CFL_UINT8 cfl_date_getHour(CFL_DATEP date);
extern CFL_UINT8 cfl_date_getMin(CFL_DATEP date);
extern CFL_UINT8 cfl_date_getSec(CFL_DATEP date);
extern CFL_UINT16 cfl_date_getMillis(CFL_DATEP date);
extern void cfl_date_setYear(CFL_DATEP date, CFL_INT16 year);
extern void cfl_date_setMonth(CFL_DATEP date, CFL_UINT8 month);
extern void cfl_date_setDay(CFL_DATEP date, CFL_UINT8 day);
extern void cfl_date_setHour(CFL_DATEP date, CFL_UINT8 hour);
extern void cfl_date_setMin(CFL_DATEP date, CFL_UINT8 min);
extern void cfl_date_setSec(CFL_DATEP date, CFL_UINT8 sec);
extern void cfl_date_setMillis(CFL_DATEP date, CFL_UINT16 millis);
extern void cfl_date_copy(CFL_DATEP fromDate, CFL_DATEP toDate);
//extern CFL_UINT64 cfl_date_toMillis(CFL_DATEP date);
//extern CFL_UINT64 cfl_date_diffMillis(CFL_DATEP date1, CFL_DATEP date2);
//extern CFL_UINT64 cfl_date_diffSec(CFL_DATEP date1, CFL_DATEP date2);
//extern CFL_UINT16 cfl_date_get(CFL_DATEP date, CFL_UINT8 field);
//extern void cfl_date_set(CFL_DATEP date, CFL_UINT8 field, CFL_UINT16 value);
//extern CFL_DATEP cfl_date_diff(CFL_DATEP date1, CFL_DATEP date2);
//extern CFL_UINT64 cfl_date_getCurrentMillis(void);

#ifdef __cplusplus
}
#endif

#endif
