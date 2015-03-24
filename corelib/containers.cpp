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
#include <ucommon/memory.h>
#include <ucommon/thread.h>
#include <ucommon/containers.h>
#include <string.h>

namespace ucommon {

LinkedAllocator::LinkedAllocator() : Conditional()
{
    freelist = NULL;
}

LinkedObject *LinkedAllocator::get(void)
{
    LinkedObject *node;

    __AUTOLOCK__

    node = freelist;
    if(node)
        freelist = freelist->getNext();
    return node;
}

LinkedObject *LinkedAllocator::get(timeout_t timeout)
{
    struct timespec ts;
    bool rtn = true;
    LinkedObject *node = NULL;

    if(timeout && timeout != Timer::inf)
        set(&ts, timeout);

    __AUTOLOCK__

    while(!freelist && rtn) {
        if(timeout == Timer::inf)
            Conditional::wait();
        else if(timeout)
            rtn = Conditional::wait(&ts);
        else
            rtn = false;
    }
    if(rtn && freelist) {
        node = freelist;
        if(node)
            freelist = freelist->getNext();
    }
    return node;
}

void LinkedAllocator::release(LinkedObject *node)
{
    __AUTOLOCK__

    node->enlist(&freelist);
    signal();
}

LinkedAllocator::operator bool() const
{
    bool rtn = false;

    __AUTOLOCK__

    if(freelist)
        rtn = true;
    return rtn;
}

bool LinkedAllocator::operator!() const
{
    bool rtn = false;

    __AUTOLOCK__

    if(!freelist)
        rtn = true;
    return rtn;
}

Buffer::Buffer(size_t osize, size_t c) :
Conditional()
{
    assert(osize > 0 && c > 0);

    bufsize = osize * c;
    objsize = osize;
    objcount = 0;
    limit = c;

    if(osize) {
        buf = (char *)malloc(bufsize);
        crit(buf != NULL, "buffer alloc failed");
    }
    else
        buf = NULL;

    head = tail = buf;
}

Buffer::~Buffer()
{
    if(buf)
        free(buf);
    buf = NULL;
}

unsigned Buffer::count(void) const
{
    unsigned bcount = 0;

    __AUTOLOCK__

    if(tail > head)
        bcount = (unsigned)((size_t)(tail - head) / objsize);
    else if(head > tail)
        bcount = (unsigned)((((buf + bufsize) - head) + (tail - buf)) / objsize);
    return bcount;
}

unsigned Buffer::size(void) const
{
    return bufsize / objsize;
}

void *Buffer::get(void)
{
    caddr_t dbuf;

    __AUTOLOCK__

    while(!objcount)
        wait();
    dbuf = head;
    return dbuf;
}

void *Buffer::invalid(void) const
{
    return NULL;
}

void *Buffer::peek(unsigned offset)
{
    caddr_t dbuf;

    __AUTOLOCK__

    if(offset >= objcount) {
        return invalid();
    }

    dbuf = head + (objsize * offset);
    if(dbuf >= buf + bufsize)
        dbuf -= bufsize;
    return dbuf;
}

void Buffer::copy(void *data)
{
    assert(data != NULL);

    void *ptr = get();
    memcpy(data, ptr, objsize);
    release();
}

bool Buffer::copy(void *data, timeout_t timeout)
{
    assert(data != NULL);

    void *ptr = get(timeout);
    if(!ptr)
        return false;

    memcpy(data, ptr, objsize);
    release();
    return true;
}

void *Buffer::get(timeout_t timeout)
{
    caddr_t dbuf = NULL;
    struct timespec ts;
    bool rtn = true;

    if(timeout && timeout != Timer::inf)
        set(&ts, timeout);

    __AUTOLOCK__

    while(!objcount && rtn) {
        if(timeout == Timer::inf)
            wait();
        else if(timeout)
            rtn = wait(&ts);
        else
            rtn = false;
    }
    if(objcount && rtn)
        dbuf = head;
    return dbuf;
}

void Buffer::release(void)
{
    __AUTOLOCK__

    head += objsize;
    if(head >= buf + bufsize)
        head = buf;
    --objcount;
    signal();
}

void Buffer::put(void *dbuf)
{
    assert(dbuf != NULL);

    __AUTOLOCK__

    while(objcount == limit)
        wait();

    memcpy(tail, dbuf, objsize);
    tail += objsize;

    if(tail >= (buf + bufsize))
        tail = buf;

    ++objcount;
    signal();
}

bool Buffer::put(void *dbuf, timeout_t timeout)
{
    assert(dbuf != NULL);

    bool rtn = true;
    struct timespec ts;

    if(timeout && timeout != Timer::inf)
        set(&ts, timeout);

    __AUTOLOCK__

    while(objcount == limit && rtn) {
        if(timeout == Timer::inf)
            wait();
        else if(timeout)
            rtn = wait(&ts);
        else
            rtn = false;
    }
    if(rtn && objcount < limit) {
        memcpy(tail, dbuf, objsize);
        tail += objsize;
        if(tail >= (buf + bufsize))
            tail = 0;
        ++objcount;
        signal();
    }
    return rtn;
}


Buffer::operator bool() const
{
    bool rtn = false;

    __AUTOLOCK__

    if(buf && head != tail)
        rtn = true;
    return rtn;
}

bool Buffer::operator!() const
{
    bool rtn = false;

    __AUTOLOCK__

    if(!buf || head == tail)
        rtn = true;
    return rtn;
}

Queue::member::member(Queue *q, ObjectProtocol *o) :
OrderedObject(q)
{
    assert(o != NULL);

    o->retain();
    object = o;
}

Queue::Queue(mempager *p, size_t size) :
OrderedIndex(), Conditional()
{
    assert(size > 0);

    pager = p;
    freelist = NULL;
    used = 0;
    limit = size;
}

Queue::~Queue()
{
    linked_pointer<member> mp;
    OrderedObject *next;

    if(pager)
        return;

    mp = freelist;
    while(is(mp)) {
        next = mp->getNext();
        delete *mp;
        mp = next;
    }

    mp = head;
    while(is(mp)) {
        next = mp->getNext();
        delete *mp;
        mp = next;
    }
}

bool Queue::remove(ObjectProtocol *o)
{
    assert(o != NULL);

    bool rtn = false;
    linked_pointer<member> node;

    __AUTOLOCK__

    node = begin();
    while(node) {
        if(node->object == o)
            break;
        node.next();
    }
    if(node) {
        --used;
        rtn = true;
        node->object->release();
        node->delist(this);
        node->LinkedObject::enlist(&freelist);
    }
    return rtn;
}

ObjectProtocol *Queue::lifo(timeout_t timeout)
{
    struct timespec ts;
    bool rtn = true;
    member *member;
    ObjectProtocol *obj = NULL;

    if(timeout && timeout != Timer::inf)
        set(&ts, timeout);

    __AUTOLOCK__

    while(!tail && rtn) {
        if(timeout == Timer::inf)
            Conditional::wait();
        else if(timeout)
            rtn = Conditional::wait(&ts);
        else
            rtn = false;
    }
    if(rtn && tail) {
        --used;
        member = static_cast<Queue::member *>(tail);
        obj = member->object;
        member->delist(this);
        member->LinkedObject::enlist(&freelist);
    }
    if(rtn)
        signal();
    return obj;
}

ObjectProtocol *Queue::fifo(timeout_t timeout)
{
    bool rtn = true;
    struct timespec ts;
    linked_pointer<member> node;
    ObjectProtocol *obj = NULL;

    if(timeout && timeout != Timer::inf)
        set(&ts, timeout);

    __AUTOLOCK__

    while(rtn && !head) {
        if(timeout == Timer::inf)
            Conditional::wait();
        else if(timeout)
            rtn = Conditional::wait(&ts);
        else
            rtn = false;
    }

    if(rtn && head) {
        --used;
        node = begin();
        obj = node->object;
        head = head->getNext();
        if(!head)
            tail = NULL;
        node->LinkedObject::enlist(&freelist);
    }
    if(rtn)
        signal();
    return obj;
}

ObjectProtocol *Queue::invalid(void) const
{
    return NULL;
}

ObjectProtocol *Queue::get(unsigned back)
{
    linked_pointer<member> node;
    ObjectProtocol *obj;

    __AUTOLOCK__

    node = begin();

    do {
        if(!is(node)) {
            obj = invalid();
            break;
        }
        obj = node->object;
        node.next();

    } while(back-- > 0);

    return obj;
}

bool Queue::post(ObjectProtocol *object, timeout_t timeout)
{
    assert(object != NULL);

    bool rtn = true;
    struct timespec ts;
    LinkedObject *mem;

    if(timeout && timeout != Timer::inf)
        set(&ts, timeout);

    __AUTOLOCK__

    while(rtn && limit && used == limit) {
        if(timeout == Timer::inf)
            Conditional::wait();
        else if(timeout)
            rtn = Conditional::wait(&ts);
        else
            rtn = false;
    }

    if(!rtn) {
        return false;
    }

    ++used;
    if(freelist) {
        mem = freelist;
        freelist = freelist->getNext();
        new((void *)mem) member(this, object);
    }
    else {
        if(pager)
            new((void *)(pager->alloc(sizeof(member)))) member(this, object);
        else
            new member(this, object);
    }
    signal();
    return true;
}

size_t Queue::count(void) const
{
    size_t qcount;

    __AUTOLOCK__
    qcount = used;
    return qcount;
}


Stack::member::member(Stack *S, ObjectProtocol *o) :
LinkedObject((&S->usedlist))
{
    assert(o != NULL);

    o->retain();
    object = o;
}

Stack::Stack(mempager *p, size_t size) :
Conditional()
{
    assert(size > 0);

    pager = p;
    freelist = usedlist = NULL;
    limit = size;
    used = 0;
}

Stack::~Stack()
{
    linked_pointer<member> mp;
    LinkedObject *next;

    if(pager)
        return;

    mp = freelist;
    while(is(mp)) {
        next = mp->getNext();
        delete *mp;
        mp = next;
    }

    mp = usedlist;
    while(is(mp)) {
        next = mp->getNext();
        delete *mp;
        mp = next;
    }
}

bool Stack::remove(ObjectProtocol *o)
{
    assert(o != NULL);

    bool rtn = false;
    linked_pointer<member> node;

    __AUTOLOCK__

    node = static_cast<member*>(usedlist);
    while(node) {
        if(node->object == o)
            break;
        node.next();
    }
    if(node) {
        --used;
        rtn = true;
        node->object->release();
        node->delist(&usedlist);
        node->enlist(&freelist);
    }
    return rtn;
}

ObjectProtocol *Stack::invalid(void) const
{
    return NULL;
}

ObjectProtocol *Stack::get(unsigned back)
{
    linked_pointer<member> node;
    ObjectProtocol *obj;

    __AUTOLOCK__

    node = usedlist;

    do {
        if(!is(node)) {
            obj = invalid();
            break;
        }
        obj = node->object;
        node.next();

    } while(back-- > 0);

    return obj;
}

const ObjectProtocol *Stack::peek(timeout_t timeout)
{
    bool rtn = true;
    struct timespec ts;
    member *member;
    ObjectProtocol *obj = NULL;

    if(timeout && timeout != Timer::inf)
        set(&ts, timeout);

    __AUTOLOCK__

    while(rtn && !usedlist) {
        if(timeout == Timer::inf)
            Conditional::wait();
        else if(timeout)
            rtn = Conditional::wait(&ts);
        else
            rtn = false;
    }

    if(!rtn) {
        return NULL;
    }
    if(usedlist) {
        member = static_cast<Stack::member *>(usedlist);
        obj = member->object;
    }
    if(rtn)
        signal();
    return obj;
}

ObjectProtocol *Stack::pull(timeout_t timeout)
{
    bool rtn = true;
    struct timespec ts;
    member *member;
    ObjectProtocol *obj = NULL;

    if(timeout && timeout != Timer::inf)
        set(&ts, timeout);

    __AUTOLOCK__

    while(rtn && !usedlist) {
        if(timeout == Timer::inf)
            Conditional::wait();
        else if(timeout)
            rtn = Conditional::wait(&ts);
        else
            rtn = false;
    }
    if(!rtn) {
        return NULL;
    }
    if(usedlist) {
        member = static_cast<Stack::member *>(usedlist);
        obj = member->object;
        usedlist = member->getNext();
        member->enlist(&freelist);
    }
    if(rtn)
        signal();
    return obj;
}

bool Stack::push(ObjectProtocol *object, timeout_t timeout)
{
    assert(object != NULL);

    bool rtn = true;
    struct timespec ts;
    LinkedObject *mem;

    if(timeout && timeout != Timer::inf)
        set(&ts, timeout);

    __AUTOLOCK__

    while(rtn && limit && used == limit) {
        if(timeout == Timer::inf)
            Conditional::wait();
        else if(timeout)
            rtn = Conditional::wait(&ts);
        else
            rtn = false;
    }

    if(!rtn) {
        return false;
    }

    ++used;
    if(freelist) {
        mem = freelist;
        freelist = freelist->getNext();
        new((void *)mem) member(this, object);
    }
    else {
        if(pager) {
            void *ptr = (caddr_t)pager->alloc(sizeof(member));
            new(ptr) member(this, object);
        }
        else
            new member(this, object);
    }
    signal();
    return true;
}

size_t Stack::count(void) const
{
    size_t scount;

    __AUTOLOCK__

    scount = used;
    return scount;
}

} // namespace ucommon
