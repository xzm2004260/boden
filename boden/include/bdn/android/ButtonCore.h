#ifndef BDN_ANDROID_ButtonCore_H_
#define BDN_ANDROID_ButtonCore_H_

#include <bdn/android/ViewCore.h>
#include <bdn/android/JButton.h>
#include <bdn/IButtonCore.h>
#include <bdn/Button.h>


namespace bdn
{
namespace android
{


class ButtonCore : public ViewCore, BDN_IMPLEMENTS IButtonCore
{
private:
    static P<JButton> _createJButton(Button* pOuter)
    {
        // we need to know the context to create the view.
        // If we have a parent then we can get that from the parent's core.
        P<View> 	pParent = pOuter->getParentView();
        if(pParent==nullptr)
            throw ProgrammingError("ButtonCore instance requested for a Button that does not have a parent.");

        P<ViewCore> pParentCore = cast<ViewCore>( pParent->getViewCore() );
        if(pParentCore==nullptr)
            throw ProgrammingError("ButtonCore instance requested for a Button with core-less parent.");

        JContext context = pParentCore->getJView().getContext();

        return newObj<JButton>(context);
    }

public:
    ButtonCore( Button* pOuterButton )
     : ViewCore( pOuterButton, _createJButton(pOuterButton) )
    {
        _pJButton = cast<JButton>( &getJView() );

        setLabel( pOuterButton->label() );
    }

    JButton& getJButton()
    {
        return *_pJButton;
    }


    void setLabel(const String& label) override
    {
        _pJButton->setText( label );

        getOuterView()->needSizingInfoUpdate();
    }

    void clicked() override
    {
        ClickEvent evt( getOuterView() );
            
        cast<Button>(getOuterView())->onClick().notify(evt);
    }

private:
    P<JButton> _pJButton;
};


}    
}


#endif

