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
 * Atomic pointers and locks.  These are meant to use atomic CPU operations
 * and hence offer maximum performance.
 * @file ucommon/atomic.h
 * @author David Sugar <dyfet@gnutelephony.org>
 */

#ifndef _UCOMMON_ATOMIC_H_
#define _UCOMMON_ATOMIC_H_

#ifndef _UCOMMON_CONFIG_H_
#include <ucommon/platform.h>
#endif

#if defined(_MSWINDOWS_)
typedef LONG atomic_t;
#else
typedef int atomic_t;
#endif

namespace ucommon {

/**
 * Generic atomic class for referencing atomic objects and static functions.
 * We have an atomic counter and spinlock, and in the future we may add
 * other atomic classes and atomic manipulations needed to create lockfree
 * data structures.  The atomic classes use mutexes if no suitable atomic
 * code is available.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT Atomic
{
private:
    __DELETE_DEFAULTS(Atomic);

public:
    /**
     * Atomic counter class.  Can be used to manipulate value of an
     * atomic counter without requiring explicit thread locking.
     * @author David Sugar <dyfet@gnutelephony.org>
     */
    class __EXPORT counter
    {
    private:
        mutable volatile atomic_t value;

        __DELETE_COPY(counter);

    public:
        counter(atomic_t initial = 0);

        // optimized reference count semantics
        atomic_t fetch_retain() volatile;
        atomic_t fetch_release() volatile;

        // fetch add/sub optimized semantics
        atomic_t fetch_add(atomic_t offset = 1) volatile;
        atomic_t fetch_sub(atomic_t offset = 1) volatile;

        atomic_t operator++() volatile;
        atomic_t operator--() volatile;
        atomic_t operator+=(atomic_t offset) volatile;
        atomic_t operator-=(atomic_t offset) volatile;
        atomic_t get() volatile;
        void clear() volatile;

        inline operator atomic_t() volatile {
            return get();
        }

        inline atomic_t operator*() volatile {
            return get();
        }
    };

    /**
     * Atomic spinlock class.  Used as high-performance sync lock between
     * threads.
     * @author David Sugar <dyfet@gnutelephony.org>
     */
    class __EXPORT spinlock
    {
    private:
#ifdef  __GNUC__
        mutable volatile atomic_t value __attribute__ ((aligned(16)));
#else
        mutable volatile atomic_t value;
#endif
        __DELETE_COPY(spinlock);

    public:
        /**
         * Construct and initialize spinlock.
         */
        spinlock();

        /**NAMESPACE_UCOMMON
         * Acquire the lock.  If the lock is not acquired, one "spins"
         * by doing something else.  One suggestion is using thread::yield.
         * @return true if acquired.
         */
        bool acquire(void) volatile;

        /**
         * Wait for and aquire spinlock.
         */
        void wait(void) volatile;

        /**
         * Release an acquired spinlock.
         */
        void release(void) volatile;
    };

    class __EXPORT Aligned
    {
    private:
        __DELETE_DEFAULTS(Aligned);

    protected:
        void *address;
        size_t offset;

        Aligned(size_t object, size_t offset = 0);
    
    public:
        virtual ~Aligned();
    };

    template<typename T, unsigned alignment = 0>
    class aligned : public Aligned
    {
    protected:
        inline T* get() const {
            return static_cast<T*>(address);
        }

    public:
        inline aligned() : Aligned(sizeof(T), alignment) { 
            new((caddr_t)address) T;
        }
        
        inline T& operator*() const {
            return *(static_cast<T*>(address));
        }

        inline operator T&() {
            return *get();
        }

        inline void operator()(T value) {
            *get() = value;
        }
    };

    static bool is_lockfree(void);
};

} // namespace ucommon

#endif
