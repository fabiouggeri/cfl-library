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
#include <stdio.h>

#include "cfl_socket.h"
#include "cfl_buffer.h"
#include "cfl_str.h"

#if defined(CFL_OS_WINDOWS)
   #if defined(__BORLANDC__)
      #include <winsock2.h>
      #include <io.h>
   #elif defined(_MSC_VER)
      #include <winsock2.h>
      #include <um\ws2ipdef.h>
      #include <um\ws2tcpip.h>
      #include <mstcpip.h>
   #elif defined(__MINGW64__) || defined(__MINGW32__)
      #include <winsock2.h>
      #include <ws2ipdef.h>
      #include <ws2tcpip.h>
      #include <mstcpip.h>
   #endif
   #include <windows.h>
#else
   #include <sys/socket.h>
   #include <unistd.h>
   #include <netdb.h>
   #include <string.h>
   #include <arpa/inet.h>
   #include <fcntl.h>
   #include <netinet/tcp.h>
   #include <signal.h>
#endif

#if defined(__BORLANDC__)
   #define INET6_ADDRSTRLEN 65
#endif

#if defined(CFL_OS_WINDOWS)
   #define OS_SOCKET           SOCKET
   #define OS_INVALID_SOCKET   INVALID_SOCKET
   #define OS_SOCKET_ERROR     SOCKET_ERROR

   #define SHUTDOWN_READ       SD_RECEIVE
   #define SHUTDOWN_WRITE      SD_SEND
   #define SHUTDOWN_READ_WRITE SD_BOTH
#else
   #define OS_SOCKET           int
   #define OS_INVALID_SOCKET   -1
   #define OS_SOCKET_ERROR     -1

   #define SHUTDOWN_READ       SHUT_RD
   #define SHUTDOWN_WRITE      SHUT_WR
   #define SHUTDOWN_READ_WRITE SHUT_RDWR
#endif

#define STR_EMPTY(s) ((s) == NULL || (s)[0] == '\0')

#define CREATE_SOCKET(s) ((s) != OS_INVALID_SOCKET ? (CFL_SOCKET)(s) : CFL_INVALID_SOCKET)
#define GET_OS_SOCKET(s) ((OS_SOCKET)(s))

