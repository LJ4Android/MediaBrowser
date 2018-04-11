//
// Created by wh on 2017/12/10.
//

#ifndef MEDIABROWSER_MEDIAREMOTE_NOTIFY_H
#define MEDIABROWSER_MEDIAREMOTE_NOTIFY_H

#include <android/log.h>
#include <jni.h>
#include <PltMediaItem.h>

class JNIMediaRemoteEventNotify
{
public:
    JNIMediaRemoteEventNotify(JavaVM* jvm_, jobject thiz, jobject weak_thiz);
    ~JNIMediaRemoteEventNotify();
    void postDeviceChange(int event,const char* uuid, const char* name, const char* type);
    void postMSMediaBrowser(int result,PLT_MediaObjectListReference objectList);
    void postMRActionResponse(int event, int result, int value);
    void postMRActionRequest(int event,const char *value,const char *data);
    void postMRStateVariablesChanged(int event, int value);
    void postMRMediaDataChanged(int event, const char *value);
private:
    jclass      mClass;     // Reference to SinaDLNA class
    jobject     mObject;    // Weak ref to SinaDLNA Java object to call on
    JavaVM* 	jvm;
    jmethodID   post_device_event;
    jmethodID   post_browser_event;
    jmethodID   post_mr_response_event;
    jmethodID   post_mr_request_event;
    jmethodID   post_mr_state_event;
    jmethodID   post_mr_media_event;
};

#endif //MEDIABROWSER_MEDIAREMOTE_NOTIFY_H
