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

typeref<Type::SecChars>::storage::storage(caddr_t addr, size_t objsize, const char *str, strtype_t strtype) : 
TypeRef::Counted(addr, objsize)
{
    if(str)
    	String::set(mem, objsize + 1, str);
    else
	    mem[0] = 0;

    type = strtype;
}

void typeref<Type::SecChars>::storage::dealloc(void)
{
	memset(&mem[0], 0, size);
	Counted::dealloc();
}

typeref<Type::SecChars>::typeref() :
TypeRef() {}

typeref<Type::SecChars>::typeref(const typeref<Type::SecChars>& copy) :
TypeRef(copy) {}

typeref<Type::SecChars>::typeref(const char *str, strtype_t strtype) :
TypeRef()
{
    size_t size = 16;
    
    if(str)
        size = fix(strlen(str));

    caddr_t p = TypeRef::alloc(sizeof(storage) + size);
    TypeRef::set(new(mem(p)) storage(p, size, str, strtype));
}

Type::SecChars::strtype_t typeref<Type::SecChars>::type(void) const
{
    storage *v = polystatic_cast<storage *>(ref);
    if(v)
        return v->type;

    return GENERIC_STRING;
}

size_t typeref<Type::SecChars>::len(void) const
{
    storage *v = polystatic_cast<storage *>(ref);
    if(!v)
        return 0;

    return v->len();
}

size_t typeref<Type::SecChars>::size(void) const
{
    storage *v = polystatic_cast<storage *>(ref);
    size_t len = v->len();

    if(!v)
        return 0;

    switch(v->type) {
    case GENERIC_STRING:
        return len * 8;
    case B64_STRING:
        len = (v->len() * 3) / 4;
        if(v->mem[v->len() - 1] == '=') {
            --len;
            if(v->mem[v->len() - 2] == '=')
                --len;
        }
        return len * 8;
    default:
        return len * 4;
    }
}

void typeref<Type::SecChars>::set(const char *str, strtype_t strtype)
{
    clear();
    size_t size = 16;

    if(str)
        size = fix(strlen(str));

    caddr_t p = TypeRef::alloc(sizeof(storage) + size);
    TypeRef::set(new(mem(p)) storage(p, size, str, strtype));
}

const char *typeref<Type::SecChars>::operator*() const 
{
    storage *v = polystatic_cast<storage *>(ref);
    if(!v)
        return NULL;

    return &v->mem[0];
}

bool typeref<Type::SecChars>::operator==(const typeref<Type::SecChars>& ptr) const
{
    storage *v1 = polystatic_cast<storage*>(ref);
    storage *v2 = polystatic_cast<storage*>(ptr.ref);
    if(!v1 || !v2)
        return false;
    return eq(&(v1->mem[0]), &(v2->mem[0]));
}

bool typeref<Type::SecChars>::operator==(const char *obj) const
{
    storage *v = polystatic_cast<storage *>(ref);
    if(!v)
        return false;
    return eq(&(v->mem[0]), obj);
}

typeref<Type::SecChars>& typeref<Type::SecChars>::operator=(const typeref<Type::SecChars>& objref)
{
    TypeRef::set(objref);
    return *this;
}

typeref<Type::SecChars>& typeref<Type::SecChars>::operator=(const char *str)
{
    set(str);
    return *this;
}

void typeref<Type::SecChars>::b64(const uint8_t *bytes, size_t bsize)
{
    clear();
    size_t len = ((bsize * 4 / 3) + 1);

    caddr_t p = TypeRef::alloc(sizeof(storage) + len);
    storage *s = new(mem(p)) storage(p, len, NULL, secure::B64_STRING);
    String::b64encode(&s->mem[0], bytes, bsize);
    TypeRef::set(s);
}

