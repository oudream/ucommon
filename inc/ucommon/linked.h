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

/**
 * Linked objects, lists, templates, and containers.
 * Common support for objects that might be organized as single and double
 * linked lists, rings and queues, and tree oriented data structures.  These
 * generic classes may be used to help form anything from callback
 * registration systems and indexed memory hashes to xml parsed tree nodes.
 * @file ucommon/linked.h
 */

/**
 * An example of the linked object classes and their use.
 * @example linked.cpp
 */

#ifndef _UCOMMON_LINKED_H_
#define _UCOMMON_LINKED_H_

#ifndef _UCOMMON_CONFIG_H_
#include <ucommon/platform.h>
#endif

#ifndef _UCOMMON_OBJECT_H_
#include <ucommon/object.h>
#endif

namespace ucommon {

class OrderedObject;

/**
 * Common base class for all objects that can be formed into a linked list.
 * This base class is used directly for objects that can be formed into a
 * single linked list.  It is also used directly as a type for a pointer to the
 * start of list of objects that are linked together as a list.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT LinkedObject : public ObjectProtocol
{
private:
    friend class OrderedIndex;
    friend class NamedObject;

protected:
    LinkedObject *Next;

    /**
     * Construct base class attached to a chain of objects.
     * @param root pointer to chain of objects we are part of.
     */
    LinkedObject(LinkedObject **root);

    /**
     * Construct base class unattached to anyone.  This might be
     * used to construct intermediary base classes that may form
     * lists through indexing objects.
     */
    LinkedObject();

    LinkedObject(const LinkedObject& from);

public:
    virtual ~LinkedObject();

    /**
     * Release list, mark as no longer linked.  Inherited from base Object.
     */
    virtual void release(void) __OVERRIDE;

    /**
     * Retain by marking as self referenced list. Inherited from base Object.
     */
    virtual void retain(void) __OVERRIDE;

    /**
     * Add our object to an existing linked list through a pointer.  This
     * forms a container sorted in lifo order since we become the head
     * of the list, and the previous head becomes our next.
     * @param root pointer to list we are adding ourselves to.
     */
    void enlist(LinkedObject **root);

    /**
     * Locate and remove ourselves from a list of objects.  This searches
     * the list to locate our object and if found relinks the list around
     * us.
     * @param root pointer to list we are removing ourselves from.
     */
    void delist(LinkedObject **root);

    /**
     * Search to see if we are a member of a specific list.
     * @return true if we are member of the list.
     */
    bool is_member(LinkedObject *list) const;

    /**
     * Release all objects from a list.
     * @param root pointer to list we are purging.
     */
    static void purge(LinkedObject *root);

    /**
     * Count the number of linked objects in a list.
     * @param root pointer to list we are counting.
     */
    static unsigned count(const LinkedObject *root);

    /**
     * Get member by index.
     * @return indexed member in linked list.
     * @param root pointer to list we are indexing.
     * @param index member to find.
     */
    static LinkedObject *getIndexed(LinkedObject *root, unsigned index);

    /**
     * Get next effective object when iterating.
     * @return next linked object in list.
     */
    inline LinkedObject *getNext(void) const {
        return Next;
    }
};

/**
 * Reusable objects for forming private heaps.  Reusable objects are
 * linked objects that may be allocated in a private heap, and are
 * returned to a free list when they are no longer needed so they can
 * be reused without having to be re-allocated.  The free list is the
 * root of a linked object chain.  This is used as a base class for those
 * objects that will be managed through reusable heaps.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT ReusableObject : public LinkedObject
{
    friend class ReusableAllocator;

protected:
    virtual void release(void) __OVERRIDE;

public:
    /**
     * Get next effective reusable object when iterating.
     * @return next reusable object in list.
     */
    inline ReusableObject *getNext(void) {
        return polypointer_cast<ReusableObject*>(LinkedObject::getNext());
    }
};

