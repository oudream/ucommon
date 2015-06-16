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

typedef int atomic_t;

namespace ucommon {

/**
 * Generic atomic class for referencing atomic objects and static functions.
 * We have an atomic counter and spinlock, and in the future we may add
 * other atomic classes and atomic manipulations needed to create lockfree
 * data structures.  The atomic classes use mutexes if no suitable atomic
 * code is available.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT atomic
{
public:
    /**
     * Set to true if atomics have to be simulated with mutexes.
     */
    static const bool simulated;

    /**
     * Atomic counter class.  Can be used to manipulate value of an
     * atomic counter without requiring explicit thread locking.
     * @author David Sugar <dyfet@gnutelephony.org>
     */
    class __EXPORT counter
    {
    private:
#ifdef  __GNUC__
        mutable volatile atomic_t value __attribute__ ((aligned(16)));
#else
        mutable volatile atomic_t value;
#endif

    public:
        counter(atomic_t initial = 0);

        // returns pre-modified values (fetch_and_change behavior)

        atomic_t operator++() volatile;
        atomic_t operator--() volatile;
        atomic_t operator+=(atomic_t offset) volatile;
        atomic_t operator-=(atomic_t offset) volatile;
        atomic_t get() const volatile;
        void clear() volatile;

        inline operator atomic_t() const volatile {
            return get();
        }

        inline atomic_t operator*() const volatile {
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

    /**
     * Atomically aligned heap alloc function.
     * @param size of memory to allocate.
     * @return pointer or NULL if cannot alloc.
     */
    void *alloc(size_t size);
};

} // namespace ucommon

#endif
