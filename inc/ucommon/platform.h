// Copyright (C) 2006-2014 David Sugar, Tycho Softworks.
// Copyright (C) 2015 Cherokees of Idaho.
//
// This file is part of GNU uCommon C++.
//
// GNU uCommon C++ is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GNU uCommon C++ is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with GNU uCommon C++.  If not, see <http://www.gnu.org/licenses/>.

/**
 * Various miscellaneous platform specific headers and defines.
 * This is used to support ucommon on different platforms.  The ucommon
 * library assumes at least a real posix threading library is present or
 * will build thread support native on Microsoft Windows legacy platform.
 * This header also deals with issues related to common base types.
 * @file ucommon/platform.h
 */


#include <cstdlib>
#include <cstddef>
#if __cplusplus >= 201103L
#include <memory>
#endif

#if defined(sun) && defined(unix)
#include <malloc.h>
#endif

#ifndef _UCOMMON_PLATFORM_H_
#define _UCOMMON_PLATFORM_H_
#define UCOMMON_ABI 7

#ifndef UCOMMON_SYSRUNTIME
#ifndef NEW_STDCPP
#define NEW_STDCPP
#endif
#define _UCOMMON_EXTENDED_
#include <stdexcept>
#define __THROW_SIZE(x)       throw std::length_error(x)
#define __THROW_RANGE(x)      throw std::out_of_range(x)
#define __THROW_RUNTIME(x)    throw std::runtime_error(x)
#define __THROW_ALLOC()       throw std::bad_alloc()
#define __THROW_DEREF(v)      if(v == nullptr) \
                                throw std::runtime_error("Dereference NULL")
#define __THROW_UNDEF(v,x)    if(v == nullptr) throw std::runtime_error(x)
#else
#define __THROW_RANGE(x)      abort()
#define __THROW_SIZE(x)       abort()
#define __THROW_RUNTIME(x)    abort()
#define __THROW_ALLOC()       abort()
#define __THROW_DEREF(v)      if(v == nullptr) abort()
#define __THROW_UNDEF(v,x)    if(v == nullptr) abort()
#endif

/**
 * Common namespace for all ucommon objects.
 * We are using a common namespace to easily separate ucommon from other
 * libraries.  This namespace usage is set to the package name and controlled
 * by macros so future changes will be hidden from user applications so long
 * as the namespace macros (UCOMMON_NAMESPACE, NAMESPACE_UCOMMON,
 * END_NAMESPACE) are used in place of direct namespace declarations.
 * @namespace ucommon
 */

#define UCOMMON_NAMESPACE   ucommon
#define NAMESPACE_UCOMMON   namespace ucommon {
#define END_NAMESPACE       }

#ifndef _REENTRANT
#define _REENTRANT 1
#endif

#ifndef __PTH__
#ifndef _THREADSAFE
#define _THREADSAFE 1
#endif

#ifndef _POSIX_PTHREAD_SEMANTICS
#define _POSIX_PTHREAD_SEMANTICS
#endif
#endif

#if !defined(__GNUC__) && !defined(__has_feature) && !defined(_MSC_VER)
#define UCOMMON_RTTI    1
#endif

#if __GNUC__ > 3 && defined(__GXX_RTTI)
#define UCOMMON_RTTI    1
#endif

#if defined(_MSC_VER) && defined(_CPPRTTI)
#define UCOMMON_RTTI    1
#endif

#if defined(__has_feature)
#if __has_feature(cxx_rtti)
#define UCOMMON_RTTI    1
#endif
#endif

#ifdef  UCOMMON_RTTI
#define __PROTOCOL   virtual
template<typename T, typename S>
T protocol_cast(S *s) {
    return dynamic_cast<T>(s);
}
#else
#define __PROTOCOL
template<typename T, typename S>
T protocol_cast(S *s) {
    return static_cast<T>(s);
}
#endif

#if defined(__GNUC__) && (__GNUC < 3) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#if !defined(__GNUC_PREREQ__)
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define __GNUC_PREREQ__(maj, min) ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
#define __GNUC_PREREQ__(maj, min) 0
#endif
#endif

#if __GNUC_PREREQ__(3,3)
#define __PRINTF(x,y)   __attribute__ ((format (printf, x, y)))
#define __SCANF(x, y) __attribute__ ((format (scanf, x, y)))
#define __MALLOC      __attribute__ ((malloc))
#define __NORETURN    __attribute__ ((__noreturn__))
#endif

