/*****************************************************************
|
|   Platinum - Miccro Media Controller
|
| Copyright (c) 2004-2010, Plutinosoft, LLC.
| All rights reserved.
| http://www.plutinosoft.com
|
| This program is free software; you can redistribute it and/or
| modify it under the terms of the GNU General Public License
| as published by the Free Software Foundation; either version 2
| of the License, or (at your option) any later version.
|
| OEMs, ISVs, VARs and other distributors that combine and 
| distribute commercially licensed software with Platinum software
| and do not wish to distribute the source code for the commercially
| licensed software under version 2, or (at your option) any later
| version, of the GNU General Public License (the "GPL") must enter
| into a commercial license agreement with Plutinosoft, LLC.
| licensing@plutinosoft.com
| 
| This program is distributed in the hope that it will be useful,
| but WITHOUT ANY WARRANTY; without even the implied warranty of
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
| GNU General Public License for more details.
|
| You should have received a copy of the GNU General Public License
| along with this program; see the file LICENSE.txt. If not, write to
| the Free Software Foundation, Inc., 
| 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
| http://www.gnu.org/licenses/gpl-2.0.html
|
****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "PltMicroMediaController.h"
#include "PltLeaks.h"
#include "PltDownloader.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#if SINA_DLNA_ANDROID
NPT_SET_LOCAL_LOGGER("platinum.android.jni")
#endif

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::PLT_MicroMediaController
+---------------------------------------------------------------------*/
PLT_MicroMediaController::PLT_MicroMediaController() :
    //PLT_SyncMediaBrowser(ctrlPoint),
    //PLT_MediaController(ctrlPoint),
	m_mr_listener(NULL),
	m_TrackDuration(-1),
	m_MediaDuration(-1)
{
    // create the stack that will be the directory where the
    // user is currently browsing. 
    // push the root directory onto the directory stack.
    //m_CurBrowseDirectoryStack.Push("0");
	this->uPnP = new PLT_UPnP();
	this->ctrlPoint = new PLT_CtrlPoint();
	plt_mediaController=new PLT_MediaController(this->ctrlPoint);
	plt_mediaController->SetDelegate(this);
	plt_syncMediaBrowser=new PLT_SyncMediaBrowser(this->ctrlPoint);
	plt_syncMediaBrowser->SetDelegate(this);
	this->uPnP->AddCtrlPoint(this->ctrlPoint);
	//根目录
	m_CurBrowseDirectoryStack.Push("0");
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::PLT_MicroMediaController
+---------------------------------------------------------------------*/
PLT_MicroMediaController::~PLT_MicroMediaController()
{
}

NPT_Result PLT_MicroMediaController::startMediaController() {
	int ret=NPT_FAILURE;
	if(this->uPnP){
		ret = this->uPnP->Start();
	}
	if(!this->ctrlPoint.IsNull()){
        ctrlPoint->Search(
                NPT_HttpUrl("239.255.255.250", 1900, "*"),
                "urn:schemas-upnp-org:service:ContentDirectory:1", 2, 10000., NPT_TimeInterval(10.0));
        this->ctrlPoint->Search(NPT_HttpUrl("239.255.255.250", 1900, "*"), "urn:schemas-upnp-org:device:MediaServer:1", 1, 0.0);
        this->ctrlPoint->Search(NPT_HttpUrl("239.255.255.250", 1900, "*"), "urn:schemas-upnp-org:device:MediaRenderer:1", 1, 0.0);
		this->ctrlPoint->Discover(NPT_HttpUrl("255.255.255.255", 1900, "*"), "upnp:rootdevice", 1, 6000. /*6000*/);
		this->ctrlPoint->Discover(NPT_HttpUrl("239.255.255.250", 1900, "*"), "upnp:rootdevice", 1, 6000. /*6000*/);
	}
	return ret;
}

NPT_Result PLT_MicroMediaController::stopMediaController() {
	int ret=NPT_FAILURE;
	if(this->plt_mediaController){
		delete this->plt_mediaController;
		this->plt_mediaController=NULL;
	}
	if(this->plt_syncMediaBrowser){
		delete this->plt_syncMediaBrowser;
		this->plt_syncMediaBrowser=NULL;
	}
	if (this->uPnP){
		if(this->uPnP->IsRunning()){
			ret= this->uPnP->Stop();
		}
		delete this->uPnP;
		this->uPnP=NULL;
	}
	if(!this->ctrlPoint.IsNull()){
		this->ctrlPoint.Detach();
	}
	return ret;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::OnMSAdded
+---------------------------------------------------------------------*/
bool
PLT_MicroMediaController::OnMSAdded(PLT_DeviceDataReference& device)
{
	NPT_String uuid = device->GetUUID();

	// test if it's a media renderer
	PLT_Service* service;
    // Issue special action upon discovering MediaConnect server
    if (NPT_SUCCEEDED(device->FindServiceByType("urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:*", service))) {
        PLT_ActionReference action;
        ctrlPoint->CreateAction(
                device,
                "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1",
                "IsAuthorized",
                action);
        if (!action.IsNull()) {
            action->SetArgumentValue("DeviceID", "");
            ctrlPoint->InvokeAction(action, 0);
        }

        ctrlPoint->CreateAction(
                device,
                "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1",
                "IsValidated",
                action);
        if (!action.IsNull()) {
            action->SetArgumentValue("DeviceID", "");
            ctrlPoint->InvokeAction(action, 0);
        }
    }
	if (NPT_SUCCEEDED(device->FindServiceByType("urn:schemas-upnp-org:service:ContentDirectory:*", service))) {
		NPT_AutoLock lock(m_Devices);
		m_Devices.Put(uuid, device);

		NPT_AutoLock lock2(m_mr_listener_lock);
		if(m_mr_listener){
			m_mr_listener->OnDeviceChange(MediaControllerListener::DEVICE_EVENT_ADDED,
										  (const char*)uuid,
										  (const char*)device->GetFriendlyName(),(const char*)device->GetType());
		}
	}
	return true;
}

void PLT_MicroMediaController::OnMSRemoved(PLT_DeviceDataReference & device) {
	NPT_String uuid = device->GetUUID();

	{
		NPT_AutoLock lock(m_Devices);
		m_Devices.Erase(uuid);

		NPT_AutoLock lock2(m_mr_listener_lock);
		if(m_mr_listener){
			m_mr_listener->OnDeviceChange(MediaControllerListener::DEVICE_EVENT_REMOVED,
										  (const char*)uuid,
										  (const char*)device->GetFriendlyName(),(const char*)device->GetType());
		}
	}

	{
		NPT_AutoLock lock(m_CurMediaServerLock);
		// if it's the currently selected one, we have to get rid of it
		if (!m_CurMediaServer.IsNull() && m_CurMediaServer == device) {
			m_CurMediaServer = NULL;
		}
	}
}

void PLT_MicroMediaController::OnMSStateVariablesChanged(PLT_Service *service,
                                                         NPT_List<PLT_StateVariable *> *vars) {

}

void PLT_MicroMediaController::OnBrowseResult(NPT_Result res, PLT_DeviceDataReference &device,
                                              PLT_BrowseInfo *info, void *userdata) {
    NPT_COMPILER_UNUSED(device);

    if (!userdata) return;

    PLT_BrowseDataReference* data = (PLT_BrowseDataReference*) userdata;
    (*data)->res = res;
    if (NPT_SUCCEEDED(res) && info) {
        (*data)->info = *info;
    }
    (*data)->shared_var.SetValue(1);
    delete data;
}

void PLT_MicroMediaController::OnSearchResult(NPT_Result res, PLT_DeviceDataReference &device,
                                              PLT_BrowseInfo *info, void *userdata) {

}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::OnMRAdded
+---------------------------------------------------------------------*/
bool
PLT_MicroMediaController::OnMRAdded(PLT_DeviceDataReference& device)
{
	
    NPT_String uuid = device->GetUUID();
	
    // test if it's a media renderer
    PLT_Service* service;
    if (NPT_SUCCEEDED(device->FindServiceByType("urn:schemas-upnp-org:service:AVTransport:*", service))) {
        NPT_AutoLock lock(m_Devices);
		m_Devices.Put(uuid, device);

		NPT_AutoLock lock2(m_mr_listener_lock);
		if(m_mr_listener){
			m_mr_listener->OnDeviceChange(MediaControllerListener::DEVICE_EVENT_ADDED,
										  (const char*)uuid,
										  (const char*)device->GetFriendlyName(),(const char*)device->GetType());
		}
    }
    
    return true;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::OnMRRemoved
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::OnMRRemoved(PLT_DeviceDataReference& device)
{
    NPT_String uuid = device->GetUUID();

    {
        NPT_AutoLock lock(m_Devices);
		m_Devices.Erase(uuid);
		
		NPT_AutoLock lock2(m_mr_listener_lock);
		if(m_mr_listener){
			m_mr_listener->OnDeviceChange(MediaControllerListener::DEVICE_EVENT_REMOVED,
										  (const char*)uuid,
										  (const char*)device->GetFriendlyName(),(const char*)device->GetType());
		}
    }

    {
        NPT_AutoLock lock(m_CurMediaServerLock);
        // if it's the currently selected one, we have to get rid of it
        if (!m_CurMediaRenderer.IsNull() && m_CurMediaRenderer == device) {
			m_CurMediaRenderer = NULL;
        }
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::OnMRStateVariablesChanged
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::OnMRStateVariablesChanged(PLT_Service*                  service,
                                                    NPT_List<PLT_StateVariable*>* vars)
{
	//应该通知设备的状态变化操作
	int notify = 0;
	PLT_DeviceDataReference cur_device;
    GetCurMediaRenderer(cur_device);
    LOGI("OnMRStateVariablesChanged");
	if ( !cur_device.IsNull() && cur_device->GetUUID()==service->GetDevice()->GetUUID() ) {
		notify = 1;
    }
	
    NPT_String uuid = service->GetDevice()->GetUUID();
    NPT_List<PLT_StateVariable*>::Iterator var = vars->GetFirstItem();
    while (var) {
        NPT_LOG_INFO_5("%s Received state var \"%s:%s:%s\" changes: \"%s\"\n",
				__FUNCTION__,
               (const char*)uuid,
               (const char*)service->GetServiceID(),
               (const char*)(*var)->GetName(),
               (const char*)(*var)->GetValue());
		
		if(notify && m_mr_listener){
			NPT_String name = (*var)->GetName();
			NPT_String value= (*var)->GetValue();
            LOGI("OnMRStateVariablesChanged name: %s,value: %s",name.GetChars(),value.GetChars());
			if (name.Compare("TransportState", true) == 0) {
			    if(value.Compare("STOPPED", true) == 0){
					m_mr_listener->OnMRStateVariablesChanged(MediaControllerListener::PLAYBACK_STATE_CHANGE_STOP,0);
				}
				else if(value.Compare("PLAYING", true) == 0){
					m_mr_listener->OnMRStateVariablesChanged(MediaControllerListener::PLAYBACK_STATE_CHANGE_PLAY,0);
				}
				else if(value.Compare("PAUSED_PLAYBACK", true) == 0){
					m_mr_listener->OnMRStateVariablesChanged(MediaControllerListener::PLAYBACK_STATE_CHANGE_PAUSE,0);
				}
			}else if(name.Compare("Mute",true) == 0){
				if(value.Compare("0", true) == 0){
					m_mr_listener->OnMRStateVariablesChanged(MediaControllerListener::PLAYBACK_STATE_CHANGE_MUTE,0);
				} else if (value.Compare("1", true) == 0){
					m_mr_listener->OnMRStateVariablesChanged(MediaControllerListener::PLAYBACK_STATE_CHANGE_MUTE,1);
				}
			}else if (name.Compare("Volume", true) == 0){
				int val;
				value.ToInteger(val);
				m_mr_listener->OnMRStateVariablesChanged(MediaControllerListener::PLAYBACK_STATE_CHANGE_VOLUME,val);
			} else if(name.Compare("CurrentMediaDuration", true) == 0){
				NPT_UInt32 seconds;
				PLT_Didl::ParseTimeStamp(value, seconds);
				m_mr_listener->OnMRStateVariablesChanged(MediaControllerListener::PLAYBACK_STATE_CHANGE_DURATION,seconds);
			}else if(name.Compare("RelativeTimePosition", true) == 0){
				NPT_UInt32 seconds;
				PLT_Didl::ParseTimeStamp(value, seconds);
				m_mr_listener->OnMRStateVariablesChanged(MediaControllerListener::PLAYBACK_STATE_CHANGE_POSITION,seconds);
            }else if(name.Compare("AVTransportURI", true) == 0){
                if(!value.IsEmpty())
                    m_mr_listener->OnMRMediaDataChanged(MediaControllerListener::PLAYBACK_STATE_CHANGE_AV_TRANSPORT_URI,
                                                        value.GetChars());
            } else if(name.Compare("AVTransportURIMetadata", true) == 0){
                if(!value.IsEmpty())
                    m_mr_listener->OnMRMediaDataChanged(MediaControllerListener::PLAYBACK_STATE_CHANGE_AV_TRANSPORT_URI_METADATA,
                                                        value.GetChars());
            }
		}
        ++var;
    }
}


// AVTransport
void 
PLT_MicroMediaController::OnGetMediaInfoResult(
        NPT_Result               res,
        PLT_DeviceDataReference& device,
        PLT_MediaInfo*           info ,
        void*                    userdata)
{
	NPT_LOG_INFO_1("%s into.", __FUNCTION__);
	
	PLT_DeviceDataReference cur_device;
    GetCurMediaRenderer(cur_device);
    if (cur_device.IsNull()) {
		NPT_LOG_INFO_1("%s current device null", __FUNCTION__);
        return;
    }
	if( cur_device->GetUUID() != device->GetUUID() ){
		NPT_LOG_INFO_1("%s device not equal current device", __FUNCTION__);
		return;
	}
	
	NPT_AutoLock lock(m_mr_listener_lock);
	
	if(m_mr_listener){
		if(info){
			m_MediaDuration = info->media_duration.ToSeconds()*1000;
			if(m_MediaDuration==0){
				NPT_LOG_INFO_1("%s MediaInfo return bad MediaDuration.", __FUNCTION__);
			}
			if(m_MediaDuration==0 && m_TrackDuration>0){
				m_MediaDuration = m_TrackDuration;
				NPT_LOG_INFO_1("%s use PositionInfo TrackDuration instead of MediaDuration.", __FUNCTION__);
				m_mr_listener->OnMRActionResponse(MediaControllerListener::ACTION_RESPONSE_DURATION, res, m_MediaDuration) ;
			}else{
				m_mr_listener->OnMRActionResponse(MediaControllerListener::ACTION_RESPONSE_DURATION, res, info->media_duration.ToSeconds()*1000) ;
			}
		}else{
			NPT_LOG_INFO_1("%s info NULL, bad result!", __FUNCTION__);
			m_mr_listener->OnMRActionResponse(MediaControllerListener::ACTION_RESPONSE_DURATION, res, 0 ) ;
		}
	}
}

void 
PLT_MicroMediaController::OnGetPositionInfoResult(
        NPT_Result               res ,
        PLT_DeviceDataReference& device ,
        PLT_PositionInfo*        info ,
        void*                    userdata )
{
	NPT_LOG_INFO_1("%s into.", __FUNCTION__);
	
	PLT_DeviceDataReference cur_device;
    GetCurMediaRenderer(cur_device);
    if (cur_device.IsNull()) {
		NPT_LOG_INFO_1("%s current device null", __FUNCTION__);
        return;
    }
	if( cur_device->GetUUID() != device->GetUUID() ){
		NPT_LOG_INFO_1("%s device not equal current device", __FUNCTION__);
		return;
	}
	
	NPT_AutoLock lock(m_mr_listener_lock);
	if(m_mr_listener){
		if(info){
			if(m_MediaDuration==0){
				NPT_LOG_INFO_1("%s MediaInfo return bad MediaDuration.", __FUNCTION__);
				m_TrackDuration = info->track_duration.ToSeconds()*1000;
				if(m_TrackDuration>0){
					m_MediaDuration = m_TrackDuration;
					NPT_LOG_INFO_1("%s use PositionInfo TrackDuration instead of MediaDuration.", __FUNCTION__);
					m_mr_listener->OnMRActionResponse(MediaControllerListener::ACTION_RESPONSE_DURATION, res, m_MediaDuration) ;
				}
			}
			m_mr_listener->OnMRActionResponse(MediaControllerListener::ACTION_RESPONSE_POSITION, res, info->rel_time.ToSeconds()*1000) ;
		}else{
			NPT_LOG_INFO_1("%s info NULL, bad result!", __FUNCTION__);
			m_mr_listener->OnMRActionResponse(MediaControllerListener::ACTION_RESPONSE_DURATION, res, 0 ) ;
		}
	}
}
		
void 
PLT_MicroMediaController::OnSetAVTransportURIResult(
        NPT_Result               res ,
        PLT_DeviceDataReference& device ,
        void*                    userdata)
{
	NPT_LOG_INFO_1("%s into.", __FUNCTION__);
	
	PLT_DeviceDataReference cur_device;
    GetCurMediaRenderer(cur_device);
    if (cur_device.IsNull()) {
		NPT_LOG_INFO_1("%s current device null", __FUNCTION__);
        return;
    }
	if( cur_device->GetUUID() != device->GetUUID() ){
		NPT_LOG_INFO_1("%s device not equal current device", __FUNCTION__);
		return;
	}
	
	NPT_AutoLock lock(m_mr_listener_lock);
	if(m_mr_listener){
		m_mr_listener->OnMRActionResponse(MediaControllerListener::ACTION_RESPONSE_OPEN, res, 0) ;
	}
}

		
void 
PLT_MicroMediaController::OnPlayResult(
        NPT_Result               res ,
        PLT_DeviceDataReference& device ,
        void*                    userdata)
{
	NPT_LOG_INFO_1("%s into.", __FUNCTION__);
	
	PLT_DeviceDataReference cur_device;
    GetCurMediaRenderer(cur_device);
    if (cur_device.IsNull()) {
		NPT_LOG_INFO_1("%s current device null", __FUNCTION__);
        return;
    }
	if( cur_device->GetUUID() != device->GetUUID() ){
		NPT_LOG_INFO_1("%s device not equal current device", __FUNCTION__);
		return;
	}
	
	NPT_AutoLock lock(m_mr_listener_lock);
	if(m_mr_listener){
		m_mr_listener->OnMRActionResponse(MediaControllerListener::ACTION_RESPONSE_PLAY, res, 0) ;
	}
}

void 
PLT_MicroMediaController::OnPauseResult(
        NPT_Result               res ,
        PLT_DeviceDataReference& device,
        void*                     userdata )
{
	NPT_LOG_INFO_1("%s into.", __FUNCTION__);
	
	PLT_DeviceDataReference cur_device;
    GetCurMediaRenderer(cur_device);
    if (cur_device.IsNull()) {
		NPT_LOG_INFO_1("%s current device null", __FUNCTION__);
        return;
    }
	if( cur_device->GetUUID() != device->GetUUID() ){
		NPT_LOG_INFO_1("%s device not equal current device", __FUNCTION__);
		return;
	}
	
	NPT_AutoLock lock(m_mr_listener_lock);
	if(m_mr_listener){
		m_mr_listener->OnMRActionResponse(MediaControllerListener::ACTION_RESPONSE_PAUSE, res, 0) ;
	}
}


void 
PLT_MicroMediaController::OnSeekResult(
        NPT_Result               res ,
        PLT_DeviceDataReference& device,
        void*                    userdata)
{
	NPT_LOG_INFO_1("%s into.", __FUNCTION__);
	
	PLT_DeviceDataReference cur_device;
    GetCurMediaRenderer(cur_device);
    if (cur_device.IsNull()) {
		NPT_LOG_INFO_1("%s current device null", __FUNCTION__);
        return;
    }
	if( cur_device->GetUUID() != device->GetUUID() ){
		NPT_LOG_INFO_1("%s device not equal current device", __FUNCTION__);
		return;
	}
	
	NPT_AutoLock lock(m_mr_listener_lock);
	if(m_mr_listener){
		m_mr_listener->OnMRActionResponse(MediaControllerListener::ACTION_RESPONSE_SEEK, res, 0) ;
	}	
}


void 
PLT_MicroMediaController::OnStopResult(
        NPT_Result               res ,
        PLT_DeviceDataReference& device ,
        void*                    userdata)
{
	NPT_LOG_INFO_1("%s into.", __FUNCTION__);
	
	PLT_DeviceDataReference cur_device;
    GetCurMediaRenderer(cur_device);
    if (cur_device.IsNull()) {
		NPT_LOG_INFO_1("%s current device null", __FUNCTION__);
        return;
    }
	if( cur_device->GetUUID() != device->GetUUID() ){
		NPT_LOG_INFO_1("%s device not equal current device", __FUNCTION__);
		return;
	}
	
	NPT_AutoLock lock(m_mr_listener_lock);
	if(m_mr_listener){
		m_mr_listener->OnMRActionResponse(MediaControllerListener::ACTION_RESPONSE_STOP, res, 0) ;
	}	
}
	
// RenderingControl
void 
PLT_MicroMediaController::OnSetMuteResult(
        NPT_Result               res,
        PLT_DeviceDataReference& device,
        void*                    userdata)
{
	NPT_LOG_INFO_1("%s into.", __FUNCTION__);
	
	PLT_DeviceDataReference cur_device;
    GetCurMediaRenderer(cur_device);
    if (cur_device.IsNull()) {
		NPT_LOG_INFO_1("%s current device null", __FUNCTION__);
        return;
    }
	if( cur_device->GetUUID() != device->GetUUID() ){
		NPT_LOG_INFO_1("%s device not equal current device", __FUNCTION__);
		return;
	}
	
	NPT_AutoLock lock(m_mr_listener_lock);
	if(m_mr_listener){
		m_mr_listener->OnMRActionResponse(MediaControllerListener::ACTION_RESPONSE_SET_MUTE, res, 0) ;
	}
}

void 
PLT_MicroMediaController::OnGetMuteResult(
        NPT_Result               res ,
        PLT_DeviceDataReference& device ,
        const char*              channel,
        bool                     mute,
        void*                    userdata) 
{
	NPT_LOG_INFO_1("%s into.", __FUNCTION__);
	
	PLT_DeviceDataReference cur_device;
    GetCurMediaRenderer(cur_device);
    if (cur_device.IsNull()) {
		NPT_LOG_INFO_1("%s current device null", __FUNCTION__);
        return;
    }
	if( cur_device->GetUUID() != device->GetUUID() ){
		NPT_LOG_INFO_1("%s device not equal current device", __FUNCTION__);
		return;
	}
	
	NPT_AutoLock lock(m_mr_listener_lock);
	if(m_mr_listener){
		m_mr_listener->OnMRActionResponse(MediaControllerListener::ACTION_RESPONSE_GET_MUTE, res, mute) ;
	}
}

void 
PLT_MicroMediaController::OnSetVolumeResult(
        NPT_Result               res ,
        PLT_DeviceDataReference& device,
        void*                    userdata)
{
	NPT_LOG_INFO_1("%s into.", __FUNCTION__);
	
	PLT_DeviceDataReference cur_device;
    GetCurMediaRenderer(cur_device);
    if (cur_device.IsNull()) {
		NPT_LOG_INFO_1("%s current device null", __FUNCTION__);
        return;
    }
	if( cur_device->GetUUID() != device->GetUUID() ){
		NPT_LOG_INFO_1("%s device not equal current device", __FUNCTION__);
		return;
	}
	
	NPT_AutoLock lock(m_mr_listener_lock);
	if(m_mr_listener){
		m_mr_listener->OnMRActionResponse(MediaControllerListener::ACTION_RESPONSE_SET_VOLUME, res, 0) ;
	}
}

void PLT_MicroMediaController::OnGetVolumeResult(
        NPT_Result               res,
        PLT_DeviceDataReference& device,
		const char*              channel,
    	NPT_UInt32				 volume ,
	    void*                    userdata)
{
	NPT_LOG_INFO_1("%s into.", __FUNCTION__);
	
	PLT_DeviceDataReference cur_device;
    GetCurMediaRenderer(cur_device);
    if (cur_device.IsNull()) {
		NPT_LOG_INFO_1("%s current device null", __FUNCTION__);
        return;
    }
	if( cur_device->GetUUID() != device->GetUUID() ){
		NPT_LOG_INFO_1("%s device not equal current device", __FUNCTION__);
		return;
	}
	
	NPT_AutoLock lock(m_mr_listener_lock);
	if(m_mr_listener){
		m_mr_listener->OnMRActionResponse(MediaControllerListener::ACTION_RESPONSE_GET_VOLUME, res, volume) ;
	}
}
		
/*----------------------------------------------------------------------
|   PLT_MicroMediaController::GetCurMediaRenderer
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::GetCurMediaRenderer(PLT_DeviceDataReference& renderer)
{
    NPT_AutoLock lock(m_CurMediaRendererLock);

    if (m_CurMediaRenderer.IsNull()) {
        NPT_LOG_INFO("No renderer selected, select one with setmr\n");
    } else {
        renderer = m_CurMediaRenderer;
    }
}

void
PLT_MicroMediaController::GetCurMediaServer(PLT_DeviceDataReference& server)
{
	NPT_AutoLock lock(m_CurMediaServerLock);

	if (m_CurMediaServer.IsNull()) {
		printf("No server selected, select one with setms\n");
	} else {
		server = m_CurMediaServer;
	}
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_setmr
+---------------------------------------------------------------------*/
void 
PLT_MicroMediaController::setDMR(const char *uuid)
{
	PLT_DeviceDataReference* result = NULL;
	{
		NPT_AutoLock lock(m_Devices);
		m_Devices.Get(uuid, result);
	}
	if(result==NULL){
		NPT_LOG_INFO_2("%s cannot find mr, uuid=[%s]",__FUNCTION__,uuid);
	}else{
		NPT_LOG_INFO_3("%s find mr, uuid=[%s] name=[%s]",__FUNCTION__,uuid,(const char*)(*result)->GetFriendlyName());
	}
	
    NPT_AutoLock lock(m_CurMediaRendererLock);
	m_CurMediaRenderer = result ? *result:PLT_DeviceDataReference(); // return empty reference if not device was selected
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_getmr
+---------------------------------------------------------------------*/
const char*
PLT_MicroMediaController::getDMR()
{
    PLT_DeviceDataReference device;
 
    GetCurMediaRenderer(device);

    if (!device.IsNull()) {
		return device->GetUUID().GetChars();
    }
	return NULL;
}

void PLT_MicroMediaController::setDMS(const char *uuid) {
	PLT_DeviceDataReference* result = NULL;
	{
		NPT_AutoLock lock(m_Devices);
		m_Devices.Get(uuid, result);
	}
	if(result==NULL){
		NPT_LOG_INFO_2("%s cannot find mr, uuid=[%s]",__FUNCTION__,uuid);
	}else{
		NPT_LOG_INFO_3("%s find mr, uuid=[%s] name=[%s]",__FUNCTION__,uuid,(const char*)(*result)->GetFriendlyName());
	}

	NPT_AutoLock lock(m_CurMediaServerLock);
	m_CurMediaServer = result?*result:PLT_DeviceDataReference(); // return empty reference if not device was selected
}

const char*
PLT_MicroMediaController::getDMS()
{
	PLT_DeviceDataReference device;

	GetCurMediaServer(device);

	if (!device.IsNull()) {
		return device->GetUUID().GetChars();
	}
	return NULL;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_open
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::open(const char* url, const char* didl)
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
		// invoke the setUri
		NPT_LOG_INFO_3("Issuing SetAVTransportURI with url=%s , didl=%s", __FUNCTION__,url, didl);
		if(plt_mediaController)
			plt_mediaController->SetAVTransportURI(device, 0, url, didl, NULL);

    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_play
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::play()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
		if(plt_mediaController)
			plt_mediaController->Play(device, 0, "1", NULL);
    }
}

void 	
PLT_MicroMediaController::pause()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
		if(plt_mediaController)
			plt_mediaController->Pause(device, 0, NULL);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_seek
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::seek(const char* command)
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        // remove first part of command ("seek")
        NPT_String target = command;
        //NPT_List<NPT_String> args = target.Split(" ");
        //if (args.GetItemCount() < 2) return;

        //args.Erase(args.GetFirstItem());
        //target = NPT_String::Join(args, " ");
        
        //Seek(device, 0, (target.Find(":")!=-1)?"REL_TIME":"X_DLNA_REL_BYTE", target, NULL);
		if(plt_mediaController)
			plt_mediaController->Seek(device, 0, "REL_TIME", target, NULL);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_stop
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::stop()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
		if(plt_mediaController)
			plt_mediaController->Stop(device, 0, NULL);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_mute
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::setMute(bool yes)
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
		if(plt_mediaController)
			plt_mediaController->SetMute(device, 0, "Master", yes, NULL);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_unmute
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::getMute()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
		if(plt_mediaController)
			plt_mediaController->GetMute(device, 0, "Master", NULL);
    }
}

void    
PLT_MicroMediaController::getVolumeRange(int& volMin, int& volMax)
{
	PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        PLT_Service* service = NULL;
        if (NPT_SUCCEEDED(device->FindServiceByType("urn:schemas-upnp-org:service:RenderingControl:*", service))) {
                if(service){
                        PLT_StateVariable* stval = service->FindStateVariable("Volume");
                        if(stval){
                                const NPT_AllowedValueRange* avr = stval->GetAllowedValueRange();
                                if(avr){
									volMin = avr->min_value;
									volMax = avr->max_value;
									NPT_LOG_INFO_2("[min vol=%d,max vol=%d]",avr->min_value,avr->max_value);
                                }else{
									NPT_LOG_INFO("*** StateVariable Volume AllowedValueRange not found\n");
                                }
                        }else{
							NPT_LOG_INFO("*** StateVariable Volume not found\n");
                        }
                }else{
					NPT_LOG_INFO("*** serice NULL,RenderingControl servie not found\n");
                }
        }else{
			NPT_LOG_INFO("*** RenderingControl servie not found\n");
        }
    }
}


void    
PLT_MicroMediaController::setVolume(int vol)
{
	NPT_LOG_INFO_2("%s into. vol=%d",__FUNCTION__,vol);
	
	PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
		if(plt_mediaController)
			plt_mediaController->SetVolume(device, 0, "Master", vol,NULL);
    }
}

void    
PLT_MicroMediaController::getVolume()
{
	NPT_LOG_INFO_1("%s into.",__FUNCTION__);
	PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
		if(plt_mediaController)
			plt_mediaController->GetVolume(device, 0, "Master", NULL);
    }
}


