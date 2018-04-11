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

#include "platinum-cp-jni.h"
#include "Platinum.h"
#include "PltMicroMediaController.h"

#include <android/log.h>

/*----------------------------------------------------------------------
|   logging
+---------------------------------------------------------------------*/
NPT_SET_LOCAL_LOGGER("platinum.android.jni")

struct fields_t {
    JavaVM*     pVM;
    jmethodID   onDlnaDeviceAddedMethodID;
    jmethodID   onDlnaDeviceRemovedMethodID;
    jmethodID   onDlnaGetPositionInfoMethodID;
    jmethodID   onDlnaGetTransportInfoMethodID;
    jmethodID   onDlnaGetVolumeInfoMethodID;
    jmethodID   onDlnaGetMuteInfoMethodID;
    jmethodID   onDlnaMediaControlMethodID;
    jmethodID   onDlnaDmrStatusChangedMethodID;
	/* qiku add */
    /* add volumedb interface, linhuaji, 2014.07.01 */
    jmethodID   onDlnaGetVolumeDBRangeInfoMethodID;
    jmethodID   onDlnaGetVolumeDBInfoMethodID;
	jmethodID  onRapidTransmitVideoMethodID;
	/* qiku end */
};
static fields_t fields;

static const char* const kClassDMCJniInterface = "com/qiku/dlna/dmc/jni/DMCJniInterface";
static const char* const kClassDeviceInfo = "com/qiku/dlna/common/DeviceInfo";
static const char* const kClassItemInfo = "com/qiku/dlna/dmc/ItemInfo";
static const char* const kClassFileInfo = "com/qiku/dlna/dmc/FileInfo";
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

class DlnaDeviceCallback : public DeviceCallback
{
private:
    JNIEnv *mEnv;
    jobject mClient;

public:
    DlnaDeviceCallback(JNIEnv *env, jobject client)
        :   mEnv(env),
            mClient(env->NewGlobalRef(client))
    {
        NPT_LOG_INFO("constructor");
    }

    virtual ~DlnaDeviceCallback()
    {
        NPT_LOG_INFO("destructor");
        mEnv->DeleteGlobalRef(mClient);
    }

    virtual int onDlnaDeviceAdded(char* name, char *uuid, int type)
    {
        NPT_LOG_INFO_3("name(%s), uuid(%s), id(%ld)", name, uuid, NPT_Thread::GetCurrentThreadId());

        if(fields.onDlnaDeviceAddedMethodID == NULL) {
            NPT_LOG_INFO("onDlnaDeviceAdded Method is null");
            return -1;
        }

        JNIEnv *env;
        fields.pVM->AttachCurrentThread(&env, NULL);

        jstring nameStr;
        if ((nameStr = env->NewStringUTF(name)) == NULL) {
            env->ExceptionClear();
            return -1;
        }

        jstring uuidStr;
        if ((uuidStr = env->NewStringUTF(uuid)) == NULL) {
            env->ExceptionClear();
            return -1;
        }

        env->CallVoidMethod(mClient, fields.onDlnaDeviceAddedMethodID, nameStr, uuidStr, type);

        env->DeleteLocalRef(nameStr);
        env->DeleteLocalRef(uuidStr);
        checkAndClearExceptionFromCallback(env, "onDlnaDeviceAdded");

        // Detach the current thread.
        fields.pVM->DetachCurrentThread();

        return 0;
    }

    virtual int onDlnaDeviceRemoved(char* name, char *uuid, int type)
    {
        NPT_LOG_INFO_3("name(%s), uuid(%s), id(%ld)", name, uuid, NPT_Thread::GetCurrentThreadId());

        if(exit == 1) {
            NPT_LOG_INFO("exit");
            return 0;
        }

        if(fields.onDlnaDeviceRemovedMethodID == NULL) {
            NPT_LOG_INFO("onDlnaDeviceRemoved Method is null");
            return -1;
        }

        JNIEnv *env;
        fields.pVM->AttachCurrentThread(&env, NULL);

        jstring nameStr;
        if ((nameStr = env->NewStringUTF(name)) == NULL) {
            env->ExceptionClear();
            return -1;
        }

        jstring uuidStr;
        if ((uuidStr = env->NewStringUTF(uuid)) == NULL) {
            env->ExceptionClear();
            return -1;
        }

        env->CallVoidMethod(mClient, fields.onDlnaDeviceRemovedMethodID, nameStr, uuidStr, type);

        env->DeleteLocalRef(nameStr);
        env->DeleteLocalRef(uuidStr);
        checkAndClearExceptionFromCallback(env, "onDlnaDeviceRemoved");

        // Detach the current thread.
        fields.pVM->DetachCurrentThread();

        return 0;
    }