#define __UNUSED(x)         (void)x

#if __cplusplus >= 201103L
#define     __ALIGNED(x)    alignas(x)
#else
#ifdef      _MSC_VER
#define     __ALIGNED(x)    __declspec(align(x))
#else
#define     __ALIGNED(x)    __attribute__(align(x))
#endif
#endif

#if __cplusplus < 201103L
#define __FINAL
#define __OVERRIDE
#define __DELETED
#define __DELETE_COPY(x)        inline x(const x&);\
                                inline x& operator=(const x&)
#define __DELETE_DEFAULTS(x)    inline x();\
                                __DELETE_COPY(x)    
#else
#define __FINAL                 final
#define __OVERRIDE              override
#define __DELETED               =delete
#define __DELETE_COPY(x)        inline x(const x&) =delete;\
                                inline x& operator=(const x&) =delete
#define __DELETE_DEFAULTS(x)    inline x() =delete;\
                                __DELETE_COPY(x)
#endif

#if __cplusplus <= 199711L && !defined(_MSC_VER)
#if defined(__GNUC_MINOR__) && !defined(__clang__)
#define nullptr __null
#elif !defined(__clang__) || (defined(__clang__) && defined(__linux__))
const class nullptr_t 
{
public:
    template<class T>
    inline operator T*() const {
        return 0; 
    }

    template<class C, class T>
    inline operator T C::*() const {
        return 0; 
    }

private:
    void operator&() const;

} nullptr = {};
#endif
#endif

#ifndef __MALLOC
#define __PRINTF(x, y)
#define __SCANF(x, y)
#define __MALLOC
#endif

#ifndef DEBUG
#ifndef NDEBUG
#define NDEBUG
#endif
#endif

#ifdef  DEBUG
#ifdef  NDEBUG
#undef  NDEBUG
#endif
#endif

// see if targeting legacy Microsoft windows platform

#if defined(_MSC_VER) || defined(WIN32) || defined(_WIN32)
#define _MSWINDOWS_

#if defined(_MSC_VER)
#define NOMINMAX
#if _MSC_VER < 1500
#warning "Probably won't build, need VS >= 2010 or later"
#endif
#endif

// minimum required version requires conditional
#ifdef _WIN32_WINNT
#if _WIN32_WINNT < 0x0600
#undef _WIN32_WINNT
#undef WINVER
#endif
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT    0x0600
#endif

#ifdef  _MSC_VER
#pragma warning(disable: 4251)
#pragma warning(disable: 4996)
#pragma warning(disable: 4355)
#pragma warning(disable: 4290)
#pragma warning(disable: 4291)
#endif

#if defined(__BORLANDC__) && !defined(__MT__)
#error Please enable multithreading
#endif

#if defined(_MSC_VER) && !defined(_MT)
#error Please enable multithreading (Project -> Settings -> C/C++ -> Code Generation -> Use Runtime Library)
#endif

// Make sure we're consistent with _WIN32_WINNT
#ifndef WINVER
#define WINVER _WIN32_WINNT
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#if defined(_MSC_VER)
typedef int socksize_t;
typedef int socklen_t;
typedef signed long ssize_t;
typedef int pid_t;
#else
typedef size_t sockword_t;
typedef size_t socksize_t;
#endif

#include <process.h>
#ifndef __EXPORT
#ifdef  UCOMMON_STATIC
#define __EXPORT
#else
#define __EXPORT    __declspec(dllimport)
#endif
#endif
#define __LOCAL

// if runtime mode then non-runtime libraries are static on windows...
#if defined(UCOMMON_RUNTIME) || defined(UCOMMON_STATIC)
#define __SHARED
#else
#define __SHARED __EXPORT
#endif

#else
typedef size_t socksize_t;
#define __EXPORT    __attribute__ ((visibility("default")))
#define __LOCAL     __attribute__ ((visibility("hidden")))
#define __SHARED    __attribute__ ((visibility("default")))
#endif

#ifdef  _MSWINDOWS_

#define _UWIN

#include <sys/stat.h>
#include <io.h>

