/**
 * @file cfl_socket.h
 * @brief Network socket abstraction layer.
 *
 * This module provides functions for network communication using sockets,
 * abstracting platform-specific differences between Windows (Winsock) and
 * POSIX sockets.
 */

#ifndef _CFL_SOCKET_H_

#define _CFL_SOCKET_H_

#include "cfl_buffer.h"
#include "cfl_str.h"
#include "cfl_types.h"

/** @brief Value indicating an invalid socket */
#define CFL_INVALID_SOCKET 0xFFFFFFFFFFFFFFFF
/** @brief Error return code for socket functions */
#define CFL_SOCKET_ERROR -1

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Type definition for a socket handle */
typedef CFL_UINT64 CFL_SOCKET;

/**
 * @brief Creates a listening socket (server).
 * @param address Address to bind to (e.g., "0.0.0.0" or NULL for any).
 * @param port Port number to listen on.
 * @param backlog Maximum length of the queue of pending connections.
 * @return The listening socket handle, or CFL_INVALID_SOCKET on error.
 */
extern CFL_SOCKET cfl_socket_listen(const char *address, CFL_UINT16 port,
                                    CFL_INT32 backlog);

/**
 * @brief Accepts a new incoming connection.
 * @param listenSocket The listening socket.
 * @param clientAddr Output string to store client IP address.
 * @param port Output pointer to store client port.
 * @return The new client socket handle, or CFL_INVALID_SOCKET on error.
 */
extern CFL_SOCKET cfl_socket_accept(CFL_SOCKET listenSocket,
                                    CFL_STRP clientAddr, CFL_UINT16 *port);

/**
 * @brief Accepts a connection with a timeout.
 * @param listenSocket The listening socket.
 * @param timeoutMillis Timeout in milliseconds.
 * @param clientAddr Output string to store client IP address.
 * @param port Output pointer to store client port.
 * @param timesUp Output pointer set to CFL_TRUE if timeout occurred.
 * @return The new client socket handle, or CFL_INVALID_SOCKET on error/timeout.
 */
extern CFL_SOCKET cfl_socket_acceptTimeout(CFL_SOCKET listenSocket,
                                           CFL_UINT32 timeoutMillis,
                                           CFL_STRP clientAddr,
                                           CFL_UINT16 *port, CFL_BOOL *timesUp);

/**
 * @brief Opens a connection to a remote server (client).
 * @param serverAddress IP address or hostname of the server.
 * @param port Port number to connect to.
 * @return The connected socket handle, or CFL_INVALID_SOCKET on error.
 */
extern CFL_SOCKET cfl_socket_open(const char *serverAddress, CFL_UINT16 port);

/**
 * @brief Closes a socket.
 * @param socket The socket handle to close.
 * @return 0 on success, or error code.
 */
extern CFL_INT32 cfl_socket_close(CFL_SOCKET socket);

/**
 * @brief Sends data from a buffer.
 * @param socket The connected socket.
 * @param buffer Buffer containing data to send.
 * @return Number of bytes sent, or CFL_SOCKET_ERROR on failure.
 */
extern CFL_INT32 cfl_socket_sendBuffer(CFL_SOCKET socket, CFL_BUFFERP buffer);

/**
 * @brief Sends raw data.
 * @param socket The connected socket.
 * @param buffer Pointer to data.
 * @param len Length of data to send.
 * @return Number of bytes sent, or CFL_SOCKET_ERROR on failure.
 */
extern CFL_INT32 cfl_socket_send(CFL_SOCKET socket, const char *buffer,
                                 CFL_INT32 len);

/**
 * @brief Helper to send all data in a buffer (looping if necessary).
 * @param socket The connected socket.
 * @param buffer Buffer containing data.
 * @return CFL_TRUE if all data sent, CFL_FALSE on error.
 */
extern CFL_BOOL cfl_socket_sendAllBuffer(CFL_SOCKET socket, CFL_BUFFERP buffer);

/**
 * @brief Helper to send all raw data (looping if necessary).
 * @param socket The connected socket.
 * @param buffer Pointer to data.
 * @param len Total length to send.
 * @return CFL_TRUE if all data sent, CFL_FALSE on error.
 */
extern CFL_BOOL cfl_socket_sendAll(CFL_SOCKET socket, const char *buffer,
                                   CFL_UINT32 len);

/**
 * @brief Receives data into a raw buffer.
 * @param socket The connected socket.
 * @param buffer Buffer to store received data.
 * @param len Maximum number of bytes to receive.
 * @return Number of bytes received, 0 on disconnect, or CFL_SOCKET_ERROR.
 */
extern CFL_INT32 cfl_socket_receive(CFL_SOCKET socket, const char *buffer,
                                    int len);

/**
 * @brief Receives exactly 'len' bytes (looping if necessary).
 * @param socket The connected socket.
 * @param buffer Buffer to store received data.
 * @param len Exact number of bytes to receive.
 * @return Number of bytes received (should match len), or error/disconnect.
 */
extern CFL_INT32 cfl_socket_receiveAll(CFL_SOCKET socket, const char *buffer,
                                       int len);

