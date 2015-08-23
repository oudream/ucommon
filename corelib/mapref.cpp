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
#include <ucommon/typeref.h>
#include <ucommon/string.h>
#include <ucommon/thread.h>
#include <ucommon/linked.h>
#include <ucommon/mapref.h>
#include <cstdlib>

namespace ucommon {

MapRef::Map::Map(void *addr, size_t indexes, size_t paging) :
Counted(addr, indexes), pool(paging)
{
    size_t index = 0;
    LinkedObject **list = get();
    free = NULL;
    
    while(index < indexes) {
        list[index++] = NULL;
    }
}

LinkedObject *MapRef::Map::path(size_t key) const
{
	return (get())[key % size];
}

LinkedObject **MapRef::Map::root(size_t key)
{
    LinkedObject **list = get();
    return &list[key % size];
}

void MapRef::Map::dealloc()
{
    size_t index = 0;
    LinkedObject **list = get();
    linked_pointer<Index> ip;

    if(!size)
        return;

    while(index < size) {
		ip = list[index];
		while(ip) {
			if(ip->key)
				ip->key->release();
			if(ip->value)
				ip->value->release();
			ip.next();
		}
		++index;
	}	
    size = 0;
	free = NULL;
	pool.purge();
    Counted::dealloc();
}

MapRef::MapRef() :
TypeRef()
{
}

LinkedObject *MapRef::path(size_t key) const
{
	Map *m = polydynamic_cast<Map *>(ref);
	if(!m || !m->size)
		return NULL;

	return m->path(key);
}

linked_pointer<MapRef::Index> MapRef::shared(size_t key) const
{
    linked_pointer<Index> ip;
	Map *m = polydynamic_cast<Map *>(ref);
	if(!m || !m->size)
		return ip;

    m->lock.share();
    ip = m->path(key);
	return ip;
}

LinkedObject **MapRef::exclusive(size_t key)
{
    Map *m = polydynamic_cast<Map *>(ref);
	if(!m || !m->size)
		return NULL;

    m->lock.exclusive();
	return m->root(key);
}

void MapRef::unlock()
{
    Map *m = polydynamic_cast<Map *>(ref);
	if(!m || !m->size)
		return;

    m->lock.release();
}

size_t MapRef::index(size_t& key, const uint8_t *addr, size_t len)
{
	while(len-- && addr) {
		key ^= (key << 3) ^ *addr;
		++addr;
	}
	return key;
}

} // namespace
