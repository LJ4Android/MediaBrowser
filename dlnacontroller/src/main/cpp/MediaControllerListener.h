//
// Created by wh on 2017/12/10.
//

#ifndef MEDIABROWSER_MEDIACONTROLLERLISTENER_H
#define MEDIABROWSER_MEDIACONTROLLERLISTENER_H

class MediaControllerListener{

public:

    enum device_change_event{
        DEVICE_EVENT_ADDED = 1,
        DEVICE_EVENT_REMOVED= 2
    };

    enum action_response_event{
        ACTION_RESPONSE_OPEN 		= 1,  //设置播放URL命令的结果事件
        ACTION_RESPONSE_PLAY 		= 2,  //播放开始命令的结果事件
        ACTION_RESPONSE_PAUSE 	= 3,  //播放暂停命令的结果事件
        ACTION_RESPONSE_STOP 		= 4,  //播放停止命令的结果事件
        ACTION_RESPONSE_SEEK 		= 5,  //播放进度命令的结果事件
        ACTION_RESPONSE_SET_MUTE 	= 6,  //静音设置命令的结果事件
        ACTION_RESPONSE_GET_MUTE 	= 7,  //获得静音设置命令的结果事件
        ACTION_RESPONSE_SET_VOLUME 	= 8,  //音量设置命令的结果事件
        ACTION_RESPONSE_GET_VOLUME 	= 9,  //获得音量设置命令的结果事件
        ACTION_RESPONSE_DURATION 	= 10, //获得视频总时长命令的结果事件
        ACTION_RESPONSE_POSITION 	= 11  //获得当前时间位置命令的结果事件
    };

    enum state_change_event{
        PLAYBACK_STATE_CHANGE_AV_TRANSPORT_URI=1,
        PLAYBACK_STATE_CHANGE_PLAY =2,
        PLAYBACK_STATE_CHANGE_PAUSE =3,
        PLAYBACK_STATE_CHANGE_STOP =4,
        PLAYBACK_STATE_CHANGE_MUTE =5,
        PLAYBACK_STATE_CHANGE_VOLUME =6,
        PLAYBACK_STATE_CHANGE_DURATION =7,
        PLAYBACK_STATE_CHANGE_POSITION =8,
        PLAYBACK_STATE_CHANGE_AV_TRANSPORT_URI_METADATA=9
    };

    virtual ~MediaControllerListener() {}
    virtual int OnDeviceChange(device_change_event event,const char* uuid, const char* name, const char* type) = 0;
    virtual int OnMSMediaBrowser(int result,PLT_MediaObjectListReference objectList) = 0;
    virtual int OnMRActionResponse(action_response_event event, int result, int value) = 0;
    virtual int OnMRStateVariablesChanged(state_change_event event,int value) = 0;
    virtual int OnMRMediaDataChanged(state_change_event event,const char* value) = 0;
};

#endif //MEDIABROWSER_MEDIACONTROLLERLISTENER_H
