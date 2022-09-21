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

#ifdef __linux__
#include <errno.h>
#include <sys/time.h>
#endif

#include "cfl_event.h"

CFL_EVENTP cfl_event_new(char *name, CFL_BOOL manualReset) {
   CFL_EVENTP event = NULL;
#ifdef _WIN32
   HANDLE handle;
   handle = CreateEvent(NULL, manualReset, CFL_FALSE, name ? TEXT(name) : NULL);
   if (handle) {
      event = malloc(sizeof (CFL_EVENT));
      event->handle = handle;
   }
#elif __linux__
   int result;
   event = malloc(sizeof (CFL_EVENT));
   if (event == NULL) {
      return NULL;
   }
   result = pthread_cond_init(&event->conditionVar, 0);
   if (result != 0) {
      free(event);
      return NULL;
   }
   result = pthread_mutex_init(&event->mutex, 0);
   if (result != 0) {
      free(event);
      return NULL;
   }
   event->state = CFL_FALSE;
   event->autoReset = !manualReset;
#endif
   return event;
}

void cfl_event_free(CFL_EVENTP event) {
#ifdef _WIN32
   CloseHandle(event->handle);
#elif __linux__
   pthread_cond_destroy(&event->conditionVar);
   pthread_mutex_destroy(&event->mutex);
#endif
   free(event);
}

void cfl_event_set(CFL_EVENTP event) {
#ifdef _WIN32
   SetEvent(event->handle);
#elif __linux__
   if (pthread_mutex_lock(&event->mutex) == 0) {
      event->state = CFL_TRUE;

      if (pthread_mutex_unlock(&event->mutex) == 0) {
         // Depending on the event type, we either trigger everyone or only one
         if (event->autoReset) {
            pthread_cond_signal(&event->conditionVar);
         } else {
            pthread_cond_broadcast(&event->conditionVar);
         }
      }
   }
#endif
}

void cfl_event_reset(CFL_EVENTP event) {
#ifdef _WIN32
   ResetEvent(event->handle);
#elif __linux__
   if (pthread_mutex_lock(&event->mutex) == 0) {
      event->state = CFL_FALSE;
      pthread_mutex_unlock(&event->mutex);
   }
#endif
}


#ifdef __linux__
static int unlockedWaitForEvent(CFL_EVENTP event, CFL_UINT64 milliseconds) {
   int result = 0;
   if (!event->state) {
      if (milliseconds == 0) {
         result = pthread_cond_wait(&event->conditionVar, &event->mutex);
      } else {
         struct timespec ts;
         struct timeval tv;
         CFL_UINT64 nanoseconds;

         gettimeofday(&tv, NULL);

         nanoseconds = ((CFL_UINT64) tv.tv_sec) * 1000 * 1000 * 1000 +
                 milliseconds * 1000 * 1000 +
                 ((CFL_UINT64) tv.tv_usec) * 1000;

         ts.tv_sec = nanoseconds / 1000 / 1000 / 1000;
         ts.tv_nsec = (long) (nanoseconds - ((CFL_UINT64) ts.tv_sec) * 1000 * 1000 * 1000);
         do {
            result = pthread_cond_timedwait(&event->conditionVar, &event->mutex, &ts);
         } while (result == 0 && !event->state);
      }

      if (result == 0 && event->autoReset) {
         event->state = CFL_FALSE;
      }
   } else if (event->autoReset) {
      result = 0;
      event->state = CFL_FALSE;
   }

   return result;
}
#endif

CFL_BOOL cfl_event_wait(CFL_EVENTP event) {
#ifdef _WIN32
   return WaitForSingleObject(event->handle, INFINITE) == WAIT_OBJECT_0 ? CFL_TRUE : CFL_FALSE;
#elif __linux__
   return pthread_mutex_lock(&event->mutex) == 0 &&
           unlockedWaitForEvent(event, 0) == 0 &&
           pthread_mutex_unlock(&event->mutex) == 0 ? CFL_TRUE : CFL_FALSE;
#endif
}

CFL_UINT8 cfl_event_wait2(CFL_EVENTP event) {
#ifdef _WIN32
   if (WaitForSingleObject(event->handle, INFINITE) == WAIT_OBJECT_0) {
      return CFL_EVENT_SET;
   }
   return CFL_EVENT_FAIL;
#elif __linux__
   return pthread_mutex_lock(&event->mutex) == 0 &&
           unlockedWaitForEvent(event, 0) == 0 &&
           pthread_mutex_unlock(&event->mutex) == 0 ? CFL_EVENT_SET : CFL_EVENT_FAIL;
#endif
}

CFL_BOOL cfl_event_waitTimeout(CFL_EVENTP event, CFL_INT32 timeout) {
#ifdef _WIN32
   DWORD res;
   res = WaitForSingleObject(event->handle, timeout);
   return res == WAIT_OBJECT_0 || res == WAIT_TIMEOUT;
#elif __linux__
   int result;
   if (timeout == 0) {
      result = pthread_mutex_trylock(&event->mutex);
   } else {
      result = pthread_mutex_lock(&event->mutex);
   }
   return result == 0 &&
           unlockedWaitForEvent(event, timeout) == 0 &&
           pthread_mutex_unlock(&event->mutex) == 0 ? CFL_TRUE : CFL_FALSE;
#endif
}

CFL_UINT8 cfl_event_waitTimeout2(CFL_EVENTP event, CFL_INT32 timeout) {
#ifdef _WIN32
   DWORD res;
   res = WaitForSingleObject(event->handle, timeout);
   switch (res) {
      case WAIT_OBJECT_0:
         return CFL_EVENT_SET;
      case WAIT_TIMEOUT:
         return CFL_EVENT_TIMEOUT;
      default:
         return CFL_EVENT_FAIL;
   }
#elif __linux__
   int result;
   if (pthread_mutex_lock(&event->mutex) == 0) {
      result = unlockedWaitForEvent(event, timeout);
      if (result == ETIMEDOUT) {
         return CFL_EVENT_TIMEOUT;
      }
      if (pthread_mutex_unlock(&event->mutex) == 0 && result == 0) {
         return CFL_EVENT_SET;
      }
   }
   return CFL_EVENT_FAIL;
#endif
}