void    
PLT_MicroMediaController::getMediaInfo()
{
	NPT_LOG_INFO_1("%s into.",__FUNCTION__);
	PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
		if(plt_mediaController)
			plt_mediaController->GetMediaInfo(device, 0, NULL);
    }
}

void    
PLT_MicroMediaController::getPositionInfo()
{
	NPT_LOG_INFO_1("%s into.",__FUNCTION__);
	PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
		if(plt_mediaController)
			plt_mediaController->GetPositionInfo(device, 0, NULL);
    }
}
/*PLT_BrowseDataReference& browse_data,
        PLT_DeviceDataReference& device,
const char*              object_id,
        NPT_Int32                index,
NPT_Int32                count,
bool                     browse_metadata,
const char*              filter,
const char*              sort*/
NPT_Result PLT_MicroMediaController::DoBrowse(const char *object_id, bool metadata) {
	NPT_Result res = NPT_FAILURE;
	PLT_DeviceDataReference device;

	GetCurMediaServer(device);
	if (!device.IsNull()) {
		NPT_String cur_object_id;
		m_CurBrowseDirectoryStack.Peek(cur_object_id);
		LOGI("DoBrowse object_id=%s, FriendlyName=%s",object_id,device->GetFriendlyName().GetChars());
		// send off the browse packet and block
		res = plt_syncMediaBrowser->BrowseSync(device,
                                               object_id?object_id:(const char*)cur_object_id,
                                               m_MostRecentBrowseResults,
                                               metadata);
		if(m_mr_listener){
			LOGI("OnMSMediaBrowser %d",res);
			m_mr_listener->OnMSMediaBrowser(res,m_MostRecentBrowseResults);
		}
	}
	return res;
}

void PLT_MicroMediaController::PopDirectoryStackToRoot(void){
	NPT_String val;
	while (NPT_SUCCEEDED(m_CurBrowseDirectoryStack.Peek(val)) && val.Compare("0")) {
		m_CurBrowseDirectoryStack.Pop(val);
	}
}