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
 * Maps of thread-safe strongly typed heap objects.  This is used for
 * maps of smart pointers to immutable heap instances of object types.
 * Shared and exclusive locking is used based on lookup or modify operations. 
 * @file ucommon/mapref.h
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
	class Map;

	class __EXPORT Index : public LinkedObject
	{
	public:
		friend class Map;

		Index(LinkedObject **origin);

		Index();

		Counted *key, *value;
		LinkedObject **root;
	};

	class __EXPORT Map : public Counted
	{
	public:
		friend class MapRef;

		memalloc pool;
		condlock_t lock;
		LinkedObject *free, *last;
		size_t count, alloc;

		explicit Map(void *addr, size_t indexes, size_t paging = 0);
	
		virtual void dealloc();

		inline LinkedObject **get(void) {
			return reinterpret_cast<LinkedObject **>(((caddr_t)(this)) + sizeof(Map));
		}

		Index *create(size_t path);

		Index *append();

		void remove(Index *index);

		LinkedObject *modify(size_t key = 0);

		LinkedObject *access(size_t key = 0);
	};

	class __EXPORT Instance
	{
	protected:
		Map *map;
		LinkedObject *index;
		size_t path;

		Instance();

		Instance(MapRef& from);

		Instance(Map *map);

		Instance(const Instance& copy);

		void assign(const Instance& copy);

		void assign(MapRef& from);

		void drop(void);

		Counted *key();

		Counted *value();

	public:
		~Instance();

		void rewind();

		bool next();

		bool eol();

		bool top();

		inline operator bool() {
			return index != NULL;
		}

		inline bool operator!() {
			return index == NULL;
		}
	};

	MapRef(size_t paths, size_t paging = 0);
	MapRef(const MapRef& copy);
	MapRef();

	void assign(TypeRef& key, TypeRef& value);

	static Map *create(size_t paths, size_t paging = 0);

	linked_pointer<Index> access(size_t keyvalue = 0);

	linked_pointer<Index> modify(size_t keyvalue = 0);

	void append(TypeRef& value);

	void add(size_t path, TypeRef& key, TypeRef& value);

	void update(Index *ind, TypeRef& value);

	void remove(Index *ind);

	void release();

	void commit();

public:
	size_t count(void);

	size_t used(void);

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

template<typename K, typename V>
class mapref : public MapRef
{
protected:
	bool erase(typeref<K>& key) {
		linked_pointer<Index> ip = modify(mapkeypath<K>(key));
		while(ip) {
			typeref<K> kv(ip->key);
			if(kv.is() && kv == key) {
				MapRef::remove(*ip);
				MapRef::commit();
				return true;
			}
			ip.next();
		}
		MapRef::commit();
		return false;
	}	

public:
	class instance : public MapRef::Instance
	{
	protected:
		inline instance(MapRef *ref) : Instance(ref) {};

	public:
		inline instance(const instance& copy) : Instance(static_cast<const Instance&>(copy)) {};

		inline instance(mapref& from) : Instance(static_cast<MapRef&>(from)) {};

		inline instance() : Instance() {};

		inline typeref<K> key() {
			return typeref<K>(Instance::key());
		}

		inline typeref<V> value() {
			return typeref<V>(Instance::value());
		}

		inline instance& operator++() {
			next();
			return *this;
		}

		inline instance& operator=(const instance& copy) {
			assign(static_cast<const Instance&>(copy));
			return *this;
		}

		inline instance& operator=(mapref& from) {
			assign(static_cast<MapRef&>(from));
			return *this;
		}
	};

	inline mapref(const mapref& copy) : MapRef(copy) {};

	inline mapref(size_t paths = 37, size_t paging = 0) : MapRef(paths, paging) {};

	inline mapref& operator=(const mapref& copy) {
		TypeRef::set(copy);
		return *this;
	}

	inline instance operator*() {
		return instance(this);
	}

