//
// Created by wh on 2017/12/10.
//

#ifndef MEDIABROWSER_MEDIAREMOTE_H
#define MEDIABROWSER_MEDIAREMOTE_H

#include <jni.h>
#include "PltMicroMediaController.h"
#include "PltMicroMediaRenderer.h"
#include "PltMicroMediaServer.h"
#include "MediaRemote_notify.h"
#include "MediaControllerListener.h"
#include "MediaRendererListener.h"

class MediaRemote : public MediaControllerListener,public MediaRendererListener{

public:
    MediaRemote();
    virtual ~MediaRemote();

    int setup(JavaVM* jvm, jobject thiz, jobject weak_thiz);
    int release();
    int  setDMR(const char *uuid); //set currenr mr by uuid
    const char*  getDMR(); //get current mr uuid

    int  setDMS(const char *uuid);
    const char*  getDMS();
    int startMediaController();
    int stopMediaController();
    int startMediaRender(const char* friendly_name,const char *uuid);
    int stopMediaRender();
    int startMediaServer(const char *path,const char *friendly_name,const char *uuid);
    int stopMediaServer();
    int DoBrowse(const char *object_id, bool metadata);
    int responseMRGenaEvent(int cmd,const char *value,const char *data);
    int open(const char* url, const char* didl);  //open play url
    int play(); //start play
    int stop();  //stop play
    int pause();  //pause play
    int seek(int msec); //seek to msec position
    int setMute(bool yes);  //set mute/unmute voice
    int getMute();  //get current mute state
    int setVolume(int vol);  //set volume
    int getVolume();     //get current volume
    int getVolumeRange(int& volMin, int& volMax);     //get volume range
    int getDuration(); //get media duration, ms
    int getPosition();  //get current position, ms

    int OnDeviceChange(device_change_event event,const char* uuid, const char* name, const char* type);
    int OnMSMediaBrowser(int result,PLT_MediaObjectListReference objectList);
    int OnMRActionResponse(action_response_event event, int result, int value);
    int OnMRActionRequest(MEDIA_RENDER_CTL_MSG event,const char *value,const char *data);
    int OnMRStateVariablesChanged(state_change_event event,int value);
    int OnMRMediaDataChanged(state_change_event event,const char* value);

private:
    PLT_MicroMediaController *controller;
    PLT_MicroMediaRenderer* mediaRenderer;
    PLT_MicroMediaServer* mediaServer;
    JNIMediaRemoteEventNotify* notify;
};
class MediaRemoteHelper
{
public:
    static MediaRemote* create();
    static void destroy(MediaRemote* dlna);
};

#endif //MEDIABROWSER_MEDIAREMOTE_H
