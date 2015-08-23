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

#ifndef _UCOMMON_MAPREF_H_
#define _UCOMMON_MAPREF_H_

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

#ifndef _UCOMMON_LINKED_H_
#include <ucommon/linked.h>
#endif

#ifndef _UCOMMON_THREAD_H_
#include <ucommon/thread.h>
#endif

namespace ucommon {

class __EXPORT MapRef : public TypeRef
{
protected:
	class Index : public LinkedObject
	{
	public:
		Index(LinkedObject *root);

		Counted *key, *value;
	};

	class Map : public Counted
	{
	public:
		memalloc pool;
		condlock_t lock;
		LinkedObject *free;
		size_t count;

		explicit Map(size_t indexes, size_t paging = 0);
	
		virtual void dealloc();

		inline Index *get(void) {
			return reinterpret_cast<Index *>(((caddr_t)(this)) + sizeof(Map));
		}

		LinkedObject *path(size_t keyvalue);
	};

	MapRef(size_t paths, size_t paging = 0);
	MapRef(const MapRef& copy);
	MapRef();

	void assign(TypeRef& key, TypeRef& value);

	static Map *create(size_t paths, size_t paging = 0);

protected:
	LinkedObject *path(size_t keyvalue);

public:
	size_t count(void);

	void purge(void);

	static size_t index(size_t& key, const uint8_t *addr, size_t len);
};

template<typename T>
inline size_t mapkeypath(typeref<T>& object)
{
	size_t path = sizeof(T);
	return MapRef::index(path, (const uint8_t *)(object()), sizeof(T));
}

template<>
inline size_t mapkeypath<const char *>(typeref<const char *>& object)
{
	size_t path = 1;
	return MapRef::index(path, (const uint8_t *)(*object), object.len());
}

template<>
inline size_t mapkeypath<const uint8_t *>(typeref<const uint8_t *>& object)
{
	size_t path = object.size();
	return MapRef::index(path, *object, object.size());
}

template<typename T>
class mapref : public MapRef
{
public:
	inline mapref() :	MapRef() {};

	inline mapref(const mapref& copy) : MapRef(copy) {};

	inline mapref(size_t paths, size_t paging = 0) : MapRef(paths, paging) {};

	inline mapref& operator=(const mapref& copy) {
		TypeRef::set(copy);
		return *this;
	}
};

} // namespace

#endif
