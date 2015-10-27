// Copyright (C) 1999-2005 Open Source Telecom Corporation.
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

#include <ucommon-config.h>
#include <ucommon/export.h>
#include <ucommon/protocols.h>
#include <ucommon/string.h>
#include <ucommon/memory.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#undef  getc
#undef  putc
#undef  puts
#undef  gets

namespace ucommon {

char *MemoryProtocol::dup(const char *str)
{
    if(!str)
        return NULL;
    size_t len = strlen(str) + 1;
    char *mem = static_cast<char *>(alloc(len));
    if(mem)
        String::set(mem, len, str);
    else
        __THROW_ALLOC();
    return mem;
}

void *MemoryProtocol::dup(void *obj, size_t size)
{
    assert(obj != NULL);
    assert(size > 0);

    char *mem = static_cast<char *>(alloc(size));
    if(mem)
        memcpy(mem, obj, size);
    else
        __THROW_ALLOC();
    return mem;
}

void *MemoryProtocol::zalloc(size_t size)
{
    void *mem = alloc(size);

    if(mem)
        memset(mem, 0, size);
    else
        __THROW_ALLOC();

    return mem;
}

MemoryProtocol::~MemoryProtocol()
{
}

MemoryRedirect::MemoryRedirect(MemoryProtocol *protocol)
{
    target = protocol;
}

void *MemoryRedirect::_alloc(size_t size)
{
    // a null redirect uses the heap...
    if(!target)
        return malloc(size);

    return target->_alloc(size);
}

void LockingProtocol::_lock(void)
{
}

void LockingProtocol::_unlock(void)
{
}

LockingProtocol::~LockingProtocol()
{
}

ObjectProtocol::~ObjectProtocol()
{
}

KeyProtocol::~KeyProtocol()
{
}

bool KeyProtocol::equal(const KeyProtocol& key) const
{
    if(keytype() != key.keytype())
        return false;

    if(keysize() != key.keysize() || !keysize())
        return false;

    if(memcmp(keydata(), key.keydata(), keysize()))
        return false;

    return true;
}

ObjectProtocol *ObjectProtocol::copy(void)
{
    retain();
    return this;
}

class __LOCAL _input_long : public InputProtocol
{
public:
    long* ref;
    size_t pos;
    char buf[32];

    _input_long(long& v);

    int _input(int code);
};

class __LOCAL _input_double : public InputProtocol
{
public:
    double* ref;
    bool dot;
    bool e;
    size_t pos;
    char buf[60];

    _input_double(double& v);

    int _input(int code);
};

_input_long::_input_long(long& v)
{
    ref = &v;
    v = 0l;
    pos = 0;
}

_input_double::_input_double(double& v)
{
    dot = e = false;
    v = 0.0;
    pos = 0;
    ref = &v;
}

int _input_long::_input(int code)
{
    if(code == '-' && !pos)
        goto valid;

    if(isdigit(code) && pos < sizeof(buf) - 1)
        goto valid;

    buf[pos] = 0;
    if(pos)
        sscanf(buf, "%ld", ref);

    return code;

valid:
    buf[pos++] = code;
    return 0;
}

int _input_double::_input(int code)
{
    if((code == '-') && !pos)
        goto valid;

    if((code == '-') && buf[pos] == 'e')
        goto valid;

    if(tolower(code) == 'e' && !e) {
        e = true;
        code = 'e';
        goto valid;
    }

    if((code == '.') && !dot) {
        dot = true;
        goto valid;
    }

    if(isdigit(code) && pos < sizeof(buf) - 1)
        goto valid;

    buf[pos] = 0;
    if(pos)
        sscanf(buf, "%lf", ref);

    return code;

valid:
    buf[pos++] = code;
    return 0;
}

PrintProtocol::~PrintProtocol()
{
}

InputProtocol::~InputProtocol()
{
}

} // namespace ucommon