/**
 * An index container for maintaining an ordered list of objects.
 * This index holds a pointer to the head and tail of an ordered list of
 * linked objects.  Fundamental methods for supporting iterators are
 * also provided.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT OrderedIndex
{
protected:
    friend class OrderedObject;
    friend class DLinkedObject;
    friend class NamedObject;

    OrderedObject *head, *tail;

public:
    void copy(const OrderedIndex& source);

    /**
     * Create and initialize an empty index.
     */
    OrderedIndex();

    inline OrderedIndex(const OrderedIndex& source) {
        copy(source);
    }

    /**
     * Destroy index.
     */
    virtual ~OrderedIndex();

    /**
     * Find a specific member in the ordered list.
     * @param offset to member to find.
     */
    LinkedObject *find(unsigned offset) const;

    /**
     * Count of objects this list manages.
     * @return number of objects in the list.
     */
    unsigned count(void) const;

    /**
     * Purge the linked list and then set the index to empty.
     */
    void purge(void);

    /**
     * Reset linked list to empty without purging.
     */
    void reset(void);

    /**
     * Used to synchronize lists managed by multiple threads.  A derived
     * locking method would be invoked.
     */
    virtual void lock_index(void);

    /**
     * Used to synchronize lists managed by multiple threads.  A derived
     * unlocking method would be invoked.
     */
    virtual void unlock_index(void);

    /**
     * Return a pointer to the head of the list.  This allows the head
     * pointer to be used like a simple root list pointer for pure
     * LinkedObject based objects.
     * @return LinkedIndex style object.
     */
    LinkedObject **index(void) const;

    /**
     * Get (pull) object off the list.  The start of the list is advanced to
     * the next object.
     * @return LinkedObject based object that was head of the list.
     */
    LinkedObject *get(void);

    /**
     * Add an object into the ordered index.
     * @param ordered object to add to the index.
     */
    void add(OrderedObject *ordered);

    /**
     * Get an indexed member from the ordered index.
     * @param index of member to fetch.
     * @return LinkedObject member of index.
     */
    inline LinkedObject *getIndexed(unsigned index) const {
        return LinkedObject::getIndexed(polystatic_cast<LinkedObject*>(head), index);
    }

    /**
     * Return first object in list for iterators.
     * @return first object in list.
     */
    inline LinkedObject *begin(void) const {
        return polystatic_cast<LinkedObject*>(head);
    }

    /**
     * Return last object in list for iterators.
     * @return last object in list.
     */
    inline LinkedObject *end(void) const {
        return polystatic_cast<LinkedObject*>(tail);
    }

    /**
     * Return head object pointer.
     * @return head pointer.
     */
    inline LinkedObject *operator*() const {
        return polystatic_cast<LinkedObject*>(head);
    }

    /**
     * Assign ordered index.
     * @param object to copy from.
     */
    OrderedIndex& operator=(const OrderedIndex& object) {
        copy(object); 
        return *this;
    }

    /**
     * Add object to our list.
     * @param object to add.
     */
    void operator*=(OrderedObject *object);
};

/**
 * A linked object base class for ordered objects.  This is used for
 * objects that must be ordered and listed through the OrderedIndex
 * class.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT OrderedObject : public LinkedObject
{
private:
    friend class DLinkedObject;
    friend class OrderedIndex;

protected:
    /**
     * Construct an ordered object aot end of a an index.
     * @param index we are listed on.
     */
    OrderedObject(OrderedIndex *index);

    /**
     * Construct an ordered object unattached.
     */
    OrderedObject();

    OrderedObject(const OrderedObject& from);

public:
    /**
     * List our ordered object at end of a linked list on an index.
     * @param index we are listing on.
     */
    void enlistTail(OrderedIndex *index);

    /**
     * List our ordered object at start of a linked list on an index.
     * @param index we are listing on.
     */
    void enlistHead(OrderedIndex *index);

    /**
     * List our ordered object in default strategy mode.  The default
     * base class uses enlistTail.
     * @param index we are listing on.
     */
    virtual void enlist(OrderedIndex *index);

    /**
     * Remove our ordered object from an existing index.
     * @param index we are listed on.
     */
    void delist(OrderedIndex *index);

    /**
     * Get next ordered member when iterating.
     * @return next ordered object.
     */
    inline OrderedObject *getNext(void) const {
        return static_cast<OrderedObject *>(LinkedObject::getNext());
    }
};

