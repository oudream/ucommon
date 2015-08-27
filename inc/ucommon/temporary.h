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
 * Temporary templates for C++.  This offers automatic management of
 * heap temporary objects.
 * @file ucommon/temporary.h
 */

#ifndef _UCOMMON_TEMPORARY_H_
#define _UCOMMON_TEMPORARY_H_

#ifndef _UCOMMON_CONFIG_H_
#include <ucommon/platform.h>
#endif

#ifndef _UCOMMON_PROTOCOLS_H_
#include <ucommon/protocols.h>
#endif

#ifndef _UCOMMON_THREAD_H_
#include <ucommon/thread.h>
#endif

#ifndef _UCOMMON_STRING_H_
#include <ucommon/string.h>
#endif

#ifndef _UCOMMON_MEMORY_H_
#include <ucommon/memory.h>
#endif

#ifndef _UCOMMON_FSYS_H_
#include <ucommon/fsys.h>
#endif

#ifndef _UCOMMON_FILE_H_
#include <ucommon/file.h>
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
 * Manage temporary object stored on the heap.  This is used to create a
 * object on the heap who's scope is controlled by the scope of a member
 * function call.  Sometimes we have data types and structures which cannot
 * themselves appear as auto variables.  We may also have a limited stack
 * frame size in a thread context, and yet have a dynamic object that we
 * only want to exist during the life of the method call.  Using temporary
 * allows any type to be created from the heap but have a lifespan of a
 * method's stack frame.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
template <typename T>
class temporary
{
private:
    __DELETE_COPY(temporary);

protected:
    T *array;
    size_t used;

public:
    /**
     * Construct a temporary object, create our stack frame reference.
     */
    inline temporary(size_t size = 1) {
        array = new T[size];
        used = size;
    }    

    inline temporary(size_t size, const T initial) {
        array = new T[size];
        used = size;
        for(size_t p = 0; p < size; ++p)
            array[p] = initial;
    }

    inline explicit temporary(const T initial) {
        array = new T[1];
        used = 1;
        array[0] = initial;
    }

    inline ~temporary() {
        if(array) {
            delete[] array;
            array = NULL;
        }
    }

    inline operator T&() const {
        return array[0];
    }        

    /**
     * Access heap object through our temporary directly.
     * @return reference to heap resident object.
     */
    inline T& operator*() const {
        return array[0];
    }

    /**
     * Access members of our heap object through our temporary.
     * @return member reference of heap object.
     */
    inline T* operator->() const {
        return &array[0];
    }

    inline operator bool() const {
        return array != NULL;
    }

    inline bool operator!() const {
        return array == NULL;
    }

    inline temporary& operator=(const T initial) {
        array[0] = initial;
        return *this;
    }

    inline void release() {
        if(array) {
            delete[] array;
            array = NULL;
        }
    }

    inline T& operator[](size_t index) const {
        crit(index < used, "array out of bound");
        return array[index];
    }

    inline T* operator()(size_t index) const {
        crit(index < used, "array out of bound");
        return &array[index];
    }

    inline void operator()(size_t index, const T value) {
        crit(index < used, "array out of bound");
        array[index] = value;
    }

    inline T& value(size_t index) const {
        crit(index < used, "array out of bound");
        return array[index];
    }

    inline void value(size_t index, const T value) {
        crit(index < used, "array out of bound");
        array[index] = value;
    }

    inline size_t read(file& fp) {
        return (*fp == NULL) || (array == NULL) ? 
            0 : fread(array, sizeof(T), used, *fp);
    }

    inline size_t write(file& fp) {
        return (*fp == NULL) || (array == NULL) ? 
            0 : fwrite(array, sizeof(T), used, *fp);
    }

    inline size_t seek(file& fp, long pos) {
        return (*fp == NULL) ? 
            0 : (fseek(*fp, sizeof(T) * pos, SEEK_CUR) / sizeof(T));
    }
};

template<>
class temporary<char *>
{
private:
    __DELETE_COPY(temporary);

protected:
    char *object;
    size_t used;

public:
    /**
     * Construct a temporary object, create our stack frame reference.
     */
    inline temporary(size_t size) {
        object = (char *)::malloc(size);
        used = size;
    }

    inline operator char *() const {
        return object;
    }

    inline size_t size() const {
        return used;
    }

    /**
     * Access heap object through our temporary directly.
     * @return reference to heap resident object.
     */
    inline char *operator*() const {
        return object;
    }

    inline operator bool() const {
        return object != NULL;
    }

    inline bool operator!() const {
        return object == NULL;
    }

    inline void release() {
        if(object) {
            ::free(object);
            object = NULL;
        }
    }

    inline ~temporary() {
        if(object) {
            ::free(object);
            object = NULL;
        }
    }

    inline size_t read(file& fp) {
        return (*fp == NULL) || (object == NULL) ? 
            0 : String::count(fgets(object, (socksize_t)used, *fp));
    }
    
    inline size_t write(file& fp) {
        return (*fp == NULL) || (object == NULL) ? 
            0 : fputs(object, *fp);
    }

    inline size_t seek(file& fp, long pos) {
        return (*fp == NULL) ? 
            0 : fseek(*fp, pos, SEEK_CUR);
    }
};

template<>
class temporary<uint8_t *>
{
private:
    inline temporary(const temporary<uint8_t *>&) {};

protected:
    uint8_t *object;
    size_t used;

public:
    /**
     * Construct a temporary object, create our stack frame reference.
     */
    inline temporary(size_t size) {
        object = (uint8_t *)::malloc(size);
        used = size;
    }

    inline operator uint8_t *() const {
        return object;
    }

    inline size_t size() const {
        return used;
    }

    /**
     * Access heap object through our temporary directly.
     * @return reference to heap resident object.
     */
    inline uint8_t *operator*() const {
        return object;
    }

    inline operator bool() const {
        return object != NULL;
    }

    inline bool operator!() const {
        return object == NULL;
    }

    inline void release() {
        if(object) {
            ::free(object);
            object = NULL;
        }
    }

    inline size_t read(file& fp) {
        return (*fp == NULL) || (object == NULL) ? 
            0 : fread(object, 1, used, *fp);
    }
    
    inline size_t write(file& fp) {
        return (*fp == NULL) || (object == NULL) ? 
            0 : fwrite(object, 1, used, *fp);
    }

    inline size_t seek(file& fp, long pos) {
        return (*fp == NULL) ? 
            0 : fseek(*fp, pos, SEEK_CUR);
    }

    inline size_t read(fsys& fs) {
        ssize_t result;
        if(!object || (result = fs.read(object, used)) < 0)
            return 0;
        return (size_t)result;
    }

    inline size_t write(fsys& fs) {
        ssize_t result;
        if(!object || (result = fs.write(object, used)) < 0)
            return 0;
        return (size_t)result;
    }

    inline ~temporary() {
        if(object) {
            ::free(object);
            object = NULL;
        }
    }
};

} // namespace ucommon

#endif
