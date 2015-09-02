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
 * Condition classes for thread sychronization and timing.
 * The theory behind ucommon sychronization objects is that all upper level
 * sychronization objects can be formed directly from a mutex and conditional.
 * This includes semaphores, barriers, rwlock, our own specialized conditional
 * lock, resource-bound locking, and recursive exclusive locks.  Using only
 * conditionals means we are not dependent on platform specific pthread
 * implementations that may not implement some of these, and hence improves
 * portability and consistency.  Given that our rwlocks are recursive access
 * locks, one can safely create read/write threading pairs where the read
 * threads need not worry about deadlocks and the writers need not either if
 * they only write-lock one instance at a time to change state.
 * @file ucommon/condition.h
 */

#ifndef _UCOMMON_CONDITION_H_
#define _UCOMMON_CONDITION_H_

#ifndef _UCOMMON_CPR_H_
#include <ucommon/cpr.h>
#endif

#ifndef _UCOMMON_ACCESS_H_
#include <ucommon/access.h>
#endif

#ifndef _UCOMMON_TIMERS_H_
#include <ucommon/timers.h>
#endif

#ifndef _UCOMMON_MEMORY_H_
#include <ucommon/memory.h>
#endif

namespace ucommon {

/**
 * Condition Mutex to pair with conditionals.  Separating the mutex means
 * we can apply it either paired with a condition variable, or shared
 * among multiple condition variables.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT ConditionMutex
{
private:
    friend class ConditionVar;
    friend class autolock;

    __DELETE_COPY(ConditionMutex);

protected:
#if defined(_MSTHREADS_)
    mutable CRITICAL_SECTION mutex;
#else
    mutable pthread_mutex_t mutex;
#endif

public:
    /**
     * Initialize and construct conditional.
     */
    ConditionMutex();

    /**
     * Destroy conditional, release any blocked threads.
     */
    ~ConditionMutex();

#ifdef  _MSTHREADS_
    inline void lock(void)
        {EnterCriticalSection(&mutex);};

    inline void unlock(void)
        {LeaveCriticalSection(&mutex);};

#else
    /**
     * Lock the conditional's supporting mutex.
     */
    inline void lock(void)
        {pthread_mutex_lock(&mutex);}

    /**
     * Unlock the conditional's supporting mutex.
     */
    inline void unlock(void)
        {pthread_mutex_unlock(&mutex);}
#endif

    class __EXPORT autolock
    {
    private:
#ifdef  _MSTHREADS_
        CRITICAL_SECTION *mutex;
#else
        pthread_mutex_t *mutex;
#endif
        __DELETE_COPY(autolock);

    public:
        inline autolock(const ConditionMutex* object) {
            mutex = &object->mutex;
#ifdef _MSTHREADS_
            EnterCriticalSection(mutex);
#else
            pthread_mutex_lock(mutex);
#endif
        }

        inline ~autolock() {
#ifdef  _MSTHREADS_
            LeaveCriticalSection(mutex);
#else
            pthread_mutex_unlock(mutex);
#endif
        }
    };
};

/**
 * The condition Var allows multiple conditions to share a mutex.  This
 * can be used to form specialized thread synchronizing classes such as
 * ordered sempahores, or to create thread completion lists.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT ConditionVar
{
private:
    __DELETE_DEFAULTS(ConditionVar);

protected:
    friend class ConditionList;

#if defined(_MSTHREADS_)
    mutable CONDITION_VARIABLE cond;
#else
    mutable pthread_cond_t cond;
#endif
    ConditionMutex *shared;

public:
    /**
     * Initialize and construct conditional.
     */
    ConditionVar(ConditionMutex *mutex);

    /**
     * Destroy conditional, release any blocked threads.
     */
    ~ConditionVar();

    /**
     * Conditional wait for signal on millisecond timeout.
     * @param timeout in milliseconds.
     * @return true if signalled, false if timer expired.
     */
    bool wait(timeout_t timeout);

    /**
     * Conditional wait for signal on timespec timeout.
     * @param timeout as a high resolution timespec.
     * @return true if signalled, false if timer expired.
     */
    bool wait(struct timespec *timeout);

