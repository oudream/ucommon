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

secure::string::storage::storage(caddr_t addr, size_t objsize, const char *str) : 
TypeRef::Counted(addr, objsize)
{
    if(str)
    	String::set(mem, objsize + 1, str);
    else
	    mem[0] = 0;
}


void secure::string::storage::dealloc(void)
{
	memset(&mem[0], 0, size);
	Counted::dealloc();
}

secure::string::string() :
TypeRef() {}

secure::string::string(const secure::string& copy) :
TypeRef(copy) {}

secure::string::string(const char *str) :
TypeRef()
{
    size_t size = 16;
    
    if(str)
        size = fix(strlen(str));

    caddr_t p = TypeRef::alloc(sizeof(storage) + size);
    TypeRef::set(new(mem(p)) storage(p, size, str));
}

void secure::string::set(const char *str)
{
    release();
    size_t size = 16;

    if(str)
        size = fix(strlen(str));

    caddr_t p = TypeRef::alloc(sizeof(storage) + size);
    TypeRef::set(new(mem(p)) storage(p, size, str));
}

const char *secure::string::operator*() const 
{
    storage *v = polystatic_cast<storage *>(ref);
    if(!v)
        return NULL;

    return &v->mem[0];
}

bool secure::string::operator==(const secure::string& ptr) const
{
    storage *v1 = polystatic_cast<storage*>(ref);
    storage *v2 = polystatic_cast<storage*>(ptr.ref);
    if(!v1 || !v2)
        return false;
    return eq(&(v1->mem[0]), &(v2->mem[0]));
}

bool secure::string::operator==(const char *obj) const
{
    storage *v = polystatic_cast<storage *>(ref);
    if(!v)
        return false;
    return eq(&(v->mem[0]), obj);
}

secure::string& secure::string::operator=(const secure::string& objref)
{
    TypeRef::set(objref);
    return *this;
}

secure::string& secure::string::operator=(const char *str)
{
    set(str);
    return *this;
}

} // namespace ucommon
