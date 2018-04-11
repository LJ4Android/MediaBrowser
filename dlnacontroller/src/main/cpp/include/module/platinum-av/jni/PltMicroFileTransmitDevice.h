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


#ifndef _PLT_MICRO_FILE_TRANSMIT_DEVICE_H_
#define _PLT_MICRO_FILE_TRANSMIT_DEVICE_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Platinum.h"
#include "PltMediaServer.h"
#include "PltSyncMediaBrowser.h"
#include "PltFileTransmitDevice.h"
#include "NptMap.h"
#include "NptStack.h"

#define FILE_TRANSMIT_DEVICE_CTL_MSG_BASE 0x100

#define FILE_TRANSMIT_DEVICE_CTL_MSG_TRANSMIT_FILE (FILE_TRANSMIT_DEVICE_CTL_MSG_BASE+0)
#define FILE_TRANSMIT_DEVICE_CTL_MSG_CANCEL (FILE_TRANSMIT_DEVICE_CTL_MSG_BASE+1)
#define FILE_TRANSMIT_DEVICE_CTL_MSG_FINISH (FILE_TRANSMIT_DEVICE_CTL_MSG_BASE+2)
#define FILE_TRANSMIT_DEVICE_CTL_MSG_GETPROGRESS (FILE_TRANSMIT_DEVICE_CTL_MSG_BASE+3)
#define FILE_TRANSMIT_DEVICE_CTL_MSG_SEEK (FILE_TRANSMIT_DEVICE_CTL_MSG_BASE+4)
#define FILE_TRANSMIT_DEVICE_CTL_MSG_SETVOLUME (FILE_TRANSMIT_DEVICE_CTL_MSG_BASE+5)
#define FILE_TRANSMIT_DEVICE_CTL_MSG_SETMUTE (FILE_TRANSMIT_DEVICE_CTL_MSG_BASE+6)
#define FILE_TRANSMIT_DEVICE_CTL_MSG_SETPLAYMODE (FILE_TRANSMIT_DEVICE_CTL_MSG_BASE+7)
#define FILE_TRANSMIT_DEVICE_CTL_MSG_PRE (FILE_TRANSMIT_DEVICE_CTL_MSG_BASE+8)
#define FILE_TRANSMIT_DEVICE_CTL_MSG_NEXT (FILE_TRANSMIT_DEVICE_CTL_MSG_BASE+9)
#define FILE_TRANSMIT_DEVICE_CTL_MSG_GETVOLUME (FILE_TRANSMIT_DEVICE_CTL_MSG_BASE+10)
#define FILE_TRANSMIT_DEVICE_CTL_MSG_GETMUTE (FILE_TRANSMIT_DEVICE_CTL_MSG_BASE+11)
#define FILE_TRANSMIT_DEVICE_CTL_MSG_SETVOLUMEDBRANGE (FILE_TRANSMIT_DEVICE_CTL_MSG_BASE+12)
#define FILE_TRANSMIT_DEVICE_CTL_MSG_SETVOLUMEDB (FILE_TRANSMIT_DEVICE_CTL_MSG_BASE+13)
#define FILE_TRANSMIT_DEVICE_CTL_MSG_GETVOLUMEDBRANGE (FILE_TRANSMIT_DEVICE_CTL_MSG_BASE+14)
#define FILE_TRANSMIT_DEVICE_CTL_MSG_GETVOLUMEDB (FILE_TRANSMIT_DEVICE_CTL_MSG_BASE+15)

#define FILE_TRANSMIT_DEVICE_TOCONTRPOINT_MSG_BASE 0x500

#define FILE_TRANSMIT_DEVICE_TOCONTRPOINT_SET_TRANSMIT_FILE (FILE_TRANSMIT_DEVICE_TOCONTRPOINT_MSG_BASE+0)
#define FILE_TRANSMIT_DEVICE_TOCONTRPOINT_SET_CANCEL (FILE_TRANSMIT_DEVICE_TOCONTRPOINT_MSG_BASE+1)
#define FILE_TRANSMIT_DEVICE_TOCONTRPOINT_SET_FINISH (FILE_TRANSMIT_DEVICE_TOCONTRPOINT_MSG_BASE+2)
#define FILE_TRANSMIT_DEVICE_TOCONTRPOINT_SET_PROGRESS (FILE_TRANSMIT_DEVICE_TOCONTRPOINT_MSG_BASE+3)
#define FILE_TRANSMIT_DEVICE_TOCONTRPOINT_SET_MEDIA_DURATION (FILE_TRANSMIT_DEVICE_TOCONTRPOINT_MSG_BASE+4)
#define FILE_TRANSMIT_DEVICE_TOCONTRPOINT_SET_MEDIA_POSITION (FILE_TRANSMIT_DEVICE_TOCONTRPOINT_MSG_BASE+5)
#define FILE_TRANSMIT_DEVICE_TOCONTRPOINT_SET_MEDIA_PLAYINGSTATE (FILE_TRANSMIT_DEVICE_TOCONTRPOINT_MSG_BASE+6)
#define FILE_TRANSMIT_DEVICE_TOCONTRPOINT_SET_MEDIA_VOLUME (FILE_TRANSMIT_DEVICE_TOCONTRPOINT_MSG_BASE+7)
#define FILE_TRANSMIT_DEVICE_TOCONTRPOINT_SET_MEDIA_MUTE (FILE_TRANSMIT_DEVICE_TOCONTRPOINT_MSG_BASE+8)
#define FILE_TRANSMIT_DEVICE_TOCONTRPOINT_SET_MEDIA_VOLUMEDB (FILE_TRANSMIT_DEVICE_TOCONTRPOINT_MSG_BASE+9)
#define FILE_TRANSMIT_DEVICE_TOCONTRPOINT_SET_MEDIA_VOLUMEDBRANGE (FILE_TRANSMIT_DEVICE_TOCONTRPOINT_MSG_BASE+10)

/*----------------------------------------------------------------------
|   ActionCallback
+---------------------------------------------------------------------*/
class ActionCallback
{
public:
    ActionCallback();
    virtual ~ActionCallback();
    virtual int onActionReflection(int cmd, char *value, char *from, char *data1 = NULL, char *data2 = NULL) { return 0; }
private:
    uint32_t mTest;
};

/*----------------------------------------------------------------------
|   PLT_MicroFileTransmitDevice
+---------------------------------------------------------------------*/
class PLT_MicroFileTransmitDevice : public PLT_FileTransmitDeviceDelegate
{
public:
    PLT_MicroFileTransmitDevice(PLT_FileTransmitDevice *ftdevice, ActionCallback *callback);
    virtual ~PLT_MicroFileTransmitDevice();

    bool ResponseGenaEvent(int cmd, char *value, char *data);

    //
    NPT_Result OnTransmitFile(PLT_ActionReference& action);
    NPT_Result OnCancel(PLT_ActionReference& action);
    NPT_Result OnFinish(PLT_ActionReference& action);
    NPT_Result OnGetProgress(PLT_ActionReference& action);

private:

private:
    PLT_FileTransmitDevice *mFileTransmitDevice;
    ActionCallback *mCallback;
};

#endif /* _PLT_MICRO_FILE_TRANSMIT_DEVICE_H_ */
