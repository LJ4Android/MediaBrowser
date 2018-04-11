//
// Created by wh on 2017/12/10.
//
#include "MediaRemote.h"


#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, "sinadlna_jni", __VA_ARGS__))

MediaRemote* MediaRemoteHelper::create() {
    MediaRemote* remote=new MediaRemote();
    return remote;
}

void MediaRemoteHelper::destroy(MediaRemote *dlna) {
    if(dlna)
    {
        delete dlna;
    }
}

MediaRemote::MediaRemote() {
    this->controller=NULL;
    this->mediaRenderer=NULL;
    this->mediaServer=NULL;
    this->notify=NULL;
}

MediaRemote::~MediaRemote(){
    this->release();
}

int MediaRemote::setup(JavaVM *jvm, jobject thiz, jobject weak_thiz) {
    if(this->notify==NULL){
        this->notify=new JNIMediaRemoteEventNotify(jvm,thiz,weak_thiz);
        if(!this->notify){
            return NPT_FAILURE;
        }
    }
    return NPT_SUCCESS;
}

int MediaRemote::release() {
    this->stopMediaServer();
    this->stopMediaRender();
    this->stopMediaController();
    return NPT_SUCCESS;
}

int MediaRemote::setDMR(const char *uuid) {
    if(this->controller==NULL){
        return NPT_FAILURE;
    }

    this->controller->setDMR(uuid);
    return NPT_SUCCESS;
}

const char* MediaRemote::getDMR() {
    if(this->controller==NULL){
        return NULL;
    }
    return this->controller->getDMR();
}

int MediaRemote::setDMS(const char *uuid) {
    if(this->controller==NULL){
        return NPT_FAILURE;
    }

    this->controller->setDMS(uuid);
    return NPT_SUCCESS;
}

const char* MediaRemote::getDMS() {
    if(this->controller==NULL){
        return NULL;
    }
    return this->controller->getDMS();
}

int MediaRemote::startMediaController() {
    if(this->controller){
        this->stopMediaController();
    }
    if(NULL == this->controller){
        this->controller=new PLT_MicroMediaController();
        this->controller->setMediaControllerListener(this);
        return this->controller->startMediaController();
    }
    return NPT_FAILURE;
}

int MediaRemote::stopMediaController() {
    if(NULL == this->controller){
        return NPT_FAILURE;
    }
    int ret=this->controller->stopMediaController();
    delete this->controller;
    this->controller=NULL;
    return ret;
}

int MediaRemote::startMediaRender(const char *friendly_name,const char *uuid) {
    if(this->mediaRenderer){
        stopMediaRender();
    }
    if(NULL == this->mediaRenderer){
        this->mediaRenderer=new PLT_MicroMediaRenderer(friendly_name,uuid);
        this->mediaRenderer->setMediaRendererListener(this);
        return this->mediaRenderer->startMediaRenderer();
    }
    return NPT_FAILURE;
}

int MediaRemote::stopMediaRender() {
    if(NULL == this->mediaRenderer){
        return NPT_FAILURE;
    }
    int ret=this->mediaRenderer->stopMediaRenderer();
    delete this->mediaRenderer;
    this->mediaRenderer=NULL;
    return ret;
}

int MediaRemote::startMediaServer(const char *path, const char *friendly_name,const char *uuid) {
    if(this->mediaServer){
        stopMediaRender();
    }
    if(NULL == this->mediaServer){
        this->mediaServer=new PLT_MicroMediaServer(path,friendly_name,uuid);
        return this->mediaServer->startMediaServer();
    }
    return NPT_FAILURE;
}

int MediaRemote::stopMediaServer() {
    if(NULL == this->mediaServer)
        return NPT_FAILURE;
    int ret=this->mediaServer->stopMediaServer();
    delete this->mediaServer;
    this->mediaServer=NULL;
    return ret;
}

int MediaRemote::DoBrowse(const char *object_id, bool metadata){
    if(this->controller==NULL){
        return NPT_FAILURE;
    }

    this->controller->DoBrowse(object_id,metadata);
    return NPT_SUCCESS;
}

