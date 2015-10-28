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
 * A common object base class with auto-pointer support.
 * A common object class is used which may be referenced counted and
 * associated with a smart auto-pointer class.  A lot of the things
 * found here were inspired by working with Objective-C.  Many of the
 * classes are designed to offer automatic heap management through
 * smart pointers and temporary objects controlled through the scope of
 * the stack frame of method calls.
 * @file ucommon/object.h
 */

#ifndef _UCOMMON_OBJECT_H_
#define _UCOMMON_OBJECT_H_

#ifndef _UCOMMON_CPR_H_
#include <ucommon/cpr.h>
#endif

#ifndef _UCOMMON_GENERICS_H_
#include <ucommon/generics.h>
#endif

#ifndef _UCOMMON_PROTOCOLS_H_
#include <ucommon/protocols.h>
#endif

#include <stdlib.h>

namespace ucommon {

/**
 * A base class for reference counted objects.  Reference counted objects
 * keep track of how many objects refer to them and fall out of scope when
 * they are no longer being referred to.  This can be used to achieve
 * automatic heap management when used in conjunction with smart pointers.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT CountedObject : public __VIRTUAL ObjectProtocol
{
private:
    volatile unsigned count;

protected:
    /**
     * Construct a counted object, mark initially as unreferenced.
     */
    CountedObject();

    /**
     * Construct a copy of a counted object.  Our instance is not a
     * reference to the original object but a duplicate, so we do not
     * retain the original and we do reset our count to mark as
     * initially unreferenced.
     */
    CountedObject(const ObjectProtocol &ref);

    /**
     * Dealloc object no longer referenced.  The dealloc routine would commonly
     * be used for a self delete to return the object back to a heap when
     * it is no longer referenced.
     */
    virtual void dealloc(void);

    /**
     * Force reset of count.
     */
    inline void reset(void) {
        count = 0;
    }

public:
    /**
     * Test if the object has copied references.  This means that more than
     * one object has a reference to our object.
     * @return true if referenced by more than one object.
     */
    inline bool is_copied(void) const {
        return count > 1;
    }

    /**
     * Test if the object has been referenced (retained) by anyone yet.
     * @return true if retained.
     */
    inline bool is_retained(void) const {
        return count > 0;
    }

    /**
     * Return the number of active references (retentions) to our object.
     * @return number of references to our object.
     */
    inline unsigned copied(void) const {
        return count;
    }

    /**
     * Increase reference count when retained.
     */
    void retain(void) __OVERRIDE;

    /**
     * Decrease reference count when released.  If no longer retained, then
     * the object is dealloc'd.
     */
    void release(void) __OVERRIDE;
};

/**
 * A general purpose smart pointer helper class.  This is particularly
 * useful in conjunction with reference counted objects which can be
 * managed and automatically removed from the heap when they are no longer
 * being referenced by a smart pointer.  The smart pointer itself would
 * normally be constructed and initialized as an auto variable in a method
 * call, and will dereference the object when the pointer falls out of scope.
 * This is actually a helper class for the typed pointer template.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT AutoObject
{
protected:
    ObjectProtocol *object;

    AutoObject();

    /**
     * Construct an auto-pointer referencing an existing object.
     * @param object we point to.
     */
    AutoObject(ObjectProtocol *object);

    /**
     * Construct an auto-pointer as a copy of another pointer.  The
     * retention of the object being pointed to will be increased.
     * @param pointer we are a copy of.
     */
    AutoObject(const AutoObject &pointer);

    /**
     * Delete auto pointer.  When it falls out of scope, the retention
     * of the object it references is reduced.  If it falls to zero in
     * a reference counted object, then the object is auto-deleted.
     */
    ~AutoObject();

    /**
     * Set our pointer to a specific object.  If the pointer currently
     * references another object, that object is released.  The pointer
     * references our new object and that new object is retained.
     * @param object to assign to.
     */
    void set(ObjectProtocol *object);

public:
    /**
     * Manually release the pointer.  This reduces the retention level
     * of the object and resets the pointer to point to nobody.
     */
    void release(void);

    /**
     * Test if the pointer is not set.
     * @return true if the pointer is not referencing anything.
     */
    bool operator!() const;

    /**
     * Test if the pointer is referencing an object.
     * @return true if the pointer is currently referencing an object.
     */
    operator bool() const;

};

