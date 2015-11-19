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

#if __cplusplus >= 201103l
#include <atomic>
#endif

// blacklist some architectures...like sparc odd 24 bit atomics
#define HAVE_ATOMICS    1

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

bool Atomic::is_lockfree(void)
{
    return true;
}

atomic_t Atomic::counter::fetch_add(atomic_t change) volatile
{
    return InterlockedExchangeAdd(&value, change);
}

atomic_t Atomic::counter::fetch_sub(atomic_t change) volatile
{
    return InterlockedExchangeAdd(&value, -change);
}

atomic_t Atomic::counter::fetch_retain() volatile
{
    return InterlockedExchangeAdd(&value, (atomic_t)1);
}

atomic_t Atomic::counter::fetch_release() volatile
{
    return InterlockedExchangeAdd(&value, (atomic_t)-1);
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

#elif __cplusplus >= 201103L && defined(HAVE_ATOMICS)
typedef std::atomic<atomic_t> *atomic_val;

bool Atomic::is_lockfree(void)
{
    atomic_val ptr;
    return std::atomic_is_lock_free(ptr);
}

atomic_t Atomic::counter::get() volatile
{
    return std::atomic_load_explicit((atomic_val)(&value), std::memory_order_acquire);
}

void Atomic::counter::clear() volatile
{
    std::atomic_fetch_and_explicit((atomic_val)(&value), (atomic_t)0, std::memory_order_release);
}

atomic_t Atomic::counter::fetch_retain() volatile
{
    return std::atomic_fetch_add_explicit((atomic_val)(&value), (atomic_t)1, std::memory_order_relaxed);
}

atomic_t Atomic::counter::fetch_release() volatile
{
    return std::atomic_fetch_sub_explicit((atomic_val)(&value), (atomic_t)1, std::memory_order_release);
}

atomic_t Atomic::counter::fetch_add(atomic_t change) volatile
{
    return std::atomic_fetch_add_explicit((atomic_val)(&value), (atomic_t)change, std::memory_order_release);
}

atomic_t Atomic::counter::fetch_sub(atomic_t change) volatile
{
    return std::atomic_fetch_sub_explicit((atomic_val)(&value), (atomic_t)change, std::memory_order_release);
}

bool Atomic::spinlock::acquire(void) volatile
{
    // if not locked by another already, then we acquired it...
    return (!std::atomic_exchange_explicit((atomic_val)(&value), (atomic_t)1, std::memory_order_acquire));
}

void Atomic::spinlock::wait(void) volatile
{
    while (std::atomic_exchange_explicit((atomic_val)(&value), (atomic_t)1, std::memory_order_acquire)) {
        while (value)
            ;
    }
}

void Atomic::spinlock::release(void) volatile
{
    std::atomic_store_explicit((atomic_val)(&value), (atomic_t)0, std::memory_order_release);
}

#elif defined(__CLANG_ATOMICS) && defined(HAVE_ATOMICS)
typedef _Atomic(atomic_t)   *atomic_val;

bool Atomic::is_lockfree(void)
{
    return true;
}

atomic_t Atomic::counter::get() volatile
{
    return __c11_atomic_load((atomic_val)(&value), __ATOMIC_ACQUIRE);
}

void Atomic::counter::clear() volatile
{
    __c11_atomic_fetch_and((atomic_val)(&value), (atomic_t)0, __ATOMIC_RELEASE);
}

atomic_t Atomic::counter::fetch_retain() volatile
{
    return __c11_atomic_fetch_add((atomic_val)(&value), (atomic_t)1, __ATOMIC_RELAXED);
}

atomic_t Atomic::counter::fetch_release() volatile
{
    return __c11_atomic_fetch_sub((atomic_val)(&value), (atomic_t)1, __ATOMIC_RELEASE);
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
    return (!__c11_atomic_exchange((atomic_val)(&value), 1, __ATOMIC_ACQUIRE));
}

void Atomic::spinlock::wait(void) volatile
{
    while (__c11_atomic_exchange((atomic_val)(&value), 1, __ATOMIC_ACQUIRE)) {
        while (value)
            ;
    }
}

void Atomic::spinlock::release(void) volatile
{
    __c11_atomic_store((atomic_val)(&value), 0, __ATOMIC_RELEASE);
}

#elif __GNUC_PREREQ__(4, 7) && defined(HAVE_ATOMICS)

bool Atomic::is_lockfree(void)
{
    return true;
}

atomic_t Atomic::counter::fetch_retain() volatile
{
    return __atomic_fetch_add(&value, (atomic_t)1, __ATOMIC_RELAXED);
}

atomic_t Atomic::counter::fetch_release() volatile
{
    return __atomic_fetch_sub(&value, (atomic_t)1, __ATOMIC_RELEASE);
}

atomic_t Atomic::counter::fetch_add(atomic_t change) volatile
{
    return __atomic_fetch_add(&value, change, __ATOMIC_RELEASE);
}

atomic_t Atomic::counter::fetch_sub(atomic_t change) volatile
{
    return __atomic_fetch_sub(&value, change, __ATOMIC_RELEASE);
}

atomic_t Atomic::counter::get() volatile
{
    return __atomic_fetch_add(&value, 0, __ATOMIC_ACQUIRE);
}

void Atomic::counter::clear() volatile
{
    __atomic_fetch_and(&value, (atomic_t)0, __ATOMIC_RELEASE);
}

bool Atomic::spinlock::acquire(void) volatile
{
    // if not locked by another already, then we acquired it...
    return (!__atomic_test_and_set(&value, __ATOMIC_ACQUIRE));
}

void Atomic::spinlock::wait(void) volatile
{
    while (__atomic_test_and_set(&value, __ATOMIC_ACQUIRE)) {
        while (value)
            ;
    }
}

void Atomic::spinlock::release(void) volatile
{
    __atomic_clear(&value, __ATOMIC_RELEASE);
}

#elif __GNUC_PREREQ__(4, 1) && defined(HAVE_ATOMICS)

bool Atomic::is_lockfree(void)
{
    return true;
}

atomic_t Atomic::counter::fetch_retain() volatile
{
    return __sync_fetch_and_add(&value, (atomic_t)1);
}

atomic_t Atomic::counter::fetch_release() volatile
{
    return __sync_fetch_and_sub(&value, (atomic_t)1);
}

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

bool Atomic::is_lockfree(void)
{
    return false;
}

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

atomic_t Atomic::counter::fetch_retain() volatile
{
    return fetch_add(1);
}

atomic_t Atomic::counter::fetch_release() volatile
{
    return fetch_sub(1);
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

Atomic::Aligned::Aligned(size_t object, size_t align)
{
    if(!align)
        align = Thread::cache();

    offset = 0;
    caddr_t base = (caddr_t)::malloc(align + object);
    size_t mask = align - 1;
    while((intptr_t)base & mask) {
        ++offset;
        ++base;
    }
    address = (void *)base;
}

Atomic::Aligned::~Aligned()
{
    caddr_t base = (caddr_t)address;
    if(base) {
        while(offset--) {
            --base;
        }
        ::free(base);
        address = nullptr;
    }
}

} // namespace ucommon