/**
 * A linked object base class with members found by name.  This class is
 * used to help form named option lists and other forms of name indexed
 * associative data structures.  The id is assumed to be passed from a
 * dupped or dynamically allocated string.  If a constant string is used
 * then you must not call delete for this object.
 *
 * Named objects are either listed on an ordered list or keyed through an
 * associate hash map table.  When using a hash table, the name id string is
 * used to determine the slot number to use in a list of n sized linked
 * object lists.  Hence, a hash index refers to a specific sized array of
 * object indexes.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT NamedObject : public OrderedObject
{
protected:
    char *Id;

    /**
     * Create an empty unnamed cell object.
     */
    NamedObject();

    /**
     * Create a named object and add to hash indexed list.
     * @param hash map table to list node on.
     * @param name of the object we are listing.
     * @param size of hash map table used.
     */
    NamedObject(NamedObject **hash, char *name, unsigned size = 1);

    /**
     * Created a named object on an ordered list.  This is commonly used
     * to form attribute lists.
     * @param index to list object on.
     * @param name of the object we are listing.
     */
    NamedObject(OrderedIndex *index, char *name);

    /**
     * Destroy named object.  We do not always destroy named objects, since
     * we may use them in reusable pools or we may initialize a list that we
     * keep permanently.  If we do invoke delete for something based on
     * NamedObject, then be aware the object id is assumed to be formed from
     * a dup'd string which will also be freed unless clearId is overridden.
     */
    ~NamedObject();

    /**
     * The behavior of clearing id's can be overridden if they are not
     * assigned as strdup's from the heap...
     */
    virtual void clearId(void);

public:
    /**
     * Add object to hash indexed list.
     * @param hash map table to list node on.
     * @param name of the object we are listing.
     * @param size of hash map table used.
     */
    void add(NamedObject **hash, char *name, unsigned size = 1);

    /**
     * Purge a hash indexed table of named objects.
     * @param hash map table to purge.
     * @param size of hash map table used.
     */
    static void purge(NamedObject **hash, unsigned size);

    /**
     * Convert a hash index into a linear object pointer array.  The
     * object pointer array is created from the heap and must be deleted
     * when no longer used.
     * @param hash map table of objects to index.
     * @param size of hash map table used.
     * @return array of named object pointers.
     */
    static NamedObject **index(NamedObject **hash, unsigned size);

    /**
     * Count the total named objects in a hash table.
     * @param hash map table of objects to index.
     * @param size of hash map table used.
     */
    static unsigned count(NamedObject **hash, unsigned size);

    /**
     * Find a named object from a simple list.  This may also use the
     * begin() member of an ordered index of named objects.
     * @param root node of named object list.
     * @param name of object to find.
     * @return object pointer or NULL if not found.
     */
    static NamedObject *find(NamedObject *root, const char *name);

    /**
     * Remove a named object from a simple list.
     * @param root node of named object list.
     * @param name of object to find.
     * @return object pointer or NULL if not found.
     */
    static NamedObject *remove(NamedObject **root, const char *name);

    /**
     * Find a named object through a hash map table.
     * @param hash map table of objects to search.
     * @param name of object to find.
     * @param size of hash map table.
     * @return object pointer or NULL if not found.
     */
    static NamedObject *map(NamedObject **hash, const char *name, unsigned size);

    /**
     * Remove an object from a hash map table.
     * @param hash map table of object to remove from.
     * @param name of object to remove.
     * @param size of hash map table.
     * @return object that is removed or NULL if not found.
     */
    static NamedObject *remove(NamedObject **hash, const char *name, unsigned size);

    /**
     * Iterate through a hash map table.
     * @param hash map table to iterate.
     * @param current named object we iterated or NULL to find start of list.
     * @param size of map table.
     * @return next named object in hash map or NULL if no more objects.
     */
    static NamedObject *skip(NamedObject **hash, NamedObject *current, unsigned size);

    /**
     * Internal function to convert a name to a hash index number.
     * @param name to convert into index.
     * @param size of map table.
     */
    static unsigned keyindex(const char *name, unsigned size);

    /**
     * Sort an array of named objects in alphabetical order.  This would
     * typically be used to sort a list created and returned by index().
     * @param list of named objects to sort.
     * @param count of objects in the list or 0 to find by NULL pointer.
     * @return list in sorted order.
     */
    static NamedObject **sort(NamedObject **list, size_t count = 0);

    /**
     * Get next effective object when iterating.
     * @return next linked object in list.
     */
    inline NamedObject *getNext(void) const {
        return static_cast<NamedObject*>(LinkedObject::getNext());
    }

    /**
     * Get the named id string of this object.
     * @return name id.
     */
    inline char *getId(void) const {
        return Id;
    };

    /**
     * Compare the name of our object to see if equal.  This is a virtual
     * so that it can be overridden when using named lists or hash lookups
     * that must be case insensitive.
     * @param name to compare our name to.
     * @return 0 if effectivily equal, used for sorting keys.
     */
    virtual int compare(const char *name) const;

    /**
     * Equal function which calls compare.
     * @param name to compare our name to.
     * @return true if equal.
     */
    inline bool equal(const char *name) const {
        return (compare(name) == 0);
    }

    /**
     * Comparison operator between our name and a string.
     * @param name to compare with.
     * @return true if equal.
     */
    inline bool operator==(const char *name) const {
        return compare(name) == 0;
    }

    /**
     * Comparison operator between our name and a string.
     * @param name to compare with.
     * @return true if not equal.
     */
    inline bool operator!=(const char *name) const {
        return compare(name) != 0;
    }
};

