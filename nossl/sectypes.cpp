// Copyright (C) 2010-2014 David Sugar, Tycho Softworks.
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

#include "local.h"

static size_t fix(size_t size)
{
    return size + (16 - (size % 16));
}

namespace ucommon {

typeref<secure_chars>::storage::storage(caddr_t addr, size_t objsize, const char *str, strtype_t strtype) : 
TypeRef::Counted(addr, objsize)
{
    if(str)
    	String::set(mem, objsize + 1, str);
    else
	    mem[0] = 0;

    type = strtype;
}

void typeref<secure_chars>::storage::dealloc(void)
{
	memset(&mem[0], 0, size);
	Counted::dealloc();
}

typeref<secure_chars>::typeref() :
TypeRef() {}

typeref<secure_chars>::typeref(const typeref<secure_chars>& copy) :
TypeRef(copy) {}

typeref<secure_chars>::typeref(const char *str, strtype_t strtype) :
TypeRef()
{
    size_t size = 16;
    
    if(str)
        size = fix(strlen(str));

    caddr_t p = TypeRef::alloc(sizeof(storage) + size);
    TypeRef::set(new(mem(p)) storage(p, size, str, strtype));
}

secure_chars::strtype_t typeref<secure_chars>::type(void)
{
    storage *v = polystatic_cast<storage *>(ref);
    if(v)
        return v->type;

    return GENERIC_STRING;
}

size_t typeref<secure_chars>::size(void)
{
    storage *v = polystatic_cast<storage *>(ref);
    if(!v)
        return 0;

    switch(v->type) {
    case GENERIC_STRING:
        return v->len() * 8;
    default:
        return v->len() * 4;
    }
}

void typeref<secure_chars>::set(const char *str, strtype_t strtype)
{
    release();
    size_t size = 16;

    if(str)
        size = fix(strlen(str));

    caddr_t p = TypeRef::alloc(sizeof(storage) + size);
    TypeRef::set(new(mem(p)) storage(p, size, str, strtype));
}

const char *typeref<secure_chars>::operator*() const 
{
    storage *v = polystatic_cast<storage *>(ref);
    if(!v)
        return NULL;

    return &v->mem[0];
}

bool typeref<secure_chars>::operator==(const typeref<secure_chars>& ptr) const
{
    storage *v1 = polystatic_cast<storage*>(ref);
    storage *v2 = polystatic_cast<storage*>(ptr.ref);
    if(!v1 || !v2)
        return false;
    return eq(&(v1->mem[0]), &(v2->mem[0]));
}

bool typeref<secure_chars>::operator==(const char *obj) const
{
    storage *v = polystatic_cast<storage *>(ref);
    if(!v)
        return false;
    return eq(&(v->mem[0]), obj);
}

typeref<secure_chars>& typeref<secure_chars>::operator=(const typeref<secure_chars>& objref)
{
    TypeRef::set(objref);
    return *this;
}

typeref<secure_chars>& typeref<secure_chars>::operator=(const char *str)
{
    set(str);
    return *this;
}

void typeref<secure_chars>::b64(const uint8_t *bytes, size_t bsize)
{
    release();
    size_t len = ((bsize * 4 / 3) + 1);

    caddr_t p = TypeRef::alloc(sizeof(storage) + len);
    storage *s = new(mem(p)) storage(p, len, NULL, secure::B64_STRING);
    String::b64encode(&s->mem[0], bytes, bsize);
    TypeRef::set(s);
}

void typeref<secure_chars>::hex(const uint8_t *bytes, size_t bsize)
{
    release();
    size_t len = bsize * 2;

    caddr_t p = TypeRef::alloc(sizeof(storage) + len);
    storage *s = new(mem(p)) storage(p, len, NULL, secure::B64_STRING);

    for(size_t index = 0; index < bsize; ++index) {
        snprintf(&s->mem[index * 2], 3, "%2.2x", bytes[index]);
    }

    TypeRef::set(s);
}

typeref<secure_keybytes>::storage::storage(caddr_t addr, size_t objsize, const uint8_t *key, keytype_t keytype) : 
TypeRef::Counted(addr, objsize)
{
    if(key)
        memcpy(&mem[0], key, objsize);
    else
        Random::key(&mem[0], objsize);

    type = keytype;
}

void typeref<secure_keybytes>::storage::dealloc(void)
{
	memset(&mem[0], 0, size);
	Counted::dealloc();
}

typeref<secure_keybytes>::typeref() :
TypeRef() {}

typeref<secure_keybytes>::typeref(const typeref<secure_keybytes>& copy) :
TypeRef(copy) {}

typeref<secure_keybytes>::typeref(const uint8_t *key, size_t keysize, keytype_t keytype) :
TypeRef()
{
    caddr_t p = TypeRef::alloc(sizeof(storage) + keysize);
    TypeRef::set(new(mem(p)) storage(p, keysize, key, keytype));
}

typeref<secure_keybytes>::typeref(size_t keysize, keytype_t keytype) :
TypeRef()
{
    caddr_t p = TypeRef::alloc(sizeof(storage) + keysize);
    TypeRef::set(new(mem(p)) storage(p, keysize, NULL, keytype));
}

secure_keybytes::keytype_t typeref<secure_keybytes>::type(void)
{
    storage *v = polystatic_cast<storage *>(ref);
    if(v)
        return v->type;

    return UNDEFINED_KEYTYPE;
}

size_t typeref<secure_keybytes>::size(void)
{
    storage *v = polystatic_cast<storage *>(ref);
    if(!v)
        return 0;

    return v->size * 8;
}

void typeref<secure_keybytes>::set(const uint8_t *key, size_t keysize, keytype_t keytype)
{
    release();
    caddr_t p = TypeRef::alloc(sizeof(storage) + keysize);
    TypeRef::set(new(mem(p)) storage(p, keysize, key, keytype));
}

void typeref<secure_keybytes>::generate(size_t keysize, keytype_t keytype)
{
    release();
    caddr_t p = TypeRef::alloc(sizeof(storage) + keysize);
    TypeRef::set(new(mem(p)) storage(p, keysize, NULL, keytype));
}


const uint8_t *typeref<secure_keybytes>::operator*() const 
{
    storage *v = polystatic_cast<storage *>(ref);
    if(!v)
        return NULL;

    return &v->mem[0];
}

bool typeref<secure_keybytes>::operator==(const typeref<secure_keybytes>& ptr) const
{
    storage *v1 = polystatic_cast<storage*>(ref);
    storage *v2 = polystatic_cast<storage*>(ptr.ref);
    if(!v1 || !v2)
        return false;
    if(v1->size != v2->size)
        return false;
    return !memcmp(&(v1->mem[0]), &(v2->mem[0]), v1->size);
}

typeref<secure_keybytes>& typeref<secure_keybytes>::operator=(const typeref<secure_keybytes>& objref)
{
    TypeRef::set(objref);
    return *this;
}

} // namespace ucommon
