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
 * Generic templates for C++.  These are templates that do not depend
 * on any ucommon classes.  They can be used for generic C++ programming.
 * @file ucommon/generics.h
 */

#ifndef _UCOMMON_GENERICS_H_
#define _UCOMMON_GENERICS_H_

#ifndef _UCOMMON_CPR_H_
#include <ucommon/cpr.h>
#endif

#include <cstdlib>
#include <cstring>
#include <stdexcept>

#ifndef UCOMMON_SYSRUNTIME
#define THROW(x)    throw x
#define THROWS(x)   throw(x)
#define THROWS_ANY  throw()
#else
#define THROW(x)    ::abort()
#define THROWS(x)
#define THROWS_ANY
#endif

namespace ucommon {

/**
 * Generic smart pointer class.  This is the original Common C++ "Pointer"
 * class with a few additions.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
template <typename T>
class pointer
{
protected:
    unsigned *counter;
    T *object;

public:
    inline void release(void) {
        if(counter && --(*counter)==0) {
            delete counter;
            delete object;
        }
        object = NULL;
        counter = NULL;
    }

    inline void retain(void) {
        if(counter)
            ++*counter;
    }

    inline void set(T* ptr) {
        if(object != ptr) {
            release();
            counter = new unsigned;
            *counter = 1;
            object = ptr;
        }
    }

    inline void set(const pointer<T> &ref) {
        if(object == ref.object)
            return;

        if(counter && --(*counter)==0) {
            delete counter;
            delete object;
        }
        object = ref.object;
        counter = ref.counter;
        if(counter)
            ++(*counter);
    }

    inline pointer() {
        counter = NULL;
        object = NULL;
    }

    inline explicit pointer(T* ptr = NULL) : object(ptr) {
        if(object) {
            counter = new unsigned;
            *counter = 1;
        }
        else
            counter = NULL;
    }

    inline pointer(const pointer<T> &ref) {
        object = ref.object;
        counter = ref.counter;
        if(counter)
            ++(*counter);
    }

    inline pointer& operator=(const pointer<T> &ref) {
        this->set(ref);
        return *this;
    }

    inline pointer& operator=(T *ptr) {
        this->set(ptr);
        return *this;
    }

    inline ~pointer() {
        release();
    }

    inline T& operator*() const {
        return *object;
    }

    inline T* operator->() const {
        return object;
    }

    inline bool operator!() const {
        return (counter == NULL);
    }

    inline operator bool() const {
        return counter != NULL;
    }
};

/**
 * Generic smart array class.  This is the original Common C++ "Pointer" class
 * with a few additions for arrays.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
template <typename T>
class array_pointer
{
protected:
    unsigned *counter;
    T *array;

public:
    inline void release(void) {
        if(counter && --(*counter)==0) {
            delete counter;
            delete[] array;
        }
        array = NULL;
        counter = NULL;
    }

    inline void retain(void) {
        if(counter)
            ++*counter;
    }

    inline void set(T* ptr) {
        if(array != ptr) {
            release();
            counter = new unsigned;
            *counter = 1;
            array = ptr;
        }
    }

    inline void set(const array_pointer<T> &ref) {
        if(array == ref.array)
            return;

        if(counter && --(*counter)==0) {
            delete counter;
            delete[] array;
        }
        array = ref.array;
        counter = ref.counter;
        if(counter)
            ++(*counter);
    }

    inline array_pointer() {
        counter = NULL;
        array = NULL;
    }

    inline explicit array_pointer(T* ptr = NULL) : array(ptr) {
        if(array) {
            counter = new unsigned;
            *counter = 1;
        }
        else
            counter = NULL;
    }

    inline array_pointer(const array_pointer<T> &ref) {
        array = ref.array;
        counter = ref.counter;
        if(counter)
            ++(*counter);
    }

    inline array_pointer& operator=(const array_pointer<T> &ref) {
        this->set(ref);
        return *this;
    }

    inline array_pointer& operator=(T *ptr) {
        this->set(ptr);
        return *this;
    }

    inline ~array_pointer() {
        release();
    }

    inline T* operator*() const {
        return array;
    }

    inline T& operator[](size_t offset) const {
        return array[offset];
    }

    inline T* operator()(size_t offset) const {
        return &array[offset];
    }

    inline bool operator!() const {
        return (counter == NULL);
    }

    inline operator bool() const {
        return counter != NULL;
    }
};

/**
 * Save and restore global objects in function call stack frames.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
template<typename T>
class save_restore
{
private:
    T *original;
    T temp;

    save_restore() __DELETED;

public:
    /**
     * Save object into local copy and keep reference to the original object.
     * @param object to save.
     */
    inline save_restore(T& object) {
        original = &object; temp = object;
    }

