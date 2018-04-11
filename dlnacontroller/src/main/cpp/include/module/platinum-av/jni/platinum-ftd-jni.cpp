/*
** Copyright 2016, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/


/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include <assert.h>
#include <jni.h>
#include <string.h>
#include <sys/types.h>

#include "platinum-ftd-jni.h"
#include "Platinum.h"
#include "PltFileTransmitDevice.h"
#include "PltMicroFileTransmitDevice.h"

#include <android/log.h>

/*----------------------------------------------------------------------
|   logging
+---------------------------------------------------------------------*/
NPT_SET_LOCAL_LOGGER("platinum.android.jni")

struct fields_t {
    JavaVM*     pVM;
    jmethodID   onActionReflectionMethodID;
};
static fields_t fields;

static const char* const kClassFTDJniInterface = "com/qiku/android/avplayer/dlna/ftd/FTDReflection";
static int exit = 0; 

static int checkAndClearExceptionFromCallback(JNIEnv* env, const char* methodName) {
    if (env->ExceptionCheck()) {
        NPT_LOG_INFO_1("An exception was thrown by callback '%s'.", methodName);
        //LOGE_EX(env);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    return 0;
}

class DlnaActionCallback : public ActionCallback
{
private:
    JNIEnv *mEnv;
    jobject mClient;

public:
    DlnaActionCallback(JNIEnv *env, jobject client)
        :   mEnv(env),
            mClient(env->NewGlobalRef(client))
    {
        NPT_LOG_INFO("constructor");
    }

    virtual ~DlnaActionCallback()
    {
        NPT_LOG_INFO("destructor");
        mEnv->DeleteGlobalRef(mClient);
    }

    virtual int onActionReflection(int cmd, char *value, char *from, char *data1, char *data2)
    {
        NPT_LOG_INFO_5("cmd(0x%x), sid(%s), from(%s), data1(%s), data2(%s)", cmd, value,from, data1, data2);

        JNIEnv *env;
        int status;
        int ret = -1;

        status = fields.pVM->AttachCurrentThread(&env, NULL);
        if(status < 0) {
            NPT_LOG_INFO_1("Failed to attach current thread: %d", status);
        }

        jstring valueStr = NULL;
        if(value != NULL) {
            if ((valueStr = env->NewStringUTF(value)) == NULL) {
                env->ExceptionClear();
                return -1;
            }
        }

	    jstring fromStr = NULL;
        if(from != NULL) {
            if ((fromStr = env->NewStringUTF(from)) == NULL) {
                env->ExceptionClear();
                return -1;
            }
        }

        jstring data1Str = NULL;
        if(data1 != NULL) {
            if ((data1Str = env->NewStringUTF(data1)) == NULL) {
                env->ExceptionClear();
                return -1;
            }
        }

        jstring data2Str = NULL;
        if(data1 != NULL) {
            if ((data2Str = env->NewStringUTF(data2)) == NULL) {
                env->ExceptionClear();
                return -1;
            }
        }

        ret = env->CallStaticIntMethod((jclass)mClient, fields.onActionReflectionMethodID, cmd, valueStr, fromStr, data1Str, data2Str);

        if(value != NULL)
            env->DeleteLocalRef(valueStr);
	    if(from != NULL)
            env->DeleteLocalRef(fromStr);
        if(data1 != NULL)
            env->DeleteLocalRef(data1Str);
        if(data2 != NULL)
            env->DeleteLocalRef(data2Str);
        checkAndClearExceptionFromCallback(env, "onActionReflection");

        // Detach the current thread.
        fields.pVM->DetachCurrentThread();

        return ret;
    }

};

/*----------------------------------------------------------------------
|   functions
+---------------------------------------------------------------------*/
__attribute__((constructor)) static void onDlOpen(void)
{
}

/*
 * Class:     com_qiku_dlna_ftd_jni_PlatinumJniProxy
 * Method:    startFileTransmitDevice
 * Signature: (Ljava/lang/string;Ljava/lang/string;)I
 */
static PLT_UPnP* upnp = NULL;
static PLT_MicroFileTransmitDevice*self = NULL;
JNIEXPORT jint JNICALL Java_com_qiku_android_avplayer_dlna_ftd_FTDJniInterface_startFileTransmitDevice(JNIEnv *env, jobject thiz, jstring name, jstring uuid)
{
    NPT_LOG_INFO("start file transmit control device");

    char ftd_name[256];
    memset(ftd_name, 0, 256);
    int len = env->GetStringLength(name);
    env->GetStringUTFRegion(name, 0, len, ftd_name);

    char ftd_uuid[256];
    memset(ftd_uuid, 0, 256);
    len = env->GetStringLength(uuid);
    env->GetStringUTFRegion(uuid, 0, len, ftd_uuid);

    // Create upnp engine
    upnp = new PLT_UPnP();

    // Create File Transmint Device
    PLT_FileTransmitDevice *ftdevice = new PLT_FileTransmitDevice(ftd_name, false, ftd_uuid);

    // Create device
    PLT_DeviceHostReference device(ftdevice);

    // Callback
    DlnaActionCallback *callback = NULL;

    // Set the virtual machine.
    env->GetJavaVM(&(fields.pVM));

    // Get Method ID
    jclass ftdJniInterface = env->FindClass(kClassFTDJniInterface);
    if (ftdJniInterface == NULL) {
        NPT_LOG_INFO_1("Class %s not found", kClassFTDJniInterface);
    } else {
        callback = new DlnaActionCallback(env, ftdJniInterface);
        fields.onActionReflectionMethodID = env->GetStaticMethodID(
                                ftdJniInterface,
                                "onActionReflection",
                                "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
        if(fields.onActionReflectionMethodID == NULL)
            NPT_LOG_INFO_1("Class %s Method onActionReflection not found", kClassFTDJniInterface);
    }

    // Create controller
    self = new PLT_MicroFileTransmitDevice(ftdevice, callback);

    // add control point to upnp engine and start it
    upnp->AddDevice(device);

    exit = 0;

    return upnp->Start();
}


/*
 * Class:     com_qiku_dlna_ftd_jni_PlatinumJniProxy
 * Method:    responseGenaEvent
 * Signature: (ILjava/lang/string;Ljava/lang/string;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_qiku_android_avplayer_dlna_ftd_FTDJniInterface_responseGenaEvent(JNIEnv *env, jobject thiz, jint cmd, jstring value, jstring data)
{
    NPT_LOG_INFO("response gena event");

    char str_value[256];
    memset(str_value, 0, 256);
    int len = env->GetStringLength(value);
    env->GetStringUTFRegion(value, 0, len, str_value);

    return self->ResponseGenaEvent(cmd, str_value, NULL);
}

/*
 * Class:     com_qiku_dlna_ftd_jni_PlatinumJniProxy
 * Method:    stopFileTransmitDevice
 * Signature: (V)I
 */
JNIEXPORT jint JNICALL Java_com_qiku_android_avplayer_dlna_ftd_FTDJniInterface_stopFileTransmitDevice(JNIEnv *env, jobject thiz)
{
    NPT_LOG_INFO("stop FileTransmitDevice");

    exit = 1;

    if(upnp == NULL) {
        NPT_LOG_INFO("stop stopFileTransmitDevice: upnp is NULL");
        return 0;
    }
    
    upnp->Stop();

    if(self != NULL) {
        delete self;
        self = NULL;
    }

    return 0;
}

