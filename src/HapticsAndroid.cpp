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

std::string getVibratorStatus() {
    auto mod = geode::Mod::get();
    std::string report = fmt::format("Saved phone toggle: {}\nStrength: {}\nMode: {}\n",
        mod->getSettingValue<bool>("phone-vibration") ? "On" : "Off",
        mod->getSettingValue<double>("strength-scale"),
        mod->getSettingValue<bool>("phone-duration-only") ? "Cocos-style" :
        mod->getSettingValue<bool>("android-modern-vibration") ?
            (mod->getSettingValue<bool>("android-legacy-vibration") ? "Modern + fallback" : "Modern only") :
            (mod->getSettingValue<bool>("android-legacy-vibration") ? "Legacy only" : "Disabled"));
    auto vm = cocos2d::JniHelper::getJavaVM();
    if (!vm) return report + "Java VM: unavailable";
    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK || !env)
        return report + "JNI: current thread is not attached";
    if (env->ExceptionCheck()) return report + "JNI: pre-existing Java exception (preserved)";
    if (env->PushLocalFrame(16) != JNI_OK) {
        if (env->ExceptionCheck()) env->ExceptionClear();
        return report + "JNI: could not allocate local frame";
    }
    JniFrame frame{env};
    auto helper = env->FindClass("org/cocos2dx/lib/Cocos2dxHelper");
    if (env->ExceptionCheck() || !helper) return report + "Activity: Cocos2dxHelper class lookup failed";
    auto getActivity = env->GetStaticMethodID(helper, "getActivity", "()Landroid/app/Activity;");
    if (env->ExceptionCheck() || !getActivity) return report + "Activity: getActivity method lookup failed";
    auto activity = env->CallStaticObjectMethod(helper, getActivity);
    if (env->ExceptionCheck() || !activity) return report + "Activity: unavailable or Java call failed";
    auto activityClass = env->GetObjectClass(activity);
    if (env->ExceptionCheck() || !activityClass) return report + "Activity: class lookup failed";

    auto permission = env->GetMethodID(activityClass, "checkSelfPermission", "(Ljava/lang/String;)I");
    if (env->ExceptionCheck() || !permission) {
        env->ExceptionClear();
        report += "VIBRATE permission: lookup failed\n";
    } else {
        auto name = env->NewStringUTF("android.permission.VIBRATE");
        if (env->ExceptionCheck() || !name) return report + "Permission: string allocation failed";
        auto granted = env->CallIntMethod(activity, permission, name);
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            report += "VIBRATE permission: query failed\n";
        } else report += granted == 0 ? "VIBRATE permission: granted\n" : "VIBRATE permission: denied\n";
    }
    auto getService = env->GetMethodID(activityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    if (env->ExceptionCheck() || !getService) return report + "Service: method lookup failed";
    auto name = env->NewStringUTF("vibrator");
    if (env->ExceptionCheck() || !name) return report + "Service: string allocation failed";
    auto vibrator = env->CallObjectMethod(activity, getService, name);
    if (env->ExceptionCheck()) return report + "Service: Java call failed";
    if (!vibrator) return report + "Service: unavailable";
    auto vibratorClass = env->GetObjectClass(vibrator);
    if (env->ExceptionCheck() || !vibratorClass) return report + "Service: class lookup failed";
    report += "Vibrator service: accessible\n";

    auto query = [&](char const* method, char const* unavailable) -> std::string {
        auto id = env->GetMethodID(vibratorClass, method, "()Z");
        if (env->ExceptionCheck() || !id) {
            env->ExceptionClear();
            return unavailable;
        }
        auto value = env->CallBooleanMethod(vibrator, id);
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
            return "Query failed";
        }
        return value == JNI_TRUE ? "Yes" : "No";
    };
    report += "Hardware present: " + query("hasVibrator", "Lookup failed") + "\n";
    report += "Intensity control: " + query("hasAmplitudeControl", "API unavailable") + "\n";
    report += "Status checks do not confirm physical vibration.";
    return report;
}

void vibratePhone(Pulse pulse) {
    if (pulse.strength <= 0.f || pulse.durationMs == 0) return;
    auto mod = geode::Mod::get();
    if (!mod->getSettingValue<bool>("phone-vibration")) return;
    auto durationOnly = mod->getSettingValue<bool>("phone-duration-only");
    auto modernEnabled = mod->getSettingValue<bool>("android-modern-vibration");
    auto legacyEnabled = mod->getSettingValue<bool>("android-legacy-vibration");
    if (!durationOnly && !modernEnabled && !legacyEnabled) return;

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

    // Cocos-style mode uses the duration-only native API, independently of
    // the intensity-aware mode's modern/legacy preferences.
    if (durationOnly) {
        vibrateLegacy(env, vibrator, vibratorClass, static_cast<jlong>(pulse.durationMs));
        return;
    }

    // Clear modern API failures before trying the optional legacy backend.
    auto fallback = [&] {
        if (env->ExceptionCheck()) env->ExceptionClear();
        if (legacyEnabled)
            vibrateLegacy(env, vibrator, vibratorClass, static_cast<jlong>(pulse.durationMs));
    };
    if (!modernEnabled) {
        fallback();
        return;
    }

    auto effectClass = env->FindClass("android/os/VibrationEffect");
    if (env->ExceptionCheck() || !effectClass) {
        fallback();
        return;
    }
    auto createOneShot = env->GetStaticMethodID(
        effectClass, "createOneShot", "(JI)Landroid/os/VibrationEffect;"
    );
    if (env->ExceptionCheck() || !createOneShot) {
        fallback();
        return;
    }
    auto vibrate = env->GetMethodID(vibratorClass, "vibrate", "(Landroid/os/VibrationEffect;)V");
    if (env->ExceptionCheck() || !vibrate) {
        fallback();
        return;
    }
    auto amplitude = static_cast<jint>(1.f + pulse.strength * 254.f);
    auto effect = env->CallStaticObjectMethod(
        effectClass, createOneShot, static_cast<jlong>(pulse.durationMs), amplitude
    );
    if (env->ExceptionCheck() || !effect) {
        fallback();
        return;
    }
    env->CallVoidMethod(vibrator, vibrate, effect);
    if (env->ExceptionCheck()) fallback();
}

} // namespace geoshake