    /**
     * Restore original when stack frame is released.
     */
    inline ~save_restore() {
        *original = temp;
    }
};

/**
 * Convenience function to validate object assuming it is castable to bool.
 * @param object we are testing.
 * @return true if object valid.
 */
template<typename T>
inline bool is(T& object) {
    return object.operator bool();
}

/**
 * Convenience function to test pointer object.  This solves issues where
 * some compilers get confused between bool and pointer operators.
 * @param object we are testing.
 * @return true if object points to NULL.
 */
template<typename T>
inline bool isnull(T& object) {
    return (bool)(object.operator*() == nullptr);
}

/**
 * Convenience function to test pointer-pointer object.  This solves issues
 * where some compilers get confused between bool and pointer operators.
 * @param object we are testing.
 * @return true if object points to NULL.
 */
template<typename T>
inline bool isnullp(T *object) {
    return (bool)(object->operator*() == nullptr);
}

/**
 * Convenience function to duplicate object pointer to heap.
 * @param object we are duping.
 * @return heap pointer instance.
 */
template<typename T>
inline T* dup(const T& object) {
    return new T(object);
}

template<typename T>
inline void dupfree(T object) {
    delete object;
}

template<>
inline char *dup<char>(const char& object) {
    return strdup(&object);
}

template<>
inline void dupfree<char*>(char* object) {
    ::free(object);
}

/**
 * Convenience function to reset an existing object.
 * @param object type to reset.
 */
template<typename T>
inline void reset_unsafe(T& object) {
    new((caddr_t)&object) T;
}

/**
 * Convenience function to zero an object and restore type info.
 * @param object to zero in memory.
 */
template<typename T>
inline void zero_unsafe(T& object) {
    memset((void *)&object, 0, sizeof(T)); new((caddr_t)&object) T;
}

/**
 * Convenience function to copy class.
 * @param target to copy into.
 * @param source to copy from.
 */
template<typename T>
inline void copy_unsafe(T* target, const T* source) {
    memcpy((void *)target, (void *)source, sizeof(T));
}

/**
 * Convenience function to store object pointer into object.
 * @param target to copy into.
 * @param source to copy from.
 */
template<typename T>
inline void store_unsafe(T& target, const T* source) {
    memcpy((void *)&target, (void *)source, sizeof(T));
}

/**
 * Convenience function to swap objects.  Can be specialized.
 * @param o1 to swap.
 * @param o2 to swap.
 */
template<typename T>
inline void swap(T& o1, T& o2) {
    cpr_memswap(&o1, &o2, sizeof(T));
}

/**
 * Convenience function to copy objects.
 */
template<typename T>
inline T copy(const T& src) {
    return T(src);
}

template<typename T>
inline T& copy(const T& src, T& to) {
    new((caddr_t)&to) T(src);
    return to;
}

/**
 * Convenience function to move objects.
 */
template<typename T>
inline T& move(T& src, T& to) {
    memcpy((void *)&to, (void *)&src, sizeof(T));
    new((caddr_t)&src) T();
    return to;
} 

template<typename T>
inline T& clear(T& o) {
    o.~T();
    new((caddr_t)&o) T();
    return o;
}

/**
 * Convenience function to check memory arrays.
 * @param pointer to validate.
 * @param base address of array.
 * @param count of objects.
 * @return true if in boundry.
 */
template<typename T>
inline bool bound(const T* pointer, const T* base, size_t count) {
    if(pointer < base || pointer >= &base[count])
        return false;
    if(((size_t)pointer) % sizeof(T))
        return false;
    return true;
}

/**
 * Convenience function to return max of two objects.
 * @param o1 to check.
 * @param o2 to check.
 * @return max object.
 */
template<typename T>
inline T& (max)(T& o1, T& o2) {
    return o1 > o2 ? o1 : o2;
}

/**
 * Convenience function to return min of two objects.
 * @param o1 to check.
 * @param o2 to check.
 * @return min object.
 */
template<typename T>
inline T& (min)(T& o1, T& o2) {
    return o1 < o2 ? o1 : o2;
}

/**
 * Convenience macro to range restrict values.
 * @param value to check.
 * @param low value.
 * @param high value.
 * @return adjusted value.
 */
template<typename T>
inline T& (limit)(T& value, T& low, T& high) {
    return (value < low) ? low : ((value > high) ? high : value);
}

/**
 * Convert a pointer to a reference with type checking.  This is
 * mostly convenience for documenting behavior.
 * @param pointer to convert.
 * @return object reference.
 */
template<typename T>
inline T& deref_pointer(T *pointer) {
    __THROW_DEREF(pointer);
    return *pointer;
}

} // namespace ucommon

#endif