CFL_SOCKET cfl_socket_listen(const char *address, CFL_UINT16 port, CFL_INT32 backlog) {
#if defined(CFL_OS_LINUX)
   OS_SOCKET socketHandle;
   struct sockaddr_in serv_addr;
   struct hostent *he;
   struct in_addr **addr_list;

   socketHandle = socket(AF_INET, SOCK_STREAM, 0);

   if (socketHandle == OS_INVALID_SOCKET) {
      return CFL_INVALID_SOCKET;
   }

   memset(&serv_addr, '0', sizeof(serv_addr));

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_port = htons(port);

   if (STR_EMPTY(address)) {
      serv_addr.sin_addr.s_addr = INADDR_ANY;
   } else if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0) {
      he = gethostbyname(address);
      if (he == NULL) {
         close(socketHandle);
         return CFL_INVALID_SOCKET;
      }
      addr_list = (struct in_addr **) he->h_addr_list;
      if (addr_list != NULL && addr_list[0] != NULL) {
         serv_addr.sin_addr = *addr_list[0];
      } else {
         close(socketHandle);
         return CFL_INVALID_SOCKET;
      }
   }
   if (bind(socketHandle, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
      close(socketHandle);
      return CFL_INVALID_SOCKET;
   }

   if (listen(socketHandle, (int)backlog) != 0) {
      close(socketHandle);
      return CFL_INVALID_SOCKET;
   }

   return CREATE_SOCKET(socketHandle);

   /* If not Borland C. */
#elif ! defined(__BORLANDC__)
   OS_SOCKET socketHandle = OS_INVALID_SOCKET;
   struct addrinfo *addr;
   struct addrinfo addrCriteria;
   struct addrinfo *serverAddr;
   char serverPort[7];
   WSADATA winsockinfo;

   if (WSAStartup(MAKEWORD(2, 2), &winsockinfo) != 0) {
      return CFL_INVALID_SOCKET;
   }

   memset(&addrCriteria, 0, sizeof(addrCriteria));
   addrCriteria.ai_family = STR_EMPTY(address) ? AF_INET : AF_UNSPEC;
   addrCriteria.ai_socktype = SOCK_STREAM;
   addrCriteria.ai_protocol = IPPROTO_TCP;

   // Get address(es)
   sprintf(serverPort, "%u", port);
   if (getaddrinfo(address, serverPort, &addrCriteria, &serverAddr) != 0) {
      return CFL_INVALID_SOCKET;
   }

   for (addr = serverAddr; addr != NULL; addr = addr->ai_next) {
      // Create a reliable, stream socket using TCP
      socketHandle = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
      if (socketHandle == OS_INVALID_SOCKET) {
         continue;
      }
   
      // Establish the connection to server
      if (bind(socketHandle, addr->ai_addr, (int) addr->ai_addrlen) == 0 && listen(socketHandle, (int)backlog) == 0) {
         break;
      }

      closesocket(socketHandle);
      socketHandle = OS_INVALID_SOCKET;
   }

   freeaddrinfo(serverAddr); // Free addrinfo allocated in getaddrinfo()
   return CREATE_SOCKET(socketHandle);
#else
   OS_SOCKET socketHandle;
   struct sockaddr_in addr;
   WSADATA winsockinfo;
   struct hostent *remoteHost;

   if (WSAStartup(MAKEWORD(2, 2), &winsockinfo) != 0) {
      return CFL_INVALID_SOCKET;
   }

   socketHandle = (OS_SOCKET) socket(AF_INET, SOCK_STREAM, 0);
   if (socketHandle == OS_INVALID_SOCKET) {
      return CFL_INVALID_SOCKET;
   }

   memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_port = htons(port);
   
   if (STR_EMPTY(address)) {
      addr.sin_addr.s_addr = INADDR_ANY;
   } else {
      addr.sin_addr.s_addr = inet_addr(address);
      /* Resolve by name */
      if (addr.sin_addr.s_addr == INADDR_NONE) {
         remoteHost = gethostbyname(address);
         if (remoteHost == NULL) {
            return CFL_INVALID_SOCKET;
         }
         if (remoteHost->h_addr_list[0] != 0) {
            addr.sin_addr.s_addr = *(u_long *) remoteHost->h_addr_list[0];
         } else {
            return CFL_INVALID_SOCKET;
         }
      }
   }

   if (bind(socketHandle, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) != 0) {
      closesocket(socketHandle);
      return CFL_INVALID_SOCKET;
   }

   if (listen(socketHandle, (int)backlog) != 0) {
      close(socketHandle);
      return CFL_INVALID_SOCKET;
   }

   return CREATE_SOCKET(socketHandle);
#endif
}

static const char *clientAddress(struct sockaddr *sockAddr, char *buffer, size_t bufferSize) {
   const char *addr;
#if ! defined(__BORLANDC__)
   if (sockAddr->sa_family == AF_INET) {
      addr = inet_ntop(sockAddr->sa_family, (void *) &(((struct sockaddr_in*)sockAddr)->sin_addr), buffer, bufferSize);
   } else {
      addr = inet_ntop(sockAddr->sa_family, (void *) &(((struct sockaddr_in6*)sockAddr)->sin6_addr), buffer, bufferSize);
   }
#else
   if (((struct sockaddr_in*)sockAddr)->sin_family == AF_INET) {
      addr = buffer;
      snprintf(addr, bufferSize, "%s", inet_ntoa(((struct sockaddr_in*)sockAddr)->sin_addr));
   } else {
      addr = NULL;
   }
#endif
   return addr;
}

