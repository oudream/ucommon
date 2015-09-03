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

namespace ucommon {

#if _POSIX_TIMERS > 0 && defined(POSIX_TIMERS)
extern int _posix_clocking;
int _posix_clocking = CLOCK_REALTIME;
#endif

struct mutex_entry
{
    pthread_mutex_t mutex;
    struct mutex_entry *next;
    const void *pointer;
    unsigned count;
};

class __LOCAL rwlock_entry : public RWLock
{
public:
    rwlock_entry();
    rwlock_entry *next;
    const void *object;
    unsigned count;
};

class __LOCAL mutex_index : public Mutex
{
public:
    struct mutex_entry *list;

    mutex_index();
};

class __LOCAL rwlock_index : public Mutex
{
public:
    rwlock_entry *list;

    rwlock_index();
};

static rwlock_index single_rwlock;
static rwlock_index *rwlock_table = &single_rwlock;
static mutex_index single_table;
static mutex_index *mutex_table = &single_table;
static unsigned mutex_indexing = 1;
static unsigned rwlock_indexing = 1;

#ifdef  __PTH__
static pth_key_t threadmap;
#else
#ifdef  _MSTHREADS_
static DWORD threadmap;
#else
static pthread_key_t threadmap;
#endif
#endif

mutex_index::mutex_index() : Mutex()
{
    list = NULL;
}

rwlock_index::rwlock_index() : Mutex()
{
    list = NULL;
}

rwlock_entry::rwlock_entry() : RWLock()
{
    count = 0;
}

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

static void pthread_shutdown(void)
{
    pth_kill();
}

inline pthread_t pthread_self(void)
    {return pth_self();};

#endif

static unsigned hash_address(const void *ptr, unsigned indexing)
{
    assert(ptr != NULL);

    unsigned key = 0;
    unsigned count = 0;
    const unsigned char *addr = (unsigned char *)(&ptr);

    if(indexing < 2)
        return 0;

    // skip lead zeros if little endian...
    while(count < sizeof(const void *) && *addr == 0) {
        ++count;
        ++addr;
    }

    while(count++ < sizeof(const void *) && *addr)
        key = (key << 1) ^ *(addr++);

    return key % indexing;
}

ReusableAllocator::ReusableAllocator() :
Conditional()
{
    freelist = NULL;
    waiting = 0;
}

void ReusableAllocator::release(ReusableObject *obj)
{
    assert(obj != NULL);

    LinkedObject **ru = (LinkedObject **)&freelist;

    obj->retain();
    obj->release();

    lock();
    obj->enlist(ru);

    if(waiting)
        signal();

    unlock();
}

#ifdef  _MSTHREADS_

bool Thread::equal(pthread_t t1, pthread_t t2)
{
    return t1 == t2;
}

#else

#include <stdio.h>
bool Thread::equal(pthread_t t1, pthread_t t2)
{
#ifdef  __PTH__
    return (t1 == t2);
#else
    return pthread_equal(t1, t2) != 0;
#endif
}

#endif

// abstract class never runs...
bool Thread::is_active(void) const
{
    return false;
}

bool JoinableThread::is_active(void) const
{
#ifdef  _MSTHREADS_
    return (running != INVALID_HANDLE_VALUE) && !joining;
#else
    return running && !joining;
#endif
}

bool DetachedThread::is_active(void) const
{
    return active;
}

RecursiveMutex::RecursiveMutex() :
Conditional()
{
    lockers = 0;
    waiting = 0;
}

void RecursiveMutex::_lock(void)
{
    lock();
}

void RecursiveMutex::_unlock(void)
{
    release();
}

bool RecursiveMutex::lock(timeout_t timeout)
{
    bool result = true;
    struct timespec ts;
    set(&ts, timeout);

    Conditional::lock();
    while(result && lockers) {
        if(Thread::equal(locker, pthread_self()))
            break;
        ++waiting;
        result = Conditional::wait(&ts);
        --waiting;
    }
    if(!lockers) {
        result = true;
        locker = pthread_self();
    }
    else
        result = false;
    ++lockers;
    Conditional::unlock();
    return result;
}

void RecursiveMutex::lock(void)
{
    Conditional::lock();
    while(lockers) {
        if(Thread::equal(locker, pthread_self()))
            break;
        ++waiting;
        Conditional::wait();
        --waiting;
    }
    if(!lockers)
        locker = pthread_self();
    ++lockers;
    Conditional::unlock();
    return;
}

void RecursiveMutex::release(void)
{
    Conditional::lock();
    --lockers;
    if(!lockers && waiting)
        Conditional::signal();
    Conditional::unlock();
}

RWLock::RWLock() :
ConditionalAccess()
{
    writers = 0;
}

bool RWLock::modify(timeout_t timeout)
{
    bool rtn = true;
    struct timespec ts;

    if(timeout && timeout != Timer::inf)
        set(&ts, timeout);

    lock();
    while((writers || sharing) && rtn) {
        if(writers && Thread::equal(writeid, pthread_self()))
            break;
        ++pending;
        if(timeout == Timer::inf)
            waitSignal();
        else if(timeout)
            rtn = waitSignal(&ts);
        else
            rtn = false;
        --pending;
    }
    assert(!max_sharing || writers < max_sharing);
    if(rtn) {
        if(!writers)
            writeid = pthread_self();
        ++writers;
    }
    unlock();
    return rtn;
}

bool RWLock::access(timeout_t timeout)
{
    struct timespec ts;
    bool rtn = true;

    if(timeout && timeout != Timer::inf)
        set(&ts, timeout);

    lock();
    while((writers || pending) && rtn) {
        ++waiting;
        if(timeout == Timer::inf)
            waitBroadcast();
        else if(timeout)
            rtn = waitBroadcast(&ts);
        else
            rtn = false;
        --waiting;
    }
    assert(!max_sharing || sharing < max_sharing);
    if(rtn)
        ++sharing;
    unlock();
    return rtn;
}

void RWLock::release(void)
{
    lock();
    assert(sharing || writers);

    if(writers) {
        assert(!sharing);
        --writers;
        if(pending && !writers)
            signal();
        else if(waiting && !writers)
            broadcast();
        unlock();
        return;
    }
    if(sharing) {
        assert(!writers);
        --sharing;
        if(pending && !sharing)
            signal();
        else if(waiting && !pending)
            broadcast();
    }
    unlock();
}

AutoProtect::AutoProtect()
{
    object = NULL;
}

AutoProtect::AutoProtect(const void *obj)
{
    object = obj;
    if(obj)
        Mutex::protect(object);
}

AutoProtect::~AutoProtect()
{
    release();
}

void AutoProtect::set(const void *obj)
{
    release();
    object = obj;
    if(obj)
        Mutex::protect(object);
}

void AutoProtect::release(void)
{
    if(object) {
        Mutex::release(object);
        object = NULL;
    }
}

Mutex::Mutex()
{
#ifdef  __PTH__
    pth_mutex_init(&mlock);
#else
    if(pthread_mutex_init(&mlock, NULL))
         __THROW_RUNTIME("mutex init failed");
#endif
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&mlock);
}