    virtual int onDlnaGetPositionInfo(int res, long duration, long relTime)
    {
        NPT_LOG_INFO_4("res(%d), duration(%ld), relTime(%ld), id(%ld)", res, duration, relTime, NPT_Thread::GetCurrentThreadId());

        if(fields.onDlnaGetPositionInfoMethodID == NULL) {
            NPT_LOG_INFO("onDlnaGetPositionInfo Method is null");
            return -1;
        }

        JNIEnv *env;
        fields.pVM->AttachCurrentThread(&env, NULL);

        env->CallVoidMethod(mClient, fields.onDlnaGetPositionInfoMethodID, (jint)res, (jlong)duration, (jlong)relTime);

        checkAndClearExceptionFromCallback(env, "onDlnaGetPositionInfo");

        // Detach the current thread.
        fields.pVM->DetachCurrentThread();

        return 0;
    }

    virtual int onDlnaGetTransportInfo(int res, char* state)
    {
        NPT_LOG_INFO_2("res(%d), state(%s)", res, state);

        if(fields.onDlnaGetTransportInfoMethodID == NULL) {
            NPT_LOG_INFO("onDlnaGetTransportInfo Method is null");
            return -1;
        }

        JNIEnv *env;
        fields.pVM->AttachCurrentThread(&env, NULL);

        jstring stateStr;
        if ((stateStr = env->NewStringUTF(state)) == NULL) {
            env->ExceptionClear();
            return -1;
        }

        env->CallVoidMethod(mClient, fields.onDlnaGetTransportInfoMethodID, (jint)res, stateStr);

        env->DeleteLocalRef(stateStr);
        checkAndClearExceptionFromCallback(env, "onDlnaGetTransportInfo");

        // Detach the current thread.
        fields.pVM->DetachCurrentThread();

        return 0;
    }
	
	/* qiku add */
    /* add volumedb interface, linhuaji, 2014.07.01 */
    virtual int onDlnaGetVolumeDBRangeInfo(int res, long min_volume, long max_volume)
    {
        NPT_LOG_INFO_3("res(%d), min_volume(%ld), max_volume(%ld)", res, min_volume, max_volume);

        if(fields.onDlnaGetVolumeDBRangeInfoMethodID == NULL) {
            NPT_LOG_INFO("onDlnaGetVolumeDBRangeInfo Method is null");
            return -1;
        }

        JNIEnv *env;
        fields.pVM->AttachCurrentThread(&env, NULL);

        env->CallVoidMethod(mClient, fields.onDlnaGetVolumeDBRangeInfoMethodID, (jint)res, (jlong)min_volume, (jlong)max_volume);

        checkAndClearExceptionFromCallback(env, "onDlnaGetVolumeDBRangeInfo");

        // Detach the current thread.
        fields.pVM->DetachCurrentThread();

        return 0;
    }
	
    virtual int onDlnaGetVolumeDBInfo(int res, long volumedb)
    {
        NPT_LOG_INFO_2("res(%d), volumedb(%ld)", res, volumedb);

        if(fields.onDlnaGetVolumeDBInfoMethodID == NULL) {
            NPT_LOG_INFO("onDlnaGetVolumeDBInfo Method is null");
            return -1;
        }

        JNIEnv *env;
        fields.pVM->AttachCurrentThread(&env, NULL);

        env->CallVoidMethod(mClient, fields.onDlnaGetVolumeDBInfoMethodID, (jint)res, (jlong)volumedb);

        checkAndClearExceptionFromCallback(env, "onDlnaGetVolumeDBInfo");

        // Detach the current thread.
        fields.pVM->DetachCurrentThread();

        return 0;
    }
	/* qiku end */

    virtual int onDlnaGetVolumeInfo(int res, long volume)
    {
        NPT_LOG_INFO_2("res(%d), volume(%ld)", res, volume);

        if(fields.onDlnaGetVolumeInfoMethodID == NULL) {
            NPT_LOG_INFO("onDlnaGetVolumeInfo Method is null");
            return -1;
        }

        JNIEnv *env;
        fields.pVM->AttachCurrentThread(&env, NULL);

        env->CallVoidMethod(mClient, fields.onDlnaGetVolumeInfoMethodID, (jint)res, (jlong)volume);

        checkAndClearExceptionFromCallback(env, "onDlnaGetVolumeInfo");

        // Detach the current thread.
        fields.pVM->DetachCurrentThread();

        return 0;
    }

