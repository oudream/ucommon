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
 * Basic array and reusable object factory heap support.
 * This offers ucommon support for forming reusable object pools.  Reusable 
 * object pools can be tied to local heaps and offer a means to create type 
 * factories that do not require global locking through malloc.
 * @file ucommon/reuse.h
 */

#ifndef _UCOMMON_REUSE_H_
#define _UCOMMON_REUSE_H_

#ifndef _UCOMMON_THREAD_H_
#include <ucommon/thread.h>
#endif

namespace ucommon {

typedef unsigned short vectorsize_t;

/**
 * An array of reusable objects.  This class is used to support the
 * array_use template.  A pool of objects are created which can be
 * allocated as needed.  Deallocated objects are returned to the pool
 * so they can be reallocated later.  This is a private fixed size heap.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT ArrayReuse : public ReusableAllocator
{
private:
    size_t objsize;
    unsigned count, limit, used;
    caddr_t mem;

    __DELETE_DEFAULTS(ArrayReuse);

protected:
    ArrayReuse(size_t objsize, unsigned c);
    ArrayReuse(size_t objsize, unsigned c, void *memory);

public:
    /**
     * Destroy reusable private heap array.
     */
    ~ArrayReuse();

protected:
    bool avail(void) const;

    ReusableObject *get(timeout_t timeout);
    ReusableObject *get(void);
    ReusableObject *request(void);
};

/**
 * A mempager source of reusable objects.  This is used by the reuse_pager
 * template to allocate new objects either from a memory pager used as
 * a private heap, or from previously allocated objects that have been
 * returned for reuse.
  * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT PagerReuse : protected __VIRTUAL MemoryRedirect, protected ReusableAllocator
{
private:
    unsigned limit, count;
    size_t osize;

    __DELETE_DEFAULTS(PagerReuse);

protected:
    PagerReuse(mempager *pager, size_t objsize, unsigned count);
    ~PagerReuse();

    bool avail(void) const;
    ReusableObject *get(void);
    ReusableObject *get(timeout_t timeout);
    ReusableObject *request(void);
};

/**
 * An array of reusable types.  A pool of typed objects is created which can
 * be allocated as needed.  Deallocated typed objects are returned to the pool
 * so they can be reallocated later.  This is a private fixed size heap.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
template<class T>
class array_reuse : protected ArrayReuse
{
private:
    __DELETE_DEFAULTS(array_reuse);

public:
    /**
     * Create private heap of reusable objects of specified type.
     * @param count of objects of specified type to allocate.
     */
    inline array_reuse(unsigned count) :
        ArrayReuse(sizeof(T), count) {}

    /**
     * Create reusable objects of specific type in preallocated memory.
     * @param count of objects of specified type in memory.
     * @param memory to use.
     */
    inline array_reuse(unsigned count, void *memory) :
        ArrayReuse(sizeof(T), count, memory) {}

    /**
     * Test if typed objects available in heap or re-use list.
     * @return true if objects still are available.
     */
    inline operator bool() const {
        return avail();
    }

    /**
     * Test if the entire heap has been allocated.
     * @return true if no objects are available.
     */
    inline bool operator!() const {
        return !avail();
    }

    /**
     * Request immediately next available typed object from the heap.
     * @return typed object pointer or NULL if heap is empty.
     */
    inline T* request(void) {
        return static_cast<T*>(ArrayReuse::request());
    }

    /**
     * Get a typed object from the heap.  This function blocks when the
     * heap is empty until an object is returned to the heap.
     * @return typed object pointer from heap.
     */
    inline T* get(void) {
        return static_cast<T*>(ArrayReuse::get());
    }

    /**
     * Create a typed object from the heap.  This function blocks when the
     * heap is empty until an object is returned to the heap.
     * @return typed object pointer from heap.
     */
    inline T* create(void) {
        return init<T>(static_cast<T*>(ArrayReuse::get()));
    }

    /**
     * Get a typed object from the heap.  This function blocks until the
     * the heap has an object to return or the timer has expired.
     * @param timeout to wait for heap in milliseconds.
     * @return typed object pointer from heap or NULL if timeout.
     */
    inline T* get(timeout_t timeout) {
        return static_cast<T*>(ArrayReuse::get(timeout));
    }

    /**
     * Create a typed object from the heap.  This function blocks until the
     * the heap has an object to return or the timer has expired.
     * @param timeout to wait for heap in milliseconds.
     * @return typed object pointer from heap or NULL if timeout.
     */
    inline T* create(timeout_t timeout) {
        return init<T>(static_cast<T*>(ArrayReuse::get(timeout)));
    }

    /**
     * Release (return) a typed object back to the heap for re-use.
     * @param object to return.
     */
    inline void release(T *object) {
        ArrayReuse::release(object);
    }

    /**
     * Get a typed object from the heap by type casting reference.  This
     * function blocks while the heap is empty.
     * @return typed object pointer from heap.
     */
    inline operator T*() {
        return array_reuse::get();
    }

    /**
     * Get a typed object from the heap by pointer reference.  This
     * function blocks while the heap is empty.
     * @return typed object pointer from heap.
     */
    inline T *operator*() {
        return array_reuse::get();
    }
};

