# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := ble-lib
#LOCAL_SRC_FILES := $(LOCAL_PATH)/../jniLibs/x86/libble-lib.so
LOCAL_SRC_FILES := $(LOCAL_PATH)/../jniLibs/armeabi/libble-lib.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := native-lib
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../inc
LOCAL_SRC_FILES := \
    native-lib.cpp

LOCAL_LDLIBS += \
    -llog

LOCAL_SHARED_LIBRARIES := ble-lib

include $(BUILD_SHARED_LIBRARY)