#ifdef  _MSTHREADS_
    void wait(void);
    void signal(void);
    void broadcast(void);

#else
    /**
     * Wait (block) until signalled.
     */
    inline void wait(void)
        {pthread_cond_wait(&cond, &shared->mutex);}

    /**
     * Signal the conditional to release one waiting thread.
     */
    inline void signal(void)
        {pthread_cond_signal(&cond);}

    /**
     * Signal the conditional to release all waiting threads.
     */
    inline void broadcast(void)
        {pthread_cond_broadcast(&cond);}
#endif
};

/**
 * The conditional is a common base for other thread synchronizing classes.
 * Many of the complex sychronization objects, including barriers, semaphores,
 * and various forms of read/write locks are all built from the conditional.
 * This assures that the minimum functionality to build higher order thread
 * synchronizing objects is a pure conditional, and removes dependencies on
 * what may be optional features or functions that may have different
 * behaviors on different pthread implimentations and platforms.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT Conditional : protected ConditionMutex
{
private:
    __DELETE_COPY(Conditional);

protected:
    friend class ConditionalAccess;
    friend class ConditionVar;

#if defined(_MSTHREADS_)
    mutable CONDITION_VARIABLE cond;
#else
#ifndef __PTH__
    class __LOCAL attribute
    {
    public:
        pthread_condattr_t attr;
        attribute();
    };

    __LOCAL static attribute attr;
#endif

    mutable pthread_cond_t cond;
#endif

    friend class TimedEvent;

    /**
     * Conditional wait for signal on millisecond timeout.
     * @param timeout in milliseconds.
     * @return true if signalled, false if timer expired.
     */
    bool wait(timeout_t timeout);

    /**
     * Conditional wait for signal on timespec timeout.
     * @param timeout as a high resolution timespec.
     * @return true if signalled, false if timer expired.
     */
    bool wait(struct timespec *timeout);

#ifdef  _MSTHREADS_
    void wait(void);
    void signal(void);
    void broadcast(void);

#else
    /**
     * Wait (block) until signalled.
     */
    inline void wait(void)
        {pthread_cond_wait(&cond, &mutex);}

    /**
     * Signal the conditional to release one waiting thread.
     */
    inline void signal(void)
        {pthread_cond_signal(&cond);}

    /**
     * Signal the conditional to release all waiting threads.
     */
    inline void broadcast(void)
        {pthread_cond_broadcast(&cond);}
#endif

    /**
     * Initialize and construct conditional.
     */
    Conditional();

    /**
     * Destroy conditional, release any blocked threads.
     */
    ~Conditional();

    friend class autolock;

public:
#if !defined(_MSTHREADS_) && !defined(__PTH__)
    /**
     * Support function for getting conditional attributes for realtime
     * scheduling.
     * @return attributes to use for creating realtime conditionals.
     */
    static inline pthread_condattr_t *initializer(void)
        {return &attr.attr;}
#endif

    /**
     * Convert a millisecond timeout into use for high resolution
     * conditional timers.
     * @param hires timespec representation to set.
     * @param timeout to convert.
     */
    static void set(struct timespec *hires, timeout_t timeout);
};

/**
 * The conditional rw seperates scheduling for optizming behavior or rw locks.
 * This varient of conditonal seperates scheduling read (broadcast wakeup) and
 * write (signal wakeup) based threads.  This is used to form generic rwlock's
 * as well as the specialized condlock.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT ConditionalAccess : private Conditional
{
private:
    __DELETE_COPY(ConditionalAccess);

protected:
#if defined _MSTHREADS_
    CONDITION_VARIABLE bcast;
#else
    mutable pthread_cond_t bcast;
#endif

    unsigned pending, waiting, sharing;

    /**
     * Conditional wait for signal on millisecond timeout.
     * @param timeout in milliseconds.
     * @return true if signalled, false if timer expired.
     */
    bool waitSignal(timeout_t timeout);

    /**
     * Conditional wait for broadcast on millisecond timeout.
     * @param timeout in milliseconds.
     * @return true if signalled, false if timer expired.
     */
    bool waitBroadcast(timeout_t timeout);


    /**
     * Conditional wait for signal on timespec timeout.
     * @param timeout as a high resolution timespec.
     * @return true if signalled, false if timer expired.
     */
    bool waitSignal(struct timespec *timeout);

    /**
     * Conditional wait for broadcast on timespec timeout.
     * @param timeout as a high resolution timespec.
     * @return true if signalled, false if timer expired.
     */
    bool waitBroadcast(struct timespec *timeout);

    /**
     * Convert a millisecond timeout into use for high resolution
     * conditional timers.
     * @param hires timespec representation to set.
     * @param timeout to convert.
     */
    inline static void set(struct timespec *hires, timeout_t timeout)
        {Conditional::set(hires, timeout);}