/**
 * A reusable private pool of reusable types.  A pool of typed objects is
 * created which can be allocated from a memory pager.  Deallocated typed
 * objects are also returned to this pool so they can be reallocated later.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
template <class T>
class paged_reuse : protected PagerReuse
{
private:
    __DELETE_DEFAULTS(paged_reuse);

public:
    /**
     * Create a managed reusable typed object pool.  This manages a heap of
     * typed objects that can either be reused from released objects or
     * allocate from an existing memory pager pool.
     * @param pager pool to allocate from.
     * @param count of objects of specified type to allocate.
     */
    inline paged_reuse(mempager *pager, unsigned count) :
        PagerReuse(pager, sizeof(T), count) {}

    /**
     * Test if typed objects available from the pager or re-use list.
     * @return true if objects still are available.
     */
    inline operator bool() const {
        return PagerReuse::avail();
    }

    /**
     * Test if no objects are available for reuse or the pager.
     * @return true if no objects are available.
     */
    inline bool operator!() const {
        return !PagerReuse::avail();
    }

    /**
     * Get a typed object from the pager heap.  This function blocks when the
     * heap is empty until an object is returned to the heap.
     * @return typed object pointer from heap.
     */
    inline T *get(void) {
        return static_cast<T*>(PagerReuse::get());
    }

    /**
     * Get a typed object from the pager heap.  This function blocks when the
     * heap is empty until an object is returned to the heap.  The objects
     * default constructor is used.
     * @return typed object pointer from heap.
     */
    inline T *create(void) {
        return init<T>(static_cast<T*>(PagerReuse::get()));
    }

    /**
     * Get a typed object from the heap.  This function blocks until the
     * the heap has an object to return or the timer has expired.
     * @param timeout to wait for heap in milliseconds.
     * @return typed object pointer from heap or NULL if timeout.
     */
    inline T *get(timeout_t timeout) {
        return static_cast<T*>(PagerReuse::get(timeout));
    }

    /**
     * Create a typed object from the heap.  This function blocks until the
     * the heap has an object to return or the timer has expired.  The
     * objects default constructor is used.
     * @param timeout to wait for heap in milliseconds.
     * @return typed object pointer from heap or NULL if timeout.
     */
    inline T *create(timeout_t timeout) {
        return init<T>(static_cast<T*>(PagerReuse::get(timeout)));
    }

    /**
     * Request immediately next available typed object from the pager heap.
     * @return typed object pointer or NULL if heap is empty.
     */
    inline T *request(void) {
        return static_cast<T*>(PagerReuse::request());
    }

    /**
     * Release (return) a typed object back to the pager heap for re-use.
     * @param object to return.
     */
    inline void release(T *object) {
        PagerReuse::release(object);
    }

    /**
     * Get a typed object from the pager heap by type casting reference.  This
     * function blocks while the heap is empty.
     * @return typed object pointer from heap.
     */
    inline T *operator*() {
        return paged_reuse::get();
    }

    /**
     * Get a typed object from the pager heap by pointer reference.  This
     * function blocks while the heap is empty.
     * @return typed object pointer from heap.
     */
    inline operator T*() {
        return paged_reuse::get();
    }
};

} // namespace ucommon

#endif
