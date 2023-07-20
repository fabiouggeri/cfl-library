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

#define LEAP_YEAR(y) ((y) % 400 == 0 || ((y) % 4 == 0 && (y) % 100 != 0))

static CFL_DATEP newDateInit(CFL_UINT16 year, CFL_UINT8 month, CFL_UINT8 day, CFL_UINT8 hour, CFL_UINT8 min, CFL_UINT8 sec, CFL_UINT16 millis) {
   CFL_DATEP date;
   date = (CFL_DATEP) malloc(sizeof(CFL_DATE));
   cfl_date_setYear(date, year);
   cfl_date_setMonth(date, month);
   cfl_date_setDay(date, day);
   cfl_date_setHour(date, hour);
   cfl_date_setMin(date, min);
   cfl_date_setSec(date, sec);
   cfl_date_setMillis(date, millis);
   return date;
}

CFL_DATEP cfl_date_new(void) {
   CFL_DATEP date;
   date = (CFL_DATEP) malloc(sizeof(CFL_DATE));
   cfl_date_getCurrent(date);
   return date;
}

void cfl_date_free(CFL_DATEP date) {
   free(date);
}

CFL_DATEP cfl_date_newDate(CFL_UINT16 year, CFL_UINT8 month, CFL_UINT8 day) {
   return newDateInit(year, month, day, 0, 0, 0, 0);
}

CFL_DATEP cfl_date_newDateTime(CFL_UINT16 year, CFL_UINT8 month, CFL_UINT8 day, CFL_UINT8 hour, CFL_UINT8 min, CFL_UINT8 sec, CFL_UINT16 millis) {
   return newDateInit(year, month, day, hour, min, sec, millis);
}

CFL_DATEP cfl_date_getCurrent(CFL_DATEP date) {
   time_t curTime;
   struct tm *tm;

   time(&curTime);
   tm = localtime(&curTime);
   cfl_date_setYear(date, (CFL_UINT16) tm->tm_year);
   cfl_date_setMonth(date, (CFL_UINT8) tm->tm_mon);
   cfl_date_setDay(date, (CFL_UINT8) tm->tm_mday);
   cfl_date_setHour(date, (CFL_UINT8) tm->tm_hour);
   cfl_date_setMin(date, (CFL_UINT8) tm->tm_min);
   cfl_date_setSec(date, (CFL_UINT8) tm->tm_sec);
   cfl_date_setMillis(date, 0);
   return date;
}

void cfl_date_getDate(CFL_DATEP date, CFL_UINT16 *year, CFL_UINT8 *month, CFL_UINT8 *day) {
   *year = cfl_date_getYear(date);
   *month = cfl_date_getMonth(date);
   *day = cfl_date_getDay(date);
}

void cfl_date_getTime(CFL_DATEP date, CFL_UINT8 *hour, CFL_UINT8 *min, CFL_UINT8 *sec, CFL_UINT16 *millis) {
   *hour = cfl_date_getHour(date);
   *min = cfl_date_getMin(date);
   *sec = cfl_date_getSec(date);
   *millis = cfl_date_getMillis(date);
}

void cfl_date_getDateTime(CFL_DATEP date, CFL_UINT16 *year, CFL_UINT8 *month, CFL_UINT8 *day, CFL_UINT8 *hour, CFL_UINT8 *min, CFL_UINT8 *sec, CFL_UINT16 *millis) {
   *year = cfl_date_getYear(date);
   *month = cfl_date_getMonth(date);
   *day = cfl_date_getDay(date);
   *hour = cfl_date_getHour(date);
   *min = cfl_date_getMin(date);
   *sec = cfl_date_getSec(date);
   *millis = cfl_date_getMillis(date);
}

void cfl_date_setDate(CFL_DATEP date, CFL_UINT16 year, CFL_UINT8 month, CFL_UINT8 day) {
   cfl_date_setYear(date, year);
   cfl_date_setMonth(date, month);
   cfl_date_setDay(date, day);
}

void cfl_date_setTime(CFL_DATEP date, CFL_UINT8 hour, CFL_UINT8 min, CFL_UINT8 sec, CFL_UINT16 millis) {
   cfl_date_setHour(date, hour);
   cfl_date_setMin(date, min);
   cfl_date_setSec(date, sec);
   cfl_date_setMillis(date, millis);
}

void cfl_date_setDateTime(CFL_DATEP date, CFL_UINT16 year, CFL_UINT8 month, CFL_UINT8 day, CFL_UINT8 hour, CFL_UINT8 min, CFL_UINT8 sec, CFL_UINT16 millis) {
   cfl_date_setYear(date, year);
   cfl_date_setMonth(date, month);
   cfl_date_setDay(date, day);
   cfl_date_setHour(date, hour);
   cfl_date_setMin(date, min);
   cfl_date_setSec(date, sec);
   cfl_date_setMillis(date, millis);
}
