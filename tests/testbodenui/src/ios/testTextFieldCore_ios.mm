#include <bdn/init.h>
#include <bdn/test.h>

#include <bdn/TextField.h>
#include <bdn/Window.h>
#include <bdn/test/TestTextFieldCore.h>

#import "TestIosViewCoreMixin.hh"

using namespace bdn;

class TestIosTextFieldCore : public bdn::test::TestIosViewCoreMixin<bdn::test::TestTextFieldCore>
{
  protected:
    void initCore() override
    {
        bdn::test::TestIosViewCoreMixin<bdn::test::TestTextFieldCore>::initCore();

        _uITextField = (UITextField *)_uIView;
        REQUIRE(_uITextField != nullptr);
    }

    void verifyCoreText() override
    {
        String expectedText = _textField->text();
        String text = bdn::ios::iosStringToString(_uITextField.text);
        REQUIRE(text == expectedText);
    }

  protected:
    UITextField *_uITextField;
};

TEST_CASE("ios.TextFieldCore")
{
    P<TestIosTextFieldCore> test = newObj<TestIosTextFieldCore>();
    test->runTests();
}