void Mutex::indexing(unsigned index)
{
    if(index > 1) {
        mutex_table = new mutex_index[index];
        mutex_indexing = index;
    }
}

void RWLock::indexing(unsigned index)
{
    if(index > 1) {
        rwlock_table = new rwlock_index[index];
        rwlock_indexing = index;
    }
}

RWLock::reader::reader()
{
    object = NULL;
}

RWLock::reader::reader(const void *obj)
{
    object = obj;
    if(obj)
        if(!lock(object))
            object = NULL;
}

RWLock::reader::~reader()
{
    release();
}

void RWLock::reader::set(const void *obj)
{
    release();
    object = obj;
    if(obj)
        if(!lock(object))
            object = NULL;
}

void RWLock::reader::release(void)
{
    if(object) {
        RWLock::release(object);
        object = NULL;
    }
}

RWLock::writer::writer()
{
    object = NULL;
}

RWLock::writer::writer(const void *obj)
{
    object = obj;
    if(obj)
        if(!lock(object))
            object = NULL;
}

RWLock::writer::~writer()
{
    release();
}

void RWLock::writer::set(const void *obj)
{
    release();
    object = obj;
    if(obj)
        if(!lock(object))
            object = NULL;
}

void RWLock::writer::release(void)
{
    if(object) {
        RWLock::release(object);
        object = NULL;
    }
}