/**
 * The named tree class is used to form a tree oriented list of associated
 * objects.  Typical uses for such data structures might be to form a
 * parsed XML document, or for forming complex configuration management
 * systems or for forming system resource management trees.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT NamedTree : public NamedObject
{
protected:
    NamedTree *Parent;
    OrderedIndex Child;

    /**
     * Create a stand-alone or root tree node, with an optional name.
     * @param name for this node.
     */
    NamedTree(char *name = NULL);

    /**
     * Create a tree node as a child of an existing node.
     * @param parent node we are listed under.
     * @param name of our node.
     */
    NamedTree(NamedTree *parent, char *name);

    /**
     * Construct a copy of the tree.
     * @param source object to copy from.
     */
    NamedTree(const NamedTree& source);

    /**
     * Delete node in a tree.  If we delete a node, we must delist it from
     * it's parent.  We must also delink any child nodes.  This is done by
     * calling the purge() member.
     */
    virtual ~NamedTree();

    /**
     * Performs object destruction.  Note, if we delete a named tree object
     * the name of our member object is assumed to be a dynamically allocated
     * string that will also be free'd.
     */
    void purge(void);

public:
    /**
     * Find a child node of our object with the specified name.  This will
     * also recursivily search all child nodes that have children until
     * the named node can be found.  This seeks a child node that has
     * children.
     * @param name to search for.
     * @return tree object found or NULL.
     */
    NamedTree *find(const char *name) const;

    /**
     * Find a subnode by a dot separated list of node names.  If one or
     * more lead dots are used, then the search will go through parent
     * node levels of our node.  The dot separated list could be thought
     * of as a kind of pathname where dot is used like slash.  This implies
     * that individual nodes can never use names which contain dot's if
     * the path function will be used.
     * @param path name string being sought.
     * @return tree node object found at the path or NULL.
     */
    NamedTree *path(const char *path) const;

    /**
     * Find a child leaf node of our object with the specified name.  This
     * will recursively search all our child nodes until it can find a leaf
     * node containing the specified id but that holds no further children.
     * @param name of leaf node to search for.
     * @return tree node object found or NULL.
     */
    NamedTree *leaf(const char *name) const;

    /**
     * Find a direct child of our node which matches the specified name.
     * @param name of child node to find.
     * @return tree node object of child or NULL.
     */
    NamedTree *getChild(const char *name) const;

    /**
     * Find a direct leaf node on our node.  A leaf node is a node that has
     * no children of it's own.  This does not perform a recursive search.
     * @param name of leaf child node to find.
     * @return tree node object of leaf or NULL.
     */
    NamedTree *getLeaf(const char *name) const;

    /**
     * Get first child node in our ordered list of children.  This might
     * be used to iterate child nodes.  This may also be used to get
     * unamed child nodes.
     * @return first child node or NULL if no children.
     */
    inline NamedTree *getFirst(void) const {
        return static_cast<NamedTree *>(Child.begin());
    }

    /**
     * Get parent node we are listed as a child on.
     * @return parent node or NULL if none.
     */
    inline NamedTree *getParent(void) const {
        return static_cast<NamedTree *>(Parent);
    };

    /**
     * Get child by index number.
     * @param index of child to fetch.
     * @return indexed child node.
     */
    inline NamedTree *getIndexed(unsigned index) const {
        return static_cast<NamedTree *>(Child.getIndexed(index));
    }

    /**
     * Get the ordered index of our child nodes.
     * @return ordered index of our children.
     */
    inline OrderedIndex *getIndex(void) const {
        return const_cast<OrderedIndex*>(&Child);
    }

    /**
     * Test if this node has a name.
     * @return true if name is set.
     */
    inline operator bool() const {
        return (Id != NULL);
    }

    /**
     * Test if this node is unnamed.
     * @return false if name is set.
     */
    inline bool operator!() const {
        return (Id == NULL);
    }

    /**
     * Set or replace the name id of this node.  This will free the string
     * if a name had already been set.
     * @param name for this node to set.
     */
    void setId(char *name);

    /**
     * Remove our node from our parent list.  The name is set to NULL to
     * keep delete from freeing the name string.
     */
    void remove(void);

    /**
     * Test if node has children.
     * @return true if node contains child nodes.
     */
    inline bool is_leaf(void) const {
        return (Child.begin() == NULL);
    }

    /**
     * Test if node is root node.
     * @return true if node is root node.
     */
    inline bool is_root(void) const {
        return (Parent == NULL);
    }

    /**
     * Add leaf to a trunk, by order.  If NULL, just remove.
     * @param trunk we add leaf node to.
     */
    void relistTail(NamedTree *trunk);

    /**
     * Add leaf to a trunk, by reverse order.  If NULL, just remove.
     * @param trunk we add leaf node to.
     */
    void relistHead(NamedTree *trunk);

    /**
     * Default relist is by tail...
     * @param trunk we add leaf node to, NULL to delist.
     */
    inline void relist(NamedTree *trunk = NULL) {
        relistTail(trunk);
    }
};

