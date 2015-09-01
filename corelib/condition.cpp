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
#include <ucommon/object.h>
#include <ucommon/condition.h>
#include <ucommon/thread.h>
#include <ucommon/timers.h>
#include <ucommon/linked.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

#if _POSIX_PRIORITY_SCHEDULING > 0
#include <sched.h>
static int realtime_policy = SCHED_FIFO;
#endif

#undef  _POSIX_SPIN_LOCKS

static unsigned max_sharing = 0;

namespace ucommon {

#if !defined(_MSTHREADS_) && !defined(__PTH__)
Conditional::attribute Conditional::attr;
#endif

#ifdef  __PTH__
static int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    static pth_key_t ev_key = PTH_KEY_INIT;
    pth_event_t ev = pth_event(PTH_EVENT_TIME|PTH_MODE_STATIC, &ev_key,
        pth_time(abstime->tv_sec, (abstime->tv_nsec) / 1000));

    if(!pth_cond_await(cond, mutex, ev))
        return errno;
    return 0;
}
#endif


void ConditionalAccess::limit_sharing(unsigned max)
{
    max_sharing = max;
}

void Conditional::set(struct timespec *ts, timeout_t msec)
{
    assert(ts != NULL);

#if _POSIX_TIMERS > 0 && defined(POSIX_TIMERS)
    clock_gettime(_posix_clocking, ts);
#else
    timeval tv;
    gettimeofday(&tv, NULL);
    ts->tv_sec = tv.tv_sec;
    ts->tv_nsec = tv.tv_usec * 1000l;
#endif
    ts->tv_sec += msec / 1000;
    ts->tv_nsec += (msec % 1000) * 1000000l;
    while(ts->tv_nsec >= 1000000000l) {
        ++ts->tv_sec;
        ts->tv_nsec -= 1000000000l;
    }
}

#ifdef  _MSTHREADS_

Conditional::Conditional()
{
    InitializeCriticalSection(&mutex);
    InitializeConditionVariable(&cond);
}

Conditional::~Conditional()
{
    DeleteCriticalSection(&mutex);
}

void Conditional::wait(void)
{
    SleepConditionVariableCS(&cond, &mutex, INFINITE);
}

bool Conditional::wait(timeout_t timeout)
{
    if(SleepConditionVariableCS(&cond, &mutex, timeout))
        return true;

    return false;
}

void Conditional::signal(void)
{
    WakeConditionVariable(&cond);
}

void Conditional::broadcast(void)
{
    WakeAllConditionVariable(&cond);
}

bool Conditional::wait(struct timespec *ts)
{
    assert(ts != NULL);

    return wait((timeout_t)(ts->tv_sec * 1000 + (ts->tv_nsec / 1000000l)));
}

#else

#include <stdio.h>

#ifndef __PTH__
Conditional::attribute::attribute()
{
    Thread::init();
    pthread_condattr_init(&attr);
#if _POSIX_TIMERS > 0 && defined(HAVE_PTHREAD_CONDATTR_SETCLOCK) && defined(POSIX_TIMERS)
#if defined(_POSIX_MONOTONIC_CLOCK)
    if(!pthread_condattr_setclock(&attr, CLOCK_MONOTONIC))
        _posix_clocking = CLOCK_MONOTONIC;
#else
    pthread_condattr_setclock(&attr, CLOCK_REALTIME);
#endif
#endif
}
#endif

Conditional::Conditional()
{
#ifdef  __PTH__
    Thread::init();
    pth_cond_init(&cond);
    pth_mutex_init(&mutex);
#else
    crit(pthread_cond_init(&cond, &attr.attr) == 0, "conditional init failed");
    crit(pthread_mutex_init(&mutex, NULL) == 0, "mutex init failed");
#endif
}

Conditional::~Conditional()
{
#ifndef __PTH__
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
#endif
}

bool Conditional::wait(timeout_t timeout)
{
    struct timespec ts;
    set(&ts, timeout);
    return wait(&ts);
}

bool Conditional::wait(struct timespec *ts)
{
    assert(ts != NULL);

    if(pthread_cond_timedwait(&cond, &mutex, ts) == ETIMEDOUT)
        return false;

    return true;
}

#endif


#if defined(_MSTHREADS_)

ConditionalAccess::ConditionalAccess()
{
    waiting = pending = sharing = 0;
    InitializeConditionVariable(&bcast);
}

ConditionalAccess::~ConditionalAccess()
{
}

bool ConditionalAccess::waitBroadcast(timeout_t timeout)
{
    if(SleepConditionVariableCS(&bcast, &mutex, timeout))
        return true;

    return false;
}

void ConditionalAccess::waitBroadcast()
{
    SleepConditionVariableCS(&bcast, &mutex, INFINITE);
}

void ConditionalAccess::waitSignal()
{
    SleepConditionVariableCS(&cond, &mutex, INFINITE);
}

bool ConditionalAccess::waitSignal(timeout_t timeout)
{
    if(SleepConditionVariableCS(&cond, &mutex, timeout))
        return true;

    return false;
}

