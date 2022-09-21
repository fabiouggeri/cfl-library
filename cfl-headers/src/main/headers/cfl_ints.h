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

#ifndef _CFL_INTS_H

#define _CFL_INTS_H

#if (defined(_MSC_VER) && _MSC_VER < 1600) || (defined(__BORLANDC__) && __BORLANDC__ < 0x0600)
   typedef signed char      int8_t;
   typedef signed short     int16_t;
   typedef signed long      int32_t;
   typedef signed __int64   int64_t;
   typedef unsigned char    uint8_t;
   typedef unsigned short   uint16_t;
   typedef unsigned long    uint32_t;
   typedef unsigned __int64 uint64_t;
#else
   #include <stdint.h>
#endif

#endif
