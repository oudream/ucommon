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
 * Thread classes and sychronization objects.
 * The theory behind ucommon thread classes is that they would be used
 * to create derived classes where thread-specific data can be stored as
 * member data of the derived class.  The run method is called when the
 * context is executed.  Since we use a pthread foundation, we support
 * both detached threads and joinable threads.  Objects based on detached
 * threads should be created with new, and will automatically delete when
 * the thread context exits.  Joinable threads will be joined with deleted.
 * @file ucommon/thread.h
 */

/**
 * An example of the thread queue class.  This may be relevant to producer-
 * consumer scenarios and realtime applications where queued messages are
 * stored on a re-usable object pool.
 * @example queue.cpp
 */

/**
 * A simple example of threading and join operation.
 * @example thread.cpp
 */

#ifndef _UCOMMON_THREAD_H_
#define _UCOMMON_THREAD_H_

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

#ifndef _UCOMMON_CONDITION_H_
#include <ucommon/condition.h>
#endif

namespace ucommon {

/**
 * A generic and portable implementation of Read/Write locking.  This
 * class implements classical read/write locking, including "timed" locks.
 * Support for scheduling threads to avoid writer starvation is also provided
 * for.  By building read/write locks from a conditional, we make them
 * available on pthread implemetations and other platforms which do not
 * normally include optional pthread rwlock's.  We also do not restrict
 * the number of threads that may use the lock.  Finally, both the exclusive
 * and shared protocols are implemented to support exclusive_lock and
 * shared_lock referencing.  Because of the thread locking semantics this
 * is part of thread rather than condition, and was originally called
 * ThreadLock in older ucommon/commoncpp releases.  Our autolock semantics
 * are also different as we protect a target object, not a rwlock instance.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT RWLock : private ConditionalAccess, public ExclusiveAccess, public SharedAccess
{
private:
    __DELETE_COPY(RWLock);

protected:
    unsigned writers;
    pthread_t writeid;

    virtual void _share(void) __OVERRIDE;
    
    virtual void _lock(void) __OVERRIDE;

    virtual void _unlock(void) __OVERRIDE;

    virtual void _unshare(void) __OVERRIDE;

public:
    typedef autoshared<RWLock> autoreader;

    typedef autoexclusive<RWLock> autowriter;

    /**
     * Apply automatic scope based access locking to objects.  The rwlock
     * is located from the rwlock pool rather than contained in the target
     * object, and the read lock is released when the guard object falls out of
     * scope.  This is essentially an automation mechanism for mutex::reader.
     * @author David Sugar <dyfet@gnutelephony.org>
     */
    class __EXPORT reader
    {
    private:
        const void *object;

        __DELETE_COPY(reader);

    public:
        /**
          * Create an unitialized instance of guard.  Usually used with a
          * guard = operator.
          */
        reader();

        /**
         * Construct a guard for a specific object.
         * @param object to guard.
         */
        reader(const void *object);

        /**
         * Release mutex when guard falls out of scope.
         */
        ~reader();

        /**
         * Set guard to mutex lock a new object.  If a lock is currently
         * held, it is released.
         * @param object to guard.
         */
        void set(const void *object);

        /**
         * Prematurely release a guard.
         */
        void release(void);

        /**
         * Set guard to read lock a new object.  If a lock is currently
         * held, it is released.
         * @param pointer to object to guard.
         */
        inline void operator=(const void *pointer)
            {set(pointer);}

       /**
        * Shared access to an arbitrary object.  This is based on the protect
        * function of mutex.
        * @param object to share.
        * @param timeout in milliseconds to wait for lock.
        * @return true if shared, false if timeout.
        */
        static bool lock(const void *object, timeout_t timeout = Timer::inf);
    };

    /**
     * Apply automatic scope based exclusive locking to objects.  The rwlock
     * is located from the rwlock pool rather than contained in the target
     * object, and the write lock is released when the guard object falls out of
     * scope.  This is essentially an automation mechanism for mutex::writer.
     * @author David Sugar <dyfet@gnutelephony.org>
     */
    class __EXPORT writer
    {
    private:
        const void *object;

        __DELETE_COPY(writer);

    public:
        /**
          * Create an unitialized instance of guard.  Usually used with a
          * guard = operator.
          */
        writer();

        /**
         * Construct a guard for a specific object.
         * @param object to guard.
         */
        writer(const void *object);

