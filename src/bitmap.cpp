// Copyright (C) 2006-2007 David Sugar, Tycho Softworks.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.

#include <config.h>
#include <ucommon/bitmap.h>
#include <stdlib.h>
#include <unistd.h>

using namespace UCOMMON_NAMESPACE;

bitmap::bitmap(size_t count)
{
	size_t mem = count / 8;
	size = count;
	bus = BMALLOC;

	if(count % 8)
		++mem;

	addr.a = ::malloc(mem);
	clear();
}

bitmap::bitmap(void *ptr, size_t count, bus_t access)
{
	addr.a = ptr;
	bus = access;
}

bitmap::~bitmap()
{
	if(bus == BMALLOC && addr.b)
		::free(addr.b);
	addr.b = NULL;
}

unsigned bitmap::memsize(void)
{
	switch(bus) {
	case B64:
		return 64;
	case B32:
		return 32;
	case B16:
		return 16;
	default:
		return 8;
	}
}

void bitmap::set(size_t offset, bool bit)
{
	unsigned bs = memsize();
	size_t pos = offset / bs;
	unsigned rem = offset % bs;
	uint64_t b64;
	uint32_t b32;
	uint16_t b16;
	uint8_t b8;

	if(offset >= size)
		return;

	switch(bus) {
	case B64:
		b64 = 1<<rem;
		if(bit)
			addr.d[pos] |= b64;
		else
			addr.d[pos] &= ~b64;
		break;
    case B32:
        b32 = 1<<rem;
        if(bit)
            addr.l[pos] |= b32;
        else
            addr.l[pos] &= ~b32;
        break;
    case B16:
        b16 = 1<<rem;
        if(bit)
            addr.w[pos] |= b16;
        else
            addr.w[pos] &= ~b16;
        break;
	default:
        b8 = 1<<rem;
        if(bit)
            addr.b[pos] |= b8;
        else
            addr.b[pos] &= ~b8;
        break;
	}
}

bool bitmap::get(size_t offset)
{
	unsigned bs = memsize();
	size_t pos = offset / bs;
	unsigned rem = offset % bs;

	if(offset >= size)
		return false;

	switch(bus) {
	case B64:
		return addr.d[pos] & 1ll<<rem;
    case B32:
		return addr.l[pos] & 1l<<rem;
	case B16:
		return addr.w[pos] & 1<<rem;
	default:
		return addr.b[pos] & 1<<rem;
	}
}

void bitmap::clear(void)
{
	unsigned bs = memsize();
	addr_t ptr = addr;

	if(size % bs)
		++size;

	while(size--) {
		switch(bus) {
		case B64:
			*(addr.d++) = 0ll;
			break;
		case B32:
			*(addr.l++) = 0l;
			break;
		case B16:
			*(addr.w++) = 0;
			break;
		default:
			*(addr.b++) = 0;
		}
	}
}