int MediaRemote::responseMRGenaEvent(int cmd, const char *value,const char *data) {
    if(this->mediaRenderer){
        return this->mediaRenderer->ResponseGenaEvent(cmd,value,data);
    }
    return NPT_FAILURE;
}

int MediaRemote::open(const char *url, const char *didl) {
    if(this->controller==NULL){
        return NPT_FAILURE;
    }

    this->controller->open(url, didl);
    return NPT_SUCCESS;
}

int MediaRemote::play() {
    if(this->controller==NULL){
        return NPT_FAILURE;
    }

    this->controller->play();
    return NPT_SUCCESS;
}

int MediaRemote::pause() {
    if(this->controller==NULL){
        return NPT_FAILURE;
    }

    this->controller->pause();
    return NPT_SUCCESS;
}

int MediaRemote::stop() {
    if(this->controller==NULL){
        return NPT_FAILURE;
    }

    this->controller->stop();
    return NPT_SUCCESS;
}

int MediaRemote::seek(int msec) {
    if(this->controller==NULL){
        return NPT_FAILURE;
    }
    int hour=0, min=0, sec=0;
    min = (msec/1000)/60;
    if(min>=60){
        hour = min/60;
        min = min%60;
    }else{
        hour = 0;
    }
    sec = (msec/1000)%60;

    char strpos[50];
    memset(strpos,0,sizeof(strpos));
    sprintf(strpos,"%02d:%02d:%02d",hour,min,sec);

    this->controller->seek(strpos);
    return NPT_SUCCESS;
}

int MediaRemote::setMute(bool yes) {
    if(this->controller==NULL){
        return NPT_FAILURE;
    }

    this->controller->setMute(yes);
    return NPT_SUCCESS;
}

int MediaRemote::getMute() {
    if(this->controller==NULL){
        return NPT_FAILURE;
    }

    this->controller->getMute();
    return NPT_SUCCESS;
}

int MediaRemote::setVolume(int vol) {
    if(this->controller==NULL){
        return NPT_FAILURE;
    }

    this->controller->setVolume(vol);
    return NPT_SUCCESS;
}

int MediaRemote::getVolume() {
    if(this->controller==NULL){
        return NPT_FAILURE;
    }

    this->controller->getVolume();
    return NPT_SUCCESS;
}

int MediaRemote::getVolumeRange(int &volMin, int &volMax) {
    if(this->controller==NULL){
        return NPT_FAILURE;
    }

    this->controller->getVolumeRange(volMin,volMax);
    return NPT_SUCCESS;
}

int MediaRemote::getDuration() {
    if(this->controller==NULL){
        return NPT_FAILURE;
    }

    this->controller->getMediaInfo();
    return NPT_SUCCESS;
}

int MediaRemote::getPosition() {
    if(this->controller==NULL){
        return NPT_FAILURE;
    }

    this->controller->getPositionInfo();
    return NPT_SUCCESS;
}

int MediaRemote::OnDeviceChange(device_change_event event,const char* uuid, const char* name, const char* type) {
    if(this->notify){
        this->notify->postDeviceChange(event,uuid,name,type);
    }
    return NPT_SUCCESS;
}

int MediaRemote::OnMSMediaBrowser(int result,PLT_MediaObjectListReference objectList) {
    if(this->notify){
        this->notify->postMSMediaBrowser(result,objectList);
    }
    return NPT_SUCCESS;
}

int MediaRemote::OnMRActionResponse(action_response_event event, int result, int value) {
    if(this->notify){
        this->notify->postMRActionResponse(event,result,value);
    }
    return NPT_SUCCESS;
}

int MediaRemote::OnMRActionRequest(MEDIA_RENDER_CTL_MSG event,const char *value,const char *data) {
    if(this->notify){
        this->notify->postMRActionRequest(event,value,data);
    }
    return NPT_SUCCESS;
}

int MediaRemote::OnMRStateVariablesChanged(state_change_event event,int value) {
    if(this->notify){
        this->notify->postMRStateVariablesChanged(event,value);
    }
    return NPT_SUCCESS;
}

int MediaRemote::OnMRMediaDataChanged(state_change_event event, const char *value) {
    if(this->notify){
        this->notify->postMRMediaDataChanged(event,value);
    }
    return NPT_SUCCESS;
}