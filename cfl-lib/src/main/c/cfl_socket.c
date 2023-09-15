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
#include "cfl_str.h"

#if defined(CFL_OS_LINUX)
   #include <string.h>
   #include <arpa/inet.h>
   #include <fcntl.h>
   #include <netinet/tcp.h>
   #include <signal.h>
#elif defined(CFL_OS_WINDOWS) && ! defined(__BORLANDC__)
   #include <mstcpip.h>
#endif

#if defined(__BORLANDC__)
   #define INET6_ADDRSTRLEN 65
#endif

#if defined(CFL_OS_LINUX)
   #define SHUTDOWN_READ       SHUT_RD
   #define SHUTDOWN_WRITE      SHUT_WR
   #define SHUTDOWN_READ_WRITE SHUT_RDWR
#else
   #define SHUTDOWN_READ       SD_RECEIVE
   #define SHUTDOWN_WRITE      SD_SEND
   #define SHUTDOWN_READ_WRITE SD_BOTH
#endif

#define STR_EMPTY(s) ((s) == NULL || (s)[0] == '\0')

#define MAX_ERROR_MSG_SIZE 256

static CFL_INT32 s_lastErrorCode = 0;
static char s_lastErrorMsg[MAX_ERROR_MSG_SIZE] = {0};

static void setLastError(void) {
#if defined(CFL_OS_LINUX)
   s_lastErrorCode = errno;
   if (s_lastErrorCode != 0) {
      snprintf(s_lastErrorMsg, MAX_ERROR_MSG_SIZE, "%s", strerror(s_lastErrorCode));
   } else {
      s_lastErrorMsg[0] = '\0';
   }
#else
   s_lastErrorCode = WSAGetLastError();
   if (s_lastErrorCode != 0) {
      FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    (DWORD) s_lastErrorCode,
                    0,
                    s_lastErrorMsg,
                    MAX_ERROR_MSG_SIZE,
                    NULL);
   } else {
      s_lastErrorMsg[0] = '\0';
   }
#endif
}

CFL_SOCKET cfl_socket_listen(const char *address, CFL_UINT16 port, CFL_INT32 backlog) {
#if defined(CFL_OS_LINUX)
   CFL_SOCKET socketHandle;
   struct sockaddr_in serv_addr;
   struct hostent *he;
   struct in_addr **addr_list;

   socketHandle = socket(AF_INET, SOCK_STREAM, 0);

   if (socketHandle == CFL_INVALID_SOCKET) {
      setLastError();
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
         setLastError();
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
      setLastError();
      close(socketHandle);
      return CFL_INVALID_SOCKET;
   }

   if (listen(socketHandle, (int)backlog) != 0) {
      setLastError();
      close(socketHandle);
      return CFL_INVALID_SOCKET;
   }

   return socketHandle;

   /* If not Borland C. */
#elif ! defined(__BORLANDC__)
   CFL_SOCKET socketHandle = CFL_INVALID_SOCKET;
   struct addrinfo *addr;
   struct addrinfo addrCriteria;
   struct addrinfo *serverAddr;
   char serverPort[7];
   WSADATA winsockinfo;

   if (WSAStartup(MAKEWORD(2, 2), &winsockinfo) != 0) {
      setLastError();
      return CFL_INVALID_SOCKET;
   }

   memset(&addrCriteria, 0, sizeof(addrCriteria));
   addrCriteria.ai_family = STR_EMPTY(address) ? AF_INET : AF_UNSPEC;
   addrCriteria.ai_socktype = SOCK_STREAM;
   addrCriteria.ai_protocol = IPPROTO_TCP;

   // Get address(es)
   sprintf(serverPort, "%u", port);
   if (getaddrinfo(address, serverPort, &addrCriteria, &serverAddr) != 0) {
      setLastError();
      return CFL_INVALID_SOCKET;
   }

   for (addr = serverAddr; addr != NULL; addr = addr->ai_next) {
      // Create a reliable, stream socket using TCP
      socketHandle = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
      if (socketHandle == CFL_INVALID_SOCKET) {
         setLastError();
         continue;
      }
   
      // Establish the connection to server
      if (bind(socketHandle, addr->ai_addr, (int) addr->ai_addrlen) == 0 && listen(socketHandle, (int)backlog) == 0) {
         break;
      }

      setLastError();
      closesocket(socketHandle);
      socketHandle = CFL_INVALID_SOCKET;
   }

   freeaddrinfo(serverAddr); // Free addrinfo allocated in getaddrinfo()
   return socketHandle;
#else
   CFL_SOCKET socketHandle;
   struct sockaddr_in addr;
   WSADATA winsockinfo;
   struct hostent *remoteHost;

   if (WSAStartup(MAKEWORD(2, 2), &winsockinfo) != 0) {
      setLastError();
      return CFL_INVALID_SOCKET;
   }

   socketHandle = (CFL_SOCKET) socket(AF_INET, SOCK_STREAM, 0);
   if (socketHandle == CFL_INVALID_SOCKET) {
      setLastError();
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
            setLastError();
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
      setLastError();
      closesocket(socketHandle);
      return CFL_INVALID_SOCKET;
   }

   if (listen(socketHandle, (int)backlog) != 0) {
      setLastError();
      close(socketHandle);
      return CFL_INVALID_SOCKET;
   }

   return socketHandle;
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
   CFL_SOCKET clientSocket;
   struct sockaddr sockAddr;
   int len = sizeof(sockAddr);

   clientSocket = accept(listenSocket, &sockAddr, &len);
   if (clientSocket != CFL_INVALID_SOCKET) {
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
   } else {
      setLastError();
   }
   return clientSocket;
}

