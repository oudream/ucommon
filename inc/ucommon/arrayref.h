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
 * Arrays of thread-safe strongly typed heap objects.  This is used for
 * arrays of smart pointers to immutable heap instances of object types 
 * that are reference counted and automatically deleted when no longer used.
 * @file ucommon/arrayref.h
 */

#ifndef _UCOMMON_ARRAYREF_H_
#define _UCOMMON_ARRAYREF_H_

#ifndef _UCOMMON_CPR_H_
#include <ucommon/cpr.h>
#endif

#ifndef _UCOMMON_ATOMIC_H_
#include <ucommon/atomic.h>
#endif

#ifndef _UCOMMON_PROTOCOLS_H_
#include <ucommon/protocols.h>
#endif

#ifndef _UCOMMON_OBJECT_H_
#include <ucommon/object.h>
#endif

#ifndef	_UCOMMON_TYPEREF_H_
#include <ucommon/typeref.h>
#endif

#ifndef _UCOMMON_THREAD_H_
#include <ucommon/thread.h>
#endif

namespace ucommon {

class __EXPORT ArrayRef : public TypeRef
{
protected:
	typedef enum {ARRAY, STACK, QUEUE, FALLBACK} arraytype_t;

	class Array : public Counted, public ConditionalAccess
	{
	protected:
		friend class ArrayRef;

		size_t head, tail;

		arraytype_t type;

		explicit Array(arraytype_t mode, void *addr, size_t size);

		void assign(size_t index, Counted *object);

		Counted *remove(size_t index);

		size_t count(void);

		virtual void dealloc();

		inline Counted **get(void) {
			return reinterpret_cast<Counted **>(((caddr_t)(this)) + sizeof(Array));
		}

		Counted *get(size_t index);
	};

	ArrayRef(arraytype_t mode, size_t size);
	ArrayRef(arraytype_t mode, size_t size, TypeRef& object);
	ArrayRef(const ArrayRef& copy);
	ArrayRef();

	void assign(size_t index, TypeRef& t);

	void reset(TypeRef& object);

	void reset(Counted *object);

	Counted *get(size_t index);

	bool is(size_t index);

	static Array *create(arraytype_t type, size_t size);

protected:
	void push(const TypeRef& object);

	void pull(TypeRef& object);	

	bool push(const TypeRef& object, timeout_t timeout);

	void pull(TypeRef& object, timeout_t timeout);	

public:
	size_t count(void);

	void resize(size_t size);

	void realloc(size_t size);

	void clear(void);

	void pop(void);
};

template<typename T>
class stackref : public ArrayRef
{
public:
	inline stackref() :	ArrayRef() {};

	inline stackref(const stackref& copy) : ArrayRef(copy) {};

	inline stackref(size_t size) : ArrayRef(STACK, size + 1) {};

	inline stackref& operator=(const stackref& copy) {
		TypeRef::set(copy);
		return *this;
	}

	inline typeref<T> operator[](size_t index) {
		return typeref<T>(ArrayRef::get(index));
	}

	inline typeref<T> operator()(size_t index) {
		return typeref<T>(ArrayRef::get(index));
	}

	inline typeref<T> at(size_t index) {
		return typeref<T>(ArrayRef::get(index));
	}

	inline void release(void) {
		TypeRef::set(nullptr);
	}

	inline typeref<T> pull() {
		typeref<T> obj;
		ArrayRef::pull(obj);
		return obj;
	}

	inline typeref<T> pull(timeout_t timeout) {
		typeref<T> obj;
		ArrayRef::pull(obj, timeout);
		return obj;
	}

	inline stackref& operator>>(typeref<T>& target) {
		ArrayRef::pull(target);
		return *this;
	}

	inline void push(const typeref<T>& source) {
		ArrayRef::push(source);
	}

	inline bool push(const typeref<T>& source, timeout_t timeout) {
		return ArrayRef::push(source, timeout);
	}

	inline stackref& operator<<(const typeref<T>& source) {
		ArrayRef::push(source);
		return *this;
	}

