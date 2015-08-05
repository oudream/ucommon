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

#ifndef DEBUG
#define DEBUG
#endif

#include <ucommon/ucommon.h>

#include <stdio.h>

using namespace ucommon;

static int tval = 100;
static int dval = 0;
static int cval = 0;

class special
{
public:
    int x;
    
    special() {
        x = ++cval;
    }

    ~special() {
        ++dval;
    }
};

extern "C" int main()
{
    stringlist_t mylist;
    stringlistitem_t *item;

    mylist.add("100");
    mylist.add("050");
    mylist.add("300");

    assert(eq("100", mylist[0u]));
    mylist.sort();
    item = mylist.begin();
    assert(eq(item->get(), "050"));

    assert(eq(mylist[1u], "100"));
    assert(eq(mylist[2u], "300"));

    const char *str = *mylist;
    assert(eq(str, "050"));
    assert(eq(mylist[0u], "100"));

    char **list = mylist;
    assert(eq(list[0], "100"));
    assert(eq(list[1], "300"));

    assert(list[2] == NULL);

    int *pval = &tval;
    int& rval = deref_pointer<int>(pval);
    assert(&rval == pval);

    typeref<int> iptr;
    iptr = (int)3;
    typeref<int> jptr = iptr;
    assert((int)iptr == 3);
    assert(iptr.copies() == 2);

    iptr = 17;
    assert((int)iptr == 17);
    assert(iptr.copies() == 1);

    special spec;
    cval = 0;
    typeref<special> sptr = spec;
    typeref<special> xptr = sptr;
    assert(xptr->x == 1);
    assert(cval == 1);
    assert(xptr.copies() == 2);
    sptr.release();
    xptr.release();
    assert(dval == 1);

    stringref sref = "this is a test";
    stringref xref = sref;
    assert(*sref == *xref);
    assert(!strcmp((const char *)sref, "this is a test"));
    assert(sref.size() == 14);
    assert(sref.copies() == 2);

    arrayref<int> ints(32);
    ints(4, 27);
    ints(6, 30);
    assert(ints.size() == 32);
    assert(ints[4] == 27);
    typeref<int> member = ints.at(6);
    assert(member.copies() == 2);
    ints.put(member, 6);
    assert(member.copies() == 2);

    return 0;
}