// gcc mingw can do native high performance win32 conditionals...
#if defined(UCOMMON_WINPTHREAD) && __GNUC_PREREQ__(4, 8) && !defined(UCOMMON_SYSRUNTIME)
#define __MINGW_WINPTHREAD__
#include <pthread.h>   // gnu libstdc++ now requires a win pthread
typedef size_t stacksize_t;
#else   
#define _MSTHREADS_
typedef DWORD pthread_t;
typedef DWORD pthread_key_t;
typedef unsigned stacksize_t;
typedef CRITICAL_SECTION pthread_mutex_t;
#endif
typedef char *caddr_t;
typedef HANDLE fd_t;
typedef SOCKET socket_t;

#ifdef  _MSC_VER
typedef struct timespec {
    time_t tv_sec;
    long  tv_nsec;
} timespec_t;
#endif

inline void sleep(int seconds)
    {::Sleep((seconds * 1000l));}

extern "C" {

    #define __SERVICE(id, argc, argv) void WINAPI service_##id(DWORD argc, LPSTR *argv)
    #define SERVICE_MAIN(id, argc, argv) void WINAPI service_##id(DWORD argc, LPSTR *argv)

    typedef LPSERVICE_MAIN_FUNCTION cpr_service_t;

#ifdef _MSTHREADS_
    inline void pthread_exit(void *p)
        {_endthreadex((DWORD)0);}

    inline pthread_t pthread_self(void)
        {return (pthread_t)GetCurrentThreadId();}

    inline int pthread_mutex_init(pthread_mutex_t *mutex, void *x)
        {InitializeCriticalSection(mutex); return 0;}

    inline void pthread_mutex_destroy(pthread_mutex_t *mutex)
        {DeleteCriticalSection(mutex);}

    inline void pthread_mutex_lock(pthread_mutex_t *mutex)
        {EnterCriticalSection(mutex);}

    inline void pthread_mutex_unlock(pthread_mutex_t *mutex)
        {LeaveCriticalSection(mutex);}
#endif
}

#elif defined(__PTH__)

#include <pth.h>
#include <sys/wait.h>

typedef size_t stacksize_t;
typedef int socket_t;
typedef int fd_t;
#define INVALID_SOCKET -1
#define INVALID_HANDLE_VALUE -1
#include <signal.h>

#define pthread_mutex_t pth_mutex_t
#define pthread_cond_t pth_cond_t
#define pthread_t pth_t

inline int pthread_sigmask(int how, const sigset_t *set, sigset_t *oset)
    {return pth_sigmask(how, set, oset);};

inline void pthread_exit(void *p)
    {pth_exit(p);};

inline void pthread_kill(pthread_t tid, int sig)
    {pth_raise(tid, sig);};

inline int pthread_mutex_init(pthread_mutex_t *mutex, void *x)
    {return pth_mutex_init(mutex) != 0;};

inline void pthread_mutex_destroy(pthread_mutex_t *mutex)
    {};

inline void pthread_mutex_lock(pthread_mutex_t *mutex)
    {pth_mutex_acquire(mutex, 0, nullptr);};

inline void pthread_mutex_unlock(pthread_mutex_t *mutex)
    {pth_mutex_release(mutex);};

inline void pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
    {pth_cond_await(cond, mutex, nullptr);};

inline void pthread_cond_signal(pthread_cond_t *cond)
    {pth_cond_notify(cond, FALSE);};

inline void pthread_cond_broadcast(pthread_cond_t *cond)
    {pth_cond_notify(cond, TRUE);};

#else

#include <pthread.h>

typedef size_t stacksize_t;
typedef int socket_t;
typedef int fd_t;
#define INVALID_SOCKET -1
#define INVALID_HANDLE_VALUE -1
#include <signal.h>

#endif

#ifdef _MSC_VER
typedef signed __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef signed __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef signed __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
typedef char *caddr_t;

#include <stdio.h>
#define snprintf(p, s, f, ...) _snprintf_s(p, s, _TRUNCATE, f, __VA_ARGS__) 
#define vsnprintf(p, s, f, a) _vsnprintf_s(p, s, _TRUNCATE, f, a)

#else

#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

#endif

#undef  getchar
#undef  putchar

#ifndef _GNU_SOURCE
typedef void (*sighandler_t)(int);  /**< Convenient typedef for signal handlers. */
#endif
typedef unsigned long timeout_t;

#include <cctype>
#include <climits>
#include <cerrno>
#ifndef UCOMMON_RUNTIME
#include <new>
#endif

