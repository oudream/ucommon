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

extern "C" int main()
{
    stringlist_t mylist;
    stringlistitem_t *item;

    mylist.add("100");
    mylist.add("050");
    mylist.add("300");

    item = mylist.begin();
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

    return 0;
}
