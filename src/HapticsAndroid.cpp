#include "Haptics.hpp"

#include <Geode/Geode.hpp>
#include <Geode/cocos/platform/android/jni/JniHelper.h>
#include <jni.h>

namespace geoshake {
namespace {

// Release every local reference and contain Java errors on every return path.
struct JniFrame {
    JNIEnv* env;
    ~JniFrame() {
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->PopLocalFrame(nullptr);
    }
};

void vibrateLegacy(JNIEnv* env, jobject vibrator, jclass vibratorClass, jlong duration) {
    auto vibrate = env->GetMethodID(vibratorClass, "vibrate", "(J)V");
    if (env->ExceptionCheck() || !vibrate) return;
    env->CallVoidMethod(vibrator, vibrate, duration);
    // The caller's frame clears exceptions, including missing VIBRATE permission.
}

} // namespace

void vibratePhone(Pulse pulse) {
    if (pulse.strength <= 0.f || pulse.durationMs == 0) return;

    auto vm = cocos2d::JniHelper::getJavaVM();
    if (!vm) return;
    JNIEnv* env = nullptr;
    // Shake callbacks normally run on the attached game thread. Skip haptics
    // if called from another thread; never reuse a different thread's JNIEnv.
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK || !env) return;
    if (env->ExceptionCheck()) return; // Preserve exceptions belonging to the caller.
    if (env->PushLocalFrame(16) != JNI_OK) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        return;
    }
    JniFrame frame{env};

    auto helper = env->FindClass("org/cocos2dx/lib/Cocos2dxHelper");
    if (env->ExceptionCheck() || !helper) return;
    auto getActivity = env->GetStaticMethodID(helper, "getActivity", "()Landroid/app/Activity;");
    if (env->ExceptionCheck() || !getActivity) return;
    auto activity = env->CallStaticObjectMethod(helper, getActivity);
    if (env->ExceptionCheck() || !activity) return;
    auto activityClass = env->GetObjectClass(activity);
    if (env->ExceptionCheck() || !activityClass) return;
    auto getService = env->GetMethodID(
        activityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;"
    );
    if (env->ExceptionCheck() || !getService) return;
    auto serviceName = env->NewStringUTF("vibrator");
    if (env->ExceptionCheck() || !serviceName) return;
    auto vibrator = env->CallObjectMethod(activity, getService, serviceName);
    if (env->ExceptionCheck() || !vibrator) return;
    auto vibratorClass = env->GetObjectClass(vibrator);
    if (env->ExceptionCheck() || !vibratorClass) return;

    auto effectClass = env->FindClass("android/os/VibrationEffect");
    if (env->ExceptionCheck() || !effectClass) {
        env->ExceptionClear();
        vibrateLegacy(env, vibrator, vibratorClass, static_cast<jlong>(pulse.durationMs));
        return;
    }
    auto createOneShot = env->GetStaticMethodID(
        effectClass, "createOneShot", "(JI)Landroid/os/VibrationEffect;"
    );
    if (env->ExceptionCheck() || !createOneShot) {
        env->ExceptionClear();
        vibrateLegacy(env, vibrator, vibratorClass, static_cast<jlong>(pulse.durationMs));
        return;
    }
    auto vibrate = env->GetMethodID(vibratorClass, "vibrate", "(Landroid/os/VibrationEffect;)V");
    if (env->ExceptionCheck() || !vibrate) {
        env->ExceptionClear();
        vibrateLegacy(env, vibrator, vibratorClass, static_cast<jlong>(pulse.durationMs));
        return;
    }
    auto amplitude = static_cast<jint>(1.f + pulse.strength * 254.f);
    auto effect = env->CallStaticObjectMethod(
        effectClass, createOneShot, static_cast<jlong>(pulse.durationMs), amplitude
    );
    if (env->ExceptionCheck() || !effect) return;
    env->CallVoidMethod(vibrator, vibrate, effect);
}

} // namespace geoshake