bool ConditionalAccess::waitBroadcast(struct timespec *ts)
{
    assert(ts != NULL);

    return waitBroadcast((timeout_t)(ts->tv_sec * 1000 + (ts->tv_nsec / 1000000l)));
}

bool ConditionalAccess::waitSignal(struct timespec *ts)
{
    assert(ts != NULL);

    return waitSignal((timeout_t)(ts->tv_sec * 1000 + (ts->tv_nsec / 1000000l)));
}

#else

ConditionalAccess::ConditionalAccess()
{
    waiting = pending = sharing = 0;
#ifdef  __PTH__
    pth_cond_init(&bcast);
#else
    crit(pthread_cond_init(&bcast, &attr.attr) == 0, "conditional init failed");
#endif
}

ConditionalAccess::~ConditionalAccess()
{
#ifndef __PTH__
    pthread_cond_destroy(&bcast);
#endif
}

bool ConditionalAccess::waitSignal(timeout_t timeout)
{
    struct timespec ts;
    set(&ts, timeout);
    return waitSignal(&ts);
}

bool ConditionalAccess::waitBroadcast(struct timespec *ts)
{
    assert(ts != NULL);

    if(pthread_cond_timedwait(&bcast, &mutex, ts) == ETIMEDOUT)
        return false;

    return true;
}

bool ConditionalAccess::waitBroadcast(timeout_t timeout)
{
    struct timespec ts;
    set(&ts, timeout);
    return waitBroadcast(&ts);
}

bool ConditionalAccess::waitSignal(struct timespec *ts)
{
    assert(ts != NULL);

    if(pthread_cond_timedwait(&cond, &mutex, ts) == ETIMEDOUT)
        return false;

    return true;
}

#endif

void ConditionalAccess::modify(void)
{
    lock();
    while(sharing) {
        ++pending;
        waitSignal();
        --pending;
    }
}

void ConditionalAccess::commit(void)
{
    if(pending)
        signal();
    else if(waiting)
        broadcast();
    unlock();
}

void ConditionalAccess::access(void)
{
    lock();
    assert(!max_sharing || sharing < max_sharing);
    while(pending) {
        ++waiting;
        waitBroadcast();
        --waiting;
    }
    ++sharing;
    unlock();
}

void ConditionalAccess::release(void)
{
   lock();
    assert(sharing);

    --sharing;
    if(pending && !sharing)
        signal();
    else if(waiting && !pending)
        broadcast();
    unlock();
}

ConditionalLock::ConditionalLock() :
ConditionalAccess()
{
    contexts = NULL;
}

ConditionalLock::~ConditionalLock()
{
    linked_pointer<Context> cp = contexts, next;
    while(cp) {
        next = cp->getNext();
        delete *cp;
        cp = next;
    }
}

ConditionalLock::Context *ConditionalLock::getContext(void)
{
    Context *slot = NULL;
    pthread_t tid = Thread::self();
    linked_pointer<Context> cp = contexts;

    while(cp) {
        if(cp->count && Thread::equal(cp->thread, tid))
            return *cp;
        if(!cp->count)
            slot = *cp;
        cp.next();
    }
    if(!slot) {
        slot = new Context(&this->contexts);
        slot->count = 0;
    }
    slot->thread = tid;
    return slot;
}

void ConditionalLock::_share(void)
{
    access();
}

void ConditionalLock::_unlock(void)
{
    release();
}

void ConditionalLock::modify(void)
{
    Context *context;

    lock();
    context = getContext();

    assert(context && sharing >= context->count);

    sharing -= context->count;
    while(sharing) {
        ++pending;
        waitSignal();
        --pending;
    }
    ++context->count;
}

void ConditionalLock::commit(void)
{
    Context *context = getContext();
    --context->count;

    if(context->count) {
        sharing += context->count;
        unlock();
    }
    else
        ConditionalAccess::commit();
}

void ConditionalLock::release(void)
{
    Context *context;

    lock();
    context = getContext();
    assert(sharing && context && context->count > 0);
    --sharing;
    --context->count;
    if(pending && !sharing)
        signal();
    else if(waiting && !pending)
        broadcast();
    unlock();
}

void ConditionalLock::access(void)
{
    Context *context;
    lock();
    context = getContext();
    assert(context && (!max_sharing || sharing < max_sharing));

    // reschedule if pending exclusives to make sure modify threads are not
    // starved.

    ++context->count;

    while(context->count < 2 && pending) {
        ++waiting;
        waitBroadcast();
        --waiting;
    }
    ++sharing;
    unlock();
}

void ConditionalLock::exclusive(void)
{
    Context *context;

    lock();
    context = getContext();
    assert(sharing && context && context->count > 0);
    sharing -= context->count;
    while(sharing) {
        ++pending;
        waitSignal();
        --pending;
    }
}

void ConditionalLock::share(void)
{
    Context *context = getContext();
    assert(!sharing && context && context->count);
    sharing += context->count;
    unlock();
}

} // namespace ucommon
