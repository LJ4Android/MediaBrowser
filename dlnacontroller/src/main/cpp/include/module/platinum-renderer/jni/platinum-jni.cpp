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

#include "platinum-jni.h"
#include "Platinum.h"
#include "PltMediaRenderer.h"
#include "PltMicroMediaRenderer.h"

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

static const char* const kClassDMRJniInterface = "com/qiku/dlna/dmr/jni/PlatinumReflection";
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

    virtual int onActionReflection(int cmd, char *value, char *data)
    {
        NPT_LOG_INFO_3("cmd(0x%x), value(%s), data(%s)", cmd, value, data);

        JNIEnv *env;
        int status;

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

        jstring dataStr = NULL;
        if(data != NULL) {
            if ((dataStr = env->NewStringUTF(data)) == NULL) {
                env->ExceptionClear();
                return -1;
            }
        }

        env->CallStaticVoidMethod((jclass)mClient, fields.onActionReflectionMethodID, cmd, valueStr, dataStr);

        if(value != NULL)
            env->DeleteLocalRef(valueStr);
        if(data != NULL)
            env->DeleteLocalRef(dataStr);
        checkAndClearExceptionFromCallback(env, "onActionReflection");

        // Detach the current thread.
        fields.pVM->DetachCurrentThread();

        return 0;
    }

};

/*----------------------------------------------------------------------
|   functions
+---------------------------------------------------------------------*/
__attribute__((constructor)) static void onDlOpen(void)
{
}

/*----------------------------------------------------------------------
|    JNI_OnLoad
+---------------------------------------------------------------------*/
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    NPT_LogManager::GetDefault().Configure("plist:.level=FINE;.handlers=ConsoleHandler;.ConsoleHandler.outputs=2;.ConsoleHandler.colors=false;.ConsoleHandler.filter=59");
    return JNI_VERSION_1_4;
}

/*
 * Class:     com_qiku_dlna_dmr_jni_PlatinumJniProxy
 * Method:    startMediaRender
 * Signature: (Ljava/lang/string;Ljava/lang/string;)I
 */
PLT_UPnP* upnp = NULL;
PLT_MicroMediaRenderer *self = NULL;
JNIEXPORT jint JNICALL Java_com_qiku_dlna_dmr_jni_PlatinumJniProxy_startMediaRender(JNIEnv *env, jobject thiz, jstring name, jstring uuid)
{
    NPT_LOG_INFO("start MediaRender");

    char render_name[256];
    int len = env->GetStringLength(name);
    env->GetStringUTFRegion(name, 0, len, render_name);

    char render_uuid[256];
    len = env->GetStringLength(uuid);
    env->GetStringUTFRegion(uuid, 0, len, render_uuid);

    // Create upnp engine
    upnp = new PLT_UPnP();

    // Create media render
    PLT_MediaRenderer *render = new PLT_MediaRenderer(render_name, false, render_uuid);

    // Create device
    PLT_DeviceHostReference device(render);

    // Callback
    DlnaActionCallback *callback = NULL;

    // Set the virtual machine.
    env->GetJavaVM(&(fields.pVM));

    // Get Method ID
    jclass dmrJniInterface = env->FindClass(kClassDMRJniInterface);
    if (dmrJniInterface == NULL) {
        NPT_LOG_INFO_1("Class %s not found", kClassDMRJniInterface);
    } else {
        callback = new DlnaActionCallback(env, dmrJniInterface);
        fields.onActionReflectionMethodID = env->GetStaticMethodID(
                                dmrJniInterface,
                                "onActionReflection",
                                "(ILjava/lang/String;Ljava/lang/String;)V");
        if(fields.onActionReflectionMethodID == NULL)
            NPT_LOG_INFO_1("Class %s Method onActionReflection not found", kClassDMRJniInterface);
    }

    // Create controller
    //PLT_MicroMediaRenderer *self = new PLT_MicroMediaRenderer(device, callback);
    self = new PLT_MicroMediaRenderer(render, callback);

    // add control point to upnp engine and start it
    upnp->AddDevice(device);

    exit = 0;

    return upnp->Start();
}


/*
 * Class:     com_qiku_dlna_dmr_jni_PlatinumJniProxy
 * Method:    responseGenaEvent
 * Signature: (ILjava/lang/string;Ljava/lang/string;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_qiku_dlna_dmr_jni_PlatinumJniProxy_responseGenaEvent(JNIEnv *env, jobject thiz, jint cmd, jstring value, jstring data)
{
    //NPT_LOG_INFO("response gena event");

    char str_value[256];
    int len = env->GetStringLength(value);
    env->GetStringUTFRegion(value, 0, len, str_value);

    return self->ResponseGenaEvent(cmd, str_value, NULL);
}

/*
 * Class:     com_qiku_dlna_dmr_jni_PlatinumJniProxy
 * Method:    stopMediaRender
 * Signature: (V)I
 */
JNIEXPORT jint JNICALL Java_com_qiku_dlna_dmr_jni_PlatinumJniProxy_stopMediaRender(JNIEnv *env, jobject thiz)
{
    NPT_LOG_INFO("stop MediaRender");

    exit = 1;

    if(upnp == NULL) {
        NPT_LOG_INFO("stop MediaRender: upnp is NULL");
        return 0;
    }
    
    upnp->Stop();

    if(self != NULL) {
        delete self;
        self = NULL;
    }

    return 0;
}

