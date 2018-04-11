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
#include <sys/stat.h>
#include <errno.h>


#include "platinum-ms-jni.h"
#include "Platinum.h"
#include "PltMediaServer.h"
#include "PltMicroMediaServer.h"


#include <android/log.h>

/*----------------------------------------------------------------------
|   logging
+---------------------------------------------------------------------*/
NPT_SET_LOCAL_LOGGER("platinum.android.jni")

static int exit = 0; 

struct fields_t {
    JavaVM*     pVM;
    jmethodID   onProgressChangeMethodID;
};
static fields_t fields;

static const char* const kClassDMSJniInterface = "com/qiku/android/avplayer/dlna/dms/DMSCallback";

static int checkAndClearExceptionFromCallback(JNIEnv* env, const char* methodName) {
    if (env->ExceptionCheck()) {
        NPT_LOG_INFO_1("An exception was thrown by callback '%s'.", methodName);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return -1;
    }
    return 0;
}

class DlnaDMSCallback : public FileMediaServerCallback
{
private:
    JNIEnv *mEnv;
    jobject mClient;

public:
    DlnaDMSCallback(JNIEnv *env, jobject client)
        :   mEnv(env),
            mClient(env->NewGlobalRef(client))
    {
        NPT_LOG_INFO("constructor");
    }

    virtual ~DlnaDMSCallback()
    {
        NPT_LOG_INFO("destructor");
        mEnv->DeleteGlobalRef(mClient);
    }