/**
 * A sparse array of managed objects.  This might be used as a simple
 * array class for reference counted objects.  This class assumes that
 * objects in the array exist when assigned, and that gaps in the array
 * are positions that do not reference any object.  Objects are automatically
 * created (create on access/modify when an array position is referenced
 * for the first time.  This is an abstract class because it is a type
 * factory for objects who's derived class form constructor is not known
 * in advance and is a helper class for the sarray template.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
class __EXPORT SparseObjects
{
private:
    ObjectProtocol **vector;
    unsigned max;

    __DELETE_DEFAULTS(SparseObjects);

protected:
    /**
     * Object factory for creating members of the spare array when they
     * are initially requested.
     * @return new object.
     */
    virtual ObjectProtocol *create(void) = 0;

    /**
     * Purge the array by deleting all created objects.
     */
    void purge(void);

    virtual ObjectProtocol *invalid(void) const;

    /**
     * Get (reference) an object at a specified offset in the array.
     * @param offset in array.
     * @return new or existing object.
     */
    ObjectProtocol *get(unsigned offset);

    /**
     * Create a sparse array of known size.  No member objects are
     * created until they are referenced.
     * @param size of array.
     */
    SparseObjects(unsigned size);

    /**
     * Destroy sparse array and delete all generated objects.
     */
    virtual ~SparseObjects();

public:
    /**
     * Get count of array elements.
     * @return array elements.
     */
    unsigned count(void);
};

/**
 * Generate a typed sparse managed object array.  Members in the array
 * are created when they are first referenced.  The types for objects
 * that are generated by sarray must have Object as a base class.  Managed
 * sparse arrays differ from standard arrays in that the member elements
 * are not allocated from the heap when the array is created, but rather
 * as they are needed.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
template <class T>
class sarray : public SparseObjects
{
private:
    __DELETE_DEFAULTS(sarray);

public:
    /**
     * Generate a sparse typed array of specified size.
     * @param size of array to create.
     */
    inline sarray(unsigned size) : SparseObjects(size) {}

    /**
     * Get typed member of array.  If the object does not exist, it is
     * created.
     * @param offset in array for object.
     * @return pointer to typed object.
     */
    inline T *get(unsigned offset) {
        return static_cast<T*>(SparseObjects::get(offset));
    }

    /**
     * Array operation to access member object.  If the object does not
     * exist, it is created.
     * @param offset in array for object.
     * @return pointer to typed object.
     */
    inline T& operator[](unsigned offset) {
        return reference_cast<T>(get(offset));
    }

    inline T& at(unsigned offset) {
        return reference_cast<T>(SparseObjects::get(offset));
    }

    inline const T* operator()(unsigned offset) const {
        return get(offset);
    }

    inline void operator()(unsigned offset, T value) {
        T& ref = at(offset);
        ref = value;
    }

private:
    __LOCAL ObjectProtocol *create(void) __FINAL {
        return new T;
    }
};

/**
 * Typed smart pointer class.  This is used to manage references to
 * a specific typed object on the heap that is derived from the base Object
 * class.  This is most commonly used to manage references to reference
 * counted heap objects so their heap usage can be auto-managed while there
 * is active references to such objects.  Pointers are usually created on
 * the stack frame and used to reference an object during the life of a
 * member function.  They can be created in other objects that live on the
 * heap and can be used to maintain active references so long as the object
 * they are contained in remains in scope as well.
 * @author David Sugar <dyfet@gnutelephony.org>
 */
template <class T>
class object_pointer : public AutoObject
{
public:
    /**
     * Create a pointer with no reference.
     */
    inline object_pointer() : AutoObject() {}

    /**
     * Create a pointer with a reference to a heap object.
     * @param object we are referencing.
     */
    inline object_pointer(T* object) : AutoObject(object) {}

    inline object_pointer(const object_pointer& copy) : AutoObject(copy) {}

    /**
     * Reference object we are pointing to through pointer indirection.
     * @return pointer to object we are pointing to.
     */
    inline T* operator*() const {
        return protocol_cast<T*>(object);
    }

    /**
     * Reference object we are pointing to through function reference.
     * @return object we are pointing to.
     */
    inline T& operator()() const {
        return reference_cast<T>(object);
    }

    /**
     * Reference member of object we are pointing to.
     * @return reference to member of pointed object.
     */
    inline T* operator->() const {
        return protocol_cast<T*>(object);
    }

    /**
     * Get pointer to object.
     * @return pointer or NULL if we are not referencing an object.
     */
    inline T* get(void) const {
        return protocol_cast<T*>(object);
    }

    /**
     * Perform assignment operator to existing object.
     * @param typed object to assign.
     */
    inline object_pointer& operator=(T *typed) {
        AutoObject::set(polypointer_cast<ObjectProtocol*>(typed));
        return *this;
    }

    inline object_pointer& operator=(const object_pointer& from) {
        AutoObject::set(polypointer_cast<ObjectProtocol*>(from.object));
        return *this;
    }

    /**
     * See if pointer is set.
     */
    inline operator bool() const {
        return object != NULL;
    }

    /**
     * See if pointer is not set.
     */
    inline bool operator!() const {
        return object == NULL;
    }
};

} // namespace ucommon

#endif
