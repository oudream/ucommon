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

#if defined(_MSC_VER) && _MSC_VER >= 1800
#include <malloc.h>
#ifndef HAVE_ALIGNED_ALLOC
#define HAVE_ALIGNED_ALLOC 1
#endif
#define aligned_alloc(a, s) _aligned_malloc(s, a)
#endif

namespace ucommon {

Atomic::counter::counter(atomic_t init)
{
    value = init;
}

Atomic::spinlock::spinlock()
{
    value = 0;
}

#if defined(__has_feature) && defined(__has_extension)
#if __has_feature(c_atomic) || __has_extension(c_atomic)
#define __CLANG_ATOMICS
#endif
#endif

#if defined(_MSWINDOWS_)

atomic_t Atomic::counter::fetch_add(atomic_t change) volatile
{
    return InterlockedExchangeAdd(&value, change);
}

atomic_t Atomic::counter::fetch_sub(atomic_t change) volatile
{
    return InterlockedExchangeAdd(&value, -change);
}

atomic_t Atomic::counter::get() volatile
{
    return fetch_add(0);
}

void Atomic::counter::clear() volatile
{
    _InterlockedAnd(&value, 0);
}

bool Atomic::spinlock::acquire() volatile
{
    return !InterlockedBitTestAndSet(&value, 1);
}

void Atomic::spinlock::wait() volatile
{
    while(InterlockedBitTestAndSet(&value, 1)) {
        while(value)
            ;
    }
}

void Atomic::spinlock::release() volatile
{
    InterlockedBitTestAndReset(&value, 1);
}

#elif defined(__CLANG_ATOMICS) && defined(HAVE_ATOMICS)
typedef _Atomic(atomic_t)   *atomic_val;

atomic_t Atomic::counter::get() volatile
{
    return __c11_atomic_load((atomic_val)(&value), __ATOMIC_SEQ_CST);
}

void Atomic::counter::clear() volatile
{
    __c11_atomic_fetch_and((atomic_val)(&value), (atomic_t)0, __ATOMIC_SEQ_CST);
}

atomic_t Atomic::counter::fetch_add(atomic_t change) volatile
{
    return __c11_atomic_fetch_add((atomic_val)(&value), change, __ATOMIC_SEQ_CST);
}

atomic_t Atomic::counter::fetch_sub(atomic_t change) volatile
{
    return __c11_atomic_fetch_sub((atomic_val)(&value), change, __ATOMIC_SEQ_CST);
}

bool Atomic::spinlock::acquire(void) volatile
{
    // if not locked by another already, then we acquired it...
    return (!__c11_atomic_exchange((atomic_val)(&value), 1, __ATOMIC_SEQ_CST));
}

void Atomic::spinlock::wait(void) volatile
{
    while (__c11_atomic_exchange((atomic_val)(&value), 1, __ATOMIC_SEQ_CST)) {
        while (value)
            ;
    }
}

void Atomic::spinlock::release(void) volatile
{
    __c11_atomic_store((atomic_val)(&value), 0, __ATOMIC_SEQ_CST);
}

#elif __GNUC_PREREQ__(4, 7) && defined(HAVE_ATOMICS)

atomic_t Atomic::counter::fetch_add(atomic_t change) volatile
{
    return __atomic_fetch_add(&value, change, __ATOMIC_SEQ_CST);
}

atomic_t Atomic::counter::fetch_sub(atomic_t change) volatile
{
    return __atomic_fetch_sub(&value, change, __ATOMIC_SEQ_CST);
}

atomic_t Atomic::counter::get() volatile
{
    return fetch_add(0);
}

void Atomic::counter::clear() volatile
{
    __atomic_fetch_and(&value, (atomic_t)0, __ATOMIC_SEQ_CST);
}

bool Atomic::spinlock::acquire(void) volatile
{
    // if not locked by another already, then we acquired it...
    return (!__atomic_test_and_set(&value, __ATOMIC_SEQ_CST));
}

void Atomic::spinlock::wait(void) volatile
{
    while (__atomic_test_and_set(&value, __ATOMIC_SEQ_CST)) {
        while (value)
            ;
    }
}

void Atomic::spinlock::release(void) volatile
{
    __atomic_clear(&value, __ATOMIC_SEQ_CST);
}

#elif __GNUC_PREREQ__(4, 1) && defined(HAVE_ATOMICS)

atomic_t Atomic::counter::fetch_add(atomic_t change) volatile
{
    return __sync_fetch_and_add(&value, change);
}

atomic_t Atomic::counter::fetch_sub(atomic_t change) volatile
{
    return __sync_fetch_and_sub(&value, change);
}

atomic_t Atomic::counter::get() volatile
{
    return fetch_add(0);
}

void Atomic::counter::clear() volatile
{
    __sync_fetch_and_and(&value, (atomic_t)0);
}

bool Atomic::spinlock::acquire(void) volatile
{
    // if not locked by another already, then we acquired it...
    return (!__sync_lock_test_and_set(&value, 1));
}

void Atomic::spinlock::wait(void) volatile
{
    while (__sync_lock_test_and_set(&value, 1)) {
        while (value)
            ;
    }
}

void Atomic::spinlock::release(void) volatile
{
    __sync_lock_release(&value);
}

#else

#define SIMULATED true

atomic_t Atomic::counter::get() volatile
{
    atomic_t rval;
    Mutex::protect((void *)&value);
    rval = value;
    Mutex::release((void *)&value);
    return rval;
}

void Atomic::counter::clear() volatile
{
    Mutex::protect((void *)&value);
    value = 0;
    Mutex::release((void *)&value);
}

atomic_t Atomic::counter::fetch_add(atomic_t change) volatile
{
    atomic_t rval;
    Mutex::protect((void *)&value);
    rval = value;
    value += change;
    Mutex::release((void *)&value);
    return rval;
}

atomic_t Atomic::counter::fetch_sub(atomic_t change) volatile
{
    atomic_t rval;
    Mutex::protect((void *)&value);
    rval = value;
    value -= change;
    Mutex::release((void *)&value);
    return rval;
}

bool Atomic::spinlock::acquire(void) volatile
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

void Atomic::spinlock::wait(void) volatile
{
    while(!acquire())
        ;
}

void Atomic::spinlock::release(void) volatile
{
    Mutex::protect((void *)&value);
    value = 0;
    Mutex::release((void *)&value);
}

#endif

#ifdef SIMULATED
const bool Atomic::simulated = true;
#else
const bool Atomic::simulated = false;
#endif

atomic_t Atomic::counter::operator++() volatile
{
    return fetch_add(1) + 1;
}

atomic_t Atomic::counter::operator--() volatile
{
    return fetch_sub(1) - 1;
}

atomic_t Atomic::counter::operator+=(atomic_t change) volatile
{
    return fetch_add(change) + change;
}

atomic_t Atomic::counter::operator-=(atomic_t change) volatile
{
    return fetch_sub(change) - change;
}


} // namespace ucommon