CFL_SOCKET cfl_socket_accept(CFL_SOCKET listenSocket, CFL_STRP clientAddr, CFL_UINT16 *port) {
   OS_SOCKET clientSocket;
   struct sockaddr sockAddr;
   int len = sizeof(sockAddr);

   clientSocket = accept(GET_OS_SOCKET(listenSocket), &sockAddr, &len);
   if (clientSocket != OS_INVALID_SOCKET) {
      if (clientAddr != NULL) {
         char addrBuffer[INET6_ADDRSTRLEN];
         const char *addr = clientAddress(&sockAddr, addrBuffer, sizeof(addrBuffer));
         if (addr != NULL) {
            cfl_str_setValue(clientAddr, addr);
         }
      }
      if (port != NULL) {
         *port = ntohs(((struct sockaddr_in*)&sockAddr)->sin_port);
      }
   }
   return CREATE_SOCKET(clientSocket);
}

CFL_SOCKET cfl_socket_acceptTimeout(CFL_SOCKET listenSocket, CFL_UINT32 timeoutMillis, CFL_STRP clientAddr, CFL_UINT16 *port, CFL_BOOL *timesUp) {
   OS_SOCKET clientSocket = OS_INVALID_SOCKET;
   struct sockaddr sockAddr;
   int len = sizeof(sockAddr);
   fd_set fdSet;
   struct timeval timewait;
   struct timeval *timeout;

   if (listenSocket == CFL_INVALID_SOCKET) {
      return CFL_INVALID_SOCKET;
   }

   FD_ZERO(&fdSet);
   FD_SET(GET_OS_SOCKET(listenSocket), &fdSet);

   if (timeoutMillis == CFL_WAIT_FOREVER) {
      timeout = NULL;
   } else {
      timewait.tv_sec = (time_t) (timeoutMillis / 1000);
      timewait.tv_usec = (timeoutMillis % 1000) * 1000;
      timeout = &timewait;
   }
   if (select(((int)GET_OS_SOCKET(listenSocket)) + 1, &fdSet, NULL, NULL, timeout) > 0) {
      *timesUp = CFL_FALSE;
      clientSocket = accept(GET_OS_SOCKET(listenSocket), &sockAddr, &len);
      if (clientSocket != OS_INVALID_SOCKET) {
         if (clientAddr != NULL) {
            char addrBuffer[INET6_ADDRSTRLEN];
            const char *addr = clientAddress(&sockAddr, addrBuffer, sizeof(addrBuffer));
            if (addr != NULL) {
               cfl_str_setValue(clientAddr, addr);
            }
         }
         if (port != NULL) {
            *port = ntohs(((struct sockaddr_in*)&sockAddr)->sin_port);
         }
      }
   } else {
      *timesUp = CFL_TRUE;
   }

   return CREATE_SOCKET(clientSocket);
}

