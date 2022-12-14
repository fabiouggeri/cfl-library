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

#include "cfl_socket.h"
#include "cfl_types.h"
#include "cfl_buffer.h"

#ifdef __linux__
   #include <string.h>
   #include <arpa/inet.h>
   #include <fcntl.h>
   #include <netinet/tcp.h>
#elif defined(_WIN32) && ! defined(__BORLANDC__)
   #include <mstcpip.h>
#endif

CFL_SOCKET cfl_socket_open(const char *serverAddress, CFL_UINT16 port) {
#ifdef __linux__
   CFL_SOCKET socketHandle;
   struct sockaddr_in serv_addr;
   struct hostent *he;
   struct in_addr **addr_list;

   socketHandle = socket(AF_INET, SOCK_STREAM, 0);

   memset(&serv_addr, '0', sizeof(serv_addr));

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_port = htons(port);

   if (inet_pton(AF_INET, serverAddress, &serv_addr.sin_addr) <= 0) {
      he = gethostbyname(serverAddress);
      if (he == NULL) {
         return CFL_INVALID_SOCKET;
      }
      addr_list = (struct in_addr **) he->h_addr_list;
      if (addr_list != NULL && addr_list[0] != NULL) {
         serv_addr.sin_addr = *addr_list[0];
      } else {
         return CFL_INVALID_SOCKET;
      }
   }

   if (connect(socketHandle, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      return CFL_INVALID_SOCKET;
   }

   return socketHandle;

   /* If not Borland C. */
#elif ! defined(__BORLANDC__)
   int retVal;
   CFL_SOCKET socketHandle = CFL_INVALID_SOCKET;
   struct addrinfo *addr;
   struct addrinfo addrCriteria;
   struct addrinfo *serverAddr;
   char serverPort[7];
   WSADATA winsockinfo;

   retVal = WSAStartup(MAKEWORD(2, 2), &winsockinfo);
   if (retVal != 0) {
      return CFL_INVALID_SOCKET;
   }

   memset(&addrCriteria, 0, sizeof(addrCriteria));
   addrCriteria.ai_family = AF_UNSPEC;
   addrCriteria.ai_socktype = SOCK_STREAM;
   addrCriteria.ai_protocol = IPPROTO_TCP;

   // Get address(es)
   sprintf(serverPort, "%u", port);
   retVal = getaddrinfo(serverAddress, serverPort, &addrCriteria, &serverAddr);
   if (retVal != 0) {
      return CFL_INVALID_SOCKET;
   }

   for (addr = serverAddr; addr != NULL; addr = addr->ai_next) {
      // Create a reliable, stream socket using TCP
      socketHandle = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
      if (socketHandle == CFL_INVALID_SOCKET) {
         continue;
      }

      // Establish the connection to the echo server
      if (connect(socketHandle, addr->ai_addr, (int) addr->ai_addrlen) == 0) {
         break;
      }

      closesocket(socketHandle);
      socketHandle = CFL_INVALID_SOCKET;
   }

   freeaddrinfo(serverAddr); // Free addrinfo allocated in getaddrinfo()
   return socketHandle;
#else
   int retVal;
   CFL_SOCKET socketHandle;
   struct sockaddr_in addr;
   WSADATA winsockinfo;
   struct hostent *remoteHost;

   retVal = WSAStartup(MAKEWORD(2, 2), &winsockinfo);
   if (retVal != 0) {
      return CFL_INVALID_SOCKET;
   }

   socketHandle = (CFL_SOCKET) socket(AF_INET, SOCK_STREAM, 0);
   if (socketHandle == CFL_INVALID_SOCKET) {
      return CFL_INVALID_SOCKET;
   }

   memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = inet_addr(serverAddress);
   addr.sin_port = htons(port);
   /* Resolve by name */
   if (addr.sin_addr.s_addr == INADDR_NONE) {
      remoteHost = gethostbyname(serverAddress);
      if (remoteHost == NULL) {
         return CFL_INVALID_SOCKET;
      }
      if (remoteHost->h_addr_list[0] != 0) {
         addr.sin_addr.s_addr = *(u_long *) remoteHost->h_addr_list[0];
      } else {
         return CFL_INVALID_SOCKET;
      }
   }

   retVal = connect(socketHandle, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
   if (retVal < 0) {
      closesocket(socketHandle);
      return CFL_INVALID_SOCKET;
   }
   return socketHandle;
#endif
}

CFL_INT32 cfl_socket_close(CFL_SOCKET socket) {
#ifdef __linux__
   return close(socket);
#else
   CFL_INT32 rc;
   rc = closesocket(socket);
   WSACleanup();
   return rc;
#endif
}

CFL_INT32 cfl_socket_sendBuffer(CFL_SOCKET socket, CFL_BUFFERP buffer) {
   CFL_INT32 bytesSent;
   bytesSent = send(socket, (const char *) cfl_buffer_getDataPtr(buffer), (int) cfl_buffer_remaining(buffer), 0);
   if (bytesSent > 0) {
      cfl_buffer_skip(buffer, bytesSent);
   }
   return bytesSent;
}

CFL_INT32 cfl_socket_send(CFL_SOCKET socket, char *buffer, CFL_INT32 len) {
   return send(socket, buffer, (int) len, 0);
}

CFL_BOOL cfl_socket_sendAllBuffer(CFL_SOCKET socket, CFL_BUFFERP buffer) {
   CFL_UINT32 totalSent = 0; // how many bytes we've sent
   CFL_UINT32 bytesLeft;
   CFL_INT32 bytesSent;
   const char *data;

   bytesLeft = cfl_buffer_remaining(buffer);
   data = (const char *) cfl_buffer_getDataPtr(buffer);
   while (bytesLeft > 0) {
      bytesSent = send(socket, data + totalSent, (int) bytesLeft, 0);
      if (bytesSent < 0) {
         break;
      }
      totalSent += bytesSent;
      bytesLeft -= bytesSent;
   }
   cfl_buffer_skip(buffer, totalSent);
   return bytesLeft == 0;
}

CFL_BOOL cfl_socket_sendAll(CFL_SOCKET socket, char *buffer, CFL_UINT32 len) {
   CFL_UINT32 totalSent = 0; // how many bytes we've sent
   CFL_UINT32 bytesLeft;
   CFL_INT32 bytesSent;

   bytesLeft = len;
   while (bytesLeft > 0) {
      bytesSent = send(socket, buffer + totalSent, (int) bytesLeft, 0);
      if (bytesSent < 0) {
         break;
      }
      totalSent += bytesSent;
      bytesLeft -= bytesSent;
   }
   return bytesLeft == 0;
}

CFL_INT32 cfl_socket_receive(CFL_SOCKET socket, char *buffer, int len) {
   return recv(socket, buffer, len, 0);
}

CFL_INT32 cfl_socket_receiveAll(CFL_SOCKET socket, char *buffer, int len) {
   int retVal;
   int toRead = len;

   while (toRead > 0) {
      retVal = recv(socket, buffer, toRead, 0);
      if (retVal <= 0) {
         return retVal;
      }
      buffer += retVal;
      toRead -= retVal;
   }
   return len;
}

CFL_BOOL cfl_socket_receiveAllBuffer(CFL_SOCKET socket, CFL_BUFFERP buffer, CFL_UINT32 packetLen, CFL_UINT32 timeout) {
   int retVal;
   CFL_UINT32 bodyLen;
   char *dataRead;
   int readLen;

   // wait until timeout or data received
   retVal = cfl_socket_selectRead(socket, timeout, 0);
   if (retVal <= 0) {
      return CFL_FALSE;
   }

   dataRead = (char *) malloc(packetLen * sizeof(char));
   if (dataRead == NULL) {
      return CFL_FALSE;
   }
   // data must be here, so do a normal recv()
   bodyLen = cfl_buffer_remaining(buffer);
   while (bodyLen > 0) {
      readLen = bodyLen > packetLen ? packetLen : bodyLen;
      retVal = recv(socket, dataRead, readLen, 0);
      if (retVal == CFL_SOCKET_ERROR) {
         free(dataRead);
         return CFL_FALSE;
      } else if (retVal == 0) {
         free(dataRead);
         return CFL_FALSE;
      }
      bodyLen -= retVal;
      cfl_buffer_put(buffer, dataRead, retVal);
   }
   free(dataRead);
   return CFL_TRUE;
}

CFL_INT32 cfl_socket_selectRead(CFL_SOCKET socket, long sec, long mSec) {
   fd_set fdSet;
   struct timeval time;
   struct timeval * timeout;

   FD_ZERO(&fdSet);
   FD_SET(socket, &fdSet);

   if (sec >= 0 && mSec >= 0) {
      time.tv_sec = sec < 0 ? 0 : sec;
      time.tv_usec = mSec < 0 ? 0 : mSec;
      timeout = &time;
   } else {
      timeout = NULL;
   }

   return select((int) socket + 1, &fdSet, NULL, NULL, timeout);
}

CFL_INT32 cfl_socket_selectWrite(CFL_SOCKET socket, long sec, long mSec) {
   fd_set fdSet;
   struct timeval time;
   struct timeval * timeout;

   FD_ZERO(&fdSet);
   FD_SET(socket, &fdSet);

   if (sec >= 0 && mSec >= 0) {
      time.tv_sec = sec < 0 ? 0 : sec;
      time.tv_usec = mSec < 0 ? 0 : mSec;
      timeout = &time;
   } else {
      timeout = NULL;
   }

   return select((int) socket + 1, NULL, &fdSet, NULL, timeout);
}

CFL_INT32 cfl_socket_select(CFL_SOCKET socket, long sec, long mSec) {
   fd_set fdSet;
   struct timeval time;
   struct timeval * timeout;

   FD_ZERO(&fdSet);
   FD_SET(socket, &fdSet);

   if (sec >= 0 && mSec >= 0) {
      time.tv_sec = sec < 0 ? 0 : sec;
      time.tv_usec = mSec < 0 ? 0 : mSec;
      timeout = &time;
   } else {
      timeout = NULL;
   }

   return select((int) socket + 1, &fdSet, &fdSet, NULL, timeout);
}

CFL_INT32 cfl_socket_lastErrorCode(void) {
#ifdef __linux__
   return errno;
#else
   return WSAGetLastError();
#endif
}

CFL_BOOL cfl_socket_setBlockingMode(CFL_SOCKET socket, CFL_BOOL block) {
   CFL_BOOL ret;
#ifdef __linux__
    const int flags = fcntl(socket, F_GETFL, 0);
    if ((flags & O_NONBLOCK) && !block) {
       return CFL_TRUE;
    }
    if (!(flags & O_NONBLOCK) && block) {
       return CFL_TRUE;
    }
    ret = fcntl(socket, F_SETFL, block ? flags ^ O_NONBLOCK : flags | O_NONBLOCK) == 0 ? CFL_TRUE : CFL_FALSE;
#else
   u_long mode = block ? 1 : 0;
   ret = ioctlsocket(socket, FIONBIO, &mode) == NO_ERROR ? CFL_TRUE : CFL_FALSE;
#endif
   return ret;
}

CFL_BOOL cfl_socket_setNoDelay(CFL_SOCKET socket, CFL_BOOL delay) {
   return setsockopt(socket, SOL_SOCKET, TCP_NODELAY, (char *) &delay, sizeof(CFL_BOOL)) == 0 ? CFL_TRUE : CFL_FALSE;
}

CFL_BOOL cfl_socket_setReceiveBufferSize(CFL_SOCKET socket, int size) {
   return setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char *) &size, sizeof(int)) == 0 ? CFL_TRUE : CFL_FALSE;
}

