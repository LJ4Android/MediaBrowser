//
// Created by wh on 2017/7/25.
//

#include "PltMicroMediaRenderer.h"

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, "sinadlna_jni", __VA_ARGS__))

PLT_MicroMediaRenderer::PLT_MicroMediaRenderer(const char *friendly_name,const char *uuid){
    LOGI("PLT_MicroMediaRenderer::startMediaRenderer");
    renderer=new PLT_MediaRenderer(friendly_name, false, uuid);
    renderer->SetDelegate(this);
    PLT_DeviceHostReference device(renderer);
    LOGI("PLT_DeviceHostReference");
    this->upnp = new PLT_UPnP();
    LOGI("new PLT_UPnP");
    this->upnp->AddDevice(device);
    LOGI("upnp->AddDevice");
}

NPT_Result PLT_MicroMediaRenderer::startMediaRenderer() {
    int ret=NPT_FAILURE;
    if(this->upnp){
        ret=this->upnp->Start();
    }
    return ret;
}

NPT_Result PLT_MicroMediaRenderer::stopMediaRenderer() {
    int ret=NPT_FAILURE;
    if (this->upnp){
        if(this->upnp->IsRunning()){
            ret= this->upnp->Stop();
        }
        delete this->upnp;
        this->upnp=NULL;
    }
    if(renderer){
        renderer=NULL;
    }
    return ret;
}

bool PLT_MicroMediaRenderer::IsRunning() {
    if(NULL != this->upnp){
        return this->upnp->IsRunning();
    }
    return false;
}

//Response
NPT_Result PLT_MicroMediaRenderer::ResponseGenaEvent(int cmd,const char *value,const char *data) {
    PLT_Service *service;
    if(cmd == MediaRendererListener::MEDIA_RENDER_CTL_MSG_SETVOLUME ||
       cmd == MediaRendererListener::MEDIA_RENDER_CTL_MSG_SETMUTE ||
       cmd == MediaRendererListener::MEDIA_RENDER_CTL_MSG_SETVOLUMEDB ||
       cmd == MediaRendererListener::MEDIA_RENDER_CTL_MSG_GETVOLUMEDBRANGE) {
        renderer->FindServiceById("urn:upnp-org:serviceId:RenderingControl", service);
    } else {
        renderer->FindServiceById("urn:upnp-org:serviceId:AVTransport", service);
    }
    if(service){
        switch (cmd){
            case MediaRendererListener::MEDIA_RENDER_CTL_MSG_SETVOLUME:
                service->SetStateVariable("Volume", value);
                return NPT_SUCCESS;
            case MediaRendererListener::MEDIA_RENDER_CTL_MSG_SETMUTE:
                service->SetStateVariable("Mute", value);
                return NPT_SUCCESS;
            case MediaRendererListener::MEDIA_RENDER_CTL_MSG_GETVOLUMEDBRANGE:
            case MediaRendererListener::MEDIA_RENDER_CTL_MSG_SETVOLUMEDB:
                service->SetStateVariable("VolumeDB", value);
                return NPT_SUCCESS;
            case MediaRendererListener::MEDIA_RENDER_CTL_MSG_SET_AV_URL:
                service->SetStateVariable("AVTransportURI", value);
                service->SetStateVariable("AVTransportURIMetadata", data);
                return NPT_SUCCESS;
            case MediaRendererListener::MEDIA_RENDER_CTL_MSG_PLAY:
                service->SetStateVariable("TransportState", "PLAYING");
                return NPT_SUCCESS;
            case MediaRendererListener::MEDIA_RENDER_CTL_MSG_PAUSE:
                service->SetStateVariable("TransportState", "PAUSED_PLAYBACK");
                return NPT_SUCCESS;
            case MediaRendererListener::MEDIA_RENDER_CTL_MSG_STOP:
                service->SetStateVariable("TransportState", "STOPPED");
                return NPT_SUCCESS;
            case MediaRendererListener::MEDIA_RENDER_CTL_MSG_DURATION:
                service->SetStateVariable("CurrentMediaDuration", value);
                return NPT_SUCCESS;
            case MediaRendererListener::MEDIA_RENDER_CTL_MSG_POSITION:
                service->SetStateVariable("RelativeTimePosition", value);
                return NPT_SUCCESS;
            default:
                return NPT_FAILURE;
        }
    }
    return NPT_FAILURE;
}

// ConnectionManager
NPT_Result PLT_MicroMediaRenderer::OnGetCurrentConnectionInfo(PLT_ActionReference &action) {
    return NPT_SUCCESS;
}