    virtual int onDlnaGetMuteInfo(int res, bool mute)
    {
        NPT_LOG_INFO_2("res(%d), mute(%d)", res, mute);

        if(fields.onDlnaGetMuteInfoMethodID == NULL) {
            NPT_LOG_INFO("onDlnaGetMuteInfo Method is null");
            return -1;
        }

        JNIEnv *env;
        fields.pVM->AttachCurrentThread(&env, NULL);

        env->CallVoidMethod(mClient, fields.onDlnaGetMuteInfoMethodID, (jint)res, (jboolean)mute);

        checkAndClearExceptionFromCallback(env, "onDlnaGetMuteInfo");

        // Detach the current thread.
        fields.pVM->DetachCurrentThread();

        return 0;
    }

    virtual int onDlnaMediaControl(int res, int cmd)
    {
        NPT_LOG_INFO_1("res(%d)", res);

        if(fields.onDlnaMediaControlMethodID == NULL) {
            NPT_LOG_INFO("onDlnaMediaControl Method is null");
            return -1;
        }

        JNIEnv *env;
        fields.pVM->AttachCurrentThread(&env, NULL);

        env->CallVoidMethod(mClient, fields.onDlnaMediaControlMethodID, (jint)res, (jint)cmd);

        checkAndClearExceptionFromCallback(env, "onDlnaMediaControl");

        // Detach the current thread.
        fields.pVM->DetachCurrentThread();

        return 0;
    }

    virtual int onDlnaDmrStatusChanged(char* name, char *value)
    {
        NPT_LOG_INFO_2("name(%s), value(%s)", name, value);

        if(fields.onDlnaDmrStatusChangedMethodID == NULL) {
            NPT_LOG_INFO("onDlnaDmrStatusChanged Method is null");
            return -1;
        }

        JNIEnv *env;
        fields.pVM->AttachCurrentThread(&env, NULL);

        jstring nameStr;
        if ((nameStr = env->NewStringUTF(name)) == NULL) {
            env->ExceptionClear();
            return -1;
        }

        jstring valueStr;
        if ((valueStr = env->NewStringUTF(value)) == NULL) {
            env->ExceptionClear();
            return -1;
        }

        env->CallVoidMethod(mClient, fields.onDlnaDmrStatusChangedMethodID, nameStr, valueStr);

        env->DeleteLocalRef(nameStr);
        env->DeleteLocalRef(valueStr);
        checkAndClearExceptionFromCallback(env, "onDlnaDmrStatusChanged");

        // Detach the current thread.
        fields.pVM->DetachCurrentThread();

        return 0;
    }

    virtual int onRapidTransmitVideo(const char* filename)
    {
        NPT_LOG_INFO_1("filename(%s)", filename);

        if(fields.onRapidTransmitVideoMethodID == NULL) {
            NPT_LOG_INFO("onRapidTransmitVideo Method is null");
            return -1;
        }

        JNIEnv *env;
        fields.pVM->AttachCurrentThread(&env, NULL);

        jstring filenameStr;
        if ((filenameStr = env->NewStringUTF(filename)) == NULL) {
            env->ExceptionClear();
            return -1;
        } 

        env->CallVoidMethod(mClient, fields.onRapidTransmitVideoMethodID, filenameStr);

        env->DeleteLocalRef(filenameStr); 
        checkAndClearExceptionFromCallback(env, "onRapidTransmitVideoMethodID");

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

    JNIEnv* env = NULL;
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        NPT_LOG_INFO("ERROR: GetEnv failed\n");
        return -1;
    }