bool RWLock::reader::lock(const void *ptr, timeout_t timeout)
{
    rwlock_index *index = &rwlock_table[hash_address(ptr, rwlock_indexing)];
    rwlock_entry *entry, *empty = NULL;

    if(!ptr)
        return false;

    index->acquire();
    entry = index->list;
    while(entry) {
        if(entry->count && entry->object == ptr)
            break;
        if(!entry->count)
            empty = entry;
        entry = entry->next;
    }
    if(!entry) {
        if(empty)
            entry = empty;
        else {
            entry = new rwlock_entry;
            entry->next = index->list;
            index->list = entry;
        }
    }
    entry->object = ptr;
    ++entry->count;
    index->release();
    if(entry->access(timeout))
        return true;
    index->acquire();
    --entry->count;
    index->release();
    return false;
}

bool RWLock::writer::lock(const void *ptr, timeout_t timeout)
{
    rwlock_index *index = &rwlock_table[hash_address(ptr, rwlock_indexing)];
    rwlock_entry *entry, *empty = NULL;

    if(!ptr)
        return false;

    index->acquire();
    entry = index->list;
    while(entry) {
        if(entry->count && entry->object == ptr)
            break;
        if(!entry->count)
            empty = entry;
        entry = entry->next;
    }
    if(!entry) {
        if(empty)
            entry = empty;
        else {
            entry = new rwlock_entry;
            entry->next = index->list;
            index->list = entry;
        }
    }
    entry->object = ptr;
    ++entry->count;
    index->release();
    if(entry->modify(timeout))
        return true;
    index->acquire();
    --entry->count;
    index->release();
    return false;
}

void RWLock::_lock(void)
{
    modify();
}

void RWLock::_share(void)
{
    access();
}

void RWLock::_unlock(void)
{
    release();
}

void RWLock::_unshare(void)
{
    release();
}

bool Mutex::protect(const void *ptr)
{
    mutex_index *index = &mutex_table[hash_address(ptr, mutex_indexing)];
    mutex_entry *entry, *empty = NULL;

    if(!ptr)
        return false;

    index->acquire();
    entry = index->list;
    while(entry) {
        if(entry->count && entry->pointer == ptr)
            break;
        if(!entry->count)
            empty = entry;
        entry = entry->next;
    }
    if(!entry) {
        if(empty)
            entry = empty;
        else {
            entry = new struct mutex_entry;
            entry->count = 0;
            pthread_mutex_init(&entry->mutex, NULL);
            entry->next = index->list;
            index->list = entry;
        }
    }
    entry->pointer = ptr;
    ++entry->count;
//  printf("ACQUIRE %p, THREAD %d, POINTER %p, COUNT %d\n", entry, Thread::self(), entry->pointer, entry->count);
    index->release();
    pthread_mutex_lock(&entry->mutex);
	return true;
}

bool RWLock::release(const void *ptr)
{
    rwlock_index *index = &rwlock_table[hash_address(ptr, rwlock_indexing)];
    rwlock_entry *entry;

    if(!ptr)
        return false;

    index->acquire();
    entry = index->list;
    while(entry) {
        if(entry->count && entry->object == ptr)
            break;
        entry = entry->next;
    }

	bool rtn = false;
    if(entry) {
		rtn = true;
        entry->release();
        --entry->count;
    }
    index->release();
	return rtn;
}

