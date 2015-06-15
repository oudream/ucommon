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

namespace ucommon {

atomic::counter::counter(long init)
{
    value = init;
}

atomic::spinlock::spinlock()
{
    value = 0;
}

#if __GNUC__ > 3

long atomic::counter::get()
{
    return __atomic_load_n(&value, __ATOMIC_RELAXED);
}

long atomic::counter::operator++()
{
    return __sync_fetch_and_add(&value, 1);
}

long atomic::counter::operator--()
{
    return __sync_fetch_and_sub(&value, 1);
}

long atomic::counter::operator+=(long change)
{
    return __sync_fetch_and_add(&value, change);
}

long atomic::counter::operator-=(long change)
{
    return __sync_fetch_and_sub(&value, change);
}

bool atomic::spinlock::acquire(void)
{
    // if not locked by another already, then we acquired it...
    return (__sync_lock_test_and_set(&value, 1) == 0);
}

void atomic::spinlock::wait(void)
{
    while (__sync_lock_test_and_set(&value, 1)) {
        while (value)
            ;
    }
}

void atomic::spinlock::release(void)
{
    __sync_lock_release(&value);
}

#else

#define SIMULATED true

long atomic::counter::operator++()
{
    long rval;
    Mutex::protect((void *)&value);
    rval = value;
    value++;
    Mutex::release((void *)&value);
    return rval;
}

long atomic::counter::operator--()
{
    long rval;
    Mutex::protect((void *)&value);
    rval = value;
    value--;
    Mutex::release((void *)&value);
    return rval;
}

long atomic::counter::operator+=(long change)
{
    long rval;
    Mutex::protect((void *)&value);
    rval = value;
    value += change;
    Mutex::release((void *)&value);
    return rval;
}

long atomic::counter::operator-=(long change)
{
    long rval;
    Mutex::protect((void *)&value);
    rval = value;
    value -= change;
    Mutex::release((void *)&value);
    return rval;
}

bool atomic::spinlock::acquire(void)
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

void atomic::spinlock::release(void)
{
    Mutex::protect((void *)&value);
    value = 0;
    Mutex::release((void *)&value);
}

#endif

#ifdef SIMULATED
const bool atomic::simulated = true;
#else
const bool atomic::simulated = false;
#endif

} // namespace ucommon