#ifdef  _MSTHREADS_
    inline void lock(void)
        {EnterCriticalSection(&mutex);};

    inline void unlock(void)
        {LeaveCriticalSection(&mutex);};

    void waitSignal(void);

    void waitBroadcast(void);

    inline void signal(void)
        {Conditional::signal();};

    inline void broadcast(void)
        {Conditional::broadcast();};

#else
    /**
     * Lock the conditional's supporting mutex.
     */
    inline void lock(void)
        {pthread_mutex_lock(&mutex);}

    /**
     * Unlock the conditional's supporting mutex.
     */
    inline void unlock(void)
        {pthread_mutex_unlock(&mutex);}

    /**
     * Wait (block) until signalled.
     */
    inline void waitSignal(void)
        {pthread_cond_wait(&cond, &mutex);}

    /**
     * Wait (block) until broadcast.
     */
    inline void waitBroadcast(void)
        {pthread_cond_wait(&bcast, &mutex);}


    /**
     * Signal the conditional to release one signalled thread.
     */
    inline void signal(void)
        {pthread_cond_signal(&cond);}

    /**
     * Signal the conditional to release all broadcast threads.
     */
    inline void broadcast(void)
        {pthread_cond_broadcast(&bcast);}
#endif
public:
    /**
     * Initialize and construct conditional.
     */
    ConditionalAccess();

    /**
     * Destroy conditional, release any blocked threads.
     */
    ~ConditionalAccess();

    /**
     * Access mode shared thread scheduling.
     */
    void access(void);

    /**
     * Exclusive mode write thread scheduling.
     */
    void modify(void);

    /**
     * Release access mode read scheduling.
     */
    void release(void);

    /**
     * Complete exclusive mode write scheduling.
     */
    void commit(void);

    /**
     * Specify a maximum sharing (access) limit.  This can be used
     * to detect locking errors, such as when aquiring locks that are
     * not released.
     * @param max sharing level.
     */
    void limit_sharing(unsigned max);
};

/**
 * An optimized and convertable shared lock.  This is a form of read/write
 * lock that has been optimized, particularly for shared access.  Support
 * for scheduling access around writer starvation is also included.  The
 * other benefits over traditional read/write locks is that the code is
 * a little lighter, and read (shared) locks can be converted to exclusive
 * (write) locks to perform brief modify operations and then returned to read
 * locks, rather than having to release and re-aquire locks to change mode.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT ConditionalLock : protected ConditionalAccess, public SharedAccess
{
private:
    __DELETE_COPY(ConditionalLock);

protected:
    class Context : public LinkedObject
    {
    private:
        __DELETE_COPY(Context);

    public:
        inline Context(LinkedObject **root) : LinkedObject(root) {}

        pthread_t thread;
        unsigned count;
    };

    LinkedObject *contexts;

    virtual void _share(void) __OVERRIDE;
    virtual void _unlock(void) __OVERRIDE;

    Context *getContext(void);

public:
    /**
     * Construct conditional lock for default concurrency.
     */
    ConditionalLock();

    /**
     * Destroy conditional lock.
     */
    ~ConditionalLock();

    /**
     * Acquire write (exclusive modify) lock.
     */
    void modify(void);

    /**
     * Commit changes / release a modify lock.
     */
    void commit(void);

    /**
     * Acquire access (shared read) lock.
     */
    void access(void);

    /**
     * Release a shared lock.
     */
    void release(void);

    /**
     * Convert read lock into exclusive (write/modify) access.  Schedule
     * when other readers sharing.
     */
    virtual void exclusive(void);

    /**
     * Return an exclusive access lock back to share mode.
     */
    virtual void share(void);
};

