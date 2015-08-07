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
 * A thread-safe atomic heap management system.  This is used to manage
 * immutable heap instances of object types that are reference counted
 * and automatically deleted when no longer used.  All references to the
 * object are through smart typeref pointers.  Both specific classes for
 * strings and byte arrays, and generic templates to support generic
 * types in the heap are offered.
 * @file ucommon/typeref.h
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

namespace ucommon {

class __EXPORT ArrayRef : public TypeRef
{
protected:
	class Array : public Counted
	{
	protected:
		friend class ArrayRef;

		explicit Array(void *addr, size_t size);

		void assign(size_t index, Counted *object);

		virtual void dealloc();

		inline Counted **get(void) {
			return reinterpret_cast<Counted **>(((caddr_t)(this)) + sizeof(Array));
		}

		Counted *get(size_t index);
	};

	ArrayRef(size_t size);
	ArrayRef(const ArrayRef& copy);
	ArrayRef();

	void assign(size_t index, TypeRef& t);

	void reset(TypeRef& object);

	void reset(Counted *object);

	Counted *get(size_t index);

	bool is(size_t index);

	static Array *create(size_t size);

public:
	void resize(size_t size);

	void clear(void);
};

template<typename T>
class arrayref : public ArrayRef
{
public:
	inline arrayref() :	ArrayRef() {};

	inline arrayref(const arrayref& copy) : ArrayRef(copy) {};

	inline arrayref(size_t size) : ArrayRef(size) {};

	inline arrayref(size_t size, typeref<T> t) : ArrayRef(size) {
		reset(t);
	}

	inline arrayref(size_t size, T t) : ArrayRef(size) {
		typeref<T> v(t);
		reset(v);
	}

	inline arrayref& operator=(const arrayref& copy) {
		TypeRef::set(copy);
		return *this;
	}

	inline arrayref& operator=(T t) {
		typeref<T> v(t);
		reset(v);
		return *this;
	}

	inline arrayref& operator=(typeref<T> t) {
		reset(t);
		return *this;
	}

	inline const T& operator[](size_t index) {
		const T* p = typeref<T>::data(ArrayRef::get(index));
		return *p;
	}

	inline typeref<T> operator()(size_t index) {
		return typeref<T>(ArrayRef::get(index));
	}

	inline typeref<T> at(size_t index) {
		return typeref<T>(ArrayRef::get(index));
	}

	inline const T& value(size_t index) {
		const T* p = typeref<T>::data(ArrayRef::get(index));
		return *p;
	}

	inline void value(size_t index, T t) {
		typeref<T> v(t);
		ArrayRef::assign(index, v);
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
		TypeRef::set(NULL);
	}

	inline void realloc(size_t size) {
		TypeRef::set(ArrayRef::create(size));
	}
};

template<>
class arrayref<stringref> : public ArrayRef
{
public:
	inline arrayref() :	ArrayRef() {};

	inline arrayref(const arrayref& copy) : ArrayRef(copy) {};

	inline arrayref(size_t size) : ArrayRef(size) {};

	inline arrayref(size_t size, stringref s) : ArrayRef(size) {
		reset(s);
	}

	inline arrayref(size_t size, stringref::value *v) : ArrayRef(size) {
		reset(v);
	}

	inline arrayref(size_t size, const char *s) : ArrayRef(size) {
		stringref v(s);
		reset(v);
	}

	inline arrayref& operator=(const arrayref& copy) {
		TypeRef::set(copy);
		return *this;
	}

	inline arrayref& operator=(stringref::value *v) {
		reset(v);
		return *this;
	}

	inline arrayref& operator=(const char *s) {
		stringref v(s);
		reset(v);
		return *this;
	}

	inline arrayref& operator=(stringref t) {
		reset(t);
		return *this;
	}

	inline const char *operator[](size_t index) {
		return stringref::str(ArrayRef::get(index));
	}

	inline stringref operator()(size_t index) {
		return stringref(ArrayRef::get(index));
	}

	inline stringref at(size_t index) {
		return stringref(ArrayRef::get(index));
	}

	inline void put(stringref& target, size_t index) {
		TypeRef::put(target, ArrayRef::get(index));
	}

	inline void operator()(size_t index, stringref& t) {
		ArrayRef::assign(index, t);
	}

	inline void operator()(size_t index, const char *s) {
		stringref v(s);
		ArrayRef::assign(index, v);
	}

	inline const char *value(size_t index) {
		return stringref::str(ArrayRef::get(index));
	}

	inline void value(size_t index, const char *s) {
		stringref v(s);
		ArrayRef::assign(index, v);
	}

	inline void value(size_t index, stringref& t) {
		ArrayRef::assign(index, t);
	}

	inline void release(void) {
		TypeRef::set(NULL);
	}

	inline void realloc(size_t size) {
		TypeRef::set(ArrayRef::create(size));
	}
};

template<>
class arrayref<byteref> : public ArrayRef
{
public:
	inline arrayref() :	ArrayRef() {};

	inline arrayref(const arrayref& copy) : ArrayRef(copy) {};

	inline arrayref(size_t size) : ArrayRef(size) {};

	inline arrayref(size_t size, byteref b) : ArrayRef(size) {
		reset(b);
	}

	inline arrayref(size_t size, byteref::value *v) : ArrayRef(size) {
		reset(v);
	}

	inline arrayref(size_t size, const uint8_t *a, size_t s) : ArrayRef(size) {
		byteref v(a, s);
		reset(v);
	}

	inline arrayref& operator=(const arrayref& copy) {
		TypeRef::set(copy);
		return *this;
	}

	inline arrayref& operator=(byteref::value *v) {
		reset(v);
		return *this;
	}

	inline arrayref& operator=(byteref t) {
		reset(t);
		return *this;
	}

	inline const uint8_t *operator[](size_t index) {
		return byteref::data(ArrayRef::get(index));
	}

	inline byteref operator()(size_t index) {
		return byteref(ArrayRef::get(index));
	}

	inline byteref at(size_t index) {
		return byteref(ArrayRef::get(index));
	}

	inline void put(byteref& target, size_t index) {
		TypeRef::put(target, ArrayRef::get(index));
	}

	inline void operator()(size_t index, stringref& t) {
		ArrayRef::assign(index, t);
	}

	inline void operator()(size_t index, const uint8_t *a, size_t s) {
		byteref v(a, s);
		ArrayRef::assign(index, v);
	}

	inline const uint8_t *value(size_t index) {
		return byteref::data(ArrayRef::get(index));
	}

	inline void value(size_t index, const uint8_t *p, size_t s) {
		byteref v(p, s);
		ArrayRef::assign(index, v);
	}

	inline void value(size_t index, byteref& t) {
		ArrayRef::assign(index, t);
	}

	inline void release(void) {
		TypeRef::set(NULL);
	}

	inline void realloc(size_t size) {
		TypeRef::set(ArrayRef::create(size));
	}
};

typedef arrayref<byteref> bytearray_t;
typedef arrayref<stringref> stringarray_t;

} // namespace

#endif
