#include <bdn/init.h>
#include <bdn/android/JNativeRootView.h>

#include <bdn/java/Env.h>
#include <bdn/entry.h>

#include <bdn/android/WindowCore.h>
#include <bdn/android/JContext.h>

extern "C" JNIEXPORT void JNICALL
Java_io_boden_android_NativeRootView_created(JNIEnv *pEnv, jobject rawSelf)
{
    bdn::platformEntryWrapper(
        [&]() {
            bdn::android::WindowCore::_rootViewCreated(
                bdn::java::Reference::convertExternalLocal(rawSelf));
        },
        true, pEnv);
}

extern "C" JNIEXPORT void JNICALL
Java_io_boden_android_NativeRootView_disposed(JNIEnv *pEnv, jobject rawSelf)
{
    bdn::platformEntryWrapper(
        [&]() {
            bdn::android::WindowCore::_rootViewDisposed(
                bdn::java::Reference::convertExternalLocal(rawSelf));
        },
        true, pEnv);
}

extern "C" JNIEXPORT void JNICALL
Java_io_boden_android_NativeRootView_sizeChanged(JNIEnv *pEnv, jobject rawSelf,
                                                 int newWidth, int newHeight)
{
    bdn::platformEntryWrapper(
        [&]() {
            bdn::android::WindowCore::_rootViewSizeChanged(
                bdn::java::Reference::convertExternalLocal(rawSelf), newWidth,
                newHeight);
        },
        true, pEnv);
}

extern "C" JNIEXPORT void JNICALL
Java_io_boden_android_NativeRootView_configurationChanged(JNIEnv *pEnv,
                                                          jobject rawSelf,
                                                          jobject rawNewConfig)
{
    bdn::platformEntryWrapper(
        [&]() {
            bdn::android::JConfiguration newConfig(
                (bdn::java::Reference::convertExternalLocal(rawNewConfig)));

            bdn::android::WindowCore::_rootViewConfigurationChanged(
                bdn::java::Reference::convertExternalLocal(rawSelf), newConfig);
        },
        true, pEnv);
}