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

#ifndef CFL_ERROR_H_

#define CFL_ERROR_H_

#include "cfl_types.h"

struct _CFL_ERROR {
   CFL_STRP  message;
   void      *userData;
   CFL_INT32 code;
   CFL_UINT8 type;
   CFL_BOOL  allocated;
};

extern CFL_ERRORP cfl_error_new();
extern void cfl_error_free(CFL_ERRORP pError);
extern void cfl_error_init(CFL_ERRORP pError);
extern CFL_UINT8 cfl_error_getType(CFL_ERRORP pError);
extern void cfl_error_setType(CFL_ERRORP pError, CFL_UINT8 type);
extern CFL_UINT32 cfl_error_getCode(CFL_ERRORP pError);
extern void cfl_error_setCode(CFL_ERRORP pError, CFL_UINT32 code);
extern CFL_STRP cfl_error_getMessage(CFL_ERRORP pError);
extern const char * cfl_error_getMessageStr(CFL_ERRORP pError);
extern void cfl_error_setMessage(CFL_ERRORP pError, char *message);
extern void * cfl_error_getUserData(CFL_ERRORP pError);
extern void cfl_error_setUserData(CFL_ERRORP pError, void *userData);
extern void cfl_error_clear(CFL_ERRORP pError);

#endif
