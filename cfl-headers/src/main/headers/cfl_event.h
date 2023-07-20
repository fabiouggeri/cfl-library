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

#ifndef CFL_EVENT_H_

#define CFL_EVENT_H_

#include "cfl_types.h"

#define CFL_EVENT_FAIL    0
#define CFL_EVENT_SET     1
#define CFL_EVENT_TIMEOUT 2

struct _CFL_EVENT {
#if defined(CFL_OS_WINDOWS)
   HANDLE handle;
#elif defined(CFL_OS_LINUX)
   pthread_cond_t  conditionVar;
   pthread_mutex_t mutex;
   CFL_BOOL        autoReset;
   CFL_BOOL        state;
#endif
};

extern CFL_EVENTP cfl_event_new(char *name, CFL_BOOL manualReset);
extern void cfl_event_free(CFL_EVENTP event);
extern void cfl_event_set(CFL_EVENTP event);
extern void cfl_event_reset(CFL_EVENTP event);
extern CFL_BOOL cfl_event_wait(CFL_EVENTP event);
extern CFL_UINT8 cfl_event_wait2(CFL_EVENTP event);
extern CFL_BOOL cfl_event_waitTimeout(CFL_EVENTP event, CFL_INT32 timeout);
extern CFL_UINT8 cfl_event_waitTimeout2(CFL_EVENTP event, CFL_INT32 timeout);

#endif
