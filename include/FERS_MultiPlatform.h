/******************************************************************************
 *
 * CAEN SpA - Front End Division
 * Via Vetraia, 11 - 55049 - Viareggio ITALY
 * +390594388398 - www.caen.it
 *
 ***************************************************************************/
/**
 * \note TERMS OF USE:
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation. This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the
 * software, documentation and results solely at his own risk.
 ******************************************************************************/

#ifndef FERS_MULTIPLATFORM_H
#define FERS_MULTIPLATFORM_H

/* =========================================================================
 * Standard headers - always included, platform-independent
 * ========================================================================= */
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

/* =========================================================================
 * Platform detection
 *   FERS_OS_WINDOWS  - any Windows (MSVC, MinGW, Cygwin with WIN32)
 *   FERS_OS_MACOS    - Apple macOS / Darwin
 *   FERS_OS_LINUX    - Linux
 *   FERS_OS_POSIX    - macOS or Linux (anything with pthreads / BSD sockets)
 * ========================================================================= */
#if defined(_WIN32) || defined(_WIN64)
#define FERS_OS_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
#define FERS_OS_MACOS 1
#define FERS_OS_POSIX 1
#elif defined(__linux__)
#define FERS_OS_LINUX 1
#define FERS_OS_POSIX 1
#else
#error "Unsupported platform"
#endif

/* =========================================================================
 * Platform-specific system headers
 * ========================================================================= */
#ifdef FERS_OS_WINDOWS
#define _WINSOCKAPI_ /* prevent winsock.h being pulled by windows.h */
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <process.h> /* _beginthreadex */
#include <direct.h>	 /* _mkdir */
#include <io.h>		 /* _access */
#include <time.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#endif

#ifdef FERS_OS_POSIX
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#endif

#ifdef FERS_OS_LINUX
#include <endian.h>
#endif

/* =========================================================================
 * Portable infinite timeout constant
 *   - On Windows INFINITE is already defined as DWORD 0xFFFFFFFF
 *   - We define our own signed version for use in f_sem_wait()
 * ========================================================================= */
#define FERS_WAIT_INFINITE INT32_C(-1)

/* =========================================================================
 * Socket abstraction
 * ========================================================================= */

/* Shutdown flags - same numeric value on all platforms */
#define FERS_SHUT_SEND 1 /* SD_SEND  / SHUT_WR  */
#define FERS_SHUT_BOTH 2 /* SD_BOTH  / SHUT_RDWR */

#ifdef FERS_OS_WINDOWS
typedef SOCKET f_socket_t;
typedef int ssize_t; /* send()/recv() return int on Windows */

#define f_socket_errno WSAGetLastError()
#define f_socket_h_errno WSAGetLastError()
#define f_socket_invalid INVALID_SOCKET
#define f_socket_error SOCKET_ERROR
#define f_socket_close(s) closesocket(s)
#define f_socket_cleanup() WSACleanup()

#define j_strdup _strdup
#define f_access(p, m) _access((p), (m))
#else /* FERS_OS_POSIX */
typedef int f_socket_t;

#define f_socket_errno errno
#define f_socket_h_errno h_errno
#define f_socket_invalid (-1)
#define f_socket_error (-1)
#define f_socket_close(s) close(s)
#define f_socket_cleanup() /* nothing */

#define j_strdup strdup
#define f_access(p, m) access((p), (m))
#endif

/* =========================================================================
 * Thread abstraction
 * ========================================================================= */
#ifdef FERS_OS_WINDOWS
typedef HANDLE f_thread_t;
typedef HANDLE f_sem_t;
typedef HANDLE f_mutex_t;

/* Mutex */
#define f_mutex_init(m) (((m) = CreateMutexA(NULL, FALSE, NULL)) == NULL \
							 ? (int)GetLastError()                       \
							 : 0)
#define f_mutex_destroy(m) (CloseHandle(m) ? 0 : (int)GetLastError())
#define f_mutex_lock(m) (WaitForSingleObject((m), INFINITE) == WAIT_FAILED \
							 ? (int)GetLastError()                         \
							 : 0)
#define f_mutex_unlock(m) (ReleaseMutex(m) ? 0 : (int)GetLastError())
#define f_mutex_trylock(m) ((int)WaitForSingleObject((m), 0))

/* Thread */
#define f_thread_create(f, p, id)                                                                   \
	((*(id) = (HANDLE)_beginthreadex(NULL, 0,                                                       \
									 (unsigned int(__stdcall *)(void *))(f), (p), 0, NULL)) == NULL \
		 ? (int)GetLastError()                                                                      \
		 : 0)
#define f_thread_join(id) ((int)WaitForSingleObject((id), INFINITE))

/* Sleep */
#define f_sleep_ms(ms) Sleep((DWORD)(ms))

#else /* FERS_OS_POSIX */
typedef pthread_t f_thread_t;
typedef sem_t f_sem_t;
typedef pthread_mutex_t f_mutex_t;

/* Mutex */
#define f_mutex_init(m) pthread_mutex_init(&(m), NULL)
#define f_mutex_destroy(m) pthread_mutex_destroy(&(m))
#define f_mutex_lock(m) pthread_mutex_lock(&(m))
#define f_mutex_unlock(m) pthread_mutex_unlock(&(m))
#define f_mutex_trylock(m) pthread_mutex_trylock(&(m))

/* Thread */
#define f_thread_create(f, p, id) pthread_create((id), NULL, (f), (p))
#define f_thread_join(id) pthread_join((id), NULL)

/* Sleep */
#define f_sleep_ms(ms) usleep((unsigned int)(ms) * 1000u)
#endif

/* =========================================================================
 * C / C++ linkage guard
 * ========================================================================= */
#ifdef __cplusplus
extern "C"
{
#endif

	/* =========================================================================
	 * Time utilities
	 * ========================================================================= */

	/**
	 * @brief  Return current wall-clock time in milliseconds since the Unix epoch.
	 * @return Milliseconds since epoch (uint64_t).
	 */
	uint64_t get_time(void);

	/**
	 * @brief  Return the last-modification time of a file as a Unix timestamp.
	 * @param  fname  Path to the file.
	 * @param  ftime  Output: seconds since the Unix epoch (0 on error).
	 * @return 0 on success, -1 on error.
	 */
	int GetFileUpdateTime(const char *fname, uint64_t *ftime);

	/* =========================================================================
	 * Semaphore utilities
	 * ========================================================================= */

	/**
	 * @brief  Initialise a semaphore to count 0.
	 * @return FERSLIB_SUCCESS or FERSLIB_ERR_GENERIC.
	 */
	int f_sem_init(f_sem_t *s);

	/**
	 * @brief  Destroy a semaphore.
	 * @return FERSLIB_SUCCESS or FERSLIB_ERR_GENERIC.
	 */
	int f_sem_destroy(f_sem_t *s);

	/**
	 * @brief  Wait on a semaphore.
	 * @param  s   Semaphore pointer.
	 * @param  ms  Timeout in milliseconds; pass FERS_WAIT_INFINITE to block forever.
	 * @return FERSLIB_SUCCESS, or FERSLIB_ERR_GENERIC on timeout / error.
	 */
	int f_sem_wait(f_sem_t *s, int32_t ms);

	/**
	 * @brief  Post (signal) a semaphore.
	 * @return FERSLIB_SUCCESS or FERSLIB_ERR_GENERIC.
	 */
	int f_sem_post(f_sem_t *s);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FERS_MULTIPLATFORM_H */