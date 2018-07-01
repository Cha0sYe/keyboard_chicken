LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := chicken-common

LOCAL_SRC_FILES := \
	chicken.c \

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := chicken

LOCAL_STATIC_LIBRARIES := chicken-common

include $(BUILD_EXECUTABLE)