    // Get Method ID
    jclass dmcJniInterface = env->FindClass(kClassDMCJniInterface);
    if (dmcJniInterface == NULL) {
        NPT_LOG_INFO_1("Class %s not found", kClassDMCJniInterface);
        checkAndClearExceptionFromCallback(env, "FindClass"); 
    } else {
        // Set the virtual machine.
        env->GetJavaVM(&(fields.pVM));

        fields.onDlnaDeviceAddedMethodID = env->GetMethodID(dmcJniInterface,
                                "onDlnaDeviceAdded", "(Ljava/lang/String;Ljava/lang/String;I)V");
        if(fields.onDlnaDeviceAddedMethodID == NULL) {
            NPT_LOG_INFO("Method onDlnaDeviceAdded not found");
            checkAndClearExceptionFromCallback(env, "GetMethodID"); 
        }

        fields.onDlnaDeviceRemovedMethodID = env->GetMethodID(dmcJniInterface,
                                "onDlnaDeviceRemoved", "(Ljava/lang/String;Ljava/lang/String;I)V");
        if(fields.onDlnaDeviceRemovedMethodID == NULL) {
            NPT_LOG_INFO("Method onDlnaDeviceRemoved not found");
            checkAndClearExceptionFromCallback(env, "GetMethodID"); 
        }

        fields.onDlnaGetPositionInfoMethodID = env->GetMethodID(dmcJniInterface, "onDlnaGetPositionInfo", "(IJJ)V");
        if(fields.onDlnaGetPositionInfoMethodID == NULL) {
            NPT_LOG_INFO("Method onDlnaGetPositionInfo not found");
            checkAndClearExceptionFromCallback(env, "GetMethodID"); 
        }

        fields.onDlnaGetTransportInfoMethodID = env->GetMethodID(dmcJniInterface, "onDlnaGetTransportInfo", "(ILjava/lang/String;)V");
        if(fields.onDlnaGetTransportInfoMethodID == NULL) {
            NPT_LOG_INFO("Method onDlnaGetTransportInfo not found");
            checkAndClearExceptionFromCallback(env, "GetMethodID"); 
        }
		
        /* qiku add */
        /* add volumedb interface, linhuaji, 2014.07.01 */
		fields.onDlnaGetVolumeDBRangeInfoMethodID = env->GetMethodID(dmcJniInterface, "onDlnaGetVolumeDBRangeInfo", "(IJJ)V");
        if(fields.onDlnaGetVolumeDBRangeInfoMethodID == NULL) {
            NPT_LOG_INFO("Method onDlnaGetVolumeDBRangeInfoMethodID not found");
            checkAndClearExceptionFromCallback(env, "GetMethodID"); 
        }
		
		fields.onDlnaGetVolumeDBInfoMethodID = env->GetMethodID(dmcJniInterface, "onDlnaGetVolumeDBInfo", "(IJ)V");
        if(fields.onDlnaGetVolumeDBInfoMethodID == NULL) {
            NPT_LOG_INFO("Method onDlnaGetVolumeDBInfoMethodID not found");
            checkAndClearExceptionFromCallback(env, "GetMethodID"); 
        }
		/* qiku end */

        fields.onDlnaGetVolumeInfoMethodID = env->GetMethodID(dmcJniInterface, "onDlnaGetVolumeInfo", "(IJ)V");
        if(fields.onDlnaGetVolumeInfoMethodID == NULL) {
            NPT_LOG_INFO("Method onDlnaGetVolumeInfo not found");
            checkAndClearExceptionFromCallback(env, "GetMethodID"); 
        }	

        fields.onDlnaGetMuteInfoMethodID = env->GetMethodID(dmcJniInterface, "onDlnaGetMuteInfo", "(IZ)V");
        if(fields.onDlnaGetMuteInfoMethodID == NULL) {
            NPT_LOG_INFO("Method onDlnaGetMuteInfo not found");
            checkAndClearExceptionFromCallback(env, "GetMethodID"); 
        }

        fields.onDlnaMediaControlMethodID = env->GetMethodID(dmcJniInterface, "onDlnaMediaControl", "(II)V");
        if(fields.onDlnaMediaControlMethodID == NULL) {
            NPT_LOG_INFO("Method onDlnaMediaControl not found");
            checkAndClearExceptionFromCallback(env, "GetMethodID"); 
        }

        fields.onDlnaDmrStatusChangedMethodID = env->GetMethodID(dmcJniInterface,
                                "onDlnaDmrStatusChanged", "(Ljava/lang/String;Ljava/lang/String;)V");
        if(fields.onDlnaDmrStatusChangedMethodID == NULL) {
            NPT_LOG_INFO("Method onDlnaDmrStatusChanged not found");
            checkAndClearExceptionFromCallback(env, "GetMethodID"); 
        }

	    fields.onRapidTransmitVideoMethodID = env->GetMethodID(dmcJniInterface,
                                "onRapidTransmitVideo", "(Ljava/lang/String;)V");
        if(fields.onRapidTransmitVideoMethodID == NULL) {
            NPT_LOG_INFO("Method onRapidTransmitVideo not found");
            checkAndClearExceptionFromCallback(env, "GetMethodID"); 
        }	

        env->DeleteLocalRef(dmcJniInterface);
    }