/**
 * @brief Receives exact amount of data into a CFL buffer with timeout.
 * @param socket The connected socket.
 * @param buffer CFL buffer to append received data to.
 * @param packetLen Number of bytes to receive.
 * @param timeoutMillis Timeout for operation in milliseconds.
 * @return CFL_TRUE if successfully received packetLen bytes, CFL_FALSE on
 * error/timeout.
 */
extern CFL_BOOL cfl_socket_receiveAllBuffer(CFL_SOCKET socket,
                                            CFL_BUFFERP buffer,
                                            CFL_UINT32 packetLen,
                                            CFL_UINT32 timeoutMillis);

/**
 * @brief Waits until socket is readable.
 * @param socket The socket.
 * @param timeoutMillis Timeout in milliseconds.
 * @return 1 if readable, 0 on timeout, -1 on error.
 */
extern CFL_INT32 cfl_socket_selectRead(CFL_SOCKET socket,
                                       CFL_UINT32 timeoutMillis);

/**
 * @brief Waits until socket is writable.
 * @param socket The socket.
 * @param timeoutMillis Timeout in milliseconds.
 * @return 1 if writable, 0 on timeout, -1 on error.
 */
extern CFL_INT32 cfl_socket_selectWrite(CFL_SOCKET socket,
                                        CFL_UINT32 timeoutMillis);

/**
 * @brief Waits for socket activity (read or write).
 * @param socket The socket.
 * @param timeoutMillis Timeout in milliseconds.
 * @return >0 if active, 0 on timeout, -1 on error.
 */
extern CFL_INT32 cfl_socket_select(CFL_SOCKET socket, CFL_UINT32 timeoutMillis);

/**
 * @brief Gets the last system socket error code.
 * @return System specific error code.
 */
extern CFL_INT32 cfl_socket_lastErrorCode(void);

/**
 * @brief Gets a text description of the last error.
 * @param buffer Buffer to store description.
 * @param maxLen Size of buffer.
 * @return Pointer to buffer.
 */
extern char *cfl_socket_lastErrorDescription(char *buffer, size_t maxLen);

/**
 * @brief Sets socket blocking or non-blocking mode.
 * @param socket The socket.
 * @param block CFL_TRUE for blocking, CFL_FALSE for non-blocking.
 * @return CFL_TRUE on success.
 */
extern CFL_BOOL cfl_socket_setBlockingMode(CFL_SOCKET socket, CFL_BOOL block);

/**
 * @brief Sets TCP_NODELAY option (disable Nagle's algorithm).
 * @param socket The socket.
 * @param delay CFL_FALSE to disable delay (enable no-delay).
 * @return CFL_TRUE on success.
 */
extern CFL_BOOL cfl_socket_setNoDelay(CFL_SOCKET socket, CFL_BOOL delay);

/**
 * @brief Sets the receive buffer size (SO_RCVBUF).
 * @param socket The socket.
 * @param size Size in bytes.
 * @return CFL_TRUE on success.
 */
extern CFL_BOOL cfl_socket_setReceiveBufferSize(CFL_SOCKET socket, int size);

/**
 * @brief Sets the send buffer size (SO_SNDBUF).
 * @param socket The socket.
 * @param size Size in bytes.
 * @return CFL_TRUE on success.
 */
extern CFL_BOOL cfl_socket_setSendBufferSize(CFL_SOCKET socket, int size);

/**
 * @brief Configures TCP keep-alive option.
 * @param socket The socket.
 * @param active Enable/Disable keep-alive.
 * @param time Keep-alive time.
 * @param interval Keep-alive interval.
 * @return CFL_TRUE on success.
 */
extern CFL_BOOL cfl_socket_setKeepAlive(CFL_SOCKET socket, CFL_BOOL active,
                                        CFL_UINT32 time, CFL_UINT32 interval);

/**
 * @brief Configures SO_LINGER option.
 * @param socket The socket.
 * @param active Enable linger.
 * @param lingerSeconds Linger time in seconds.
 * @return CFL_TRUE on success.
 */
extern CFL_BOOL cfl_socket_setLinger(CFL_SOCKET socket, CFL_BOOL active,
                                     CFL_UINT16 lingerSeconds);

/**
 * @brief Shuts down part of or the full connection.
 * @param socket The socket.
 * @param read Shutdown reading.
 * @param write Shutdown writing.
 * @return CFL_TRUE on success.
 */
extern CFL_BOOL cfl_socket_shutdown(CFL_SOCKET socket, CFL_BOOL read,
                                    CFL_BOOL write);

/**
 * @brief Gets the local hostname.
 * @param hostname Buffer to store hostname.
 * @param hostnameLen Size of buffer.
 * @return Pointer to hostname buffer.
 */
extern char *cfl_socket_hostname(char *hostname, CFL_UINT32 hostnameLen);

/**
 * @brief Gets the local host IP address.
 * @param hostAddress Buffer to store address string.
 * @param hostAddressLen Size of buffer.
 * @return Pointer to hostAddress buffer.
 */
extern char *cfl_socket_hostAddress(char *hostAddress,
                                    CFL_UINT32 hostAddressLen);

#ifdef __cplusplus
}
#endif

#endif
