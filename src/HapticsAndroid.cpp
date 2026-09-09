#include "Haptics.hpp"

#include <Geode/Geode.hpp>
#include <jni.h>

using namespace geode::prelude;

namespace geoshake {

void vibratePhone(Pulse pulse) {
    if (pulse.strength <= 0.f || pulse.durationMs == 0) return;

    auto env = cocos2d::JniHelper::getEnv();
    auto helper = env->FindClass("org/cocos2dx/lib/Cocos2dxHelper");
    if (!helper) { env->ExceptionClear(); return; }
    auto getActivity = env->GetStaticMethodID(
        helper, "getActivity", "()Landroid/app/Activity;"
    );
    if (!getActivity) { env->ExceptionClear(); env->DeleteLocalRef(helper); return; }
    auto activity = env->CallStaticObjectMethod(helper, getActivity);
    auto activityClass = env->GetObjectClass(activity);
    auto getService = env->GetMethodID(
        activityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;"
    );
    auto serviceName = env->NewStringUTF("vibrator");
    auto vibrator = env->CallObjectMethod(activity, getService, serviceName);
    auto vibratorClass = env->GetObjectClass(vibrator);

    auto effectClass = env->FindClass("android/os/VibrationEffect");
    auto createOneShot = effectClass ? env->GetStaticMethodID(
        effectClass, "createOneShot", "(JI)Landroid/os/VibrationEffect;"
    ) : nullptr;
    auto vibrateEffect = env->GetMethodID(
        vibratorClass, "vibrate", "(Landroid/os/VibrationEffect;)V"
    );
    if (effectClass && createOneShot && vibrateEffect) {
        auto amplitude = static_cast<jint>(1.f + pulse.strength * 254.f);
        auto effect = env->CallStaticObjectMethod(
            effectClass, createOneShot, static_cast<jlong>(pulse.durationMs), amplitude
        );
        env->CallVoidMethod(vibrator, vibrateEffect, effect);
        env->DeleteLocalRef(effect);
    } else {
        env->ExceptionClear();
        auto legacy = env->GetMethodID(vibratorClass, "vibrate", "(J)V");
        if (legacy) env->CallVoidMethod(vibrator, legacy, static_cast<jlong>(pulse.durationMs));
    }

    if (effectClass) env->DeleteLocalRef(effectClass);
    env->DeleteLocalRef(vibratorClass);
    env->DeleteLocalRef(vibrator);
    env->DeleteLocalRef(serviceName);
    env->DeleteLocalRef(activityClass);
    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(helper);
}

} // namespace geoshake
