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
#include <ucommon/access.h>

namespace ucommon {

UnlockAccess::~UnlockAccess()
{
}

SharedAccess::~SharedAccess()
{
}

ExclusiveAccess::~ExclusiveAccess()
{
}

void SharedAccess::exclusive(void)
{
}

void SharedAccess::share(void)
{
}

SharedAccess::Locking::Locking(SharedAccess *obj)
{
    assert(obj != NULL);
    lock = obj;
    modify = false;
    state = 0;
    lock->shared_lock();
}

SharedAccess::Locking::Locking(const Locking& copy)
{
    assert(copy.modify == false);

    lock = copy.lock;
    modify = false;
    state = 0;
    if(lock)           
        lock->shared_lock();
}

SharedAccess::Locking& SharedAccess::Locking::operator=(const Locking& copy)
{
    assert(copy.modify == false);

    release();
    lock = copy.lock;
    state = 0;
    if(lock)
        lock->shared_lock();

    return *this;
}

ExclusiveAccess::Locking::Locking(ExclusiveAccess *obj)
{
    assert(obj != NULL);
    lock = obj;
    lock->exclusive_lock();
}

SharedAccess::Locking::~Locking()
{
    if(lock) {
        if(modify)
            lock->share();
        lock->release_share();
        lock = NULL;
        modify = false;
    }
}

ExclusiveAccess::Locking::~Locking()
{
    if(lock) {
        lock->release_exclusive();
        lock = NULL;
    }
}

void SharedAccess::Locking::release()
{
    if(lock) {
        if(modify)
            lock->share();
        lock->release_share();
        lock = NULL;
        modify = false;
    }
}

void ExclusiveAccess::Locking::release()
{
    if(lock) {
        lock->release_exclusive();
        lock = NULL;
    }
}

void SharedAccess::Locking::exclusive(void)
{
    if(lock && !modify) {
        lock->exclusive();
        modify = true;
    }
}

void SharedAccess::Locking::share(void)
{
    if(lock && modify) {
        lock->share();
        modify = false;
    }
}

} // namespace ucommon
