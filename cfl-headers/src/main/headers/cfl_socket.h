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

#ifndef _CFL_SOCKET_H_

#define _CFL_SOCKET_H_

#include <stdio.h>

#include "cfl_types.h"

#ifdef _WIN32
   #define CFL_SOCK_ERROR_CONN_RESET WSAECONNRESET
#else 
   #define CFL_SOCK_ERROR_CONN_RESET ECONNRESET
#endif

//struct _CFL_SOCKET {
//   socket     handle;
//   CFL_INT32  errorCode;
//   CFL_STRP   errorMessage;
//   CFL_UINT32 bufferSize;
//   char      *buffer;
//};

extern CFL_SOCKET cfl_socket_listen(const char *address, CFL_UINT16 port, CFL_INT32 backlog);
extern CFL_SOCKET cfl_socket_accept(CFL_SOCKET listenSocket, CFL_STRP clientAddr);
extern CFL_SOCKET cfl_socket_open(const char *serverAddress, CFL_UINT16 port);
extern CFL_INT32 cfl_socket_close(CFL_SOCKET socket);
extern CFL_INT32 cfl_socket_sendBuffer(CFL_SOCKET socket, CFL_BUFFERP buffer);
extern CFL_INT32 cfl_socket_send(CFL_SOCKET socket, const char *buffer, CFL_INT32 len);
extern CFL_BOOL cfl_socket_sendAllBuffer(CFL_SOCKET socket, CFL_BUFFERP buffer);
extern CFL_BOOL cfl_socket_sendAll(CFL_SOCKET socket, const char *buffer, CFL_UINT32 len);
extern CFL_INT32 cfl_socket_receive(CFL_SOCKET socket, const char *buffer, int len);
extern CFL_INT32 cfl_socket_receiveAll(CFL_SOCKET socket, const char *buffer, int len);
extern CFL_BOOL cfl_socket_receiveAllBuffer(CFL_SOCKET socket, CFL_BUFFERP buffer, CFL_UINT32 packetLen, CFL_UINT32 timeout);
extern CFL_INT32 cfl_socket_selectRead(CFL_SOCKET socket, long sec, long mSec);
extern CFL_INT32 cfl_socket_selectWrite(CFL_SOCKET socket, long sec, long mSec);
extern CFL_INT32 cfl_socket_select(CFL_SOCKET socket, long sec, long mSec);
extern CFL_INT32 cfl_socket_lastErrorCode(void);
extern CFL_BOOL cfl_socket_setBlockingMode(CFL_SOCKET socket, CFL_BOOL block);
extern CFL_BOOL cfl_socket_setNoDelay(CFL_SOCKET socket, CFL_BOOL delay);
extern CFL_BOOL cfl_socket_setReceiveBufferSize(CFL_SOCKET socket, int size);
extern CFL_BOOL cfl_socket_setSendBufferSize(CFL_SOCKET socket, int size);
extern CFL_BOOL cfl_socket_setKeepAlive(CFL_SOCKET socket, CFL_BOOL active, CFL_UINT32 time, CFL_UINT32 interval);

#endif