#ifdef  _MSWINDOWS_
#ifndef ENETDOWN
#define ENETDOWN        ((int)(WSAENETDOWN))
#endif
#ifndef EINPROGRESS
#define EINPROGRESS     ((int)(WSAEINPROGRESS))
#endif
#ifndef ENOPROTOOPT
#define ENOPROTOOPT     ((int)(WSAENOPROTOOPT))
#endif
#ifndef EADDRINUSE
#define EADDRINUSE      ((int)(WSAEADDRINUSE))
#endif
#ifndef EADDRNOTAVAIL
#define EADDRNOTAVAIL   ((int)(WSAEADDRNOTAVAIL))
#endif
#ifndef ENETUNREACH
#define ENETUNREACH     ((int)(WSAENETUNREACH))
#endif
#ifndef EHOSTUNREACH
#define EHOSTUNREACH    ((int)(WSAEHOSTUNREACH))
#endif
#ifndef EHOSTDOWN
#define EHOSTDOWN       ((int)(WSAEHOSTDOWN))
#endif
#ifndef ENETRESET
#define ENETRESET       ((int)(WSAENETRESET))
#endif
#ifndef ECONNABORTED
#define ECONNABORTED    ((int)(WSAECONNABORTED))
#endif
#ifndef ECONNRESET
#define ECONNRESET      ((int)(WSAECONNRESET))
#endif
#ifndef EISCONN
#define EISCONN         ((int)(WSAEISCONN))
#endif
#ifndef ENOTCONN
#define ENOTCONN        ((int)(WSAENOTCONN))
#endif
#ifndef ESHUTDOWN
#define ESHUTDOWN       ((int)(WSAESHUTDOWN))
#endif
#ifndef ETIMEDOUT
#define ETIMEDOUT       ((int)(WSAETIMEDOUT))
#endif
#ifndef ECONNREFUSED
#define ECONNREFUSED    ((int)(WSAECONNREFUSED))
#endif
#endif

#ifndef DEBUG
#ifndef NDEBUG
#define NDEBUG
#endif
#endif

#ifdef  DEBUG
#ifdef  NDEBUG
#undef  NDEBUG
#endif
#endif

#ifndef __PROGRAM
#define __PROGRAM(c,v)              extern "C" int main(int c, char **v)
#define PROGRAM_MAIN(argc, argv)    extern "C" int main(int argc, char **argv)
#define PROGRAM_EXIT(code)          return code
#endif

#ifndef __SERVICE
#define __SERVICE(id, c, v)         void service_##id(int c, char **v)
#define SERVICE_MAIN(id, argc, argv)    void service_##id(int argc, char **argv)
typedef void (*cpr_service_t)(int argc, char **argv);
#endif

#include <assert.h>
#ifdef  DEBUG
#define crit(x, text)   assert(x)
#else
#define crit(x, text) if(!(x)) cpr_runtime_error(text)
#endif

/**
 * Template function to initialize memory by invoking default constructor.
 * If NULL is passed, then NULL is returned without any constructor called.
 * @param memory to initialize.
 * @return memory initialized.
 */
template<class T>
inline T *init(T *memory)
    {return ((memory) ? new(((void *)memory)) T : nullptr);}

typedef long Integer;
typedef unsigned long Unsigned;
typedef double Real;
typedef uint8_t ubyte_t;

/**
 * Matching function for strdup().
 * @param string to release from allocated memory.
 */
inline void strfree(char *str)
    {::free(str);}

template<class T, class S>
inline T polypointer_cast(S *s)
{
#if defined(DEBUG) && defined(UCOMMON_RTTI)
    if(s == nullptr)
        return nullptr;
    T ptr = dynamic_cast<T>(s);
    __THROW_DEREF(ptr);
    return ptr;
#else
    return static_cast<T>(s);
#endif
}   

template<class T, class S>
inline T polyconst_cast(S *s)
{
    return const_cast<T>(polypointer_cast<T>(s));
}

template<class T, class S>
inline T polystatic_cast(S *s)
{
    return static_cast<T>(s);
}    

template<class T, class S>
inline T polydynamic_cast(S *s)
{
#if defined(UCOMMON_RTTI)
    return dynamic_cast<T>(s);
#else
    return static_cast<T>(s);
#endif
}    

template<class T, class S>
inline T& polyreference_cast(S *s)
{
    __THROW_DEREF(s);
    return *(static_cast<T*>(s));
}    

template<typename T>
inline T& reference_cast(T *pointer) {
    __THROW_DEREF(pointer);
    return *pointer;
}

template<typename T>
inline const T immutable_cast(T p)
{
    return static_cast<const T>(p);
}

#endif
