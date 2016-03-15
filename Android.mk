LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

FFMPEG_SRC_DIR := $(TOP)/vendor/proprietary/ffmpeg
FFMPEG_CONFIG_DIR := android/$(TARGET_PRODUCT)-$(TARGET_BUILD_VARIANT)/arm

LOCAL_SRC_FILES += \
    CFFUtils.cpp                \
    CFFProbe.cpp                \
    CFFSource.cpp               \
    CMediaExtensionUtils.cpp    \

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/include \
    $(TOP)/frameworks/av/include \
    $(FFMPEG_SRC_DIR)/$(FFMPEG_CONFIG_DIR) \

LOCAL_C_INCLUDES += \
    $(FFMPEG_SRC_DIR) \
    $(FFMPEG_SRC_DIR)/android/include

LOCAL_SHARED_LIBRARIES := \
    libutils          \
    libcutils         \
    libavcodec        \
    libavformat       \
    libavutil         \

LOCAL_MODULE:= libffscanner_plugin

LOCAL_MODULE_TAGS := optional

#ifeq ($(TARGET_2ND_ARCH),arm)
#    LOCAL_CFLAGS += -Wno-psabi
#endif

#LOCAL_CFLAGS += -D__STDC_CONSTANT_MACROS=1 -D__STDINT_LIMITS=1
LOCAL_CFLAGS += -Wno-multichar

#ifeq ($(TARGET_ARCH),arm)
#    LOCAL_CFLAGS += -fpermissive
#endif

include $(BUILD_SHARED_LIBRARY)
