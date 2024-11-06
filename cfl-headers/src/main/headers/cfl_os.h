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

#ifndef _CFL_OS_H_

   #define _CFL_OS_H_


#if defined(_WIN32)
   #define CFL_OS_WINDOWS
   #if defined(_WIN64)
      #define CFL_ARCH_64
   #else
      #define CFL_ARCH_32
   #endif
#elif defined(__linux__)
   #include <pthread.h>
   #include <errno.h>
   #include <sched.h>
   #define CFL_OS_LINUX
   #if defined(__LP64__) || defined(__x86_64__) || defined(__ppc64__)
      #define CFL_ARCH_64
   #else
      #define CFL_ARCH_32
   #endif
#endif

#endif
