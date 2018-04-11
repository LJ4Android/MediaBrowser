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
|   includes
+---------------------------------------------------------------------*/
#include "PltMicroFileTransmitDevice.h"
#include "PltLeaks.h"
#include "PltDownloader.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <android/log.h>

NPT_SET_LOCAL_LOGGER("platinum.tests.microfiletransmitdevice")

ActionCallback::ActionCallback()
    :   mTest(0)
{
}

ActionCallback::~ActionCallback()
{
}

/*----------------------------------------------------------------------
|   PLT_MicroFileTransmitDevice::PLT_MicroFileTransmitDevice
+---------------------------------------------------------------------*/
PLT_MicroFileTransmitDevice::PLT_MicroFileTransmitDevice(PLT_FileTransmitDevice *ftDevice, ActionCallback *callback)
{
    mFileTransmitDevice = ftDevice;
    mCallback = callback;

    mFileTransmitDevice->SetDelegate(this);
}

/*----------------------------------------------------------------------
|   PLT_MicroFileTransmitDevice::~PLT_MicroFileTransmitDevice
+---------------------------------------------------------------------*/
PLT_MicroFileTransmitDevice::~PLT_MicroFileTransmitDevice()
{
    NPT_LOG_INFO("~PLT_MicroFileTransmitDevice");
    if(mCallback != NULL) {
        delete mCallback;
        mCallback = NULL;
    }
}

bool PLT_MicroFileTransmitDevice::ResponseGenaEvent(int cmd, char *value, char *data)
{
    PLT_Service *service;

    if(cmd == FILE_TRANSMIT_DEVICE_TOCONTRPOINT_SET_PROGRESS ) {
        mFileTransmitDevice->FindServiceById("urn:upnp-org:serviceId:FileTransmitControl", service);
    }

    if(service != NULL) {
        switch(cmd) {
        case FILE_TRANSMIT_DEVICE_TOCONTRPOINT_SET_PROGRESS:
            NPT_LOG_INFO_1("FILE_TRANSMIT_DEVICE_TOCONTRPOINT_SET_PROGRESS: %s", value);
            //value like 30
            service->SetStateVariable("INT_VALUE_TransmitProgress", value);
            break;

        default:
            NPT_LOG_INFO_2("Unknown cmd: %d, value: %s", cmd, value);
            return false;
        }
        return true;
    } else {
        NPT_LOG_INFO("urn:upnp-org:serviceId:FileTransmitControl not found");
    }

    return false;
}

// AVTransport
NPT_Result PLT_MicroFileTransmitDevice::OnTransmitFile(PLT_ActionReference& action)
{
    // default implementation is using state variable
    NPT_String filename;
    NPT_CHECK_WARNING(action->GetArgumentValue("FileName", filename));

    NPT_String size;
    NPT_CHECK_WARNING(action->GetArgumentValue("FileSize", size));

    NPT_String id;
    NPT_CHECK_WARNING(action->GetArgumentValue("SID", id));

    NPT_String from;
    NPT_CHECK_WARNING(action->GetArgumentValue("From", from));
    NPT_LOG_INFO_4("OnTransmitFile filename:%s, filesize: %s, sid:%s, from:%s",(char*)filename, (char*)size, (char*)id, (char*)from);

    return mCallback->onActionReflection(FILE_TRANSMIT_DEVICE_CTL_MSG_TRANSMIT_FILE , (char *)id, (char*)from, (char *)filename, (char *)size);
}

NPT_Result PLT_MicroFileTransmitDevice::OnCancel(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnCancel");

    NPT_String id;
    NPT_CHECK_WARNING(action->GetArgumentValue("SID", id));
    return mCallback->onActionReflection(FILE_TRANSMIT_DEVICE_CTL_MSG_CANCEL, (char *)id, NULL,NULL);
}

NPT_Result PLT_MicroFileTransmitDevice::OnFinish(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnFinish");
	
    NPT_String id;
    NPT_CHECK_WARNING(action->GetArgumentValue("SID", id));
    return mCallback->onActionReflection(FILE_TRANSMIT_DEVICE_CTL_MSG_FINISH, (char *)id, NULL, NULL);
}

NPT_Result PLT_MicroFileTransmitDevice::OnGetProgress(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnGetProgress");
	
    NPT_String sid;
    NPT_CHECK_WARNING(action->GetArgumentValue("InSID", sid));
    NPT_Result result = mCallback->onActionReflection(FILE_TRANSMIT_DEVICE_CTL_MSG_GETPROGRESS, (char *)sid, NULL, NULL);
    if(result == NPT_SUCCESS) {
        PLT_Service *service;
        NPT_String value = "0";

        mFileTransmitDevice->FindServiceById("urn:upnp-org:serviceId:FileTransmitControl", service);
	service->GetStateVariableValue("INT_VALUE_TransmitProgress", value);
	NPT_LOG_INFO_1("GetStateVariableValue INT_VALUE_TransmitProgress: %s", (char*)value);
       NPT_CHECK_WARNING(action->SetArgumentValue("CurrentTransmitProgress", (const char *)value));
	NPT_CHECK_WARNING(action->SetArgumentValue("OutSID", (const char *)sid));
    }
    return result;
}