/**
 * A double linked list object.  This is used as a base class for objects
 * that will be organized through ordered double linked lists which allow
 * convenient insertion and deletion of list members anywhere in the list.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT DLinkedObject : public OrderedObject
{
protected:
    friend class ObjectQueue;

    DLinkedObject *Prev;
    OrderedIndex *Root;

    /**
     * Construct and add our object to an existing double linked list at end.
     * @param index of linked list we are listed in.
     */
    DLinkedObject(OrderedIndex *index);

    /**
     * Construct an unlinked object.
     */
    DLinkedObject();

    DLinkedObject(const DLinkedObject& from);

    /**
     * Delete linked list object.  If it is a member of a list of objects,
     * then the list is reformed around us.
     */
    virtual ~DLinkedObject();

public:
    /**
     * Remove our object from the list it is currently part of.
     */
    void delist(void);

    /**
     * Attach our object to the start of a linked list though an ordered index.
     * If we are already attached to a list we are delisted first.
     * @param index of linked list we are joining.
     */
    void enlistHead(OrderedIndex *index);

    /**
     * Attach our object to the end of a linked list though an ordered index.
     * If we are already attached to a list we are delisted first.
     * @param index of linked list we are joining.
     */
    void enlistTail(OrderedIndex *index);

    /**
     * Attach our object to a linked list.  The default strategy is to add
     * to tail.
     * @param index of linked list we are joining.
     */
    void enlist(OrderedIndex *index);

    /**
     * Test if we are at the head of a list.
     * @return true if we are the first node in a list.
     */
    inline bool is_head(void) const {
        return polypointer_cast<DLinkedObject *>(Root->head) == this;
    }

    /**
     * Test if we are at the end of a list.
     * @return true if we are the last node in a list.
     */
    inline bool is_tail(void) const {
        return polypointer_cast<DLinkedObject *>(Root->tail) == this;
    }

    /**
     * Get previous node in the list for reverse iteration.
     * @return previous node in list.
     */
    inline DLinkedObject *getPrev(void) const {
        return static_cast<DLinkedObject*>(Prev);
    }

    /**
     * Get next node in the list when iterating.
     * @return next node in list.
     */
    inline DLinkedObject *getNext(void) const {
        return static_cast<DLinkedObject*>(LinkedObject::getNext());
    }

    /**
     * Insert object behind our object.
     * @param object to add to list.
     */
    void insertTail(DLinkedObject *object);

    /**
     * Insert object in front of our object.
     * @param object to add to list.
     */
    void insertHead(DLinkedObject *object);

    /**
     * Insert object, method in derived object.
     * @param object to add to list.
     */
    virtual void insert(DLinkedObject *object);

    /**
     * Insert object behind our object.
     * @param object to add to list.
     */
    inline DLinkedObject& operator+=(DLinkedObject *object) {
        insertTail(object);
        return *this;
    }

    /**
     * Insert object in front of our object.
     * @param object to add to list.
     */
    inline DLinkedObject& operator-=(DLinkedObject *object) {
        insertHead(object);
        return *this;
    }

    /**
     * Insert object in list with our object.
     * @param object to add to list.
     */
    inline DLinkedObject& operator*=(DLinkedObject *object) {
        insert(object);
        return *this;
    }
};

