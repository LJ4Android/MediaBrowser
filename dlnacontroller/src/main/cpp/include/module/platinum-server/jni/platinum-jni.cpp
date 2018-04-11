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
#include "PltMediaServer.h"

#include <android/log.h>

/*----------------------------------------------------------------------
|   logging
+---------------------------------------------------------------------*/
NPT_SET_LOCAL_LOGGER("platinum.android.jni")

static int exit = 0; 

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
 * Class:     com_github_mediaserver_server_jni_DMSJniInterface
 * Method:    startServer
 * Signature: (Ljava/lang/string;Ljava/lang/string;Ljava/lang/string;)I
 */
PLT_UPnP* upnp = NULL;
JNIEXPORT jint JNICALL Java_com_github_mediaserver_server_jni_DMSJniInterface_startServer(JNIEnv *env, jobject thiz, jstring dir, jstring name, jstring uuid)
{
    NPT_LOG_INFO("start MediaServer");

    char server_dir[256];
    int len = env->GetStringLength(dir);
    env->GetStringUTFRegion(dir, 0, len, server_dir);

    char server_name[256];
    len = env->GetStringLength(name);
    env->GetStringUTFRegion(name, 0, len, server_name);

    char server_uuid[256];
    len = env->GetStringLength(uuid);
    env->GetStringUTFRegion(uuid, 0, len, server_uuid);

    NPT_LOG_INFO_1("dir: %s", server_dir);
    NPT_LOG_INFO_1("name: %s", server_name);
    NPT_LOG_INFO_1("uuid: %s", server_uuid);

    // Create upnp engine
    upnp = new PLT_UPnP();

    // Create media server
    PLT_DeviceHostReference device(
        new PLT_FileMediaServer(
            server_dir, 
            server_name,
            false,
            server_uuid, // NULL for random ID
            0)
            );

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

    exit = 0;

    return upnp->Start();
}

/*
 * Class:     com_github_mediaserver_server_jni_DMSJniInterface
 * Method:    stopServer
 * Signature: (V)I
 */
JNIEXPORT jint JNICALL Java_com_github_mediaserver_server_jni_DMSJniInterface_stopServer(JNIEnv *env, jobject thiz)
{
    NPT_LOG_INFO("stop MediaServer");

    exit = 1;

    if(upnp == NULL) {
        NPT_LOG_INFO("stop MediaServer: upnp is NULL");
        return 0;
    }
    
    upnp->Stop();

    return 0;
}

