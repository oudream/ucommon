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

MapRef::Index::Index() :
LinkedObject()
{
    key = value = NULL;
    root = NULL;
}

MapRef::Index::Index(LinkedObject **origin) :
LinkedObject(origin)
{
    key = value = NULL;
    root = origin;
}

MapRef::Map::Map(void *addr, size_t indexes, size_t paging) :
Counted(addr, indexes), pool(paging)
{
    size_t index = 0;
    LinkedObject **list = get();
    free = last = NULL;
    count = alloc = 0;
    
    while(index < indexes) {
        list[index++] = NULL;
    }
}

MapRef::Index *MapRef::Map::create(size_t key)
{
    LinkedObject **list = get();
    caddr_t p = (caddr_t)(free);
    if(free)
        free = free->getNext();
    else {
        ++alloc;
        p = (caddr_t)pool.alloc(sizeof(Index));
    }
    ++count;
    return new(p) Index(&list[key % size]);
}

MapRef::Index *MapRef::Map::append()
{
    LinkedObject **list = get();
    caddr_t p = (caddr_t)(free);
    if(free)
        free = free->getNext();
    else {
        ++alloc;
        p = (caddr_t)pool.alloc(sizeof(Index));
    }
    ++count;
    Index *ip = new(p) Index();
    if(last) {
        Index *lp = static_cast<Index *>(last);
        lp->Next = ip;
    }
    else
        list[0] = ip;
    last = ip;
    ip->Next = NULL;
    ip->root = &list[0];
    return ip;
}       

void MapRef::Map::remove(Index *index)
{
    if(!index)
        return;

    if(index->key)
        index->key->release();

    if(index->value)
        index->value->release();

    --count;
    if(last && index == last) {
        last = *(index->root);
        if(last == index)
            last = NULL;
        else {
            while(last && last->getNext() != index) {
                last = last->getNext();
            }
        }
    }
    index->delist(index->root); 
    index->enlist(&free);
}

LinkedObject *MapRef::Map::access(size_t key)
{
    lock.access();
	return (get())[key % size];
}

LinkedObject *MapRef::Map::modify(size_t key)
{
    lock.modify();
    return (get())[key % size];
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
	free = last = NULL;
	pool.purge();
    Counted::dealloc();
}

MapRef::Instance::Instance(Map *vmap)
{
    if(!vmap)
        return;

    map = vmap;
    map->retain();
    map->lock.access();
    rewind();
}

MapRef::Instance::Instance(MapRef& from)
{
    map = static_cast<Map*>(from.ref);
    if(!map)
        return;

    map->retain();
    map->lock.access();
    rewind();
}

MapRef::Instance::Instance()
{
    map = NULL;
    index = NULL;
    path = 0;
}

MapRef::Instance::Instance(const Instance& copy)
{
    map = copy.map;
    index = copy.index;
    path = copy.path;
    if(!map)
        return;

    map->retain();
    map->lock.access();
}

MapRef::Instance::~Instance()
{
    drop();
}

void MapRef::Instance::drop()
{
    if(!map)
        return;

    map->lock.release();
    map->release();
    map = NULL;
    index = NULL;
    path = 0;
}

void MapRef::Instance::assign(const Instance& copy)
{
    drop();
    map = copy.map;
    index = copy.index;
    path = copy.path;
    if(!map)
        return;

    map->retain();
    map->lock.access();
}

void MapRef::Instance::assign(MapRef& from)
{
    drop();
    map = static_cast<Map*>(from.ref);
    if(!map)
        return;

    map->retain();
    map->lock.access();
    rewind();
}

void MapRef::Instance::rewind()
{
    if(!map)
        return;

    path = 0;
    index = (map->get())[0];
    if(!index)
        next();
}

bool MapRef::Instance::top()
{
    if(!map)
        return false;

    if(path > 0)
        return false;

    if(index != (map->get())[0])
        return false;

    return true;
}

TypeRef::Counted *MapRef::Instance::key()
{
    if(!index)
        return NULL;

    return (static_cast<Index*>(index))->key;
}

TypeRef::Counted *MapRef::Instance::value()
{
    if(!index)
        return NULL;

    return (static_cast<Index*>(index))->value;
}

bool MapRef::Instance::eol()
{
    if(!map)
        return false;

    if(path < map->size)
        return false;

    return true;
}

bool MapRef::Instance::next()
{
    if(!map)
        return false;

    if(index)
        index = index->getNext();

    if(index)
        return true;

    while(++path < map->size) {
        index = (map->get())[path];
        if(index)
            return true;
    }
    return false;
}

MapRef::MapRef() :
TypeRef()
{
}

MapRef::MapRef(const MapRef& copy) :
TypeRef(copy)
{
}

MapRef::MapRef(size_t indexes, size_t paging) :
TypeRef(create(indexes, paging))
{
}

size_t MapRef::used()
{
    Map *m = polydynamic_cast<Map *>(ref);
	if(!m)
        return 0;

    return m->alloc;
}

size_t MapRef::count()
{
    Map *m = polydynamic_cast<Map *>(ref);
	if(!m)
        return 0;

    return m->count;
}

void MapRef::remove(Index *ind)
{
    Map *m = polydynamic_cast<Map *>(ref);
	if(!m)
        return;

    m->remove(ind);
}

MapRef::Map *MapRef::create(size_t indexes, size_t paging)
{
    if(!indexes)
        return NULL;

    size_t s = sizeof(Map) + (indexes * sizeof(Index *));
    caddr_t p = TypeRef::alloc(s);
    return new(mem(p)) Map(p, indexes, paging);
}

void MapRef::update(Index *ind, TypeRef& value)
{
    if(!ind)
        return;

    if(ind->value)
        ind->value->release();
    ind->value = value.ref;
    if(ind->value)
        ind->value->retain();
}

void MapRef::append(TypeRef& value)
{
    Map *m = polydynamic_cast<Map *>(ref);
	if(!m || !m->size)
		return;

    m->lock.modify();
    Index *ind = m->append();
    if(!ind) {
        m->lock.commit();
        return;
    }
    ind->key = NULL;
    ind->value = value.ref;
    if(ind->value)
        ind->value->retain();
    m->lock.commit();
}

void MapRef::add(size_t keypath, TypeRef& key, TypeRef& value)
{
    Map *m = polydynamic_cast<Map *>(ref);
	if(!m || !m->size)
		return;

    Index *ind = m->create(keypath);
    if(!ind) {
        return;
    }
    ind->key = key.ref;
    ind->value = value.ref;
    if(ind->key)
        ind->key->retain();
    if(ind->value)
        ind->value->retain();
}

linked_pointer<MapRef::Index> MapRef::access(size_t key)
{
    linked_pointer<Index> ip;
	Map *m = polydynamic_cast<Map *>(ref);
	if(!m || !m->size)
		return ip;

    m->retain();
    ip = m->access(key);
	return ip;
}

linked_pointer<MapRef::Index> MapRef::modify(size_t key)
{
    linked_pointer<Index> ip;
	Map *m = polydynamic_cast<Map *>(ref);
	if(!m || !m->size)
		return ip;

    m->retain();
    ip = m->modify(key);
	return ip;
}

void MapRef::commit()
{
    Map *m = polydynamic_cast<Map *>(ref);
	if(!m || !m->size)
		return;

    m->lock.commit();
    m->release();
}

void MapRef::release()
{
    Map *m = polydynamic_cast<Map *>(ref);
	if(!m || !m->size)
		return;

    m->lock.release();
    m->release();
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
