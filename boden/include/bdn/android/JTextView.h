#ifndef BDN_ANDROID_JTextView_H_
#define BDN_ANDROID_JTextView_H_

#include <bdn/android/JView.h>
#include <bdn/android/JContext.h>
#include <bdn/android/JTextPaint.h>
#include <bdn/java/JString.h>

namespace bdn
{
namespace android
{


/** Accessor for Java android.widget.TextView objects.*/
class JTextView : public JView
{
private:
    static bdn::java::Reference newInstance_(JContext context)
    {
        static bdn::java::MethodId constructorId;

        return getStaticClass_().newInstance_(constructorId, context);
    }

public:
    /** @param objectRef the reference to the Java object.
    *      The JObject instance will copy this reference and keep its type.
    *      So if you want the JObject instance to hold a strong reference
    *      then you need to call toStrong() on the reference first and pass the result.
    *      */
    explicit JTextView(const bdn::java::Reference& objectRef)
        : JView(objectRef)
    {
    }


    JTextView( JContext context )
    : JTextView( newInstance_(context) )
    {
    }


    /** Returns the TextPaint object that is used by this text view.*/
    JTextPaint getPaint()
    {
        static bdn::java::MethodId methodId;

        return invoke_<JTextPaint>(getStaticClass_(), methodId, "getPaint");
    }


    /** Returns the JClass object for this class.s
     *
     *  Note that the returned class object is not necessarily unique for the whole
     *  process.
     *  You might get different objects if this function is called from different
     *  shared libraries.
     *
     *  If you want to check for type equality then you should compare the type name
     *  (see getTypeName() )
     *  */
    static bdn::java::JClass& getStaticClass_ ()
    {
        static bdn::java::JClass cls( "android/widget/TextView" );

        return cls;
    }

    void setText(const String& text)
    {
        static bdn::java::MethodId methodId;

        invoke_<void>(getStaticClass_(), methodId, "setText", bdn::java::JCharSequence( bdn::java::JString(text).getRef_() ) );
    }

};


}
}


#endif

