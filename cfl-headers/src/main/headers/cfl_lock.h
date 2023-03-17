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

#ifndef CFL_LOCK_H_

#define CFL_LOCK_H_

#include "cfl_types.h"

#if ! defined(__BORLANDC__)
   #define _CFL_CONDITION_VAR
#endif

#define CFL_LOCK_SUCCESS 0
#define CFL_LOCK_ERROR   1
#define CFL_LOCK_TIMEOUT 2

struct _CFL_CONDITION_VARIABLE {
   CFL_CONDITION_HANDLE handle;
   CFL_BOOL             isAllocated;
};

struct _CFL_LOCK {
   CFL_LOCK_HANDLE        handle;
   CFL_THREAD_ID          lockOwner;
   CFL_UINT32             lockCount;
   CFL_CONDITION_VARIABLE notLocked;
   CFL_BOOL               isAllocated;
};

extern CFL_LOCKP cfl_lock_new(void);
extern void cfl_lock_free(CFL_LOCKP pLock);
extern void cfl_lock_init(CFL_LOCKP pLock);
extern CFL_BOOL cfl_lock_tryExclusive(CFL_LOCKP pLock);
extern void cfl_lock_acquire(CFL_LOCKP pLock);
extern CFL_BOOL cfl_lock_tryShared(CFL_LOCKP pLock);
extern void cfl_lock_acquireShared(CFL_LOCKP pLock);
extern void cfl_lock_release(CFL_LOCKP pLock);
extern void cfl_lock_initConditionVar(CFL_CONDITION_VARIABLEP pVar);
extern CFL_CONDITION_VARIABLEP cfl_lock_newConditionVar(void);
extern void cfl_lock_freeConditionVar(CFL_CONDITION_VARIABLEP pVar);
extern CFL_BOOL cfl_lock_conditionWait(CFL_LOCKP pLock, CFL_CONDITION_VARIABLEP pVar);
extern CFL_UINT8 cfl_lock_conditionWaitTimeout(CFL_LOCKP pLock, CFL_CONDITION_VARIABLEP pVar, CFL_UINT32 timeout);
extern void cfl_lock_conditionWake(CFL_CONDITION_VARIABLEP pVar);
extern void cfl_lock_conditionWakeAll(CFL_CONDITION_VARIABLEP pVar);
extern CFL_INT32 cfl_lock_lastErrorCode(void);

#endif