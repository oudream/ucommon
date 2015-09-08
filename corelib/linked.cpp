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
#include <ucommon/linked.h>
#include <ucommon/string.h>
#include <ucommon/thread.h>

namespace ucommon {

LinkedObject::LinkedObject(LinkedObject **root)
{
    assert(root != nullptr);
    enlist(root);
}

LinkedObject::~LinkedObject()
{
}

void LinkedObject::purge(LinkedObject *root)
{
    LinkedObject *after;

    assert(root != nullptr);

    while(root) {
        after = root->Next;
        root->release();
        root = after;
    }
}

bool LinkedObject::is_member(LinkedObject *list) const
{
    assert(list != nullptr);

    while(list) {
        if(list == this)
            return true;
        list = list->Next;
    }
    return false;
}

void LinkedObject::enlist(LinkedObject **root)
{
    assert(root != nullptr);

    Next = *root;
    *root = this;
}

void LinkedObject::delist(LinkedObject **root)
{
    assert(root != nullptr);

    LinkedObject *prior = nullptr, *node = *root;

    while(node && node != this) {
        prior = node;
        node = node->Next;
    }

    if(!node)
        return;

    if(!prior)
        *root = Next;
    else
        prior->Next = Next;
}

void ReusableObject::release(void)
{
    Next = nullptr;
}

NamedObject::NamedObject() :
OrderedObject()
{
    Id = nullptr;
}

NamedObject::NamedObject(OrderedIndex *root, char *nid) :
OrderedObject()
{
    assert(root != nullptr);
    assert(nid != nullptr && *nid != 0);

    NamedObject *node = static_cast<NamedObject*>(root->head), *prior = nullptr;

    while(node) {
        if(node->equal(nid)) {
            if(prior)
                prior->Next = node->getNext();
            else
                root->head = node->getNext();
            node->release();
            break;
        }
        prior = node;
        node = node->getNext();
    }
    Next = nullptr;
    Id = nid;
    if(!root->head)
        root->head = this;
    if(!root->tail)
        root->tail = this;
    else
        root->tail->Next = this;
}

// One thing to watch out for is that the id is freed in the destructor.
// This means that you should use a dup'd string for your nid.  Otherwise
// you will need to set it to NULL before destroying the object.

NamedObject::NamedObject(NamedObject **root, char *nid, unsigned max) :
OrderedObject()
{
    assert(root != nullptr);
    assert(nid != nullptr && *nid != 0);
    assert(max > 0);

    Id = nullptr;
    add(root, nid, max);
}

void NamedObject::add(NamedObject **root, char *nid, unsigned max)
{
    assert(root != nullptr);
    assert(nid != nullptr && *nid != 0);
    assert(max > 0);

    clearId();

    NamedObject *node, *prior = nullptr;

    if(max < 2)
        max = 0;
    else
        max = keyindex(nid, max);

    node = root[max];
    while(node) {
        if(node && node->equal(nid)) {
            if(prior) {
                prior->Next = this;
                Next = node->Next;
            }
            else
                root[max] = node->getNext();
            node->release();
            break;
        }
        prior = node;
        node = node->getNext();
    }

    if(!node) {
        Next = root[max];
        root[max] = this;
    }
    Id = nid;
}

void NamedObject::clearId(void)
{
    if(Id) {
        free(Id);
        Id = nullptr;
    }
}

NamedObject::~NamedObject()
{
    // this assumes the id is a malloc'd or strdup'd string.
    // maybe overridden if virtual...

    clearId();
}

// Linked objects are assumed to be freeable if they are released.  The retain
// simply marks it into a self reference state which can never otherwise happen
// naturally.  This is used to mark avoid freeing during release.

void LinkedObject::retain(void)
{
    Next = this;
}


void LinkedObject::release(void)
{
    if(Next != this) {
        Next = this;
        delete this;
    }
}

LinkedObject *LinkedObject::getIndexed(LinkedObject *root, unsigned index)
{
    while(index-- && root != nullptr)
        root = root->Next;

    return root;
}

unsigned LinkedObject::count(const LinkedObject *root)
{
    assert(root != nullptr);

    unsigned c = 0;
    while(root) {
        ++c;
        root = root->Next;
    }
    return c;
}

unsigned NamedObject::keyindex(const char *id, unsigned max)
{
    assert(id != nullptr && *id != 0);
    assert(max > 1);

    unsigned val = 0;

    while(*id)
        val = (val << 1) ^ (*(id++) & 0x1f);

    return val % max;
}

int NamedObject::compare(const char *cid) const
{
    assert(cid != nullptr && *cid != 0);

#ifdef  HAVE_STRCOLL
    return strcoll(Id, cid);
#else
    return strcmp(Id, cid);
#endif
}

extern "C" {

    static int ncompare(const void *o1, const void *o2)
    {
        assert(o1 != nullptr);
        assert(o2 != nullptr);
        const NamedObject * const *n1 = static_cast<const NamedObject * const*>(o1);
        const NamedObject * const*n2 = static_cast<const NamedObject * const*>(o2);
        return ((*n1)->compare((*n2)->getId()));
    }
}

NamedObject **NamedObject::sort(NamedObject **list, size_t size)
{
    assert(list != nullptr);

    if(!size) {
        while(list[size])
            ++size;
    }

    qsort(static_cast<void *>(list), size, sizeof(NamedObject *), &ncompare);
    return list;
}

NamedObject **NamedObject::index(NamedObject **idx, unsigned max)
{
    assert(idx != nullptr);
    assert(max > 0);
    NamedObject **op = new NamedObject *[count(idx, max) + 1];
    unsigned pos = 0;
    NamedObject *node = skip(idx, nullptr, max);

    while(node) {
        op[pos++] = node;
        node = skip(idx, node, max);
    }
    op[pos] = nullptr;
    return op;
}

NamedObject *NamedObject::skip(NamedObject **idx, NamedObject *rec, unsigned max)
{
    assert(idx != nullptr);
    assert(max > 0);

    unsigned key = 0;
    if(rec && !rec->Next)
        key = keyindex(rec->Id, max) + 1;

    if(!rec || !rec->Next) {
        while(key < max && !idx[key])
            ++key;
        if(key >= max)
            return nullptr;
        return idx[key];
    }

    return rec->getNext();
}

void NamedObject::purge(NamedObject **idx, unsigned max)
{
    assert(idx != nullptr);
    assert(max > 0);

    LinkedObject *root;

    if(max < 2)
        max = 0;

    while(max--) {
        root = idx[max];
        LinkedObject::purge(root);
    }
}

unsigned NamedObject::count(NamedObject **idx, unsigned max)
{
    assert(idx != nullptr);
    assert(max > 0);

    unsigned count = 0;
    LinkedObject *node;

    if(max < 2)
        max = 1;

    while(max--) {
        node = idx[max];
        while(node) {
            ++count;
            node = node->Next;
        }
    }
    return count;
}

NamedObject *NamedObject::remove(NamedObject **idx, const char *id, unsigned max)
{
    assert(idx != nullptr);
    assert(id != nullptr && *id != 0);
    assert(max > 0);

    if(max < 2)
        return remove(idx, id);

    return remove(&idx[keyindex(id, max)], id);
}

NamedObject *NamedObject::map(NamedObject **idx, const char *id, unsigned max)
{
    assert(idx != nullptr);
    assert(id != nullptr && *id != 0);
    assert(max > 0);

    if(max < 2)
        return find(*idx, id);

    return find(idx[keyindex(id, max)], id);
}

NamedObject *NamedObject::find(NamedObject *root, const char *id)
{
    assert(id != nullptr && *id != 0);

    while(root) {
        if(root->equal(id))
            break;
        root = root->getNext();
    }
    return root;
}

NamedObject *NamedObject::remove(NamedObject **root, const char *id)
{
    assert(id != nullptr && *id != 0);
    assert(root != nullptr);

    NamedObject *prior = nullptr;
    NamedObject *node = *root;

    while(node) {
        if(node->equal(id))
            break;
        prior = node;
        node = node->getNext();
    }

    if(!node)
        return nullptr;

    if(prior == nullptr)
        *root = node->getNext();
    else
        prior->Next = node->getNext();

    return node;
}

// Like in NamedObject, the nid that is used will be deleted by the
// destructor through calling purge.  Hence it should be passed from
// a malloc'd or strdup'd string.

NamedTree::NamedTree(char *nid) :
NamedObject(), Child()
{
    Id = nid;
    Parent = nullptr;
}

NamedTree::NamedTree(const NamedTree& source)
{
    Id = source.Id;
    Parent = nullptr;
    Child = source.Child;
}

NamedTree::NamedTree(NamedTree *p, char *nid) :
NamedObject(), Child()
{
    assert(p != nullptr);
    assert(nid != nullptr && *nid != 0);

    enlistTail(&p->Child);
    Id = nid;
    Parent = p;
}

NamedTree::~NamedTree()
{
    Id = nullptr;
    purge();
}

NamedTree *NamedTree::getChild(const char *tid) const
{
    assert(tid != nullptr && *tid != 0);

    linked_pointer<NamedTree> node = Child.begin();

    while(node) {
        if(eq(node->Id, tid))
            return *node;
        node.next();
    }
    return nullptr;
}

void NamedTree::relistTail(NamedTree *trunk)
{
    // if moving to same place, just return...
    if(Parent == trunk)
        return;

    if(Parent)
        delist(&Parent->Child);
    Parent = trunk;
    if(Parent)
        enlistTail(&Parent->Child);
}

void NamedTree::relistHead(NamedTree *trunk)
{
    if(Parent == trunk)
        return;

    if(Parent)
        delist(&Parent->Child);
    Parent = trunk;
    if(Parent)
        enlistHead(&Parent->Child);
}

NamedTree *NamedTree::path(const char *tid) const
{
    assert(tid != nullptr && *tid != 0);

    const char *np;
    char buf[65];
    char *ep;
    NamedTree *node = const_cast<NamedTree*>(this);

    if(!tid || !*tid)
        return const_cast<NamedTree*>(this);

    while(*tid == '.') {
        if(!node->Parent)
            return nullptr;
        node = node->Parent;

        ++tid;
    }

    while(tid && *tid && node) {
        String::set(buf, sizeof(buf), tid);
        ep = strchr(buf, '.');
        if(ep)
            *ep = 0;
        np = strchr(tid, '.');
        if(np)
            tid = ++np;
        else
            tid = nullptr;
        node = node->getChild(buf);
    }
    return node;
}

NamedTree *NamedTree::getLeaf(const char *tid) const
{
    assert(tid != nullptr && *tid != 0);

    linked_pointer<NamedTree> node = Child.begin();

    while(node) {
        if(node->is_leaf() && eq(node->Id, tid))
            return *node;
        node.next();
    }
    return nullptr;
}

NamedTree *NamedTree::leaf(const char *tid) const
{
    assert(tid != nullptr && *tid != 0);

    linked_pointer<NamedTree> node = Child.begin();
    NamedTree *obj;

    while(node) {
        if(node->is_leaf() && eq(node->Id, tid))
            return *node;
        obj = nullptr;
        if(!node->is_leaf())
            obj = node->leaf(tid);
        if(obj)
            return obj;
        node.next();
    }
    return nullptr;
}

NamedTree *NamedTree::find(const char *tid) const
{
    assert(tid != nullptr && *tid != 0);

    linked_pointer<NamedTree> node = Child.begin();
    NamedTree *obj;

    while(node) {
        if(!node->is_leaf()) {
            if(eq(node->Id, tid))
                return *node;
            obj = node->find(tid);
            if(obj)
                return obj;
        }
        node.next();
    }
    return nullptr;
}

void NamedTree::setId(char *nid)
{
    assert(nid != nullptr && *nid != 0);

    Id = nid;
}

// If you remove the tree node, the id is NULL'd also.  This keeps the
// destructor from freeing it.

void NamedTree::remove(void)
{
    if(Parent)
        delist(&Parent->Child);

    Id = nullptr;
}

void NamedTree::purge(void)
{
    linked_pointer<NamedTree> node = Child.begin();
    NamedTree *obj;

    if(Parent)
        delist(&Parent->Child);

    while(node) {
        obj = *node;
		if (!obj)
			break;
        obj->Parent = nullptr; // save processing
        node = obj->getNext();
        delete obj;
    }

    // this assumes the object id is a malloc'd/strdup string.
    // may be overridden if virtual...
    clearId();
}

LinkedObject::LinkedObject()
{
    Next = nullptr;
}

LinkedObject::LinkedObject(const LinkedObject& from)
{
    Next = nullptr;
}

OrderedObject::OrderedObject() : LinkedObject()
{
}

OrderedObject::OrderedObject(const OrderedObject& from) : LinkedObject()
{
}

OrderedObject::OrderedObject(OrderedIndex *root) :
LinkedObject()
{
    assert(root != nullptr);
    Next = nullptr;
    enlistTail(root);
}

void OrderedObject::delist(OrderedIndex *root)
{
    assert(root != nullptr);

    OrderedObject *prior = nullptr, *node;

    node = root->head;

    while(node && node != this) {
        prior = node;
        node = node->getNext();
    }

    if(!node)
        return;

    if(!prior)
        root->head = getNext();
    else
        prior->Next = Next;

    if(this == root->tail)
        root->tail = prior;
}

void OrderedObject::enlist(OrderedIndex *root)
{
    assert(root != nullptr);

    Next = nullptr;
    enlistTail(root);
}

void OrderedObject::enlistTail(OrderedIndex *root)
{
    assert(root != nullptr);

    if(root->head == nullptr)
        root->head = this;
    else if(root->tail)
        root->tail->Next = this;

    root->tail = this;
}

void OrderedObject::enlistHead(OrderedIndex *root)
{
    assert(root != nullptr);

    Next = nullptr;
    if(root->tail == nullptr)
        root->tail = this;
    else if(root->head)
        Next = root->head;

    root->head = this;
}

LinkedList::LinkedList()
{
    Root = nullptr;
    Prev = nullptr;
    Next = nullptr;
}

LinkedList::LinkedList(OrderedIndex *r)
{
    Root = nullptr;
    Next = Prev = nullptr;
    if(r)
        enlist(r);
}

void LinkedList::enlist(OrderedIndex *r)
{
    assert(r != nullptr);

    enlistTail(r);
}

void LinkedList::insert(LinkedList *o)
{
    assert(o != nullptr);

    insertTail(o);
}

void LinkedList::insertHead(LinkedList *o)
{
    assert(o != nullptr);

    if(o->Root)
        o->delist();

    if(Prev) {
        o->Next = this;
        o->Prev = Prev;
    }
    else {
        Root->head = o;
        o->Prev = nullptr;
    }
    o->Root = Root;
    o->Next = this;
    Prev = o;
}

void LinkedList::insertTail(LinkedList *o)
{
    assert(o != nullptr);

    if(o->Root)
        o->delist();

    if(Next) {
        o->Prev = this;
        o->Next = Next;
    }
    else {
        Root->tail = o;
        o->Next = nullptr;
    }
    o->Root = Root;
    o->Prev = this;
    Next = o;
}

void LinkedList::enlistHead(OrderedIndex *r)
{
    assert(r != nullptr);

    if(Root)
        delist();

    Root = r;
    Prev = nullptr; 
    Next = nullptr;

    if(!Root->tail) {
        Root->tail = Root->head = static_cast<OrderedObject *>(this);
        return;
    }

    Next = static_cast<LinkedList *>(Root->head);
    ((LinkedList*)Next)->Prev = this;
    Root->head = static_cast<OrderedObject *>(this);
}


void LinkedList::enlistTail(OrderedIndex *r)
{
    assert(r != nullptr);

    if(Root)
        delist();

    Root = r;
    Next = Prev = nullptr;

    if(!Root->head) {
        Root->head = Root->tail = static_cast<OrderedObject *>(this);
        return;
    }

    Prev = static_cast<LinkedList *>(Root->tail);
    Prev->Next = this;
    Root->tail = static_cast<OrderedObject *>(this);
}

void LinkedList::delist(void)
{
    if(!Root)
        return;

    if(Prev)
        Prev->Next = Next;
    else if(Root->head == static_cast<OrderedObject *>(this))
        Root->head = static_cast<OrderedObject *>(Next);

    if(Next)
        (static_cast<LinkedList *>(Next))->Prev = Prev;
    else if(Root->tail == static_cast<OrderedObject *>(this))
        Root->tail = static_cast<OrderedObject *>(Prev);

    Root = nullptr;
    Next = Prev = nullptr;
}

LinkedList::~LinkedList()
{
    delist();
}

OrderedIndex::OrderedIndex()
{
    head = tail = nullptr;
}

OrderedIndex::~OrderedIndex()
{
    head = tail = nullptr;
}

void OrderedIndex::copy(const OrderedIndex& source)
{
    head = source.head;
    tail = source.tail;
}

void OrderedIndex::operator*=(OrderedObject *object)
{
    assert(object != nullptr);

    object->enlist(this);
}

void OrderedIndex::add(OrderedObject *object)
{
    assert(object != nullptr);

    object->enlist(this);
}

LinkedObject *OrderedIndex::get(void)
{
    LinkedObject *node;

    if(!head)
        return nullptr;

    node = head;
    head = static_cast<OrderedObject *>(node->getNext());
    if(!head)
        tail = nullptr;

    return static_cast<LinkedObject *>(node);
}

void OrderedIndex::purge(void)
{
    if(head) {
        LinkedObject::purge((LinkedObject *)head);
        head = tail = nullptr;
    }
}

void OrderedIndex::reset(void)
{
    head = tail = nullptr;
}

void OrderedIndex::lock_index(void)
{
}

void OrderedIndex::unlock_index(void)
{
}

DLinkedObject::DLinkedObject() : OrderedObject()
{
    Prev = nullptr;
}

DLinkedObject::DLinkedObject(const DLinkedObject& from) : OrderedObject()
{
    Prev = nullptr;
}

void DLinkedObject::delist(void)
{
    if(Prev)
        Prev->Next = Next;

    if(Next)
        ((DLinkedObject *)Next)->Prev = Prev;

    Next = Prev = nullptr;
}

LinkedObject **OrderedIndex::index(void) const
{
    LinkedObject **op = new LinkedObject *[count() + 1];
    LinkedObject *node;
    unsigned idx = 0;

    node = head;
    while(node) {
        op[idx++] = node;
        node = node->Next;
    }
    op[idx] = nullptr;
    return op;
}

LinkedObject *OrderedIndex::find(unsigned index) const
{
    unsigned count = 0;
    LinkedObject *node;

    node = head;

    while(node && ++count < index)
        node = node->Next;

    return node;
}

unsigned OrderedIndex::count(void) const
{
    unsigned count = 0;
    LinkedObject *node;

    node = head;

    while(node) {
        node = node->Next;
        ++count;
    }
    return count;
}

} // namespace ucommon
