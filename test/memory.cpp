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

typedef struct {
    char key[12];
    int v;
} maptest;

static uint8_t memdata[7] = {0x20, 0x55, 0x77, 0x78, 0x33, 0x66, 0x55};

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
    sptr.clear();
    xptr.clear();
    assert(dval == 1);

    stringref sref = "this is a test";
    stringref xref = sref;
    assert(*sref == *xref);
    assert(!strcmp((const char *)sref, "this is a test"));
    assert(sref.size() == 14);
    assert(sref.copies() == 2);

    arrayref<int> ints(32, 99);
    ints(4, 27);
    ints(6, 30);
    assert(ints.size() == 32);
    assert(ints[4] == 27);
    typeref<int> member = ints.at(6);
    assert(member.copies() == 2);
    ints.put(member, 6);
    assert(member.copies() == 2);
    ints.put(member, 30);
    assert(*member == 99);
    assert(member.copies() == 31);

    int memval = member;
    member = 95;
    assert(memval == 99);
    assert(*member == 95);
    assert(member.copies() == 1);
    ints(17, memval);
    ints(18, member);
    assert(*ints[18] == 95);

    temporary<int> itemp;
    itemp = 37;
    itemp[0] = 39;
    assert(*itemp == 39);

    temporary<int> atemp(7, 2);
    assert(atemp[3] == 2);
    atemp[5] = 9;
    assert(atemp[5] == 9);

    sharedref<int> sint;
    sint = 3;
    typeref<int> sv = sint;
    assert(sv.copies() == 2); 

    stackref<int> stackofints(20);
    stackofints << 17;
    stackofints << 25;
    stackofints << 333;
    assert(stackofints.count() == 3);
    stackofints.pop();
    stackofints >> sv;
    assert(stackofints.count() == 1);
    assert(sv == 25);
    assert(sv.copies() == 1);

    queueref<int> queueofints(20);
    queueofints << 44;
    queueofints << 55;
    assert(queueofints.count() == 2);
    queueofints >> sv;
    assert(sv == 44);
    queueofints >> sv;
    assert(sv == 55);
    assert(mapkeypath(sv) == 8779);

    // flush without delay...
    sv = queueofints.pull(0);
    assert(!sv);
    assert(sv.copies() == 0);

    mapref<int,Type::Chars> map;
    map(3, "hello");
    stringref_t sr = map(3);
    assert(eq(*sr, "hello"));
    sr = map(2);
    assert(*sr == nullptr);
    typeref<int> ki(3);
    sr = "goodbye";
    map(ki, sr);
    sr = map(3);
    assert(eq(*sr, "goodbye"));
    map(7, "test");
    assert(map.count() == 2);

    mapref<int,Type::Chars>::instance inst = map;
    typeref<int> kv;
    unsigned passes = 0;
    while(is(inst)) {
        kv = inst.key();
        ++inst;
        ++passes;
    }
    assert(*kv == 7);
    assert(passes = 2);
    assert(kv.copies() == 2);

    map.remove(7);
    map(9, "9");
    assert(map.used() == 2);
    sr = map(7);
    assert(*sr == nullptr);

    listref<int> intlist;
    intlist << 3 << 5 << 7 << 9;
    assert(intlist.count() == 4);
    assert(*(intlist[2]) == 7);

    maptest mt[10];
    String::set(mt[0].key, 12, "hi");
    String::set(mt[1].key, 12, "bye");
    mt[0].v = 10;
    mt[1].v = 20;

    mapped_pointer<char, maptest> pmap;
    maptest *out = pmap.get("bye");
    assert(out == nullptr);
    pmap.release(out);
    pmap.set(mt[0].key, &mt[0]);
    pmap.set(mt[1].key, &mt[1]);
    out = pmap.get("hi");
    assert(eq(out->key, "hi"));
    pmap.release(out);
    pmap.remove("bye");
    out = pmap.get("bye");
    assert(out == nullptr);
    pmap.release(out);

    int a = 1, b = 2;
    swap(a, b);
    assert(b == 1);
    assert(a == 2);
    copy(b, a);
    assert(a == 1);
    b = 2; 
    move(b, a);
    assert(a == 2);
    assert(b == 0);
    clear(a);
    assert(a == 0);

    stringref s1, s2;
    s1.hex(memdata, sizeof(memdata));
    s2.b64(memdata, 4);

    assert(eq(s1, "20557778336655"));
    assert(eq(s2, "IFV3eA=="));

    return 0;
}
