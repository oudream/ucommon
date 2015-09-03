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
#include <ucommon/protocols.h>
#include <ucommon/object.h>
#include <ucommon/reuse.h>
#include <ucommon/thread.h>
#include <string.h>
#include <stdarg.h>

namespace ucommon {

ArrayReuse::ArrayReuse(size_t size, unsigned c, void *memory) :
ReusableAllocator()
{
    assert(c > 0 && size > 0 && memory != NULL);

    objsize = size;
    count = 0;
    limit = c;
    used = 0;
    mem = (caddr_t)memory;
}

ArrayReuse::ArrayReuse(size_t size, unsigned c) :
ReusableAllocator()
{
    assert(c > 0 && size > 0);

    objsize = size;
    count = 0;
    limit = c;
    used = 0;
    mem = (caddr_t)malloc(size * c);
    if(!mem)
        THROW_ALLOC();
}

ArrayReuse::~ArrayReuse()
{
    if(mem) {
        free(mem);
        mem = NULL;
    }
}

bool ArrayReuse::avail(void) const
{
    bool rtn = false;

    __AUTOLOCK(this);

    if(count < limit)
        rtn = true;
    return rtn;
}

ReusableObject *ArrayReuse::get(timeout_t timeout)
{
    bool rtn = true;
    struct timespec ts;
    ReusableObject *obj = NULL;

    if(timeout && timeout != Timer::inf)
        set(&ts, timeout);

    __AUTOLOCK(this);

    while(!freelist && used >= limit && rtn) {
        ++waiting;
        if(timeout == Timer::inf)
            wait();
        else if(timeout)
            rtn = wait(&ts);
        else
            rtn = false;
        --waiting;
    }

    if(!rtn) {
        return NULL;
    }

    if(freelist) {
        obj = freelist;
        freelist = next(obj);
    } else if(used < limit) {
        obj = (ReusableObject *)&mem[used * objsize];
        ++used;
    }
    if(obj)
        ++count;
    return obj;
}

ReusableObject *ArrayReuse::get(void)
{
    return get(Timer::inf);
}

ReusableObject *ArrayReuse::request(void)
{
    ReusableObject *obj = NULL;

    __AUTOLOCK(this);

    if(freelist) {
        obj = freelist;
        freelist = next(obj);
    }
    else if(used < limit) {
        obj = (ReusableObject *)(mem + (used * objsize));
        ++used;
    }
    if(obj)
        ++count;
    return obj;
}

PagerReuse::PagerReuse(mempager *p, size_t objsize, unsigned c) :
MemoryRedirect(p), ReusableAllocator()
{
    assert(objsize > 0 && c > 0);

    limit = c;
    count = 0;
    osize = objsize;
}

PagerReuse::~PagerReuse()
{
}

bool PagerReuse::avail(void) const
{
    bool rtn = false;

    __AUTOLOCK(this);

    if(!limit)
        return true;

    if(count < limit)
        rtn = true;
    return rtn;
}

ReusableObject *PagerReuse::request(void)
{
    ReusableObject *obj = NULL;

    __AUTOLOCK(this);

    if(!limit || count < limit) {
        if(freelist) {
            ++count;
            obj = freelist;
            freelist = next(obj);
        }
        else {
            ++count;
            return (ReusableObject *)_alloc(osize);
        }
    }
    return obj;
}

ReusableObject *PagerReuse::get(void)
{
    return get(Timer::inf);
}

ReusableObject *PagerReuse::get(timeout_t timeout)
{
    bool rtn = true;
    struct timespec ts;
    ReusableObject *obj;

    if(timeout && timeout != Timer::inf)
        set(&ts, timeout);

    __AUTOLOCK(this);

    while(rtn && limit && count >= limit) {
        ++waiting;
        if(timeout == Timer::inf)
            wait();
        else if(timeout)
            rtn = wait(&ts);
        else
            rtn = false;
        --waiting;
    }
    if(!rtn) {
        return NULL;
    }
    if(freelist) {
        obj = freelist;
        freelist = next(obj);
    }
    else {
        ++count;
        return (ReusableObject *)_alloc(osize);
    }
    if(obj)
        ++count;
    return obj;
}

} // namespace ucommon