	inline stackref& operator<<(T t) {
		typeref<T> v(t);
		ArrayRef::push(v);
		return *this;
	}
};

template<typename T>
class queueref : public ArrayRef
{
public:
	inline queueref() :	ArrayRef() {};

	inline queueref(const queueref& copy) : ArrayRef(copy) {};

	inline queueref(size_t size, bool fallback = false) : ArrayRef(fallback ? FALLBACK : QUEUE, size + 1) {};

	inline queueref& operator=(const queueref& copy) {
		TypeRef::set(copy);
		return *this;
	}

	inline typeref<T> operator[](size_t index) {
		return typeref<T>(ArrayRef::get(index));
	}

	inline typeref<T> operator()(size_t index) {
		return typeref<T>(ArrayRef::get(index));
	}

	inline typeref<T> at(size_t index) {
		return typeref<T>(ArrayRef::get(index));
	}

	inline void release(void) {
		TypeRef::set(nullptr);
	}

	inline typeref<T> pull() {
		typeref<T> obj;
		ArrayRef::pull(obj);
		return obj;
	}

	inline typeref<T> pull(timeout_t timeout) {
		typeref<T> obj;
		ArrayRef::pull(obj, timeout);
		return obj;
	}

	inline queueref& operator>>(typeref<T>& target) {
		ArrayRef::pull(target);
		return *this;
	}

	inline void push(const typeref<T>& source) {
		ArrayRef::push(source);
	}

	inline bool push(const typeref<T>& source, timeout_t timeout) {
		return ArrayRef::push(source, timeout);
	}

	inline queueref& operator<<(const typeref<T>& source) {
		ArrayRef::push(source);
		return *this;
	}

	inline queueref& operator<<(T t) {
		typeref<T> v(t);
		ArrayRef::push(v);
		return *this;
	}
};


template<typename T>
class arrayref : public ArrayRef
{
public:
	inline arrayref() :	ArrayRef() {};

	inline arrayref(const arrayref& copy) : ArrayRef(copy) {};

	inline arrayref(size_t size) : ArrayRef(ARRAY, size) {};

	inline arrayref(size_t size, typeref<T>& t) : ArrayRef(ARRAY, size, t) {};

	inline arrayref(size_t size, T t) : ArrayRef(ARRAY, size) {
		typeref<T> v(t);
		reset(v);
	}

	size_t find(typeref<T> v, size_t start = 0) {
		for(size_t index = start; index < size(); ++index) {
			if(is(index) && at(index) == v) {
				return index;
			}
		}
		return (size_t)(-1);
	}

	size_t count(typeref<T> v) {
		size_t found = 0;
		for(size_t index = 0; index < size(); ++index) {
			if(is(index) && at(index) == v)
				++found;
		}
		return found;
	}

	inline arrayref& operator=(const arrayref& copy) {
		TypeRef::set(copy);
		return *this;
	}

	inline arrayref& operator=(typeref<T>& t) {
		reset(t);
		return *this;
	}

	inline arrayref& operator=(T t) {
		typeref<T> v(t);
		reset(v);
	}

	inline typeref<T> operator[](size_t index) {
		return typeref<T>(ArrayRef::get(index));
	}

	inline typeref<T> operator()(size_t index) {
		return typeref<T>(ArrayRef::get(index));
	}

	inline typeref<T> at(size_t index) {
		return typeref<T>(ArrayRef::get(index));
	}

	inline typeref<T>  value(size_t index) {
		return typeref<T>(ArrayRef::get(index));
	}

	inline void value(size_t index, typeref<T>& t) {
		ArrayRef::assign(index, t);
	}

	inline void put(typeref<T>& target, size_t index) {
		TypeRef::put(target, ArrayRef::get(index));
	}

	inline void operator()(size_t index, typeref<T>& t) {
		ArrayRef::assign(index, t);
	}

	inline void operator()(size_t index, T t) {
		typeref<T> v(t);
		ArrayRef::assign(index, v);
	}

	inline void release(void) {
		TypeRef::set(nullptr);
	}
};

typedef arrayref<Type::Bytes> bytearray_t;
typedef arrayref<Type::Chars> stringarray_t;

} // namespace

#endif