        /**
         * Release mutex when guard falls out of scope.
         */
        ~writer();

        /**
         * Set guard to mutex lock a new object.  If a lock is currently
         * held, it is released.
         * @param object to guard.
         */
        void set(const void *object);

        /**
         * Prematurely release a guard.
         */
        void release(void);

        /**
         * Set guard to read lock a new object.  If a lock is currently
         * held, it is released.
         * @param pointer to object to guard.
         */
        inline void operator=(const void *pointer)
            {set(pointer);}

        /**
        * Write protect access to an arbitrary object.  This is like the
        * protect function of mutex.
        * @param object to protect.
        * @param timeout in milliseconds to wait for lock.
        * @return true if locked, false if timeout.
        */
        static bool lock(const void *object, timeout_t timeout = Timer::inf);
    };

    /**
     * Create an instance of a rwlock.
     */
    RWLock();

    /**
     * Request modify (write) access through the lock.
     * @param timeout in milliseconds to wait for lock.
     * @return true if locked, false if timeout.
     */
    bool modify(timeout_t timeout = Timer::inf);

    /**
     * Request shared (read) access through the lock.
     * @param timeout in milliseconds to wait for lock.
     * @return true if locked, false if timeout.
     */
    bool access(timeout_t timeout = Timer::inf);

    /**
     * Specify hash table size for guard protection.  The default is 1.
     * This should be called at initialization time from the main thread
     * of the application before any other threads are created.
     * @param size of hash table used for guarding.
     */
    static void indexing(unsigned size);

    /**
     * Release an arbitrary object that has been protected by a rwlock.
     * @param object to release.
     */
    static bool release(const void *object);

    /**
     * Release the lock.
     */
    void release(void);
};

/**
 * Event notification to manage scheduled realtime threads.  The timer
 * is advanced to sleep threads which then wakeup either when the timer
 * has expired or they are notified through the signal handler.  This can
 * be used to schedule and signal one-time completion handlers or for time
 * synchronized events signaled by an asychrononous I/O or event source.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT TimedEvent : public Timer
{
private:
#ifdef _MSTHREADS_
    HANDLE event;
#else
    mutable pthread_cond_t cond;
    bool signalled;
#endif
    mutable pthread_mutex_t mutex;

    __DELETE_COPY(TimedEvent);

protected:
    /**
     * Lock the object for wait or to manipulate derived data.  This is
     * relevant to manipulations in a derived class.
     */
    void lock(void);

    /**
     * Release the object lock after waiting.  This is relevent to
     * manipulations in a derived class.
     */
    void release(void);

    /**
     * Wait while locked.  This can be used in more complex derived
     * objects where we are concerned with synchronized access between
     * the signaling and event thread.  This can be used in place of
     * wait, but lock and release methods must be used around it.
     * @return true if time expired.
     */
    bool sync(void);

public:
    /**
     * Create event handler and timer for timing of events.
     */
    TimedEvent(void);

    /**
     * Create event handler and timer set to trigger a timeout.
     * @param timeout in milliseconds.
     */
    TimedEvent(timeout_t timeout);

    /**
     * Create event handler and timer set to trigger a timeout.
     * @param timeout in seconds.
     */
    TimedEvent(time_t timeout);

    /**
     * Destroy timer and release pending events.
     */
    ~TimedEvent();

    /**
     * Signal pending event.  Object may be locked or unlocked.  The
     * signalling thread may choose to lock and check a condition in
     * a derived class before signalling.
     */
    void signal(void);

    /**
     * Wait to be signalled or until timer expires.  This is a wrapper for
     * expire for simple completion events.
     * @param timeout to wait from last reset.
     * @return true if signaled, false if timeout.
     */
    bool wait(timeout_t timeout);

    /**
     * A simple wait until triggered.
     */
    void wait(void);

    /**
     * Reset triggered conditional.
     */
    void reset(void);
};

/**
 * Portable recursive exclusive lock.  This class is built from the
 * conditional and hence does not require support for non-standard and
 * platform specific extensions to pthread mutex to support recrusive
 * style mutex locking.  The exclusive protocol is implimented to support
 * exclusive_lock referencing.
 */
class __EXPORT RecursiveMutex : private Conditional, public ExclusiveAccess
{
private:
    __DELETE_COPY(RecursiveMutex);

protected:
    unsigned waiting;
    unsigned lockers;
    pthread_t locker;