    return JNI_VERSION_1_4;
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    init
 * Signature: ()J
 */
PLT_UPnP* upnp = NULL;
JNIEXPORT jlong JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_init(JNIEnv *env, jobject thiz)
{
    NPT_LOG_INFO("init");

    // Create upnp engine
    //PLT_UPnP* upnp = new PLT_UPnP();
    upnp = new PLT_UPnP();

    // Create control point
    PLT_CtrlPointReference ctrlPoint(new PLT_CtrlPoint());

    // Callback
    DlnaDeviceCallback *callback = new DlnaDeviceCallback(env, thiz);

    // Create controller
    PLT_MicroMediaController *self = new PLT_MicroMediaController(ctrlPoint, callback);

    // add control point to upnp engine and start it
    upnp->AddCtrlPoint(ctrlPoint);
    upnp->Start();

    exit = 0;

    return (jlong)self;
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    scanDevice
 * Signature: (JLjava/lang/string;)I
 */
JNIEXPORT void JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_scanDevice(JNIEnv *env, jobject thiz, jlong _self, jlong mx)
{
    NPT_LOG_INFO("scanDevice");

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    self->ScanDevice((long)mx);
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    getDmsList
 * Signature: (J)[Ljava/lang/object;
 */
JNIEXPORT jobjectArray JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_getDmsList(JNIEnv *env, jobject thiz, jlong _self)
{
    NPT_LOG_INFO("getDmsList");

    jclass objectClass = env->FindClass(kClassDeviceInfo);
    if (objectClass == NULL) {
        NPT_LOG_INFO_1("Class %s not found", kClassDeviceInfo);
        checkAndClearExceptionFromCallback(env, "FindClass"); 
        return NULL;
    }

    jfieldID fname = env->GetFieldID(objectClass, "mName", "Ljava/lang/String;");
    if (fname == NULL) {
        NPT_LOG_INFO("Field DeviceInfo.mName not found");
        checkAndClearExceptionFromCallback(env, "GetFieldID"); 
        return NULL;
    }

    jfieldID fuuid = env->GetFieldID(objectClass, "mUuid", "Ljava/lang/String;");
    if (fuuid == NULL) {
        NPT_LOG_INFO("Field DeviceInfo.mUuid not found");
        checkAndClearExceptionFromCallback(env, "GetFieldID"); 
        return NULL;
    }

    DeviceInfo list[10];

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;
    int num = self->GetDmsList(list);
    if(num == 0)
        return NULL;

    jobjectArray jlist = env->NewObjectArray(num, objectClass, 0);

    for(int i = 0; i < num; i++) {
        jobject obj = env->AllocObject(objectClass);

        jstring jname = env->NewStringUTF(list[i].name);
        env->SetObjectField(obj, fname, jname);

        jstring juuid = env->NewStringUTF(list[i].uuid);
        env->SetObjectField(obj, fuuid, juuid);

        env->SetObjectArrayElement(jlist, i, obj);

        env->DeleteLocalRef(jname);
        jname = NULL;

        env->DeleteLocalRef(juuid);
        juuid = NULL;

        env->DeleteLocalRef(obj);
        obj = NULL;
    }
    
    env->DeleteLocalRef(objectClass);

    return jlist;
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    getDmrList
 * Signature: (J)[Ljava/lang/object;
 */
JNIEXPORT jobjectArray JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_getDmrList(JNIEnv *env, jobject thiz, jlong _self)
{
    NPT_LOG_INFO("getDmrList");
    
    jclass objectClass = env->FindClass(kClassDeviceInfo);
    if (objectClass == NULL) {
        NPT_LOG_INFO_1("Class %s not found", kClassDeviceInfo);
        checkAndClearExceptionFromCallback(env, "FindClass"); 
        return NULL;
    }

    jfieldID fname = env->GetFieldID(objectClass, "mName", "Ljava/lang/String;");
    if (fname == NULL) {
        NPT_LOG_INFO("Field DeviceInfo.mName not found");
        checkAndClearExceptionFromCallback(env, "GetFieldID"); 
        return NULL;
    }

    jfieldID fuuid = env->GetFieldID(objectClass, "mUuid", "Ljava/lang/String;");
    if (fuuid == NULL) {
        NPT_LOG_INFO("Field DeviceInfo.mUuid not found");
        checkAndClearExceptionFromCallback(env, "GetFieldID"); 
        return NULL;
    }

    DeviceInfo list[10];

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;
    int num = self->GetDmrList(list);
    if(num == 0)
        return NULL;

    jobjectArray jlist = env->NewObjectArray(num, objectClass, 0);

    for(int i = 0; i < num; i++) {
        jobject obj = env->AllocObject(objectClass);

        jstring jname = env->NewStringUTF(list[i].name);
        env->SetObjectField(obj, fname, jname);

        jstring juuid = env->NewStringUTF(list[i].uuid);
        env->SetObjectField(obj, fuuid, juuid);

        env->SetObjectArrayElement(jlist, i, obj);

        env->DeleteLocalRef(jname);
        jname = NULL;

        env->DeleteLocalRef(juuid);
        juuid = NULL;

        env->DeleteLocalRef(obj);
        obj = NULL;
    }
    
    env->DeleteLocalRef(objectClass);

    NPT_LOG_INFO("getDmrList End");

    return jlist;
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    setDms
 * Signature: (JLjava/lang/string;)I
 */
JNIEXPORT jint JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_setDms(JNIEnv *env, jobject thiz, jlong _self, jstring dmsName)
{
    NPT_LOG_INFO("setDms");

    char dns_name[256];
    int len = env->GetStringLength(dmsName);
    env->GetStringUTFRegion(dmsName, 0, len, dns_name);

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    return self->SetDms(dns_name);
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    setDmr
 * Signature: (JLjava/lang/string;)I
 */
JNIEXPORT jint JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_setDmr(JNIEnv *env, jobject thiz, jlong _self, jstring dmrName)
{
    NPT_LOG_INFO("setDmr");

    char dmr_name[256];
    int len = env->GetStringLength(dmrName);
    env->GetStringUTFRegion(dmrName, 0, len, dmr_name);

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    return self->SetDmr(dmr_name);
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    entryDirectory
 * Signature: (JLjava/lang/string;)V
 */
JNIEXPORT void JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_entryDirectory(JNIEnv *env, jobject thiz, jlong _self, jstring dir)
{
    NPT_LOG_INFO("entryDirectory");

    char dir_name[256];
    int len = env->GetStringLength(dir);
    env->GetStringUTFRegion(dir, 0, len, dir_name);

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    self->EntryDirectory(dir_name);
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    getItemList
 * Signature: (J)[Ljava/lang/object;
 */
JNIEXPORT jobjectArray JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_getItemList(JNIEnv *env, jobject thiz, jlong _self)
{
    NPT_LOG_INFO("getItemList");

    jclass objectClass = env->FindClass(kClassItemInfo);
    if (objectClass == NULL) {
        NPT_LOG_INFO_1("Class %s not found", kClassItemInfo);
        checkAndClearExceptionFromCallback(env, "FindClass"); 
        return NULL;
    }

    jfieldID ftitle = env->GetFieldID(objectClass, "mTitle", "Ljava/lang/String;");
    if (ftitle == NULL) {
        NPT_LOG_INFO("Field ItemInfo.mTitle not found");
        checkAndClearExceptionFromCallback(env, "GetFieldID"); 
        return NULL;
    }

    jfieldID fpath = env->GetFieldID(objectClass, "mPath", "Ljava/lang/String;");
    if (fpath == NULL) {
        NPT_LOG_INFO("Field ItemInfo.mPath not found");
        checkAndClearExceptionFromCallback(env, "GetFieldID"); 
        return NULL;
    }

    jfieldID fupnpclass = env->GetFieldID(objectClass, "mUpnpClass", "Ljava/lang/String;");
    if (fupnpclass == NULL) {
        NPT_LOG_INFO("Field ItemInfo.mUpnpClass not found");
        checkAndClearExceptionFromCallback(env, "GetFieldID"); 
        return NULL;
    }

    jfieldID fsize = env->GetFieldID(objectClass, "mSize", "J");
    if (fsize == NULL) {
        NPT_LOG_INFO("Field ItemInfo.mSize not found");
        checkAndClearExceptionFromCallback(env, "GetFieldID"); 
        return NULL;
    }

    jfieldID fduration = env->GetFieldID(objectClass, "mDuration", "I");
    if (fduration == NULL) {
        NPT_LOG_INFO("Field ItemInfo.mDuration not found");
        checkAndClearExceptionFromCallback(env, "GetFieldID"); 
        return NULL;
    }

    jfieldID ftype = env->GetFieldID(objectClass, "mFileType", "I");
    if (ftype == NULL) {
        NPT_LOG_INFO("Field ItemInfo.mFileType not found");
        checkAndClearExceptionFromCallback(env, "GetFieldID"); 
        return NULL;
    }

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    int num = self->GetItemCount();
    ItemInfo *list = (ItemInfo *)malloc(num * sizeof(ItemInfo));
    if(list == NULL)
        return NULL;

    num = self->GetItemList(list);

    jobjectArray jlist = env->NewObjectArray(num, objectClass, 0);

    for(int i = 0; i < num; i++) {
        jobject obj = env->AllocObject(objectClass);

        jstring jtitle = env->NewStringUTF(list[i].title);
        env->SetObjectField(obj, ftitle, jtitle);

        jstring jpath = env->NewStringUTF(list[i].path);
        env->SetObjectField(obj, fpath, jpath);

        jstring jupnpclass = env->NewStringUTF(list[i].upnpclass);
        env->SetObjectField(obj, fupnpclass, jupnpclass);

        env->SetLongField(obj, fsize, list[i].size);
        env->SetIntField(obj, fduration, list[i].duration);
        env->SetIntField(obj, ftype, list[i].filetype);

        env->SetObjectArrayElement(jlist, i, obj);

        env->DeleteLocalRef(jtitle);
        jtitle = NULL;

        env->DeleteLocalRef(jpath);
        jpath = NULL;

        env->DeleteLocalRef(jupnpclass);
        jupnpclass = NULL;

        env->DeleteLocalRef(obj);
        obj = NULL;
    }

    if(list != NULL)
        free(list);
    
    env->DeleteLocalRef(objectClass);

    return jlist;
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    getFileList
 * Signature: (JI)[Ljava/lang/object;
 */
JNIEXPORT jobjectArray JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_getFileList(JNIEnv *env, jobject thiz, jlong _self, jint flags)
{
    NPT_LOG_INFO("getFileList");

    jclass objectClass = env->FindClass(kClassFileInfo);
    if (objectClass == NULL) {
        NPT_LOG_INFO_1("Class %s not found", kClassFileInfo);
        checkAndClearExceptionFromCallback(env, "FindClass"); 
        return NULL;
    }

    jfieldID fname = env->GetFieldID(objectClass, "mName", "Ljava/lang/String;");
    if (fname == NULL) {
        NPT_LOG_INFO("Field FileInfo.mName not found");
        checkAndClearExceptionFromCallback(env, "GetFieldID"); 
        return NULL;
    }

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    int num = self->GetFileItemCount(flags);
    FileInfo *list = (FileInfo *)malloc(num * sizeof(FileInfo));
    if(list == NULL)
        return NULL;

    num = self->GetFileList(flags, list);

    jobjectArray jlist = env->NewObjectArray(num, objectClass, 0);

    for(int i = 0; i < num; i++) {
        jobject obj = env->AllocObject(objectClass);

        jstring jstr = env->NewStringUTF(list[i].name);
        env->SetObjectField(obj, fname, jstr);

        env->SetObjectArrayElement(jlist, i, obj);

        env->DeleteLocalRef(jstr);
        jstr = NULL;

        env->DeleteLocalRef(obj);
        obj = NULL;
    }

    if(list != NULL)
        free(list);
    
    env->DeleteLocalRef(objectClass);

    return jlist;
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    getPositionInfo
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_getPositionInfo(JNIEnv *env, jobject thiz, jlong _self)
{
    NPT_LOG_INFO("getPositionInfo");

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    self->GetPosInfo();
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    getTransportInfo
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_getTransportInfo(JNIEnv *env, jobject thiz, jlong _self)
{
    NPT_LOG_INFO("getTransportInfo");

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    self->GetTransInfo();
}

/* qiku add */
/* add volumedb interface, linhuaji, 2014.07.01 */
/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    getVolumeDBRange
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_getVolumeDBRange(JNIEnv *env, jobject thiz, jlong _self)
{
    NPT_LOG_INFO("getVolumeDBRange");

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    self->GetVolumeDBRange();
}


/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    setVolumeDB
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_setVolumeDB(JNIEnv *env, jobject thiz, jlong _self, jint voldb)
{
    NPT_LOG_INFO("setVolumeDB");

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    self->SetVolumeDB(voldb);
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    getVolumeDB
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_getVolumeDB(JNIEnv *env, jobject thiz, jlong _self)
{
    NPT_LOG_INFO("getVolumeDB");

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    self->GetVolumeDB();
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    getVolume
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_getVolume(JNIEnv *env, jobject thiz, jlong _self)
{
    NPT_LOG_INFO("getVolume");

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    self->GetVol();
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    setVolume
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_setVolume(JNIEnv *env, jobject thiz, jlong _self, jint vol)
{
    NPT_LOG_INFO("setVolume");

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    self->SetVol(vol);
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    getMute
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_getMute(JNIEnv *env, jobject thiz, jlong _self)
{
    NPT_LOG_INFO("getMute");

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    self->GetMuteStatus();
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    getDownloadUri
 * Signature: (JLjava/lang/string;I)Ljava/lang/string;
 */
JNIEXPORT jstring JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_getDownloadUri(JNIEnv *env, jobject thiz, jlong _self, jstring uri, jint flags)
{
    NPT_LOG_INFO("getDownloadUri");

    char c_uri[1024];
    char dl_uri[1024];
    int len = env->GetStringLength(uri);
    env->GetStringUTFRegion(uri, 0, len, c_uri);

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    if(self->GetDownloadUri(c_uri, dl_uri, flags) == 0) {
        return env->NewStringUTF(dl_uri);
    }

    return NULL;
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    ctrlPlay2
 * Signature: (JLjava/lang/string;I)I
 */
JNIEXPORT jint JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_ctrlPlay2(JNIEnv *env, jobject thiz, jlong _self, jstring uri, jint flags)
{
    NPT_LOG_INFO("ctrlPlay2");

    char c_uri[1024];
    int len = env->GetStringLength(uri);
    env->GetStringUTFRegion(uri, 0, len, c_uri);

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    return self->CtrlPlay2(c_uri, flags);
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    ctrlPlay
 * Signature: (JLjava/lang/string;I)I
 */
JNIEXPORT jint JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_ctrlPlay(JNIEnv *env, jobject thiz, jlong _self, jstring uri, jint flags)
{
    NPT_LOG_INFO("ctrlPlay");

    char c_uri[1024];
    int len = env->GetStringLength(uri);
    env->GetStringUTFRegion(uri, 0, len, c_uri);

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    return self->CtrlPlay(c_uri, flags);
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    ctrlPlayOnline
 * Signature: (JLjava/lang/string;Ljava/lang/string;I)I
 */
JNIEXPORT jint JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_ctrlPlayOnline(JNIEnv *env, jobject thiz, jlong _self, jstring uri, jstring title, jint flags)
{
    NPT_LOG_INFO("ctrlPlayOnline");

    char c_uri[1024];
    int len = env->GetStringLength(uri);
    env->GetStringUTFRegion(uri, 0, len, c_uri);

    char c_title[1024];
    len = env->GetStringLength(title);
    env->GetStringUTFRegion(title, 0, len, c_title);

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    return self->CtrlPlayOnline(c_uri, c_title, flags);
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    ctrlSeek
 * Signature: (JLjava/lang/string;)I
 */
JNIEXPORT jint JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_ctrlSeek(JNIEnv *env, jobject thiz, jlong _self, jstring relTime)
{
    NPT_LOG_INFO("ctrlSeek");

    char c_relTime[64];
    int len = env->GetStringLength(relTime);
    env->GetStringUTFRegion(relTime, 0, len, c_relTime);

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    return self->CtrlSeek(c_relTime);
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    ctrlPause
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_ctrlPause(JNIEnv *env, jobject thiz, jlong _self)
{
    NPT_LOG_INFO("ctrlPause");

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    self->CtrlPause();
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    ctrlResume
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_ctrlResume(JNIEnv *env, jobject thiz, jlong _self)
{
    NPT_LOG_INFO("ctrlResume");

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    self->CtrlResume();
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    ctrlStop
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_ctrlStop(JNIEnv *env, jobject thiz, jlong _self)
{
    NPT_LOG_INFO("ctrlStop");

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    self->CtrlStop();
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    ctrlMute
 * Signature: (JZ)V
 */
JNIEXPORT void JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_ctrlMute(JNIEnv *env, jobject thiz, jlong _self, jboolean mute)
{
    NPT_LOG_INFO("ctrlMute");

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    self->CtrlMute(mute);
}

/*
 * Class:     com_qiku_dlna_dmc_jni_DMCJniInterface
 * Method:    uninit
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_com_qiku_dlna_dmc_jni_DMCJniInterface_uninit(JNIEnv *env, jobject thiz, jlong _self)
{
    NPT_LOG_INFO("uninit");

    PLT_MicroMediaController* self = (PLT_MicroMediaController *)_self;

    //PLT_UPnP* self = (PLT_UPnP*)_self;
    exit = 1;
    
    upnp->Stop();

    if(self != NULL) {
        delete self;
        self = NULL;
    }

    return 0;
}