bool Mutex::release(const void *ptr)
{
    mutex_index *index = &mutex_table[hash_address(ptr, mutex_indexing)];
    mutex_entry *entry;

    if(!ptr)
        return false;

    index->acquire();
    entry = index->list;
    while(entry) {
        if(entry->count && entry->pointer == ptr)
            break;
        entry = entry->next;
    }

	bool rtn = false;
    if(entry) {
//      printf("RELEASE %p, THREAD %d, POINTER %p COUNT %d\n", entry, Thread::self(), entry->pointer, entry->count);
        pthread_mutex_unlock(&entry->mutex);
        --entry->count;
		rtn = true;
    }
    index->release();
	return rtn;
}

void Mutex::_lock(void)
{
    pthread_mutex_lock(&mlock);
}

void Mutex::_unlock(void)
{
    pthread_mutex_unlock(&mlock);
}

#ifdef  _MSTHREADS_

TimedEvent::TimedEvent() :
Timer()
{
    event = CreateEvent(NULL, FALSE, FALSE, NULL);
    InitializeCriticalSection((LPCRITICAL_SECTION)&mutex);
    set();
}

TimedEvent::~TimedEvent()
{
    if(event != INVALID_HANDLE_VALUE) {
        CloseHandle(event);
        event = INVALID_HANDLE_VALUE;
    }
    DeleteCriticalSection((LPCRITICAL_SECTION)&mutex);
}

TimedEvent::TimedEvent(timeout_t timeout) :
Timer(timeout)
{
    event = CreateEvent(NULL, FALSE, FALSE, NULL);
    InitializeCriticalSection((LPCRITICAL_SECTION)&mutex);
}

TimedEvent::TimedEvent(time_t timer) :
Timer(timer)
{
    event = CreateEvent(NULL, FALSE, FALSE, NULL);
    InitializeCriticalSection((LPCRITICAL_SECTION)&mutex);
}

void TimedEvent::signal(void)
{
    SetEvent(event);
}

void TimedEvent::reset(void)
{
    set();
    ResetEvent(event);
}

bool TimedEvent::sync(void)
{
    int result;
    timeout_t timeout;

    timeout = get();
    if(!timeout)
        return false;

    LeaveCriticalSection((LPCRITICAL_SECTION)&mutex);
    result = WaitForSingleObject(event, timeout);
    EnterCriticalSection((LPCRITICAL_SECTION)&mutex);
    if(result != WAIT_OBJECT_0)
        return true;
    return false;
}

bool TimedEvent::wait(timeout_t timer)
{
    int result;
    timeout_t timeout;

    if(timer)
        operator+=(timer);

    timeout = get();
    if(!timeout)
        return false;

    result = WaitForSingleObject(event, timeout);
    if(result == WAIT_OBJECT_0)
        return true;
    return false;
}

void TimedEvent::wait(void)
{
    WaitForSingleObject(event, INFINITE);
}

#else

TimedEvent::TimedEvent() :
Timer()
{
    signalled = false;
#ifdef  __PTH__
    Thread::init();
    pth_cond_init(&cond);
    pth_mutex_init(&mutex);
#else
    if(pthread_cond_init(&cond, Conditional::initializer()))
        __THROW_RUNTIME("conditional init failed");
    if(pthread_mutex_init(&mutex, NULL))
        __THROW_RUNTIME("mutex init failed");
#endif
    set();
}

TimedEvent::TimedEvent(timeout_t timeout) :
Timer(timeout)
{
    signalled = false;
#ifdef  __PTH__
    Thread::init();
    pth_cond_init(&cond);
    pth_mutex_init(&mutex);
#else
    if(pthread_cond_init(&cond, Conditional::initializer()))
        __THROW_RUNTIME("conditional init failed");
    if(pthread_mutex_init(&mutex, NULL))
        __THROW_RUNTIME("mutex init failed");
#endif
}

