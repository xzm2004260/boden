#ifndef BDN_JAVA_Reference_H_
#define BDN_JAVA_Reference_H_

#include <jni.h>

namespace bdn
{
namespace java
{

/** Represents a reference to an object in the "Java world".
 *
 *  References can be weak global, strong global or local.
 *
 *  local references are only valid during the current JNI callback. When the native code function
 *  returns and gives control back to the Java world then these references become invalid.
 *
 *  To store references for use in a subsequent JNI callback they have to be converted to a global
 *  reference. These can be weak or strong.
 *
 *  Strong references prevent the Java object from being garbage collected. Weak references do
 *  not prevent that, so the object can be garbage collected at any time - even DURING a single JNI
 *  callback.
 *  Because of this, weak references cannot be used to access the Java object directly. Instead,
 *  they have to be converted by calling toStrong(). The returned strong reference will be null if the
 *  object has already been garbage collected. If the returned reference is not null then the object
 *  cannot be garbage collected anymore while the new strong reference exists and can thus be accessed
 *  safely now.
 *
 *  There is are convenience classes called OwnedLocalReference and ExternalLocalReference
 *  that makes it easy to create local reference objects for the types Type::ownedLocal and Type::externalLocal.
 *
 *  Note that the Reference class automatically deletes references of Type::local when the last Reference instance
 *  that refers to it is deleted. That is important because the total number of local references that can exist during a
 *  single JNI callback is limited.
 *
 * */
class Reference : public Base
{
public:

    enum class Type
    {
        /** An uninitialized reference.*/
        invalid=0,

        /** A local reference that was created on the native code side (i.e. it was returns by one of the various
         *  object creation functions that the native code side can use).
         * 
         *  Local references become invalid once the current Jni callback returns. To keep them beyond the current
         *  callback they must be converted to strong or weak references with toStrong() or toWeak().
         * 
         *  See Reference class documentation for more info.
         *  */
        ownedLocal,
        
        /** A local reference that is owned by the Java-side. Like ownedLocal references, these are also deleted
         *  when the current Jni callback returns, BUT they are under the complete control of the calling java code
         *  and must be handled differently than other local references.
         * 
         *  Use this for jobject references that you get as parameters in JNI callback functions.
         * 
         *  See Reference class documentation for more info.
         *  */
        externalLocal,

        /** A strong global reference. These prevent the object from being garbage collected
         *  and persist between Jni callbacks.
         *  See Reference class documentation for more info.
         *  */
        strong,

        /** A weak global reference. These do NOT prevent the from being garbage collected, but they
         *  remain valid between Jni callbacks.
         *  See Reference class documentation for more info.
         *  */
        weak,

    };


    /** Creates a java reference of the specified type.
     *
     *  Note that there are no constructors that create strong or weak global references.
     *  A local Reference object must first be created and then you can convert the reference
     *  to a global reference by calling toStrong() or toWeak().
     * */
    Reference(Type type, jobject ref)
     : _pShared( newObj<Shared>(type, ref) )
    {
    }


    /** Copy constructor.
     *
     *  Note that copying a Reference object is a very cheap operation. No
     *  JNI calls happen when a reference object is copied.
     *
     * */
    Reference(const Reference & o)
    : _pShared( o._pShared )
    {
    }

    /** Move constructor.*/
    Reference(Reference&& o)
    : _pShared( std::move(o._pShared) )
    {
    }


    /** Constructs a reference of type invalid. Invalid references behave the same way
     *  as null references and also correspond to null references on the java side.*/
    Reference()
    : Reference(Type::invalid, NULL )
    {
    }


    Reference& operator=(const Reference & o)
    {
        _pShared = o._pShared;

        return *this;
    }


    /** Returns the reference type.*/
    Type getType() const
    {
        return _pShared->getType();
    }

    jobject getJObject() const
    {
        return _pShared->getJObject();
    }


    /** Creates a new strong global reference to the object.
     *
     *  If the current reference is a weak reference then the returned strong reference might be null
     *  (if the object has already been garbage collected). You should use isNull on the returned
     *  reference to check for that condition.
     *
     *  If the current reference is invalid (not initialized) then a null reference is returned.
     *
     *  If the current reference is already a strong reference then another copy of the reference is returned.
     *  Each copy keeps the object alive independent of the other.
     *
     *  See Reference class documentation for more information.*/
    Reference toStrong() const;

    /** Creates a new weak global reference to the object.
     *
     *  If the current reference is invalid (not initialized) then a null reference is returned.
     *
     *  If the current reference is already a weak reference then another copy of the reference is returned.
     *
     *  See Reference class documentation for more information.*/
    Reference toWeak() const;



    /** Returns a reference object that can be used to access the object.
     *  If the current type is Type::weak then a strong reference is returned (which may be null).
     *  If the current type is not Type::weak then an identical copy of this reference object is returned.*/
    Reference toAccessible() const
    {
        if( _pShared->getType()==Type::weak )
            return toStrong();
        else
            return Reference(*this );
    }




    /** Returns true if the reference is null.
     *
     *  Note that calling this on a reference of the "weak" type does not make any sense. Weak references
     *  can become null at ANY point in time, even right after they have been checked for null.
     *  So even if isNull returns true for a weak reference, that information is immediately outdated and
     *  it is not safe to use the reference.
     *  Weak references should first be converted to strong references with toStrong(). Those do not have
     *  this problem and then isNull can be safely called on the strong reference.
     * */
    bool isNull() const;




    /** Returns true if the reference is null.*/
    bool operator==(std::nullptr_t) const
    {
        return isNull();
    }



    /** Returns true if this reference points to the same object as the other reference.*/
    bool operator==(const Reference & o) const;

    /** Returns true if the reference is null.*/
    bool operator!=(std::nullptr_t) const
    {
        return !isNull();
    }


    /** Returns true if this reference points to the same object as the other reference.*/
    bool operator!=(const Reference & o) const
    {
        return !operator==(o);
    }





private:

    class Shared : public Base
    {
    public:
        Shared(Type type, jobject ref)
        {
            _type = type;
            _ref = ref;
        }

        ~Shared();

        jobject getJObject() const
        {
            return _ref;
        }


        Type getType() const
        {
            return _type;
        }

    protected:
        Type    _type;
        jobject _ref;
    };

    P<Shared> _pShared;
};


}
}


#endif

