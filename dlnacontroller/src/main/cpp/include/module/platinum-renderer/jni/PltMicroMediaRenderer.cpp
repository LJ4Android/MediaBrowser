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
#include "PltMicroMediaRenderer.h"
#include "PltLeaks.h"
#include "PltDownloader.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <android/log.h>

NPT_SET_LOCAL_LOGGER("platinum.tests.micromediarenderer")

ActionCallback::ActionCallback()
    :   mTest(0)
{
}

ActionCallback::~ActionCallback()
{
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaRenderer::PLT_MicroMediaRenderer
+---------------------------------------------------------------------*/
PLT_MicroMediaRenderer::PLT_MicroMediaRenderer(PLT_MediaRenderer *render, ActionCallback *callback)
{
    mMediaRender = render;
    mCallback = callback;

    mMediaRender->SetDelegate(this);
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaRenderer::PLT_MicroMediaRenderer
+---------------------------------------------------------------------*/
PLT_MicroMediaRenderer::~PLT_MicroMediaRenderer()
{
    NPT_LOG_INFO("~PLT_MicroMediaRenderer");
    if(mCallback != NULL) {
        delete mCallback;
        mCallback = NULL;
    }
}

bool PLT_MicroMediaRenderer::ResponseGenaEvent(int cmd, char *value, char *data)
{
    PLT_Service *service;
	//modify by linhuaji, 2014.07.01
    if(cmd == MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_VOLUME || 
	cmd == MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_MUTE ||
	cmd == MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_VOLUMEDB ||
	cmd == MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_VOLUMEDBRANGE) {
        mMediaRender->FindServiceById("urn:upnp-org:serviceId:RenderingControl", service);
    } else {
        mMediaRender->FindServiceById("urn:upnp-org:serviceId:AVTransport", service);
    }

    if(service != NULL) {
        switch(cmd) {
        case MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_DURATION:
            NPT_LOG_INFO_1("SET_MEDIA_DURATION: %s", value);
            service->SetStateVariable("CurrentMediaDuration", value);
            service->SetStateVariable("CurrentTrackDuration", value);
            break;

        case MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_POSITION:
            NPT_LOG_INFO_1("SET_MEDIA_POSITION: %s", value);
            service->SetStateVariable("RelativeTimePosition", value);
            break;

        case MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_PLAYINGSTATE:
            NPT_LOG_INFO_1("SET_MEDIA_PLAYINGSTATE: %s", value);
            service->SetStateVariable("TransportState", value);
            break;

        case MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_VOLUME:
            NPT_LOG_INFO_1("SET_MEDIA_VOLUME: %s", value);
            service->SetStateVariable("Volume", value);
            break;

        case MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_MUTE:
            NPT_LOG_INFO_1("SET_MEDIA_MUTE: %s", value);
            service->SetStateVariable("Mute", value);
            break;
        /* qiku add */
        /* add volumedb interface, linhuaji, 2014.07.01 */
		case MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_VOLUMEDB:
            NPT_LOG_INFO_1("SET_MEDIA_VOLUMEDB: %s", value);
             service->SetStateVariable("VolumeDB", value);
            break;

	    case MEDIA_RENDER_TOCONTRPOINT_SET_MEDIA_VOLUMEDBRANGE:
            NPT_LOG_INFO_1("SET_MEDIA_VOLUMEDBRANGE: %s", value);
             service->SetStateVariable("VolumeDB", value);
            break;
        /* qiku end */

        default:
            NPT_LOG_INFO_2("Unknown cmd: %d, value: %s", cmd, value);
            return false;
        }
        return true;
    }

    return false;
}

// ConnectionManager
NPT_Result PLT_MicroMediaRenderer::OnGetCurrentConnectionInfo(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnGetCurrentConnectionInfo");
    return 0;
}

// AVTransport
NPT_Result PLT_MicroMediaRenderer::OnSetAVTransportURI(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnSetAVTransportURI");

    // default implementation is using state variable
    NPT_String uri;
    NPT_CHECK_WARNING(action->GetArgumentValue("CurrentURI", uri));

    NPT_String metadata;
    NPT_CHECK_WARNING(action->GetArgumentValue("CurrentURIMetaData", metadata));
    
    PLT_Service* serviceAVT;
    NPT_CHECK_WARNING(mMediaRender->FindServiceByType("urn:schemas-upnp-org:service:AVTransport:1", serviceAVT));

    // update service state variables
    serviceAVT->SetStateVariable("AVTransportURI", uri);
    serviceAVT->SetStateVariable("AVTransportURIMetaData", metadata);

    mCallback->onActionReflection(MEDIA_RENDER_CTL_MSG_SET_AV_URL, (char *)uri, (char *)metadata);

    return NPT_SUCCESS;
}

NPT_Result PLT_MicroMediaRenderer::OnStop(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnStop");
    return mCallback->onActionReflection(MEDIA_RENDER_CTL_MSG_STOP, NULL, NULL);
}

NPT_Result PLT_MicroMediaRenderer::OnPlay(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnPlay");
    return mCallback->onActionReflection(MEDIA_RENDER_CTL_MSG_PLAY, NULL, NULL);
}

NPT_Result PLT_MicroMediaRenderer::OnPause(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnPause");
    return mCallback->onActionReflection(MEDIA_RENDER_CTL_MSG_PAUSE, NULL, NULL);
}

NPT_Result PLT_MicroMediaRenderer::OnSeek(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnSeek");
    NPT_String unit;
    NPT_String target;
    NPT_String seekpos;

    NPT_CHECK_WARNING(action->GetArgumentValue("Unit", unit));
    NPT_CHECK_WARNING(action->GetArgumentValue("Target", target));
    seekpos = unit + "=";
    seekpos += target;

    NPT_LOG_INFO_1("OnSeek: seekpos %s", (char *)seekpos);

    return mCallback->onActionReflection(MEDIA_RENDER_CTL_MSG_SEEK, (char *)seekpos, NULL);
}

NPT_Result PLT_MicroMediaRenderer::OnPrevious(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnPrevious");
    return mCallback->onActionReflection(MEDIA_RENDER_CTL_MSG_PRE, NULL, NULL);
}

NPT_Result PLT_MicroMediaRenderer::OnNext(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnNext");
    return mCallback->onActionReflection(MEDIA_RENDER_CTL_MSG_NEXT, NULL, NULL);
}

NPT_Result PLT_MicroMediaRenderer::OnSetPlayMode(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnSetPlayMode");
    return 0;
}


// RenderingControl
NPT_Result PLT_MicroMediaRenderer::OnSetVolume(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnSetVolume");
    NPT_String volume;
    NPT_CHECK_WARNING(action->GetArgumentValue("DesiredVolume", volume));
    return mCallback->onActionReflection(MEDIA_RENDER_CTL_MSG_SETVOLUME, (char *)volume, NULL);
}

/* qiku add */
/* add volumedb interface, linhuaji, 2014.07.01 */
NPT_Result PLT_MicroMediaRenderer::OnGetVolumeDBRange(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnGetVolumeDBRange");
    NPT_Result result = mCallback->onActionReflection(MEDIA_RENDER_CTL_MSG_GETVOLUMEDBRANGE, NULL, NULL);
    if(result == NPT_SUCCESS) {
            PLT_Service *service;
            NPT_String value;
	     const char *split = "~"; 
	     char *p = NULL;
	     int index = 0;
	     char min_volume[8];
	     char max_volume[8];

            mMediaRender->FindServiceById("urn:upnp-org:serviceId:RenderingControl", service);
            service->GetStateVariableValue("VolumeDB", value);

	     NPT_LOG_INFO_1("OnGetVolumeDBRange value: %s", (char*)value);	  
   
            p = strtok (value,split);   
            while(p!=NULL) {   
               if(index == 0) {
                   strncpy(min_volume, p, 8);
		} else {
                   strncpy(max_volume, p, 8);
		}
               p = strtok(NULL,split);
		index++;
            }   

            NPT_CHECK_WARNING(action->SetArgumentValue("MinValue", (const char *)min_volume));
	    NPT_CHECK_WARNING(action->SetArgumentValue("MaxValue", (const char *)max_volume));
    }
    return result;
}

NPT_Result PLT_MicroMediaRenderer::OnSetVolumeDB(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnSetVolumeDB");
    NPT_String volumedb;
    NPT_CHECK_WARNING(action->GetArgumentValue("DesiredVolume", volumedb));
    return mCallback->onActionReflection(MEDIA_RENDER_CTL_MSG_SETVOLUMEDB, (char *)volumedb, NULL);
}

NPT_Result PLT_MicroMediaRenderer::OnGetVolumeDB(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnGetVolumeDB");
    NPT_Result result = mCallback->onActionReflection(MEDIA_RENDER_CTL_MSG_GETVOLUMEDB, NULL, NULL);
    if(result == NPT_SUCCESS) {
    PLT_Service *service;
    NPT_String value = "0";

    mMediaRender->FindServiceById("urn:upnp-org:serviceId:RenderingControl", service);
    service->GetStateVariableValue("VolumeDB", value);
    NPT_CHECK_WARNING(action->SetArgumentValue("CurrentVolume", (const char *)value));
    }
    return result;
}
/* qiku end */

NPT_Result PLT_MicroMediaRenderer::OnSetMute(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnSetMute");
    NPT_String mute;
    NPT_CHECK_WARNING(action->GetArgumentValue("DesiredMute", mute));
    return mCallback->onActionReflection(MEDIA_RENDER_CTL_MSG_SETMUTE, (char *)mute, NULL);
}

NPT_Result PLT_MicroMediaRenderer::OnGetVolume(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnGetVolume");
    NPT_Result result = mCallback->onActionReflection(MEDIA_RENDER_CTL_MSG_GETVOLUME, NULL, NULL);
    if(result == NPT_SUCCESS) {
        PLT_Service *service;
        NPT_String value;

        mMediaRender->FindServiceById("urn:upnp-org:serviceId:RenderingControl", service);
        service->GetStateVariableValue("Volume", value);

        NPT_CHECK_WARNING(action->SetArgumentValue("CurrentVolume", (const char *)value));
    }

    return result;
}

NPT_Result PLT_MicroMediaRenderer::OnGetMute(PLT_ActionReference& action)
{
    NPT_LOG_INFO("OnGetMute");
    NPT_Result result = mCallback->onActionReflection(MEDIA_RENDER_CTL_MSG_GETMUTE, NULL, NULL);
    if(result == NPT_SUCCESS) {
        PLT_Service *service;
        NPT_String value;

        mMediaRender->FindServiceById("urn:upnp-org:serviceId:RenderingControl", service);
        service->GetStateVariableValue("Mute", value);

        NPT_CHECK_WARNING(action->SetArgumentValue("CurrentMute", (const char *)value));
    }

    return result;
}
