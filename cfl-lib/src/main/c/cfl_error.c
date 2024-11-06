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

#include "cfl_error.h"
#include "cfl_str.h"

CFL_ERRORP cfl_error_new() {
   CFL_ERRORP pError = (CFL_ERRORP) CFL_MEM_ALLOC(sizeof(CFL_ERROR));
   cfl_error_init(pError);
   pError->allocated = CFL_TRUE;
   return pError;
}

void cfl_error_free(CFL_ERRORP pError){
   if (pError) {
      cfl_str_free(pError->message);
      if (pError->allocated) {
         CFL_MEM_FREE(pError);
      }
   }
}

void cfl_error_init(CFL_ERRORP pError) {
   pError->allocated = CFL_FALSE;
   pError->type = CFL_NO_ERROR_TYPE;
   pError->code = CFL_NO_ERROR_CODE;
   pError->message = cfl_str_new(30);
   pError->userData = NULL;
}

void cfl_error_clear(CFL_ERRORP pError) {
   pError->type = CFL_NO_ERROR_TYPE;
   pError->code = CFL_NO_ERROR_CODE;
   cfl_str_clear(pError->message);
   pError->userData = NULL;
}

CFL_UINT8 cfl_error_getType(CFL_ERRORP pError) {
   return pError->type;
}

void cfl_error_setType(CFL_ERRORP pError, CFL_UINT8 type) {
   pError->type = type;
}

CFL_UINT32 cfl_error_getCode(CFL_ERRORP pError) {
   return pError->code;
}

void cfl_error_setCode(CFL_ERRORP pError, CFL_UINT32 code) {
   pError->code = code;
}

CFL_STRP cfl_error_getMessage(CFL_ERRORP pError) {
   return pError->message;
}

const char * cfl_error_getMessageStr(CFL_ERRORP pError) {
   return cfl_str_getPtr(pError->message);
}

void cfl_error_setMessage(CFL_ERRORP pError, char *message) {
   cfl_str_setValue(pError->message, message);
}

void * cfl_error_getUserData(CFL_ERRORP pError) {
   return pError->userData;
}

void cfl_error_setUserData(CFL_ERRORP pError, void *userData) {
   pError->userData = userData;
}