CFL_SOCKET cfl_socket_acceptTimeout(CFL_SOCKET listenSocket, CFL_UINT32 timeoutMillis, CFL_STRP clientAddr, CFL_UINT16 *port, CFL_BOOL *timesUp) {
   CFL_SOCKET clientSocket = CFL_INVALID_SOCKET;
   struct sockaddr sockAddr;
   int len = sizeof(sockAddr);
   fd_set fdSet;
   struct timeval timewait;
   struct timeval *timeout;

   FD_ZERO(&fdSet);
   FD_SET(listenSocket, &fdSet);

   if (timeoutMillis == CFL_WAIT_FOREVER) {
      timeout = NULL;
   } else {
      timewait.tv_sec = (time_t) (timeoutMillis / 1000);
      timewait.tv_usec = (timeoutMillis % 1000) * 1000;
      timeout = &timewait;
   }
   if (listenSocket != CFL_INVALID_SOCKET && select(listenSocket + 1, &fdSet, NULL, NULL, timeout) > 0) {
      *timesUp = CFL_FALSE;
      clientSocket = accept(listenSocket, &sockAddr, &len);
      if (clientSocket != CFL_INVALID_SOCKET) {
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
      } else {
         setLastError();
      }
   } else {
      *timesUp = CFL_TRUE;
   }

   return clientSocket;
}

CFL_SOCKET cfl_socket_open(const char *serverAddress, CFL_UINT16 port) {
#if defined(CFL_OS_LINUX)
   CFL_SOCKET socketHandle;
   struct sockaddr_in serv_addr;
   struct hostent *he;
   struct in_addr **addr_list;

   socketHandle = socket(AF_INET, SOCK_STREAM, 0);
   if (socketHandle == CFL_INVALID_SOCKET) {
      setLastError();
      return CFL_INVALID_SOCKET;
   }

   memset(&serv_addr, '0', sizeof(serv_addr));

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_port = htons(port);

   if (inet_pton(AF_INET, serverAddress, &serv_addr.sin_addr) <= 0) {
      he = gethostbyname(serverAddress);
      if (he == NULL) {
         setLastError();
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
      setLastError();
      close(socketHandle);
      return CFL_INVALID_SOCKET;
   }

   return socketHandle;

   /* If not Borland C. */
#elif ! defined(__BORLANDC__)
   CFL_SOCKET socketHandle = CFL_INVALID_SOCKET;
   struct addrinfo *addr;
   struct addrinfo addrCriteria;
   struct addrinfo *serverAddr;
   char serverPort[7];
   WSADATA winsockinfo;

   if (WSAStartup(MAKEWORD(2, 2), &winsockinfo) != 0) {
      setLastError();
      return CFL_INVALID_SOCKET;
   }

   memset(&addrCriteria, 0, sizeof(addrCriteria));
   addrCriteria.ai_family = AF_UNSPEC;
   addrCriteria.ai_socktype = SOCK_STREAM;
   addrCriteria.ai_protocol = IPPROTO_TCP;

   // Get address(es)
   sprintf(serverPort, "%u", port);
   if (getaddrinfo(serverAddress, serverPort, &addrCriteria, &serverAddr) != 0) {
      setLastError();
      return CFL_INVALID_SOCKET;
   }

   for (addr = serverAddr; addr != NULL; addr = addr->ai_next) {
      // Create a reliable, stream socket using TCP
      socketHandle = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
      if (socketHandle == CFL_INVALID_SOCKET) {
         setLastError();
         continue;
      }

      // Establish the connection to server
      if (connect(socketHandle, addr->ai_addr, (int) addr->ai_addrlen) == 0) {
         break;
      }

      setLastError();
      closesocket(socketHandle);
      socketHandle = CFL_INVALID_SOCKET;
   }

   freeaddrinfo(serverAddr); // Free addrinfo allocated in getaddrinfo()
   return socketHandle;
#else
   CFL_SOCKET socketHandle;
   struct sockaddr_in addr;
   WSADATA winsockinfo;
   struct hostent *remoteHost;

   if (WSAStartup(MAKEWORD(2, 2), &winsockinfo) != 0) {
      setLastError();
      return CFL_INVALID_SOCKET;
   }

   socketHandle = (CFL_SOCKET) socket(AF_INET, SOCK_STREAM, 0);
   if (socketHandle == CFL_INVALID_SOCKET) {
      setLastError();
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
         setLastError();
         return CFL_INVALID_SOCKET;
      }
      if (remoteHost->h_addr_list[0] != 0) {
         addr.sin_addr.s_addr = *(u_long *) remoteHost->h_addr_list[0];
      } else {
         return CFL_INVALID_SOCKET;
      }
   }

   if (connect(socketHandle, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) != 0) {
      setLastError();
      closesocket(socketHandle);
      return CFL_INVALID_SOCKET;
   }
   return socketHandle;
#endif
}

