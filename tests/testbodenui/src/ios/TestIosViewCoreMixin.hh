#ifndef BDN_TEST_TestIosViewCore_H_
#define BDN_TEST_TestIosViewCore_H_

#include <bdn/View.h>
#import <bdn/ios/ViewCore.hh>
#import <bdn/ios/UiProvider.hh>
#import <bdn/ios/util.hh>

namespace bdn
{
    namespace test
    {

        /** A mixin class that adds implementations of ios view specific
         functionality on top of the base class specified in the template
         parameter BaseClass.*/
        template <class BaseClass> class TestIosViewCoreMixin : public BaseClass
        {
          protected:
            void initCore() override
            {
                BaseClass::initCore();

                _iosViewCore = cast<bdn::ios::ViewCore>(BaseClass::_view->getViewCore());
                REQUIRE(_iosViewCore != nullptr);

                _uIView = _iosViewCore->getUIView();
                REQUIRE(_uIView != nullptr);
            }

            IUiProvider &getUiProvider() override { return bdn::ios::UiProvider::get(); }

            void verifyCoreVisibility() override
            {
                bool expectedVisible = BaseClass::_view->visible();

                REQUIRE(_uIView.hidden == !expectedVisible);
            }

            Rect getFrameRect() const { return bdn::ios::iosRectToRect(_uIView.frame); }

            void verifyInitialDummyCoreSize() override
            {
                Rect rect = getFrameRect();

                // even the dummy rects of newly created controls will sometimes
                // have a valid size from the start.
                REQUIRE(rect.height >= 0);
                REQUIRE(rect.width >= 0);
            }

            void verifyCorePosition() override
            {
                Point position = getFrameRect().getPosition();
                Point expectedPosition = BaseClass::_view->position();

                REQUIRE(position == expectedPosition);
            }

            void verifyCoreSize() override
            {
                Size size = getFrameRect().getSize();
                Size expectedSize = BaseClass::_view->size();

                REQUIRE(size == expectedSize);
            }

            void verifyCorePadding() override
            {
                // the padding is not reflected in UIKit properties.
                // So nothing to test here.
            }

            P<bdn::ios::ViewCore> _iosViewCore;
            UIView *_uIView;
        };
    }
}

#endif