	void value(typeref<K>& key, typeref<V>& val) {
		size_t path = mapkeypath<K>(key);
		linked_pointer<Index> ip = modify(path);
		while(ip) {
			typeref<K> kv(ip->key);
			if(kv.is() && kv == key) {
				update(*ip, val);
				commit();
				return;
			}
			ip.next();
		}
		add(path, key, val);
		commit();
	}

	typeref<V> at(typeref<K>& key) {
		linked_pointer<Index> ip = access(mapkeypath<K>(key));
		while(ip) {
			typeref<K> kv(ip->key);
			if(kv.is() && kv == key) {
				typeref<V> result(ip->value);
				release();
				return result;
			}
			ip.next();
		}
		release();
		return typeref<V>();
	}	

	typeref<V> take(typeref<K>& key) {
		linked_pointer<Index> ip = modify(mapkeypath<K>(key));
		while(ip) {
			typeref<K> kv(ip->key);
			if(kv.is() && kv == key) {
				typeref<V> result(ip->value);
				if(result.is())
					MapRef::remove(*ip);
				commit();
				return result;
			}
			ip.next();
		}
		commit();
		return typeref<V>();
	}	

	inline bool remove(typeref<K>& key) {
		return erase(key);
	}

	inline bool remove(K k) {
		typeref<K> key(k);
		return erase(key);
	}

	inline typeref<V> operator()(typeref<K>& key) {
		return at(key);
	}

	inline typeref<V> operator()(K k) {
		typeref<K> key(k);
		return at(key);
	}

	inline void operator()(typeref<K>& key, typeref<V>& val) {
		value(key, val);
	}	

	inline void operator()(K k, V v) {
		typeref<K> key(k);
		typeref<V> val(v);
		value(key, val);
	}
};

template<typename T>
class listref : public MapRef
{
protected:
	bool erase(typeref<T>& value) {
		linked_pointer<Index> ip = modify();
		while(ip) {
			typeref<T> kv(ip->value);
			if(kv.is() && kv == value) {
				MapRef::remove(*ip);
				MapRef::commit();
				return true;
			}
			ip.next();
		}
		MapRef::commit();
		return false;
	}
	
public:
	class instance : public MapRef::Instance
	{
	protected:
		inline instance(MapRef *ref) : Instance(ref) {};

	public:
		inline instance(const instance& copy) : Instance(static_cast<const Instance&>(copy)) {};

		inline instance(listref& from) : Instance(static_cast<MapRef&>(from)) {};

		inline instance() : Instance() {};

		inline const T& operator*() {
			return *(Instance::value());
		}

		inline const T* operator->() {
			return Instance::value();
		}

		inline instance& operator++() {
			next();
			return *this;
		}

		inline instance& operator=(const instance& copy) {
			assign(static_cast<const Instance&>(copy));
			return *this;
		}

		inline instance& operator=(listref& from) {
			assign(static_cast<MapRef&>(from));
			return *this;
		}
	};

	inline listref(const listref& copy) : MapRef(copy) {};

	inline listref(size_t paging = 0) : MapRef(1, paging) {};

	inline listref& operator=(const listref& copy) {
		TypeRef::set(copy);
		return *this;
	}

	inline instance operator*() {
		return instance(this);
	}

	inline listref& operator<<(typeref<T>& value) {
		append(value);
		return *this;
	}

	inline listref& operator<<(T t) {
		typeref<T> v(t);
		append(v);
		return *this;
	}

	inline bool remove(typeref<T>& key) {
		return erase(key);
	}

	inline bool remove(T t) {
		typeref<T> key(t);
		return erase(key);
	}

	inline typeref<T> take(size_t offset) {
		linked_pointer<Index> ip = modify();
		while(ip && offset--) {
			ip.next();
		}
		typeref<T> v(ip->value);
		if(v.is())
			MapRef::remove(*ip);
		commit();
		return v;
	}

	inline typeref<T> at(size_t offset) {
		linked_pointer<Index> ip = access();
		while(ip && offset--) {
			ip.next();
		}
		typeref<T> v(ip->value);
		release();
		return v;
	}

	inline typeref<T> operator[](size_t offset) {
		return at(offset);
	}
};


} // namespace

#endif