void typeref<Type::SecChars>::hex(const uint8_t *bytes, size_t bsize)
{
    clear();
    size_t len = bsize * 2;

    caddr_t p = TypeRef::alloc(sizeof(storage) + len);
    storage *s = new(mem(p)) storage(p, len, NULL, secure::HEX_STRING);

    for(size_t index = 0; index < bsize; ++index) {
        snprintf(&s->mem[index * 2], 3, "%2.2x", bytes[index]);
    }

    TypeRef::set(s);
}

typeref<Type::KeyBytes>::storage::storage(caddr_t addr, size_t objsize, const uint8_t *key, keytype_t keytype) : 
TypeRef::Counted(addr, objsize)
{
    if(key)
        memcpy(&mem[0], key, objsize);
    else
        Random::key(&mem[0], objsize);

    type = keytype;
}

void typeref<Type::KeyBytes>::storage::dealloc(void)
{
	memset(&mem[0], 0, size);
	Counted::dealloc();
}

typeref<Type::KeyBytes>::typeref() :
TypeRef() {}

typeref<Type::KeyBytes>::typeref(const typeref<Type::KeyBytes>& copy) :
TypeRef(copy) {}

typeref<Type::KeyBytes>::typeref(const uint8_t *key, size_t keysize, keytype_t keytype) :
TypeRef()
{
    keysize /= 8;
    caddr_t p = TypeRef::alloc(sizeof(storage) + keysize);
    TypeRef::set(new(mem(p)) storage(p, keysize, key, keytype));
}

typeref<Type::KeyBytes>::typeref(size_t keysize, keytype_t keytype) :
TypeRef()
{
    keysize /= 8;
    caddr_t p = TypeRef::alloc(sizeof(storage) + keysize);
    TypeRef::set(new(mem(p)) storage(p, keysize, NULL, keytype));
}

Type::KeyBytes::keytype_t typeref<Type::KeyBytes>::type(void) const
{
    storage *v = polystatic_cast<storage *>(ref);
    if(v)
        return v->type;

    return UNDEFINED_KEYTYPE;
}

size_t typeref<Type::KeyBytes>::bits(void) const
{
    storage *v = polystatic_cast<storage *>(ref);
    if(!v)
        return 0;

    return v->size * 8;
}

size_t typeref<Type::KeyBytes>::size(void) const
{
    storage *v = polystatic_cast<storage *>(ref);
    if(!v)
        return 0;

    return v->size;
}

void typeref<Type::KeyBytes>::set(const uint8_t *key, size_t keysize, keytype_t keytype)
{
    clear();
    keysize /= 8;
    caddr_t p = TypeRef::alloc(sizeof(storage) + keysize);
    TypeRef::set(new(mem(p)) storage(p, keysize, key, keytype));
}

void typeref<Type::KeyBytes>::generate(size_t keysize, keytype_t keytype)
{
    clear();
    keysize /= 8;
    caddr_t p = TypeRef::alloc(sizeof(storage) + keysize);
    TypeRef::set(new(mem(p)) storage(p, keysize, NULL, keytype));
}


const uint8_t *typeref<Type::KeyBytes>::operator*() const 
{
    storage *v = polystatic_cast<storage *>(ref);
    if(!v)
        return NULL;

    return &v->mem[0];
}

uint8_t *typeref<Type::KeyBytes>::data()
{
    storage *v = polystatic_cast<storage *>(ref);
    if(!v)
        return NULL;

    return &v->mem[0];
}

bool typeref<Type::KeyBytes>::operator==(const typeref<Type::KeyBytes>& ptr) const
{
    storage *v1 = polystatic_cast<storage*>(ref);
    storage *v2 = polystatic_cast<storage*>(ptr.ref);
    if(!v1 || !v2)
        return false;
    if(v1->size != v2->size)
        return false;
    return !memcmp(&(v1->mem[0]), &(v2->mem[0]), v1->size);
}

typeref<Type::KeyBytes>& typeref<Type::KeyBytes>::operator=(const typeref<Type::KeyBytes>& objref)
{
    TypeRef::set(objref);
    return *this;
}

} // namespace ucommon