    virtual void _lock(void) __OVERRIDE;
    virtual void _unlock(void) __OVERRIDE;

public:
    typedef autoexclusive<RecursiveMutex> autolock;

    /**
     * Create rexlock.
     */
    RecursiveMutex();

    /**
     * Acquire or increase locking.
     */
    void lock(void);

    /**
     * Timed lock request.
     */
    bool lock(timeout_t timeout);

    /**
     * Release or decrease locking.
     */
    void release(void);
};

/**
 * Class for resource bound memory pools between threads.  This is used to
 * support a memory pool allocation scheme where a pool of reusable objects
 * may be allocated, and the pool renewed by releasing objects or back.
 * When the pool is used up, a pool consuming thread then must wait for
 * a resource to be freed by another consumer (or timeout).  This class is
 * not meant to be used directly, but rather to build the synchronizing
 * control between consumers which might be forced to wait for a resource.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT ReusableAllocator : protected Conditional
{
private:
    __DELETE_COPY(ReusableAllocator);

protected:
    ReusableObject *freelist;
    unsigned waiting;

    /**
     * Initialize reusable allocator through a conditional.  Zero free list.
     */
    ReusableAllocator();

    /**
     * Get next reusable object in the pool.
     * @param object from list.
     * @return next object.
     */
    inline ReusableObject *next(ReusableObject *object)
        {return object->getNext();}

    /**
     * Release resuable object
     * @param object being released.
     */
    void release(ReusableObject *object);
};

/**
 * Generic non-recursive exclusive lock class.  This class also impliments
 * the exclusive_lock protocol.  In addition, an interface is offered to
 * support dynamically managed mutexes which are internally pooled.  These
 * can be used to protect and serialize arbitrary access to memory and
 * objects on demand.  This offers an advantage over embedding mutexes to
 * serialize access to individual objects since the maximum number of
 * mutexes will never be greater than the number of actually running threads
 * rather than the number of objects being potentially protected.  The
 * ability to hash the pointer address into an indexed table further optimizes
 * access by reducing the chance for collisions on the primary index mutex.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT Mutex : public ExclusiveAccess
{
private:
    __DELETE_COPY(Mutex);

protected:
    mutable pthread_mutex_t mlock;

    virtual void _lock(void) __OVERRIDE;
    virtual void _unlock(void) __OVERRIDE;

public:
    typedef autoexclusive<Mutex> autolock;

    /**
     * Create a mutex lock.
     */
    Mutex();

    /**
     * Destroy mutex lock, release waiting threads.
     */
    ~Mutex();

    /**
     * Acquire mutex lock.  This is a blocking operation.
     */
    inline void acquire(void)
        {pthread_mutex_lock(&mlock);}

    /**
     * Acquire mutex lock.  This is a blocking operation.
     */
    inline void lock(void)
        {pthread_mutex_lock(&mlock);}

    /**
     * Release acquired lock.
     */
    inline void unlock(void)
        {pthread_mutex_unlock(&mlock);}

    /**
     * Release acquired lock.
     */
    inline void release(void)
        {pthread_mutex_unlock(&mlock);}

    /**
     * Convenience function to acquire os native mutex lock directly.
     * @param lock to acquire.
     */
    inline static void acquire(pthread_mutex_t *lock)
        {pthread_mutex_lock(lock);}

    /**
     * Convenience function to release os native mutex lock directly.
     * @param lock to release.
     */
    inline static void release(pthread_mutex_t *lock)
        {pthread_mutex_unlock(lock);}

    /**
     * Specify hash table size for guard protection.  The default is 1.
     * This should be called at initialization time from the main thread
     * of the application before any other threads are created.
     * @param size of hash table used for guarding.
     */
    static void indexing(unsigned size);

    /**
     * Specify pointer/object/resource to guard protect.  This uses a
     * dynamically managed mutex.
     * @param pointer to protect.
     */
    static bool protect(const void *pointer);

    /**
     * Specify a pointer/object/resource to release.
     * @param pointer to release.
     */
    static bool release(const void *pointer);
};