CFL_INT32 cfl_socket_close(CFL_SOCKET socket) {
#if defined(CFL_OS_LINUX)
   return close(socket);
#else
   CFL_INT32 rc;
   rc = closesocket(socket);
   setLastError();
//   WSACleanup();
   return rc;
#endif
}

CFL_INT32 cfl_socket_sendBuffer(CFL_SOCKET socket, CFL_BUFFERP buffer) {
   CFL_INT32 bytesSent = send(socket, (const char *) cfl_buffer_getDataPtr(buffer), (int) cfl_buffer_remaining(buffer), 0);
   if (bytesSent > 0) {
      cfl_buffer_skip(buffer, bytesSent);
   } else if (bytesSent < 0) {
      setLastError();
   }
   return bytesSent;
}

CFL_INT32 cfl_socket_send(CFL_SOCKET socket, const char *buffer, CFL_INT32 len) {
   CFL_INT32 bytesSent = send(socket, buffer, (int) len, 0);
   if (bytesSent < 0) {
      setLastError();
   }
   return bytesSent;
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
         setLastError();
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
      bytesSent = send(socket, buffer + totalSent, (int) bytesLeft, 0);
      if (bytesSent < 0) {
         setLastError();
         break;
      }
      totalSent += bytesSent;
      bytesLeft -= bytesSent;
   }
   return bytesLeft == 0;
}

CFL_INT32 cfl_socket_receive(CFL_SOCKET socket, const char *buffer, int len) {
   CFL_INT32 read = recv(socket, (char *) buffer, len, 0);
   if (read < 0) {
      setLastError();
   }
   return read;
}

