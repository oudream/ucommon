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

SharedProtocol::~SharedProtocol()
{
}

ExclusiveProtocol::~ExclusiveProtocol()
{
}

void SharedProtocol::exclusive(void)
{
}

void SharedProtocol::share(void)
{
}

SharedProtocol::Locking::Locking(SharedProtocol *obj)
{
    assert(obj != NULL);
    lock = obj;
    modify = false;
    state = 0;
    lock->_share();
}

SharedProtocol::Locking::Locking(const Locking& copy)
{
    assert(copy.modify == false);

    lock = copy.lock;
    modify = false;
    state = 0;
    if(lock)           
        lock->_share();
}

SharedProtocol::Locking& SharedProtocol::Locking::operator=(const Locking& copy)
{
    assert(copy.modify == false);

    release();
    lock = copy.lock;
    state = 0;
    if(lock)
        lock->_share();

    return *this;
}

ExclusiveProtocol::Locking::Locking(ExclusiveProtocol *obj)
{
    assert(obj != NULL);
    lock = obj;
    lock->_lock();
}

SharedProtocol::Locking::~Locking()
{
    if(lock) {
        if(modify)
            lock->share();
        lock->_unshare();
        lock = NULL;
        modify = false;
    }
}

ExclusiveProtocol::Locking::~Locking()
{
    if(lock) {
        lock->_unlock();
        lock = NULL;
    }
}

void SharedProtocol::Locking::release()
{
    if(lock) {
        if(modify)
            lock->share();
        lock->_unshare();
        lock = NULL;
        modify = false;
    }
}

void ExclusiveProtocol::Locking::release()
{
    if(lock) {
        lock->_unlock();
        lock = NULL;
    }
}

void SharedProtocol::Locking::exclusive(void)
{
    if(lock && !modify) {
        lock->exclusive();
        modify = true;
    }
}

void SharedProtocol::Locking::share(void)
{
    if(lock && modify) {
        lock->share();
        modify = false;
    }
}

} // namespace ucommon