/**
 * Template value class to embed data structure into a linked list.
 * This is used to form a class which can be linked together using
 * either an ordered index or simple linked pointer chain and that
 * contains a member value object.  Most of the core logic for this
 * template is found in and derived from the object_value template.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
template <typename T, class O = LinkedObject>
class linked_value : public O
{
protected:
    __DELETE_COPY(linked_value);

public:
    T value;

    /**
     * Create embedded value object unlinked.
     */
    inline linked_value() {}

    /**
     * Construct embedded object on a linked list.
     * @param root node or pointer for list.
     */
    inline linked_value(LinkedObject **root) {
        LinkedObject::enlist(root);
    }

    /**
     * Construct embedded object on an ordered list.
     * @param index pointer for the ordered list.
     */
    inline linked_value(OrderedIndex *index) {
        O::enlist(index);
    }

    /**
     * Assign embedded value from related type and link to list.
     * @param root node or pointer for list.
     * @param typed_value to assign.
     */
    inline linked_value(LinkedObject **root, const T& typed_value) {
        LinkedObject::enlist(root); 
        value = typed_value;
    }

    /**
     * Assign embedded value from related type and add to list.
     * @param index to list our object on.
     * @param typed_value to assign.
     */
    inline linked_value(OrderedIndex *index, const T& typed_value) {
        O::enlist(index);
        value = typed_value; 
    }

    inline void set(const T& typed_value) {
        value = typed_value;
    }

    /**
     * Assign embedded value from related type.
     * @param typed_value to assign.
     */
    inline linked_value& operator=(const T& typed_value) {
        value = typed_value;
        return *this;
    }

    inline T& operator*() {
        return value;
    }

    inline operator T&() {
        return value;
    }

    inline void operator()(const T data) {
        value = data;
    }
};