CFL_INT32 cfl_socket_receiveAll(CFL_SOCKET socket, const char *buffer, int len) {
   int retVal;
   int toRead = len;

   while (toRead > 0) {
      retVal = recv(socket, (char *) buffer, toRead, 0);
      if (retVal < 0) {
         setLastError();
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
         setLastError();
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

CFL_INT32 cfl_socket_selectRead(CFL_SOCKET socket, CFL_UINT32 timeoutMillis) {
   fd_set fdSet;
   struct timeval timewait;
   struct timeval * timeout;
   CFL_INT32 res;

   FD_ZERO(&fdSet);
   FD_SET(socket, &fdSet);

   if (timeoutMillis == CFL_WAIT_FOREVER) {
      timeout = NULL;
   } else {
      timewait.tv_sec = (time_t) (timeoutMillis / 1000);
      timewait.tv_usec = (timeoutMillis % 1000) * 1000;
      timeout = &timewait;
   }

   res = select((int) socket + 1, &fdSet, NULL, NULL, timeout);
   if (res == CFL_SOCKET_ERROR) {
      setLastError();
   }
   return res;
}

CFL_INT32 cfl_socket_selectWrite(CFL_SOCKET socket, CFL_UINT32 timeoutMillis) {
   fd_set fdSet;
   struct timeval timewait;
   struct timeval * timeout;
   CFL_INT32 res;

   FD_ZERO(&fdSet);
   FD_SET(socket, &fdSet);

   if (timeoutMillis == CFL_WAIT_FOREVER) {
      timeout = NULL;
   } else {
      timewait.tv_sec = (time_t) (timeoutMillis / 1000);
      timewait.tv_usec = (timeoutMillis % 1000) * 1000;
      timeout = &timewait;
   }

   res = select((int) socket + 1, NULL, &fdSet, NULL, timeout);
   if (res == CFL_SOCKET_ERROR) {
      setLastError();
   }
   return res;
}

CFL_INT32 cfl_socket_select(CFL_SOCKET socket, CFL_UINT32 timeoutMillis) {
   fd_set fdSet;
   struct timeval timewait;
   struct timeval * timeout;
   CFL_INT32 res;

   FD_ZERO(&fdSet);
   FD_SET(socket, &fdSet);

   if (timeoutMillis == CFL_WAIT_FOREVER) {
      timeout = NULL;
   } else {
      timewait.tv_sec = (time_t) (timeoutMillis / 1000);
      timewait.tv_usec = (timeoutMillis % 1000) * 1000;
      timeout = &timewait;
   }

   res = select((int) socket + 1, &fdSet, &fdSet, NULL, timeout);
   if (res == CFL_SOCKET_ERROR) {
      setLastError();
   }
   return res;
}

CFL_INT32 cfl_socket_lastErrorCode(void) {
   return s_lastErrorCode;
}

char * cfl_socket_lastErrorDescription(void) {
   return s_lastErrorMsg;
}

CFL_BOOL cfl_socket_setBlockingMode(CFL_SOCKET socket, CFL_BOOL block) {
   CFL_BOOL ret;
#if defined(CFL_OS_LINUX)
   const int flags = fcntl(socket, F_GETFL, 0);
   if ((flags & O_NONBLOCK) && !block) {
      return CFL_TRUE;
   }
   if (!(flags & O_NONBLOCK) && block) {
      return CFL_TRUE;
   }
   if (fcntl(socket, F_SETFL, block ? flags ^ O_NONBLOCK : flags | O_NONBLOCK) != 0) {
      setLastError();
      return CFL_FALSE;
   }
#else
   u_long mode = block ? 1 : 0;
   if (ioctlsocket(socket, FIONBIO, &mode) != NO_ERROR) {
      setLastError();
      return CFL_FALSE;
   }
#endif
   return CFL_TRUE;
}

CFL_BOOL cfl_socket_setNoDelay(CFL_SOCKET socket, CFL_BOOL delay) {
   if (setsockopt(socket, SOL_SOCKET, TCP_NODELAY, (char *) &delay, sizeof(CFL_BOOL)) != 0) {
         setLastError();
      return CFL_FALSE;
   }
   return CFL_TRUE;
}

CFL_BOOL cfl_socket_setReceiveBufferSize(CFL_SOCKET socket, int size) {
   if (setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char *) &size, sizeof(int)) != 0) {
      setLastError();
      return CFL_FALSE;
   }
   return CFL_TRUE;
}

CFL_BOOL cfl_socket_setSendBufferSize(CFL_SOCKET socket, int size) {
   if (setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (char *) &size, sizeof(int)) != 0) {
      setLastError();
      return CFL_FALSE;
   }
   return CFL_TRUE;
}

CFL_BOOL cfl_socket_setKeepAlive(CFL_SOCKET socket, CFL_BOOL active, CFL_UINT32 time, CFL_UINT32 interval) {
#if defined(CFL_OS_LINUX)

   if(setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (char *) &active, sizeof(CFL_BOOL)) != 0) {
         setLastError();
      return CFL_FALSE;
   }
   if (active) {
      int keepidle = (int) time;
      int keepintvl = (int) interval;
      if (setsockopt(socket, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(int)) != 0) {
         setLastError();
         return CFL_FALSE;
      }
      if (setsockopt(socket, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(int)) != 0) {
         setLastError();
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
   if (WSAIoctl(socket,
                   SIO_KEEPALIVE_VALS,
                   &kaSettings,
                   sizeof(kaSettings),
                   NULL,
                   0,
                   &dwBytes,
                   NULL,
                   NULL) != 0) {
      setLastError();
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
   if (setsockopt(socket, SOL_SOCKET, SO_LINGER, (char *) &sl, sizeof(sl)) != 0) {
      setLastError();
      return CFL_FALSE;
   }
#else
   if (setsockopt(socket, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl)) != 0) {
      setLastError();
      return CFL_FALSE;
   }
#endif
   return CFL_TRUE;
}

CFL_BOOL cfl_socket_shutdown(CFL_SOCKET socket, CFL_BOOL read, CFL_BOOL write) {
   int how;
   if (read || write) {
      if (read) {
         how = write ? SHUTDOWN_READ_WRITE : SHUTDOWN_READ;
      } else {
         how = SHUTDOWN_WRITE;
      }
      if (shutdown(socket, how) == 0) { 
         return CFL_TRUE;
      } else {
         setLastError();
      }
   } 
   return CFL_FALSE;
}