/**
 * Guard class to apply scope based mutex locking to objects.  The mutex
 * is located from the mutex pool rather than contained in the target
 * object, and the lock is released when the guard object falls out of
 * scope.  This is essentially an automation mechanism for mutex::protect.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT AutoProtect
{
private:
    const void *object;

    __DELETE_COPY(AutoProtect);

protected:
    /**
     * Create an unitialized instance of guard.  Usually used with a
     * guard = operator.
     */
    AutoProtect();

    /**
     * Set guard to mutex lock a new object.  If a lock is currently
     * held, it is released.
     * @param object to guard.
     */
    void set(const void *object);

    /**
     * Prematurely release a guard.
     */
    void release(void);

public:
    /**
     * Construct a guard for a specific object.
     * @param object to guard.
     */
    AutoProtect(const void *object);

    /**
     * Release mutex when guard falls out of scope.
     */
    ~AutoProtect();

    inline operator bool() const {
        return object != NULL;
    }

    inline bool operator!() const {
        return object == NULL;
    }    
};

template<typename T>
class autoprotect : public AutoProtect
{
public:
    inline autoprotect() : AutoProtect() {};

    inline autoprotect(const T *object) : AutoProtect(object) {};

    inline void set(const T *object) {
        AutoProtect::set(object);
    }

    inline void release() {
        AutoProtect::release();
    }

    inline autoprotect& operator=(const T* object) {
        AutoProtect::set(object);
        return *this;
    }

    inline T* operator->() const {
        return object;
    }

    inline T& operator*() const {
        __THROW_DEREF(object);
        return *object;
    }
};

/**
 * An abstract class for defining classes that operate as a thread.  A derived
 * thread class has a run method that is invoked with the newly created
 * thread context, and can use the derived object to store all member data
 * that needs to be associated with that context.  This means the derived
 * object can safely hold thread-specific data that is managed with the life
 * of the object, rather than having to use the clumsy thread-specific data
 * management and access functions found in thread support libraries.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT Thread
{
private:
    __DELETE_COPY(Thread);

protected:
// may be used in future if we need cancelable threads...
#ifdef  _MSTHREADS_
    HANDLE cancellor;
#else
    void *cancellor;
#endif

    enum {R_UNUSED} reserved;   // cancel mode?
    pthread_t tid;
    stacksize_t stack;
    int priority;

    /**
     * Create a thread object that will have a preset stack size.  If 0
     * is used, then the stack size is os defined/default.
     * @param stack size to use or 0 for default.
     */
    Thread(size_t stack = 0);

    /**
     * Map thread for get method.  This should be called from start of the
     * run() method of a derived class.
     */
    void map(void);

    /**
     * Check if running.
     */
    virtual bool is_active(void) const;

public:
    /**
     * Set thread priority without disrupting scheduling if possible.
     * Based on scheduling policy.  It is recommended that the process
     * is set for realtime scheduling, and this method is actually for
     * internal use.
     */
    void setPriority(void);

    /**
     * Yield execution context of the current thread. This is a static
     * and may be used anywhere.
     */
    static void yield(void);

    /**
     * Sleep current thread for a specified time period.
     * @param timeout to sleep for in milliseconds.
     */
    static void sleep(timeout_t timeout);

    /**
     * Get mapped thread object.  This returns the mapped base class of the
     * thread object of the current executing context.  You will need to
     * cast to the correct derived class to access derived thread-specific
     * storage.  If the current thread context is not mapped NULL is returned.
     */
    static Thread *get(void);

    /**
     * Abstract interface for thread context run method.
     */
    virtual void run(void) = 0;

    /**
     * Destroy thread object, thread-specific data, and execution context.
     */
    virtual ~Thread();

    /**
     * Exit the thread context.  This function should NO LONGER be called
     * directly to exit a running thread.  Instead this method will only be
     * used to modify the behavior of the thread context at thread exit,
     * including detached threads which by default delete themselves.  This
     * documented usage was changed to support Mozilla NSPR exit behavior
     * in case we support NSPR as an alternate thread runtime in the future.
     */
    virtual void exit(void);

    /**
     * Used to initialize threading library.  May be needed for some platforms.
     */
    static void init(void);

    /**
     * Used to specify scheduling policy for threads above priority "0".
     * Normally we apply static realtime policy SCHED_FIFO (default) or
     * SCHED_RR.  However, we could apply SCHED_OTHER, etc.
     */
    static void policy(int polid);

    /**
     * Set concurrency level of process.  This is essentially a portable
     * wrapper for pthread_setconcurrency.
     */
    static void concurrency(int level);

    /**
     * Determine if two thread identifiers refer to the same thread.
     * @param thread1 to test.
     * @param thread2 to test.
     * @return true if both are the same context.
     */
    static bool equal(pthread_t thread1, pthread_t thread2);

    /**
     * Get current thread id.
     * @return thread id.
     */
    static pthread_t self(void);

    inline operator bool() const
        {return is_active();}

    inline bool operator!() const
        {return !is_active();}

    inline bool isRunning(void) const
        {return is_active();}
};