/**
 * A smart pointer template for iterating linked lists.  This class allows
 * one to access a list of single or double linked objects and iterate
 * through each member of a list.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
template <class T>
class linked_pointer
{
private:
    T *ptr;

public:
    /**
     * Create a linked pointer and assign to start of a list.
     * @param pointer to first member of a linked list.
     */
    inline linked_pointer(T *pointer) {
        ptr = pointer;
    }

    /**
     * Create a copy of an existing linked pointer.
     * @param pointer to copy from.
     */
    inline linked_pointer(const linked_pointer &pointer) {
        ptr = pointer.ptr;
    }

    /**
     * Create a linked pointer assigned from a raw linked object pointer.
     * @param pointer to linked object.
     */
    inline linked_pointer(LinkedObject *pointer) {
        ptr = static_cast<T*>(pointer);
    }

    inline linked_pointer(const LinkedObject *pointer) {
        ptr = static_cast<T*>(pointer);
    }

    /**
     * Create a linked pointer to examine an ordered index.
     * @param index of linked objects to iterate through.
     */
    inline linked_pointer(OrderedIndex *index) {
        ptr = static_cast<T*>(index->begin());
    }

    /**
     * Create a linked pointer not attached to a list.
     */
    inline linked_pointer() {
        ptr = NULL;
    }

    /**
     * Assign our typed iterative pointer from a matching typed object.
     * @param pointer to typed object.
     */
    inline void operator=(T *pointer) {
        ptr = pointer;
    }

    /**
     * Assign our pointer from another pointer.
     * @param pointer to assign from.
     */
    inline void operator=(linked_pointer &pointer) {
        ptr = pointer.ptr;
    }

    /**
     * Assign our pointer from the start of an ordered index.
     * @param index to assign pointer from.
     */
    inline void operator=(OrderedIndex *index) {
        ptr = static_cast<T*>(index->begin());
    }

    /**
     * Assign our pointer from a generic linked object pointer.
     * @param pointer of linked list.
     */
    inline void operator=(LinkedObject *pointer) {
        ptr = static_cast<T*>(pointer);
    }

    /**
     * Return member from typed object our pointer references.
     * @return evaluated member of object we point to.
     */
    inline T* operator->() const {
        return ptr;
    }

    /**
     * Return object we currently point to.
     * @return object linked pointer references.
     */
    inline T* operator*() const {
        return ptr;
    }

    /**
     * Return object we point to by casting.
     * @return object linked pointer references.
     */
    inline operator T*() const {
        return ptr;
    }

    /**
     * Move (iterate) pointer to previous member in double linked list.
     */
    inline void prev(void) {
        ptr = static_cast<T*>(ptr->getPrev());
    }

    /**
     * Move (iterate) pointer to next member in linked list.
     */
    inline void next(void) {
        ptr = static_cast<T*>(ptr->getNext());
    }

    /**
     * Get the next member in linked list.  Do not change who we point to.
     * @return next member in list or NULL if end of list.
     */
    inline T *getNext(void) const {
        return static_cast<T*>(ptr->getNext());
    }

    /**
     * Get the previous member in double linked list.  Do not change who we
     * point to.
     * @return previous member in list or NULL if start of list.
     */
    inline T *getPrev(void) const {
        return static_cast<T*>(ptr->getPrev());
    }

    /**
     * Move (iterate) pointer to next member in linked list.
     */
    inline void operator++() {
        ptr = static_cast<T*>(ptr->getNext());
    }

    /**
     * Move (iterate) pointer to previous member in double linked list.
     */
    inline void operator--() {
        ptr = static_cast<T*>(ptr->getPrev());
    }

    /**
     * Test for next member in linked list.
     * @return true if there is more members after current one.
     */
    inline bool is_next(void) const {
        return (ptr->getNext() != NULL);
    }

    /**
     * Test for previous member in double linked list.
     * @return true if there is more members before current one.
     */
    inline bool is_prev(void) const {
        return (ptr->getPrev() != NULL);
    }

    /**
     * Test if linked pointer is set/we are not at end of list.
     * @return true if we are not at end of list.
     */
    inline operator bool() const {
        return (ptr != NULL);
    }

    /**
     * Test if linked list is empty/we are at end of list.
     * @return true if we are at end of list.
     */
    inline bool operator!() const {
        return (ptr == NULL);
    }

    inline bool is() const {
        return (ptr != NULL);
    }

    /**
     * Return pointer to our linked pointer to use as root node of a chain.
     * @return our object pointer as a root index.
     */
    inline LinkedObject **root(void) const {
        T **r = &ptr;
        return static_cast<LinkedObject**>(r);
    }
};

