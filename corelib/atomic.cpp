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

#include <ucommon-config.h>
#include <ucommon/export.h>
#include <ucommon/atomic.h>
#include <ucommon/thread.h>

// blacklist some architectures...like sparc odd 24 bit atomics
#if defined(sparc)
#undef  HAVE_ATOMICS
#endif

#ifdef  HAVE_STDALIGN_H
#include <stdalign.h>
#endif

#if !defined(HAVE_ALIGNED_ALLOC) && defined(_MSC_VER) && _MSC_VER >= 1800
#include <malloc.h>
#define HAVE_ALIGNED_ALLOC 1
#define aligned_alloc(a, s) _aligned_malloc(s, a)
#endif

namespace ucommon {

atomic::counter::counter(atomic_t init)
{
    value = init;
}

atomic::spinlock::spinlock()
{
    value = 0;
}

#if !defined(__GNUC_PREREQ__)
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define __GNUC_PREREQ__(maj, min) \
((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
#define __GNUC_PREREQ__(maj, min) 0
#endif
#endif 

#if defined(__has_feature) && defined(__has_extension)
#if __has_feature(c_atomic) || __has_extension(c_atomic)
#define __CLANG_ATOMICS
#endif
#endif

#if defined(_MSWINDOWS_)

atomic_t atomic::counter::fetch_add(atomic_t change) volatile
{
    return InterlockedExchangeAdd(&value, change);
}

atomic_t atomic::counter::fetch_sub(atomic_t change) volatile
{
    return InterlockedExchangeAdd(&value, -change);
}

atomic_t atomic::counter::get() volatile
{
    return fetch_add(0);
}

void atomic::counter::clear() volatile
{
    _InterlockedAnd(&value, 0);
}

bool atomic::spinlock::acquire() volatile
{
    return !InterlockedBitTestAndSet(&value, 1);
}

void atomic::spinlock::wait() volatile
{
    while(InterlockedBitTestAndSet(&value, 1)) {
        while(value)
            ;
    }
}

void atomic::spinlock::release() volatile
{
    InterlockedBitTestAndReset(&value, 1);
}

#elif defined(__CLANG_ATOMICS) && defined(HAVE_ATOMICS)
typedef _Atomic(atomic_t)   *atomic_val;

atomic_t atomic::counter::get() volatile
{
    return __c11_atomic_load((atomic_val)(&value), __ATOMIC_SEQ_CST);
}

void atomic::counter::clear() volatile
{
    __c11_atomic_fetch_and((atomic_val)(&value), (atomic_t)0, __ATOMIC_SEQ_CST);
}

atomic_t atomic::counter::fetch_add(atomic_t change) volatile
{
    return __c11_atomic_fetch_add((atomic_val)(&value), change, __ATOMIC_SEQ_CST);
}

atomic_t atomic::counter::fetch_sub(atomic_t change) volatile
{
    return __c11_atomic_fetch_sub((atomic_val)(&value), change, __ATOMIC_SEQ_CST);
}

bool atomic::spinlock::acquire(void) volatile
{
    // if not locked by another already, then we acquired it...
    return (!__c11_atomic_exchange((atomic_val)(&value), 1, __ATOMIC_SEQ_CST));
}

void atomic::spinlock::wait(void) volatile
{
    while (__c11_atomic_exchange((atomic_val)(&value), 1, __ATOMIC_SEQ_CST)) {
        while (value)
            ;
    }
}

void atomic::spinlock::release(void) volatile
{
    __c11_atomic_store((atomic_val)(&value), 0, __ATOMIC_SEQ_CST);
}

#elif __GNUC_PREREQ__(4, 7) && defined(HAVE_ATOMICS)

atomic_t atomic::counter::fetch_add(atomic_t change) volatile
{
    return __atomic_fetch_add(&value, change, __ATOMIC_SEQ_CST);
}

atomic_t atomic::counter::fetch_sub(atomic_t change) volatile
{
    return __atomic_fetch_sub(&value, change, __ATOMIC_SEQ_CST);
}

atomic_t atomic::counter::get() volatile
{
    return fetch_add(0);
}

void atomic::counter::clear() volatile
{
    __atomic_fetch_and(&value, (atomic_t)0, __ATOMIC_SEQ_CST);
}

bool atomic::spinlock::acquire(void) volatile
{
    // if not locked by another already, then we acquired it...
    return (!__atomic_test_and_set(&value, __ATOMIC_SEQ_CST));
}

void atomic::spinlock::wait(void) volatile
{
    while (__atomic_test_and_set(&value, __ATOMIC_SEQ_CST)) {
        while (value)
            ;
    }
}

void atomic::spinlock::release(void) volatile
{
    __atomic_clear(&value, __ATOMIC_SEQ_CST);
}

#elif __GNUC_PREREQ__(4, 1) && defined(HAVE_ATOMICS)

atomic_t atomic::counter::fetch_add(atomic_t change) volatile
{
    return __sync_fetch_and_add(&value, change);
}

atomic_t atomic::counter::fetch_sub(atomic_t change) volatile
{
    return __sync_fetch_and_sub(&value, change);
}

atomic_t atomic::counter::get() volatile
{
    return fetch_add(0);
}

void atomic::counter::clear() volatile
{
    __sync_fetch_and_and(&value, (atomic_t)0);
}

bool atomic::spinlock::acquire(void) volatile
{
    // if not locked by another already, then we acquired it...
    return (!__sync_lock_test_and_set(&value, 1));
}

void atomic::spinlock::wait(void) volatile
{
    while (__sync_lock_test_and_set(&value, 1)) {
        while (value)
            ;
    }
}

void atomic::spinlock::release(void) volatile
{
    __sync_lock_release(&value);
}

#else

#define SIMULATED true

atomic_t atomic::counter::get() volatile
{
    atomic_t rval;
    Mutex::protect((void *)&value);
    rval = value;
    Mutex::release((void *)&value);
    return rval;
}

atomic_t atomic::counter::fetch_add(atomic_t change) volatile
{
    atomic_t rval;
    Mutex::protect((void *)&value);
    rval = value;
    value += change;
    Mutex::release((void *)&value);
    return rval;
}

atomic_t atomic::counter::fetch_sub(atomic_t change) volatile
{
    atomic_t rval;
    Mutex::protect((void *)&value);
    rval = value;
    value -= change;
    Mutex::release((void *)&value);
    return rval;
}

bool atomic::spinlock::acquire(void) volatile
{
    bool rtn = true;

    Mutex::protect((void *)&value);
    if(value == 1)
        rtn = false;
    else
        value = 1;
    Mutex::release((void *)&value);
    return rtn;
}

void atomic::spinlock::wait(void) volatile
{
    while(!acquire())
        ;
}

void atomic::spinlock::release(void) volatile
{
    Mutex::protect((void *)&value);
    value = 0;
    Mutex::release((void *)&value);
}

#endif

#ifdef  HAVE_POSIX_MEMALIGN

void *atomic::alloc(size_t size)
{
    void *addr = NULL;
    if(!posix_memalign(&addr, 16, size))
        return NULL;
    return addr;
}

#elif  HAVE_ALIGNED_ALLOC

void *atomic::alloc(size_t size)
{
    return aligned_alloc(16, size);
}

#else

void *atomic::alloc(size_t size)
{
    caddr_t addr = (caddr_t)::malloc(size + 16);
    if(!addr)
        return NULL;
    while(((uintptr_t)addr) & 0xf)
        ++addr;
    return addr;
}

#endif

#ifdef SIMULATED
const bool atomic::simulated = true;
#else
const bool atomic::simulated = false;
#endif

atomic_t atomic::counter::operator++() volatile
{
    return fetch_add(1) + 1;
}

atomic_t atomic::counter::operator--() volatile
{
    return fetch_sub(1) - 1;
}

atomic_t atomic::counter::operator+=(atomic_t change) volatile
{
    return fetch_add(change) + change;
}

atomic_t atomic::counter::operator-=(atomic_t change) volatile
{
    return fetch_sub(change) - change;
}


} // namespace ucommon