CFL_SOCKET cfl_socket_open(const char *serverAddress, CFL_UINT16 port) {
#if defined(CFL_OS_LINUX)
   OS_SOCKET socketHandle;
   struct sockaddr_in serv_addr;
   struct hostent *he;
   struct in_addr **addr_list;

   socketHandle = socket(AF_INET, SOCK_STREAM, 0);
   if (socketHandle == OS_INVALID_SOCKET) {
      return CFL_INVALID_SOCKET;
   }

   memset(&serv_addr, '0', sizeof(serv_addr));

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_port = htons(port);

   if (inet_pton(AF_INET, serverAddress, &serv_addr.sin_addr) <= 0) {
      he = gethostbyname(serverAddress);
      if (he == NULL) {
         close(socketHandle);
         return CFL_INVALID_SOCKET;
      }
      addr_list = (struct in_addr **) he->h_addr_list;
      if (addr_list != NULL && addr_list[0] != NULL) {
         serv_addr.sin_addr = *addr_list[0];
      } else {
         close(socketHandle);
         return CFL_INVALID_SOCKET;
      }
   }

   if (connect(socketHandle, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
      close(socketHandle);
      return CFL_INVALID_SOCKET;
   }

   return CREATE_SOCKET(socketHandle);

   /* If not Borland C. */
#elif ! defined(__BORLANDC__)
   OS_SOCKET socketHandle = OS_INVALID_SOCKET;
   struct addrinfo *addr;
   struct addrinfo addrCriteria;
   struct addrinfo *serverAddr;
   char serverPort[7];
   WSADATA winsockinfo;

   if (WSAStartup(MAKEWORD(2, 2), &winsockinfo) != 0) {
      return CFL_INVALID_SOCKET;
   }

   memset(&addrCriteria, 0, sizeof(addrCriteria));
   addrCriteria.ai_family = AF_UNSPEC;
   addrCriteria.ai_socktype = SOCK_STREAM;
   addrCriteria.ai_protocol = IPPROTO_TCP;

   // Get address(es)
   sprintf(serverPort, "%u", port);
   if (getaddrinfo(serverAddress, serverPort, &addrCriteria, &serverAddr) != 0) {
      return CFL_INVALID_SOCKET;
   }

   for (addr = serverAddr; addr != NULL; addr = addr->ai_next) {
      // Create a reliable, stream socket using TCP
      socketHandle = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
      if (socketHandle == OS_INVALID_SOCKET) {
         continue;
      }

      // Establish the connection to server
      if (connect(socketHandle, addr->ai_addr, (int) addr->ai_addrlen) == 0) {
         break;
      }

      closesocket(socketHandle);
      socketHandle = OS_INVALID_SOCKET;
   }

   freeaddrinfo(serverAddr); // Free addrinfo allocated in getaddrinfo()
   return CREATE_SOCKET(socketHandle);
#else
   OS_SOCKET socketHandle;
   struct sockaddr_in addr;
   WSADATA winsockinfo;
   struct hostent *remoteHost;

   if (WSAStartup(MAKEWORD(2, 2), &winsockinfo) != 0) {
      return CFL_INVALID_SOCKET;
   }

   socketHandle = (OS_SOCKET) socket(AF_INET, SOCK_STREAM, 0);
   if (socketHandle == OS_INVALID_SOCKET) {
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

   if (connect(socketHandle, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) != 0) {
      closesocket(socketHandle);
      return CFL_INVALID_SOCKET;
   }
   return CREATE_SOCKET(socketHandle);
#endif
}

CFL_INT32 cfl_socket_close(CFL_SOCKET socket) {
   CFL_INT32 res;
   if (socket == CFL_INVALID_SOCKET) {
      return CFL_SOCKET_ERROR;
   }
#if defined(CFL_OS_LINUX)
   res = (CFL_INT32) close(GET_OS_SOCKET(socket));
#else
   res = (CFL_INT32) closesocket(GET_OS_SOCKET(socket));
#endif
   return res;
}

CFL_INT32 cfl_socket_sendBuffer(CFL_SOCKET socket, CFL_BUFFERP buffer) {
   CFL_INT32 bytesSent = send(GET_OS_SOCKET(socket), (const char *) cfl_buffer_getDataPtr(buffer), (int) cfl_buffer_remaining(buffer), 0);
   if (bytesSent > 0) {
      cfl_buffer_skip(buffer, bytesSent);
   }
   return bytesSent;
}

CFL_INT32 cfl_socket_send(CFL_SOCKET socket, const char *buffer, CFL_INT32 len) {
   return (CFL_INT32) send(GET_OS_SOCKET(socket), buffer, (int) len, 0);
}

CFL_BOOL cfl_socket_sendAllBuffer(CFL_SOCKET socket, CFL_BUFFERP buffer) {
   CFL_UINT32 totalSent = 0; // how many bytes we've sent
   CFL_UINT32 bytesLeft;
   CFL_INT32 bytesSent;
   const char *data;

   bytesLeft = cfl_buffer_remaining(buffer);
   data = (const char *) cfl_buffer_getDataPtr(buffer);
   while (bytesLeft > 0) {
      bytesSent = send(GET_OS_SOCKET(socket), data + totalSent, (int) bytesLeft, 0);
      if (bytesSent == OS_SOCKET_ERROR) {
         break;
      }
      totalSent += bytesSent;
      bytesLeft -= bytesSent;
   }
   cfl_buffer_skip(buffer, totalSent);
   return bytesLeft == 0;
}

CFL_BOOL cfl_socket_sendAll(CFL_SOCKET socket, const char *buffer, CFL_UINT32 len) {
   CFL_UINT32 totalSent = 0; // how many bytes we've sent
   CFL_UINT32 bytesLeft;
   CFL_INT32 bytesSent;

   bytesLeft = len;
   while (bytesLeft > 0) {
      bytesSent = send(GET_OS_SOCKET(socket), buffer + totalSent, (int) bytesLeft, 0);
      if (bytesSent == OS_SOCKET_ERROR) {
         break;
      }
      totalSent += bytesSent;
      bytesLeft -= bytesSent;
   }
   return bytesLeft == 0;
}

CFL_INT32 cfl_socket_receive(CFL_SOCKET socket, const char *buffer, int len) {
   return (CFL_INT32) recv(GET_OS_SOCKET(socket), (char *) buffer, len, 0);
}

CFL_INT32 cfl_socket_receiveAll(CFL_SOCKET socket, const char *buffer, int len) {
   int retVal;
   int toRead = len;

   while (toRead > 0) {
      retVal = recv(GET_OS_SOCKET(socket), (char *) buffer, toRead, 0);
      if (retVal < 0) {
         return retVal;
      } else if (retVal == 0) {
         return retVal;
      }
      buffer += retVal;
      toRead -= retVal;
   }
   return len;
}

CFL_BOOL cfl_socket_receiveAllBuffer(CFL_SOCKET socket, CFL_BUFFERP buffer, CFL_UINT32 packetLen, CFL_UINT32 timeoutMillis) {
   int retVal;
   CFL_UINT32 bodyLen;
   char *dataRead;
   int readLen;

   // wait until timeout or data received
   retVal = cfl_socket_selectRead(socket, timeoutMillis);
   if (retVal <= 0) {
      return CFL_FALSE;
   }

   dataRead = (char *) CFL_MEM_ALLOC(packetLen * sizeof(char));
   if (dataRead == NULL) {
      return CFL_FALSE;
   }
   // data must be here, so do a normal recv()
   bodyLen = cfl_buffer_remaining(buffer);
   while (bodyLen > 0) {
      readLen = bodyLen > packetLen ? packetLen : bodyLen;
      retVal = recv(GET_OS_SOCKET(socket), dataRead, readLen, 0);
      if (retVal == OS_SOCKET_ERROR) {
         CFL_MEM_FREE(dataRead);
         return CFL_FALSE;
      } else if (retVal == 0) {
         CFL_MEM_FREE(dataRead);
         return CFL_FALSE;
      }
      bodyLen -= retVal;
      cfl_buffer_put(buffer, dataRead, retVal);
   }
   CFL_MEM_FREE(dataRead);
   return CFL_TRUE;
}

CFL_INT32 cfl_socket_selectRead(CFL_SOCKET socket, CFL_UINT32 timeoutMillis) {
   fd_set fdSet;
   struct timeval timewait;
   struct timeval * timeout;

   FD_ZERO(&fdSet);
   FD_SET(GET_OS_SOCKET(socket), &fdSet);

   if (timeoutMillis == CFL_WAIT_FOREVER) {
      timeout = NULL;
   } else {
      timewait.tv_sec = (time_t) (timeoutMillis / 1000);
      timewait.tv_usec = (timeoutMillis % 1000) * 1000;
      timeout = &timewait;
   }

   return (CFL_INT32) select((int) GET_OS_SOCKET(socket) + 1, &fdSet, NULL, NULL, timeout);
}

CFL_INT32 cfl_socket_selectWrite(CFL_SOCKET socket, CFL_UINT32 timeoutMillis) {
   fd_set fdSet;
   struct timeval timewait;
   struct timeval * timeout;

   FD_ZERO(&fdSet);
   FD_SET(GET_OS_SOCKET(socket), &fdSet);

   if (timeoutMillis == CFL_WAIT_FOREVER) {
      timeout = NULL;
   } else {
      timewait.tv_sec = (time_t) (timeoutMillis / 1000);
      timewait.tv_usec = (timeoutMillis % 1000) * 1000;
      timeout = &timewait;
   }

   return (CFL_INT32) select((int) GET_OS_SOCKET(socket) + 1, NULL, &fdSet, NULL, timeout);
}

CFL_INT32 cfl_socket_select(CFL_SOCKET socket, CFL_UINT32 timeoutMillis) {
   fd_set fdSet;
   struct timeval timewait;
   struct timeval * timeout;

   FD_ZERO(&fdSet);
   FD_SET(GET_OS_SOCKET(socket), &fdSet);

   if (timeoutMillis == CFL_WAIT_FOREVER) {
      timeout = NULL;
   } else {
      timewait.tv_sec = (time_t) (timeoutMillis / 1000);
      timewait.tv_usec = (timeoutMillis % 1000) * 1000;
      timeout = &timewait;
   }

   return (CFL_INT32) select((int) GET_OS_SOCKET(socket) + 1, &fdSet, &fdSet, NULL, timeout);
}

CFL_INT32 cfl_socket_lastErrorCode(void) {
#if defined(CFL_OS_LINUX)
   return errno;
#else
   return WSAGetLastError();
#endif
}

char *cfl_socket_lastErrorDescription(char *buffer, size_t maxLen) {
#if defined(CFL_OS_LINUX)
   if (errno != 0) {
      snprintf(buffer, maxLen, "%s", strerror(errno));
   } else {
      memset(buffer, 0, maxLen);
   }
#else
   if (WSAGetLastError() != 0) {
      FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    WSAGetLastError(),
                    0,
                    buffer,
                    (DWORD) maxLen,
                    NULL);
   } else {
      memset(buffer, 0, maxLen);
   }
#endif
   return buffer;
}

CFL_BOOL cfl_socket_setBlockingMode(CFL_SOCKET socket, CFL_BOOL block) {
#if defined(CFL_OS_LINUX)
   const int flags = fcntl(GET_OS_SOCKET(socket), F_GETFL, 0);
   if ((flags & O_NONBLOCK) && !block) {
      return CFL_TRUE;
   }
   if (!(flags & O_NONBLOCK) && block) {
      return CFL_TRUE;
   }
   if (fcntl(GET_OS_SOCKET(socket), F_SETFL, block ? flags ^ O_NONBLOCK : flags | O_NONBLOCK) != 0) {
      return CFL_FALSE;
   }
#else
   u_long mode = block ? 1 : 0;
   if (ioctlsocket(GET_OS_SOCKET(socket), FIONBIO, &mode) != NO_ERROR) {
      return CFL_FALSE;
   }
#endif
   return CFL_TRUE;
}

CFL_BOOL cfl_socket_setNoDelay(CFL_SOCKET socket, CFL_BOOL delay) {
   if (setsockopt(GET_OS_SOCKET(socket), SOL_SOCKET, TCP_NODELAY, (char *) &delay, sizeof(CFL_BOOL)) != 0) {
      return CFL_FALSE;
   }
   return CFL_TRUE;
}

CFL_BOOL cfl_socket_setReceiveBufferSize(CFL_SOCKET socket, int size) {
   if (setsockopt(GET_OS_SOCKET(socket), SOL_SOCKET, SO_RCVBUF, (char *) &size, sizeof(int)) != 0) {
      return CFL_FALSE;
   }
   return CFL_TRUE;
}

CFL_BOOL cfl_socket_setSendBufferSize(CFL_SOCKET socket, int size) {
   if (setsockopt(GET_OS_SOCKET(socket), SOL_SOCKET, SO_SNDBUF, (char *) &size, sizeof(int)) != 0) {
      return CFL_FALSE;
   }
   return CFL_TRUE;
}

CFL_BOOL cfl_socket_setKeepAlive(CFL_SOCKET socket, CFL_BOOL active, CFL_UINT32 time, CFL_UINT32 interval) {
#if defined(CFL_OS_LINUX)

   if(setsockopt(GET_OS_SOCKET(socket), SOL_SOCKET, SO_KEEPALIVE, (char *) &active, sizeof(CFL_BOOL)) != 0) {
      return CFL_FALSE;
   }
   if (active) {
      int keepidle = (int) time;
      int keepintvl = (int) interval;
      if (setsockopt(GET_OS_SOCKET(socket), IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(int)) != 0) {
         return CFL_FALSE;
      }
      if (setsockopt(GET_OS_SOCKET(socket), IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(int)) != 0) {
         return CFL_FALSE;
      }
   }
   return CFL_TRUE;
#elif defined(CFL_OS_WINDOWS) && ! defined(__BORLANDC__)
   DWORD dwBytes = 0;
   struct tcp_keepalive kaSettings;
   kaSettings.onoff = active ? 1L : 0L;
   kaSettings.keepalivetime = (ULONG) time;
   kaSettings.keepaliveinterval = (ULONG) interval;
   if (WSAIoctl(GET_OS_SOCKET(socket),
                SIO_KEEPALIVE_VALS,
                &kaSettings,
                sizeof(kaSettings),
                NULL,
                0,
                &dwBytes,
                NULL,
                NULL) != 0) {
      return CFL_FALSE;
   }
   return CFL_TRUE;
#else
   CFL_UNUSED(socket);
   CFL_UNUSED(active);
   CFL_UNUSED(time);
   CFL_UNUSED(interval);
   return CFL_FALSE;
#endif
}

CFL_BOOL cfl_socket_setLinger(CFL_SOCKET socket, CFL_BOOL active, CFL_UINT16 lingerSeconds) {
   struct linger sl;
   sl.l_onoff = active ? 1 : 0;
   sl.l_linger = lingerSeconds;
#if defined(CFL_OS_WINDOWS)
   if (setsockopt(GET_OS_SOCKET(socket), SOL_SOCKET, SO_LINGER, (char *) &sl, sizeof(sl)) != 0) {
      return CFL_FALSE;
   }
#else
   if (setsockopt(GET_OS_SOCKET(socket), SOL_SOCKET, SO_LINGER, &sl, sizeof(sl)) != 0) {
      return CFL_FALSE;
   }
#endif
   return CFL_TRUE;
}

CFL_BOOL cfl_socket_shutdown(CFL_SOCKET socket, CFL_BOOL read, CFL_BOOL write) {
   int how;
   if (read) {
      how = write ? SHUTDOWN_READ_WRITE : SHUTDOWN_READ;
   } else if (write) {
      how = SHUTDOWN_WRITE;
   } else {
      how = 0;
   } 
   if (shutdown(GET_OS_SOCKET(socket), how) == 0) { 
      return CFL_TRUE;
   }
   return CFL_FALSE;
}

char *cfl_socket_hostname(char *hostname, CFL_UINT32 hostnameLen) {
   if (hostname != NULL) {
#if defined(CFL_OS_WINDOWS)
   WSADATA wsadata;
   WSAStartup(MAKEWORD(1, 1), &wsadata);
#endif
   if (gethostname(hostname, hostnameLen) != 0) {
      strncpy(hostname, "unknown", hostnameLen);
   }
#if defined(CFL_OS_WINDOWS)
   WSACleanup();
#endif
   }
   return hostname;
}

char *cfl_socket_hostAddress(char *hostAddress, CFL_UINT32 hostAddressLen) {
   char *strAddr;
   char host[256];
   struct hostent *host_entry;

   if (gethostname(host, sizeof(host)) != 0) {
      strncpy(hostAddress, "0.0.0.0", hostAddressLen);
      return hostAddress;
   }
   host_entry = gethostbyname(host);
   if (host_entry == NULL) {
      strncpy(hostAddress, "0.0.0.0", hostAddressLen);
      return hostAddress;
   }
   strAddr = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
   if (strAddr != NULL) {
      strncpy(hostAddress, strAddr, hostAddressLen);
   } else {
      strncpy(hostAddress, "0.0.0.0", hostAddressLen);
   }
   return hostAddress;
}