    virtual int onProgressChange(char *sid, int state, unsigned long long progress, unsigned long long size) 
    {
        NPT_LOG_INFO_4("sid(%s), state(%d), progress(%lld), size(%lld)", sid, state, progress, size);

        JNIEnv *env;
        int status;

        status = fields.pVM->AttachCurrentThread(&env, NULL);
        if(status < 0) {
            NPT_LOG_INFO_1("Failed to attach current thread: %d", status);
        }

        jstring sidStr = NULL;
        if(sid != NULL) {
            if ((sidStr = env->NewStringUTF(sid)) == NULL) {
                env->ExceptionClear();
                return -1;
            }
        }

        if(fields.onProgressChangeMethodID != NULL) {
            env->CallStaticVoidMethod((jclass)mClient, fields.onProgressChangeMethodID, sidStr, state, progress, size);
        } else {
            NPT_LOG_INFO("fields.onProgressChangeMethodID is NULL");
        }

        if(sid != NULL) {
            env->DeleteLocalRef(sidStr);
        }
        checkAndClearExceptionFromCallback(env, "onProgressChange");

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


/*
 * Class:     com_qiku_android_avplayer_dlna_dms_DMSJniInterface
 * Method:    startServer
 * Signature: (Ljava/lang/string;Ljava/lang/string;Ljava/lang/string;)I
 */
static PLT_UPnP* upnp = NULL;
PLT_MicroMediaServer *self = NULL;
JNIEXPORT jint JNICALL Java_com_qiku_android_avplayer_dlna_dms_DMSJniInterface_startServer(JNIEnv *env, jobject thiz, jstring dir, jstring name, jstring uuid)
{
    NPT_LOG_INFO("start MediaServer");
    int len;

    char server_dir[256];// = "/data/data/com.qiku.dlna.root/";
    memset(server_dir, 0, 256);
    len = env->GetStringLength(dir);
    env->GetStringUTFRegion(dir, 0, len, server_dir);
    //mkdir(server_dir, S_IRWXU);    
    //perror("mkdir");
    //NPT_LOG_INFO_1("mkdir error: %d", errno);

    char server_name[256];
    memset(server_name, 0, 256);
    len = env->GetStringLength(name);
    env->GetStringUTFRegion(name, 0, len, server_name);

    char server_uuid[256];
    memset(server_uuid, 0, 256);
    len = env->GetStringLength(uuid);
    env->GetStringUTFRegion(uuid, 0, len, server_uuid);

    NPT_LOG_INFO_1("dir: %s", server_dir);
    NPT_LOG_INFO_1("name: %s", server_name);
    NPT_LOG_INFO_1("uuid: %s", server_uuid);

    // Callback
    DlnaDMSCallback *callback = NULL;

    // Set the virtual machine.
    env->GetJavaVM(&(fields.pVM));

    // Get Method ID
    jclass dmsJniInterface = env->FindClass(kClassDMSJniInterface);
    if (dmsJniInterface == NULL) {
        NPT_LOG_INFO_1("Class %s not found", kClassDMSJniInterface);
    } else {
        callback = new DlnaDMSCallback(env, dmsJniInterface);
        fields.onProgressChangeMethodID = env->GetStaticMethodID(
                                dmsJniInterface,
                                "onProgressChange",
                                "(Ljava/lang/String;IJJ)V");
        if(fields.onProgressChangeMethodID == NULL)
            NPT_LOG_INFO_1("Class %s Method onProgressChange not found", kClassDMSJniInterface);
    }

    if(callback == NULL){
        NPT_LOG_INFO("callback not set!!!!!!");
    } else {
        NPT_LOG_INFO("callback setted");
    }

    // Create upnp engine
    upnp = new PLT_UPnP();

    // Create media server
    PLT_FileMediaServer *fms =  new PLT_FileMediaServer(
            server_dir, 
            server_name,
            false,
            server_uuid, // NULL for random ID
            0,
            false,
            callback);
	
    PLT_DeviceHostReference device(fms);

    //NPT_List<NPT_IpAddress> list;
    //NPT_CHECK_SEVERE(PLT_UPnPMessageHelper::GetIPAddresses(list));
    //NPT_String ip = list.GetFirstItem()->ToString();

    device->m_ModelDescription = "Platinum File Media Server";
    device->m_ModelURL = "http://www.plutinosoft.com/";
    device->m_ModelNumber = "1.0";
    device->m_ModelName = "Platinum File Media Server";
    device->m_Manufacturer = "Plutinosoft";
    device->m_ManufacturerURL = "http://www.plutinosoft.com/";

    upnp->AddDevice(device);
    //NPT_String uuid = device->GetUUID();

    self = new PLT_MicroMediaServer(fms);

    exit = 0;

    return upnp->Start();
}

/*
 * Class:     com_qiku_android_avplayer_dlna_dms_DMSJniInterface
 * Method:    stopServer
 * Signature: (V)I
 */
JNIEXPORT jint JNICALL Java_com_qiku_android_avplayer_dlna_dms_DMSJniInterface_stopServer(JNIEnv *env, jobject thiz)
{
    NPT_LOG_INFO("stop MediaServer");

    exit = 1;

    if(upnp == NULL) {
        NPT_LOG_INFO("stop MediaServer: upnp is NULL");
        return 0;
    }
    
    upnp->Stop();
    upnp = NULL;
	
    delete(self);
    self = NULL;

    return 0;
}

/*
 * Class:     com_qiku_android_avplayer_dlna_dms_DMSJniInterface
 * Method:    RapidTransmitVideo
 * Signature: (Ljava/lang/string)I
 */
JNIEXPORT jint JNICALL Java_com_qiku_android_avplayer_dlna_dms_DMSJniInterface_RapidTransmitVideo(JNIEnv *env, jobject thiz, jstring file)
{
    NPT_LOG_INFO("RapidTransmitVideo");

    char filename[256];
    memset(filename, 0, 256);
    int len = env->GetStringLength(file);
    env->GetStringUTFRegion(file, 0, len, filename);

    NPT_LOG_INFO_1("Rapid transmit video file: %s", filename);

    if(self != NULL)
        self->RapidTransmitVideo(filename);
    

    return 0;
}

/*
 * Class:     com_qiku_android_avplayer_dlna_dms_DMSJniInterface
 * Method:    getServerRootUri
 * Signature: ()Ljava/lang/string
 */
JNIEXPORT jstring JNICALL Java_com_qiku_android_avplayer_dlna_dms_DMSJniInterface_getServerRootUri(JNIEnv *env, jobject thiz)
{
    NPT_LOG_INFO("getServerRootUri");

    char rooturi[256];
    memset(rooturi, 0, 256);

    if(self != NULL) {
        if(self->getServerRootUri(rooturi) == NPT_SUCCESS){
            return env->NewStringUTF(rooturi);
        }
    }

    return NULL;
}