TimedEvent::TimedEvent(time_t timer) :
Timer(timer)
{
    signalled = false;
#ifdef  __PTH__
    Thread::init();
    pth_cond_init(&cond);
    pth_mutex_init(&mutex);
#else
    if(pthread_cond_init(&cond, Conditional::initializer()))
        __THROW_RUNTIME("conditional init failed");
    if(pthread_mutex_init(&mutex, NULL))
        __THROW_RUNTIME("mutex init failed");
#endif
}

TimedEvent::~TimedEvent()
{
#ifndef __PTH__
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
#endif
}

void TimedEvent::reset(void)
{
    pthread_mutex_lock(&mutex);
    signalled = false;
    set();
    pthread_mutex_unlock(&mutex);
}

void TimedEvent::signal(void)
{
    pthread_mutex_lock(&mutex);
    signalled = true;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

bool TimedEvent::sync(void)
{
    timeout_t timeout = get();
    struct timespec ts;

    if(signalled) {
        signalled = false;
        return true;
    }

    if(!timeout)
        return false;

    Conditional::set(&ts, timeout);

    if(pthread_cond_timedwait(&cond, &mutex, &ts) == ETIMEDOUT)
        return false;

    signalled = false;
    return true;
}

void TimedEvent::wait(void)
{
    pthread_mutex_lock(&mutex);
    if(signalled)
        signalled = false;
    else
        pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
}

bool TimedEvent::wait(timeout_t timeout)
{
    bool result = true;

    pthread_mutex_lock(&mutex);
    operator+=(timeout);
    result = sync();
    pthread_mutex_unlock(&mutex);
    return result;
}
#endif

void TimedEvent::lock(void)
{
    pthread_mutex_lock(&mutex);
}

void TimedEvent::release(void)
{
    pthread_mutex_unlock(&mutex);
}

Thread::Thread(size_t size)
{
    stack = (stacksize_t)size;
    priority = 0;
#ifdef  _MSTHREADS_
    cancellor = INVALID_HANDLE_VALUE;
#else
    cancellor = NULL;
#endif
    init();
}

#if defined(_MSTHREADS_)

void Thread::setPriority(void)
{
    HANDLE hThread = GetCurrentThread();
    priority += THREAD_PRIORITY_NORMAL;
    if(priority < THREAD_PRIORITY_LOWEST)
        priority = THREAD_PRIORITY_LOWEST;
    else if(priority > THREAD_PRIORITY_HIGHEST)
        priority = THREAD_PRIORITY_HIGHEST;

    SetThreadPriority(hThread, priority);
}
#elif _POSIX_PRIORITY_SCHEDULING > 0

void Thread::setPriority(void)
{
#ifndef __PTH__
    int policy;
    struct sched_param sp;
    pthread_t ptid = pthread_self();
    int pri = 0;

    if(!priority)
        return;

    if(pthread_getschedparam(ptid, &policy, &sp))
        return;

    if(priority > 0) {
        policy = realtime_policy;
        if(realtime_policy == SCHED_OTHER)
            pri = sp.sched_priority + priority;
        else
            pri = sched_get_priority_min(policy) + priority;
        policy = realtime_policy;
        if(pri > sched_get_priority_max(policy))
            pri = sched_get_priority_max(policy);
    } else if(priority < 0) {
        pri = sp.sched_priority - priority;
        if(pri < sched_get_priority_min(policy))
            pri = sched_get_priority_min(policy);
    }

    sp.sched_priority = pri;
    pthread_setschedparam(ptid, policy, &sp);
#endif
}

#else
void Thread::setPriority(void) {}
#endif

void Thread::concurrency(int level)
{
#if defined(HAVE_PTHREAD_SETCONCURRENCY) && !defined(_MSTHREADS_)
    pthread_setconcurrency(level);
#endif
}

void Thread::policy(int polid)
{
#if _POSIX_PRIORITY_SCHEDULING > 0
    realtime_policy = polid;
#endif
}

JoinableThread::JoinableThread(size_t size)
{
#ifdef  _MSTHREADS_
    cancellor = INVALID_HANDLE_VALUE;
    running = INVALID_HANDLE_VALUE;
    joining = false;
#else
    joining = false;
    running = false;
    cancellor = NULL;
#endif
    stack = (stacksize_t)size;
}

DetachedThread::DetachedThread(size_t size)
{
#ifdef  _MSTHREADS_
    cancellor = INVALID_HANDLE_VALUE;
#else
    cancellor = NULL;
#endif
    active = false;
    stack = (stacksize_t)size;
}

void Thread::sleep(timeout_t timeout)
{
#if defined(__PTH__)
    pth_usleep(timeout * 1000);
#elif defined(_MSTHREADS_)
	::Sleep(timeout);
#elif defined(HAVE_PTHREAD_DELAY)
    timespec ts;
    ts.tv_sec = timeout / 1000l;
    ts.tv_nsec = (timeout % 1000l) * 1000000l;
    pthread_delay(&ts);
#elif defined(HAVE_PTHREAD_DELAY_NP)
    timespec ts;
    ts.tv_sec = timeout / 1000l;
    ts.tv_nsec = (timeout % 1000l) * 1000000l;
    pthread_delay_np(&ts);
#else
    usleep(timeout * 1000);
#endif
}

void Thread::yield(void)
{
#if defined(_MSTHREADS_)
    SwitchToThread();
#elif defined(__PTH__)
    pth_yield(NULL);
#elif defined(HAVE_PTHREAD_YIELD_NP)
    pthread_yield_np();
#elif defined(HAVE_PTHREAD_YIELD)
    pthread_yield();
#else
    sched_yield();
#endif
}

void DetachedThread::exit(void)
{
    delete this;
    pthread_exit(NULL);
}

Thread::~Thread()
{
}

JoinableThread::~JoinableThread()
{
    join();
}

DetachedThread::~DetachedThread()
{
}

extern "C" {
#ifdef  _MSTHREADS_
    static unsigned __stdcall exec_thread(void *obj) {
        assert(obj != NULL);

        Thread *th = static_cast<Thread *>(obj);
        th->setPriority();
        th->run();
        th->exit();
        return 0;
    }
#else
    static void *exec_thread(void *obj)
    {
        assert(obj != NULL);

        Thread *th = static_cast<Thread *>(obj);
        th->setPriority();
        th->run();
        th->exit();
        return NULL;
    }
#endif
}

#ifdef  _MSTHREADS_
void JoinableThread::start(int adj)
{
    if(running != INVALID_HANDLE_VALUE)
        return;

    priority = adj;

    if(stack == 1)
        stack = 1024;

    joining = false;
    running = (HANDLE)_beginthreadex(NULL, stack, &exec_thread, this, 0, (unsigned int *)&tid);
    if(!running)
        running = INVALID_HANDLE_VALUE;
}

void DetachedThread::start(int adj)
{
    HANDLE hThread;;

    priority = adj;

    if(stack == 1)
        stack = 1024;

    hThread = (HANDLE)_beginthreadex(NULL, stack, &exec_thread, this, 0, (unsigned int *)&tid);
    if(hThread != INVALID_HANDLE_VALUE)
        active = true;
    CloseHandle(hThread);
}

void JoinableThread::join(void)
{
    pthread_t self = pthread_self();
    int rc;

    // already joined, so we ignore...
    if(running == INVALID_HANDLE_VALUE)
        return;

    // self join does cleanup...
    if(equal(tid, self)) {
        CloseHandle(running);
        running = INVALID_HANDLE_VALUE;
        Thread::exit();
    }

    joining = true;
    rc = WaitForSingleObject(running, INFINITE);
    if(rc == WAIT_OBJECT_0 || rc == WAIT_ABANDONED) {
        CloseHandle(running);
        running = INVALID_HANDLE_VALUE;
    }
}

#else

void JoinableThread::start(int adj)
{
    int result;

    if(running)
        return;

    joining = false;
    priority = adj;

#ifndef __PTH__
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
#if HAVE_PTHREAD_ATTR_SETINHRITSCHED
    pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
#endif
#endif
// we typically use "stack 1" for min stack...
#ifdef  PTHREAD_STACK_MIN
    if(stack && stack < PTHREAD_STACK_MIN)
        stack = PTHREAD_STACK_MIN;
#else
    if(stack && stack < 2)
        stack = 0;
#endif
#ifdef  __PTH__
    pth_attr_t attr = PTH_ATTR_DEFAULT;
    pth_attr_set(attr, PTH_ATTR_JOINABLE);
    tid = pth_spawn(attr, &exec_thread, this);
#else
    if(stack)
        pthread_attr_setstacksize(&attr, stack);
    result = pthread_create(&tid, &attr, &exec_thread, this);
    pthread_attr_destroy(&attr);
    if(!result)
        running = true;
#endif
}

void DetachedThread::start(int adj)
{
    priority = adj;
#ifndef __PTH__
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
#if HAVE_PTHREAD_ATTR_SETINHRITSCHED
    pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
#endif
#endif
// we typically use "stack 1" for min stack...
#ifdef  PTHREAD_STACK_MIN
    if(stack && stack < PTHREAD_STACK_MIN)
        stack = PTHREAD_STACK_MIN;
#else
    if(stack && stack < 2)
        stack = 0;
#endif
#ifdef  __PTH__
    tid = pth_spawn(PTH_ATTR_DEFAULT, &exec_thread, this);
#else
    if(stack)
        pthread_attr_setstacksize(&attr, stack);
    pthread_create(&tid, &attr, &exec_thread, this);
    pthread_attr_destroy(&attr);
#endif
    active = true;
}

void JoinableThread::join(void)
{
    pthread_t self = pthread_self();

    // already joined, so we ignore...
    if(!running)
        return;

    if(equal(tid, self)) {
        running = false;
        Thread::exit();
    }

    joining = true;

#ifdef  __PTH__
    if(pth_join(tid, NULL))
        running = false;
#else
    if(!pthread_join(tid, NULL))
        running = false;
#endif
}

#endif

void Thread::exit(void)
{
    pthread_exit(NULL);
}

void Thread::map(void)
{
    Thread::init();
#ifdef  __PTH__
    pth_key_setdata(threadmap, this);
#else
#ifdef  _MSTHREADS_
    TlsSetValue(threadmap, this);
#else
    pthread_setspecific(threadmap, this);
#endif
#endif
}

Thread *Thread::get(void)
{
#ifdef  __PTH__
    return (Thread *)pth_key_setdata(threadmap);
#else
#ifdef  _MSTHREADS_
    return (Thread *)TlsGetValue(threadmap);
#else
    return (Thread *)pthread_getspecific(threadmap);
#endif
#endif
}

void Thread::init(void)
{
    static volatile bool initialized = false;

    if(!initialized) {
#ifdef  __PTH__
        pth_init();
        pth_key_create(&threadmap, NULL);
        atexit(pthread_shutdown);
#else
#ifdef  _MSTHREADS_
        threadmap = TlsAlloc();
#else
        pthread_key_create(&threadmap, NULL);
#endif
#endif
        initialized = true;
    }
}

#ifdef  __PTH__
pthread_t Thread::self(void)
{
    return pth_self();
}
#else
pthread_t Thread::self(void)
{
    return pthread_self();
}
#endif

} // namespace ucommon
