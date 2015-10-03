LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

$(call import-add-path,$(LOCAL_PATH)/../../cocos2d)
$(call import-add-path,$(LOCAL_PATH)/../../cocos2d/external)
$(call import-add-path,$(LOCAL_PATH)/../../cocos2d/cocos)

LOCAL_MODULE := cocos2dcpp_shared

LOCAL_MODULE_FILENAME := libcocos2dcpp

# cpp and c files
LOCAL_SRC_FILES_JNI_PREFIXED := \
    $(wildcard $(LOCAL_PATH)/../../Classes/*.cpp) \
    $(wildcard $(LOCAL_PATH)/../../Classes/**/*.c*) \
    $(wildcard $(LOCAL_PATH)/../../Classes/**/**/*.c*) \
    $(wildcard $(LOCAL_PATH)/nend/*.cpp)

LOCAL_SRC_FILES_JNI_UNPREFIXED := $(subst jni/,, $(LOCAL_SRC_FILES_JNI_PREFIXED))

LOCAL_SRC_FILES := hellocpp/main.cpp \
                   $(LOCAL_SRC_FILES_JNI_UNPREFIXED)


# ヘッダのincludeパスを追加
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../Classes
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../cocos2d/external
LOCAL_C_INCLUDES += $(LOCAL_PATH)/nend
# LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Classes/Android

LOCAL_WHOLE_STATIC_LIBRARIES := cocos2dx_static
LOCAL_WHOLE_STATIC_LIBRARIES += cocosdenshion_static
LOCAL_WHOLE_STATIC_LIBRARIES += cocos_extension_static
LOCAL_WHOLE_STATIC_LIBRARIES += spine_static

# LOCAL_WHOLE_STATIC_LIBRARIES += box2d_static
# LOCAL_WHOLE_STATIC_LIBRARIES += cocosbuilder_static
# LOCAL_WHOLE_STATIC_LIBRARIES += spine_static
# LOCAL_WHOLE_STATIC_LIBRARIES += cocostudio_static
# LOCAL_WHOLE_STATIC_LIBRARIES += cocos_network_static


include $(BUILD_SHARED_LIBRARY)

$(call import-module,.)
$(call import-module,audio/android)
$(call import-module,extensions)
$(call import-module,editor-support/spine)

# $(call import-module,Box2D)
# $(call import-module,editor-support/cocosbuilder)
# $(call import-module,editor-support/spine)
# $(call import-module,editor-support/cocostudio)
# $(call import-module,network)