/**
 * Embed data objects into a tree structured memory database.  This can
 * be used to form XML document trees or other data structures that
 * can be organized in trees.  The NamedTree class is used to manage
 * the structure of the tree, and the type specified is embedded as a
 * data value object which can be manipulated.  Name identifiers are
 * assumed to be dynamically allocated if tree node elements are deletable.
 *
 * Embedded values can either be of direct types that are then stored as
 * part of the template object, or of class types that are data pointers.
 * The latter might be used for trees that contain data which might be
 * parsed dynamically from a document and/or saved on a heap.  Pointer trees
 * assume that NULL pointers are for nodes that are empty, and that NULL data
 * value nodes with children are trunk nodes.  Generally data values are then
 * allocated with a pointer stored in pure leaf nodes.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
template <typename T>
class treemap : public NamedTree
{
protected:
    T value;

public:
    /**
     * Construct a typed root node for the tree.  The root node may be
     * named as a stand-alone node or unnamed.
     * @param name of the node we are creating.
     */
    inline treemap(char *name = NULL) : NamedTree(name) {}

    /**
     * Construct a copy of the treemap object.
     * @param source of copy for new object.
     */
    inline treemap(const treemap& source) : NamedTree(source) {
        value = source.value;
    };

    /**
     * Construct a child node on an existing tree.
     * @param parent of this node to attach.
     * @param name of this node.
     */
    inline treemap(treemap *parent, char *name) : NamedTree(parent, name) {}

    /**
     * Construct a child node on an existing tree and assign it's value.
     * @param parent of this node to attach.
     * @param name of this node.
     * @param reference to value to assign to this node.
     */
    inline treemap(treemap *parent, char *name, T& reference) : NamedTree(parent, name) {
        value = reference;
    }

    /**
     * Return the typed value of this node.
     * @return reference to value of node.
     */
    inline const T& get(void) const {
        return value;
    }

    /**
     * Return typed value of this node by pointer reference.
     * @return value of node.
     */
    inline const T& operator*() const {
        return value;
    }

    /**
     * Return value from tree element when value is a pointer.
     * @param node in our typed tree.
     * @return value of node.
     */
    static inline T getPointer(treemap *node) {
        return (node == NULL) ? NULL : node->value;
    }

    /**
     * Test if this node is a leaf node for a tree pointer table.
     * @return true if value pointer is not NULL and there are no children.
     */
    inline bool is_attribute(void) const {
        return (!Child.begin() && value != NULL);
    }

    /**
     * Get the pointer of a pointer based value tree.
     * @return value pointer of node.
     */
    inline const T getPointer(void) const {
        return value;
    }

    /**
     * Get the data value of a data based value tree.
     * @return data value of node.
     */
    inline const T& getData(void) const {
        return value;
    }

    /**
     * Set the pointer of a pointer based value tree.
     * @param pointer to set.
     */
    inline void setPointer(const T pointer) {
        value = pointer;
    }

    /**
     * Set the value of a data based value tree.
     * @param reference to value to copy into node.
     */
    inline void set(const T& reference) {
        value = reference;
    }

    /**
     * Assign the value of our node.
     * @param data value to assign.
     */
    inline void operator=(const T& data) {
        value = data;
    }

    /**
     * Get child member node by index.
     * @param index of child member.
     * @return node or NULL if past end.
     */
    inline treemap *getIndexed(unsigned index) const {
        return static_cast<treemap*>(Child.getIndexed(index));
    }

    /**
     * Get the typed parent node for our node.
     * @return parent node or NULL if root of tree.
     */
    inline treemap *getParent(void) const {
        return static_cast<treemap*>(Parent);
    }

    /**
     * Get direct typed child node of our node of specified name.  This
     * does not perform a recursive search.
     * @param name of child node.
     * @return typed child node pointer or NULL if not found.
     */
    inline treemap *getChild(const char *name) const {
        return static_cast<treemap*>(NamedTree::getChild(name));
    }

    /**
     * Find a direct typed leaf node on our node.  A leaf node is a node that
     * has no children of it's own.  This does not perform a recursive search.
     * @param name of leaf child node to find.
     * @return typed leaf node object of leaf or NULL.
     */
    inline treemap *getLeaf(const char *name) const {
        return static_cast<treemap*>(NamedTree::getLeaf(name));
    }

    /**
     * Get the value pointer of a leaf node of a pointer tree.  This allows
     * one to find a leaf node and return it's pointer value in a single
     * operation.
     * @param name of leaf node.
     * @return value of leaf pointer if found and contains value, or NULL.
     */
    inline T getValue(const char *name) const {
        return getPointer(getLeaf(name));
    }

    /**
     * Find a subnode from our node by name.  This performs a recursive
     * search.
     * @param name to search for.
     * @return typed node that is found or NULL if none is found.
     */
    inline treemap *find(const char *name) const {
        return static_cast<treemap*>(NamedTree::find(name));
    }

    /**
     * Find a subnode by pathname.  This is the same as the NamedTree
     * path member function.
     * @param path name to search for node.
     * @return typed node that is found at path or NULL.
     */
    inline treemap *path(const char *path) const {
        return static_cast<treemap*>(NamedTree::path(path));
    }

    /**
     * Search for a leaf node of our node.  This performs a recursive
     * search.
     * @param name to search for.
     * @return typed not that is found or NULL if none is found.
     */
    inline treemap *leaf(const char *name) const {
        return static_cast<treemap*>(NamedTree::leaf(name));
    }

    /**
     * Get first child of our node.  This is useful for iterating children.
     * @return first child or NULL.
     */
    inline treemap *getFirst(void) const {
        return static_cast<treemap*>(NamedTree::getFirst());
    }
};

/**
 * Convenience typedef for root pointers of single linked lists.
 */
typedef LinkedObject *LinkedIndex;

typedef DLinkedObject LinkedList;   // compatibility for older code

} // namespace ucommon

#endif