/**
 * A portable implementation of "barrier" thread sychronization.  A barrier
 * waits until a specified number of threads have all reached the barrier,
 * and then releases all the threads together.  This implementation works
 * regardless of whether the thread library supports barriers since it is
 * built from conditional.  It also differs in that the number of threads
 * required can be changed dynamically at runtime, unlike pthread barriers
 * which, when supported, have a fixed limit defined at creation time.  Since
 * we use conditionals, another feature we can add is optional support for a
 * wait with timeout.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT Barrier : private Conditional
{
private:
    unsigned count;
    unsigned waits;

    __DELETE_DEFAULTS(Barrier);

public:
    /**
     * Construct a barrier with an initial size.
     * @param count of threads required.
     */
    Barrier(unsigned count);

    /**
     * Destroy barrier and release pending threads.
     */
    ~Barrier();

    /**
     * Dynamically alter the number of threads required.  If the size is
     * set below the currently waiting threads, then the barrier releases.
     * @param count of threads required.
     */
    void set(unsigned count);

    /**
     * Dynamically increment the number of threads required.
     */
    void inc(void);

    /**
     * Reduce the number of threads required.
     */
    void dec(void);

    /**
     * Alternative prefix form of the same increment operation.
     * @return the current amount of threads.
     */
    unsigned operator++(void);

    unsigned operator--(void);

    /**
     * Wait at the barrier until the count of threads waiting is reached.
     */
    void wait(void);

    /**
     * Wait at the barrier until either the count of threads waiting is
     * reached or a timeout has occurred.
     * @param timeout to wait in milliseconds.
     * @return true if barrier reached, false if timer expired.
     */
    bool wait(timeout_t timeout);
};

/**
 * A portable counting semaphore class.  A semaphore will allow threads
 * to pass through it until the count is reached, and blocks further threads.
 * Unlike pthread semaphore, our semaphore class supports it's count limit
 * to be altered during runtime and the use of timed waits.  This class also
 * implements the shared_lock protocol.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT Semaphore : public SharedAccess, protected Conditional
{
protected:
    unsigned count, waits, used;

    virtual void _share(void) __OVERRIDE;
    virtual void _unlock(void) __OVERRIDE;

    __DELETE_COPY(Semaphore);

public:
    /**
     * Construct a semaphore with an initial count of threads to permit.
     * @param count of threads to permit, or special case 0 group release.
     */
    Semaphore(unsigned count = 0);

    /**
     * Alternate onstructor with ability to preset available slots.
     * @param count of threads to permit.
     * @param avail instances not pre-locked.
     */
    Semaphore(unsigned count, unsigned avail);

    /**
     * Wait until the semphore usage count is less than the thread limit.
     * Increase used count for our thread when unblocked.
     */
    void wait(void);

    /**
     * Wait until the semphore usage count is less than the thread limit.
     * Increase used count for our thread when unblocked, or return without
     * changing if timed out.
     * @param timeout to wait in millseconds.
     * @return true if success, false if timeout.
     */
    bool wait(timeout_t timeout);

    /**
     * Alter semaphore limit at runtime
     * @param count of threads to allow.
     */
    void set(unsigned count);

    /**
     * Release the semaphore after waiting for it.
     */
    void release(void);

    /**
     * Convenience operator to wait on a counting semaphore.
     */
    inline void operator++(void)
        {wait();}

    /**
     * Convenience operator to release a counting semaphore.
     */
    inline void operator--(void)
        {release();}
};

/**
 * Convenience type for using conditional locks.
 */
typedef ConditionalLock condlock_t;

/**
 * Convenience type for scheduling access.
 */
typedef ConditionalAccess accesslock_t;

/**
 * Convenience type for using counting semaphores.
 */
typedef Semaphore semaphore_t;

/**
 * Convenience type for using thread barriers.
 */
typedef Barrier barrier_t;

} // namespace ucommon

#endif
