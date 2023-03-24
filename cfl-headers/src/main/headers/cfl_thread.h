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

#ifndef CFL_THREAD_H_

#define CFL_THREAD_H_

#include "cfl_types.h"

typedef CFL_INT32 (* CFL_THREAD_FUNC)(void *param);

struct _CFL_THREAD {
   CFL_THREAD_FUNC   func;
   void              *param;
   void              *data;
   CFL_THREAD_HANDLE handle;
};

extern CFL_THREADP cfl_thread_new(CFL_THREAD_FUNC func);
extern void cfl_thread_free(CFL_THREADP thread);
extern CFL_THREADP cfl_thread_getCurrent(void);
extern void * cfl_thread_getData(CFL_THREADP thread);
extern void cfl_thread_setData(CFL_THREADP thread, void *data);
extern CFL_BOOL cfl_thread_start(CFL_THREADP thread, void * param);
extern CFL_BOOL cfl_thread_wait(CFL_THREADP thread);
extern CFL_BOOL cfl_thread_waitTimeout(CFL_THREADP thread, CFL_INT32 timeout);
extern CFL_BOOL cfl_thread_kill(CFL_THREADP thread);
extern CFL_BOOL cfl_thread_sleep(CFL_UINT32 time);

#endif