// AVTransport
NPT_Result PLT_MicroMediaRenderer::OnNext(PLT_ActionReference &action) {
    if(listener){
        listener->OnMRActionRequest(MediaRendererListener::MEDIA_RENDER_CTL_MSG_NEXT,NULL,NULL);
    }
    return NPT_SUCCESS;
}

NPT_Result PLT_MicroMediaRenderer::OnPause(PLT_ActionReference &action) {
    if(listener){
        listener->OnMRActionRequest(MediaRendererListener::MEDIA_RENDER_CTL_MSG_PAUSE,NULL,NULL);
    }
    return NPT_SUCCESS;
}

NPT_Result PLT_MicroMediaRenderer::OnPlay(PLT_ActionReference &action) {
    if(listener){
        listener->OnMRActionRequest(MediaRendererListener::MEDIA_RENDER_CTL_MSG_PLAY,NULL,NULL);
    }
    return NPT_SUCCESS;
}

NPT_Result PLT_MicroMediaRenderer::OnStop(PLT_ActionReference &action) {
    if(listener){
        listener->OnMRActionRequest(MediaRendererListener::MEDIA_RENDER_CTL_MSG_STOP,NULL,NULL);
    }
    return NPT_SUCCESS;
}

NPT_Result PLT_MicroMediaRenderer::OnPrevious(PLT_ActionReference &action) {
    if(listener){
        listener->OnMRActionRequest(MediaRendererListener::MEDIA_RENDER_CTL_MSG_PRE,NULL,NULL);
    }
    return NPT_SUCCESS;
}

NPT_Result PLT_MicroMediaRenderer::OnSeek(PLT_ActionReference &action) {
    if(listener){
        NPT_String unit;
        NPT_String target;
        NPT_String seekpos;

        NPT_CHECK_WARNING(action->GetArgumentValue("Unit", unit));
        NPT_CHECK_WARNING(action->GetArgumentValue("Target", target));
        seekpos = unit + "=";
        seekpos += target;
        listener->OnMRActionRequest(MediaRendererListener::MEDIA_RENDER_CTL_MSG_SEEK,seekpos.GetChars(),NULL);
    }
    return NPT_SUCCESS;
}

NPT_Result PLT_MicroMediaRenderer::OnSetAVTransportURI(PLT_ActionReference &action) {
    if(listener){
        NPT_String uri;
        NPT_String metaData;
        action->GetArgumentValue("CurrentURI",uri);
        action->GetArgumentValue("CurrentURIMetaData",metaData);
        listener->OnMRActionRequest(MediaRendererListener::MEDIA_RENDER_CTL_MSG_SET_AV_URL,uri.GetChars(),metaData.GetChars());
    }
    return NPT_SUCCESS;
}

NPT_Result PLT_MicroMediaRenderer::OnSetPlayMode(PLT_ActionReference &action) {
    return NPT_SUCCESS;
}

// RenderingControl
NPT_Result PLT_MicroMediaRenderer::OnSetMute(PLT_ActionReference &action) {
    if(listener){
        NPT_String mute;
        action->GetArgumentValue("DesiredMute",mute);
        listener->OnMRActionRequest(MediaRendererListener::MEDIA_RENDER_CTL_MSG_SETMUTE,mute.GetChars(),NULL);
    }
    return NPT_SUCCESS;
}

NPT_Result PLT_MicroMediaRenderer::OnSetVolume(PLT_ActionReference &action) {
    if(listener){
        NPT_String vol;
        action->GetArgumentValue("DesiredVolume",vol);
        listener->OnMRActionRequest(MediaRendererListener::MEDIA_RENDER_CTL_MSG_SETVOLUME,vol.GetChars(),NULL);
    }
    return NPT_SUCCESS;
}

NPT_Result PLT_MicroMediaRenderer::OnSetVolumeDB(PLT_ActionReference &action) {
    if(listener){
        NPT_String vol;
        action->GetArgumentValue("DesiredVolume",vol);
        listener->OnMRActionRequest(MediaRendererListener::MEDIA_RENDER_CTL_MSG_SETVOLUMEDB,vol.GetChars(),NULL);
    }
    return NPT_SUCCESS;
}

NPT_Result PLT_MicroMediaRenderer::OnGetVolumeDBRange(PLT_ActionReference &action) {
    if(listener){
        listener->OnMRActionRequest(MediaRendererListener::MEDIA_RENDER_CTL_MSG_GETVOLUMEDBRANGE,NULL,NULL);
    }
    return NPT_SUCCESS;
}
