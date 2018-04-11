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


#ifndef _PLT_MICRO_MEDIA_RENDERER_H_
#define _PLT_MICRO_MEDIA_RENDERER_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Platinum.h"
#include "PltMediaServer.h"
#include "PltSyncMediaBrowser.h"
#include "PltMediaRenderer.h"
#include "NptMap.h"
#include "NptStack.h"

#define MEDIA_RENDER_CTL_MSG_BASE 0x100

#define MEDIA_RENDER_CTL_MSG_SET_AV_URL (MEDIA_RENDER_CTL_MSG_BASE+0)
#define MEDIA_RENDER_CTL_MSG_STOP (MEDIA_RENDER_CTL_MSG_BASE+1)
#define MEDIA_RENDER_CTL_MSG_PLAY (MEDIA_RENDER_CTL_MSG_BASE+2)
#define MEDIA_RENDER_CTL_MSG_PAUSE (MEDIA_RENDER_CTL_MSG_BASE+3)
#define MEDIA_RENDER_CTL_MSG_SEEK (MEDIA_RENDER_CTL_MSG_BASE+4)
#define MEDIA_RENDER_CTL_MSG_SETVOLUME (MEDIA_RENDER_CTL_MSG_BASE+5)
#define MEDIA_RENDER_CTL_MSG_SETMUTE (MEDIA_RENDER_CTL_MSG_BASE+6)
#define MEDIA_RENDER_CTL_MSG_SETPLAYMODE (MEDIA_RENDER_CTL_MSG_BASE+7)
#define MEDIA_RENDER_CTL_MSG_PRE (MEDIA_RENDER_CTL_MSG_BASE+8)
#define MEDIA_RENDER_CTL_MSG_NEXT (MEDIA_RENDER_CTL_MSG_BASE+9)
#define MEDIA_RENDER_CTL_MSG_GETVOLUME (MEDIA_RENDER_CTL_MSG_BASE+10)
#define MEDIA_RENDER_CTL_MSG_GETMUTE (MEDIA_RENDER_CTL_MSG_BASE+11)
/* add by linhuaji 2014.07.01 */
#define MEDIA_RENDER_CTL_MSG_SETVOLUMEDBRANGE (MEDIA_RENDER_CTL_MSG_BASE+12)
#define MEDIA_RENDER_CTL_MSG_SETVOLUMEDB (MEDIA_RENDER_CTL_MSG_BASE+13)
#define MEDIA_RENDER_CTL_MSG_GETVOLUMEDBRANGE (MEDIA_RENDER_CTL_MSG_BASE+14)
#define MEDIA_RENDER_CTL_MSG_GETVOLUMEDB (MEDIA_RENDER_CTL_MSG_BASE+15)
/* end by linhuaji */


#define MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_DURATION (MEDIA_RENDER_CTL_MSG_BASE+0)
#define MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_POSITION (MEDIA_RENDER_CTL_MSG_BASE+1)
#define MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_PLAYINGSTATE (MEDIA_RENDER_CTL_MSG_BASE+2)
#define MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_VOLUME (MEDIA_RENDER_CTL_MSG_BASE+3)
#define MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_MUTE (MEDIA_RENDER_CTL_MSG_BASE+4)
/* add by linhuaji 2014.07.01 */
#define MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_VOLUMEDB (MEDIA_RENDER_CTL_MSG_BASE+5)
#define MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_VOLUMEDBRANGE (MEDIA_RENDER_CTL_MSG_BASE+6)
/* end by linhuaji */

/*----------------------------------------------------------------------
|   ActionCallback
+---------------------------------------------------------------------*/
class ActionCallback
{
public:
    ActionCallback();
    virtual ~ActionCallback();
    virtual int onActionReflection(int cmd, char *value, char *data) { return 0; }
private:
    uint32_t mTest;
};

/*----------------------------------------------------------------------
|   PLT_MicroMediaRenderer
+---------------------------------------------------------------------*/
class PLT_MicroMediaRenderer : public PLT_MediaRendererDelegate
{
public:
    PLT_MicroMediaRenderer(PLT_MediaRenderer *render, ActionCallback *callback);
    virtual ~PLT_MicroMediaRenderer();

    bool ResponseGenaEvent(int cmd, char *value, char *data);

    // ConnectionManager
    NPT_Result OnGetCurrentConnectionInfo(PLT_ActionReference& action);

    // AVTransport
    NPT_Result OnNext(PLT_ActionReference& action);
    NPT_Result OnPause(PLT_ActionReference& action);
    NPT_Result OnPlay(PLT_ActionReference& action);
    NPT_Result OnPrevious(PLT_ActionReference& action);
    NPT_Result OnSeek(PLT_ActionReference& action);
    NPT_Result OnStop(PLT_ActionReference& action);
    NPT_Result OnSetAVTransportURI(PLT_ActionReference& action);
    NPT_Result OnSetPlayMode(PLT_ActionReference& action);

    // RenderingControl
    NPT_Result OnSetVolume(PLT_ActionReference& action);	
    NPT_Result OnSetVolumeDB(PLT_ActionReference& action);
	NPT_Result OnGetVolumeDBRange(PLT_ActionReference& action);
	/* add by linhuaji 2014.07.01 */
    NPT_Result OnSetVolumeDBRange(PLT_ActionReference& action);
	NPT_Result OnGetVolumeDB(PLT_ActionReference& action);
	/* end by linhuaji */
    NPT_Result OnSetMute(PLT_ActionReference& action);
    NPT_Result OnGetVolume(PLT_ActionReference& action);
    NPT_Result OnGetMute(PLT_ActionReference& action);

private:

private:
    PLT_MediaRenderer *mMediaRender;
    ActionCallback *mCallback;
};

#endif /* _PLT_MICRO_MEDIA_RENDERER_H_ */