CFL_BOOL cfl_socket_setSendBufferSize(CFL_SOCKET socket, int size) {
   return setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char *) &size, sizeof(int)) == 0 ? CFL_TRUE : CFL_FALSE;
}

CFL_BOOL cfl_socket_setKeepAlive(CFL_SOCKET socket, CFL_BOOL active, CFL_UINT32 time, CFL_UINT32 interval) {
#ifdef __linux__

   if(setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (char *) &active, sizeof(CFL_BOOL)) != 0) {
      return CFL_FALSE;
   }
   if (active) {
      int keepidle = (int) time;
      int keepintvl = (int) interval;
      if (setsockopt(socket, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(int)) != 0) {
         return CFL_FALSE;
      }
      if (setsockopt(socket, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(int)) != 0) {
         return CFL_FALSE;
      }
   }
   return CFL_TRUE;
#elif defined(_WIN32) && ! defined(__BORLANDC__)
   DWORD dwBytes = 0;
   struct tcp_keepalive kaSettings;
   kaSettings.onoff = active ? 1L : 0L;
   kaSettings.keepalivetime = (ULONG) time;
   kaSettings.keepaliveinterval = (ULONG) interval;
   return WSAIoctl(socket,
                   SIO_KEEPALIVE_VALS,
                   &kaSettings,
                   sizeof(kaSettings),
                   NULL,
                   0,
                   &dwBytes,
                   NULL,
                   NULL) == 0 ? CFL_TRUE : CFL_FALSE;
#else
   return CFL_FALSE;
#endif
}