/**
 * A child thread object that may be joined by parent.  A child thread is
 * a type of thread in which the parent thread (or process main thread) can
 * then wait for the child thread to complete and then delete the child object.
 * The parent thread can wait for the child thread to complete either by
 * calling join, or performing a "delete" of the derived child object.  In
 * either case the parent thread will suspend execution until the child thread
 * exits.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT JoinableThread : public Thread
{
private:
    __DELETE_COPY(JoinableThread);

protected:
#ifdef  _MSTHREADS_
    HANDLE running;
#else
    volatile bool running;
#endif
    volatile bool joining;

    /**
     * Create a joinable thread with a known context stack size.
     * @param size of stack for thread context or 0 for default.
     */
    JoinableThread(size_t size = 0);

    /**
     * Delete child thread.  Parent thread suspends until child thread
     * run method completes or child thread calls it's exit method.
     */
    virtual ~JoinableThread();

    /**
     * Join thread with parent.  Calling from a child thread to exit is
     * now depreciated behavior and in the future will not be supported.
     * Threads should always return through their run() method.
     */
    void join(void);

    bool is_active(void) const;

    virtual void run(void) = 0;

public:

    /**
     * Start execution of child context.  This must be called after the
     * child object is created (perhaps with "new") and before it can be
     * joined.  This method actually begins the new thread context, which
     * then calls the object's run method.  Optionally raise the priority
     * of the thread when it starts under realtime priority.
     * @param priority of child thread.
     */
    void start(int priority = 0);

    /**
     * Start execution of child context as background thread.  This is
     * assumed to be off main thread, with a priority lowered by one.
     */
    inline void background(void)
        {start(-1);}
};

/**
 * A detached thread object that is stand-alone.  This object has no
 * relationship with any other running thread instance will be automatically
 * deleted when the running thread instance exits, either by it's run method
 * exiting, or explicity calling the exit member function.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT DetachedThread : public Thread
{
private:
    __DELETE_COPY(DetachedThread);

protected:
    bool active;

    /**
     * Create a detached thread with a known context stack size.
     * @param size of stack for thread context or 0 for default.
     */
    DetachedThread(size_t size = 0);

    /**
     * Destroys object when thread context exits.  Never externally
     * deleted.  Derived object may also have destructor to clean up
     * thread-specific member data.
     */
    ~DetachedThread();

    /**
     * Exit context of detached thread.  Thread object will be deleted.
     * This function should NO LONGER be called directly to exit a running
     * thread.  Instead, the thread should only "return" through the run()
     * method to exit.  The documented usage was changed so that exit() can
     * still be used to modify the "delete this" behavior of detached threads
     * while merging thread exit behavior with Mozilla NSPR.
     */
    void exit(void);

    bool is_active(void) const;

    virtual void run(void) = 0;

public:
    /**
     * Start execution of detached context.  This must be called after the
     * object is created (perhaps with "new"). This method actually begins
     * the new thread context, which then calls the object's run method.
     * @param priority to start thread with.
     */
    void start(int priority = 0);
};

/**
 * Convenience type for using timed events.
 */
typedef TimedEvent timedevent_t;

/**
 * Convenience type for using exclusive mutex locks.
 */
typedef Mutex mutex_t;

/**
 * Convenience type for using read/write locks.
 */
typedef RWLock rwlock_t;

/**
 * Convenience type for using recursive exclusive locks.
 */
typedef RecursiveMutex rexlock_t;

#define __AUTOLOCK(x)       autolock __autolock__(x)
#define __AUTOPROTECT(x)    AutoProtect __autolock__(x)
#define __SYNC(x) for(bool _sync_flag_ = Mutex::protect(x); _sync_flag_; _sync_flag_ = !Mutex::release(x))

} // namespace ucommon

#endif
