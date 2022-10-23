package com.lj.dlnacontroller.api;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcelable;

import com.lj.dlnacontroller.main.LogHelper;

import java.lang.ref.WeakReference;
import java.util.ArrayList;

/**
 * Created by wh on 2017/12/9.
 */

final class MediaRemote {

    private static final String TAG="MediaRemote";
    static {
        try {
            System.loadLibrary("platinum-jni");
        }catch (Exception e){
            LogHelper.logStackTrace(TAG,e);
        }
    }

    private static final int DEVICE_CHANGE_EVENT=0x01;
    private static final int ACTION_RESPONSE_EVENT=0x02;
    private static final int STATE_VARIABLE_CHANGE_EVENT=0x03;
    private static final int ACTION_REQUEST_EVENT=0x04;
    private static final int MEDIA_BROWSER_EVENT=0x05;
    private static final int MEDIA_CHANGE_EVENT=0x06;

    private long mNativeContext;

    private EventHandler mEventHandler;

    private OnDeviceChangeListener mDeviceChangeListener;

    private OnMSMediaBrowserListener mMSMediaBrowserListener;

    private OnMRActionResponseListener mMRActionResponseListener;

    private OnMRStateVariablesChangedListener mMRStateVariablesChangedListener;

    private OnMRActionRequestListener mActionRequestListener;

    private static MediaRemote instance;

    static MediaRemote getInstance()throws InstantiationException{
        if(instance == null){
            synchronized (MediaRemote.class){
                if(instance == null){
                    instance = new MediaRemote();
                }
            }
        }
        return instance;
    }

    private MediaRemote() throws InstantiationException {
        Looper looper;
        if ((looper = Looper.myLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else if ((looper = Looper.getMainLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else {
            mEventHandler = null;
        }
        _setup(new WeakReference<MediaRemote>(this));
        if(mNativeContext == 0)
            throw new InstantiationException("init native context fail!");
    }

    void release(){
        _release();
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        _finalize();;
    }

    private native void _release();

    private native void _finalize();

    private native void _setup(Object sinadlna_this);

    native final int setDMR(String uuid);

    native final String getDMR() throws IllegalStateException;

    native final int setDMS(String uuid);

    native final String getDMS() throws IllegalStateException;

    native final int startMediaController();

    native final int stopMediaController();

    native final int startMediaRender(String friendly_name,String uuid)throws IllegalStateException, IllegalArgumentException,
            RuntimeException;

    native final int stopMediaRender()throws IllegalStateException;

    native final int startMediaServer(String path,String friendly_name,String uuid)throws IllegalStateException, IllegalArgumentException,
            RuntimeException;

    native final int stopMediaServer()throws IllegalStateException;

    native final int DoBrowse(String object_id, boolean metadata);

    native final int responseMRGenaEvent(int cmd, String value, String metaData);

    /**
     * 操作命令：设置播放URL
     * 参数: url 播放文件地址 ； didl 文件信息
     *  返回值：结果异步通知
     */
    native final void open(String url, String didl) throws IllegalStateException, IllegalArgumentException,
            RuntimeException;

    /**
     * 操作命令：开始播放
     * 参数:
     * 返回值：结果异步通知
     */
    native final void play() throws IllegalStateException;

    /**
     * 操作命令：停止播放
     * 参数:
     * 返回值：结果异步通知
     */
    native final void pause() throws IllegalStateException;

    /**
     * 操作命令：播放进度
     * 参数: msec 单位毫秒
     * 返回值：结果异步通知
     */
    native final void seek(int msec) throws IllegalStateException;

    /**
     * 操作命令：播放停止
     * 参数:
     * 返回值：结果异步通知
     */
    native final void stop() throws IllegalStateException;

    /**
     * 操作命令：设备静音控制
     * 参数: mute true 静音； false有音
     * 返回值：结果异步通知
     */
    native final void setMute(boolean mute) throws IllegalStateException;

    /**
     * 操作命令：获得当前静音设置
     * 参数:
     * 返回值：结果异步通知
     */
    native final void getMute() throws IllegalStateException;

    /**
     * 操作命令：获得设备最小音量
     * 参数:
     * 返回值：最小音量 -1 失败; >=0 期望值
     */
    native final int getVolumeMin() throws IllegalStateException;

    /**
     * 操作命令：获得设备最大音量
     * 参数:
     * 返回值：最大音量 -1 失败; >0 期望值
     */
    native final int getVolumeMax() throws IllegalStateException;

    /**
     * 操作命令：设置设备音量
     * 参数: vol音量大小 范围在最小音量和最大音量之间
     * 返回值：
     */
    native final void setVolume(int vol) throws IllegalStateException;

    /**
     * 操作命令：获得当前设备音量设置
     * 参数:
     * 返回值：结果异步通知
     */
    native final void getVolume() throws IllegalStateException;

    /**
     * 操作命令：获得当前播放文件的总时长
     * 参数:
     * 返回值：结果异步通知
     */
    native final void getDuration() throws IllegalStateException;

    /**
     * 操作命令：获得当前播放时间位置
     * 参数:
     * 返回值：结果异步通知
     */
    native final void getPosition() throws IllegalStateException;

    private static void postDeviceChange(Object remote_ref,int event,String uuid,String name,String type){
        LogHelper.logD(TAG,"postDeviceChange");
        MediaRemote remote = (MediaRemote) ((WeakReference) remote_ref).get();
        if(remote == null)return;
        if(remote.mEventHandler !=null){
            Device device=new Device(uuid,name,type);
            Message msg=remote.mEventHandler.obtainMessage(DEVICE_CHANGE_EVENT,event,0,device);
            remote.mEventHandler.sendMessage(msg);
        }
    }

    private static void postMSMediaBrowser(Object remote_ref, int result, ArrayList<MediaObject> objectList){
        MediaRemote remote = (MediaRemote) ((WeakReference) remote_ref).get();
        if(remote == null)return;
        if(remote.mEventHandler !=null){
            Message msg=remote.mEventHandler.obtainMessage(MEDIA_BROWSER_EVENT,result,0);
            Bundle bundle =msg.getData();
            LogHelper.logD(TAG,"list: "+objectList);
            bundle.putParcelableArrayList("list",objectList);
            remote.mEventHandler.sendMessage(msg);
        }
    }

    private static void postMRActionResponse(Object remote_ref,int event,int result,int value){
        MediaRemote remote = (MediaRemote) ((WeakReference) remote_ref).get();
        if(remote == null)return;
        if(remote.mEventHandler !=null){
            Message msg=remote.mEventHandler.obtainMessage(ACTION_RESPONSE_EVENT,event,result,value);
            remote.mEventHandler.sendMessage(msg);
        }
    }

    private static void postMRActionRequest(Object remote_ref,int event,String value,String data){
        MediaRemote remote = (MediaRemote) ((WeakReference) remote_ref).get();
        if(remote == null)return;
        if(remote.mEventHandler !=null){
            Message msg=remote.mEventHandler.obtainMessage(ACTION_REQUEST_EVENT,event,0);
            Bundle bundle=msg.getData();
            bundle.putString("value",value);
            bundle.putString("data",data);
            remote.mEventHandler.sendMessage(msg);
        }
    }

    private static void postMRStateVariablesChanged(Object remote_ref,int event,int value){
        MediaRemote remote = (MediaRemote) ((WeakReference) remote_ref).get();
        if(remote == null)return;
        if(remote.mEventHandler !=null){
            Message msg=remote.mEventHandler.obtainMessage(STATE_VARIABLE_CHANGE_EVENT,event,value);
            remote.mEventHandler.sendMessage(msg);
        }
    }

    private static void postMRMediaDataChanged(Object remote_ref,int event, String value){
        MediaRemote remote = (MediaRemote) ((WeakReference) remote_ref).get();
        if(remote == null)return;
        if(remote.mEventHandler !=null){
            Message msg=remote.mEventHandler.obtainMessage(MEDIA_CHANGE_EVENT,event,0,value);
            remote.mEventHandler.sendMessage(msg);
        }
    }

    void setOnDeviceChangeListener(OnDeviceChangeListener l){
        this.mDeviceChangeListener=l;
    }

    interface OnDeviceChangeListener{

        void onDeviceAdded(Device device);

        void onDeviceRemoved(Device device);
    }

    void setOnMSMediaBrowserListener(OnMSMediaBrowserListener l){
        this.mMSMediaBrowserListener=l;
    }

    interface OnMSMediaBrowserListener{
        void onBrowserResult(int result,ArrayList<MediaObject> objectList);
    }

    void setOnMRActionResponseListener(OnMRActionResponseListener l){
        this.mMRActionResponseListener=l;
    }

    interface OnMRActionResponseListener{

        void onOpen(boolean result);

        void onPlay(boolean result);

        void onPause(boolean result);

        void onStop(boolean result);

        void onSeek(boolean result);

        void onSetMute(boolean result);

        void onGetMute(boolean result, boolean mute);

        void onSetVolume(boolean result);

        void onGetVolume(boolean result, int vol);

        void onGetDuration(boolean result, int msec);

        void onGetPosition(boolean result, int msec);
    }

    void setOnMRStateVariablesChangedListener(OnMRStateVariablesChangedListener l){
        this.mMRStateVariablesChangedListener=l;
    }

    interface OnMRStateVariablesChangedListener{

        void onAVTransportURI(String uri);

        void onAVTransportURIMetadata(String metaData);

        void onPlay();

        void onPause();

        void onStop();

        void onMuteChange(boolean mute);

        void onVolumeChange(int vol);

        void onDurationChange(int seconds);

        void onPositionChange(int seconds);
    }

    void setOnMRActionRequestListener(OnMRActionRequestListener l){
        this.mActionRequestListener=l;
    }
    interface OnMRActionRequestListener{
        void open(String uri,String metaData);
        void stop();
        void play();
        void pause();
        void seekTo(String position);
        void previous();
        void next();
        void setVolume(String volume);
        void setMute(String mute);
        void setVolumeDB(String volumeDB);
        void getVolumeDBRange();
    }

    private static class EventHandler extends Handler{
        private WeakReference<MediaRemote> reference;

        public EventHandler(MediaRemote remote, Looper looper){
            super(looper);
            reference = new WeakReference<MediaRemote>(remote);
        }

        @Override
        public void handleMessage(Message msg) {
            MediaRemote mRemote = reference.get();
            if(mRemote == null)return;
            if(mRemote.mNativeContext == 0){
                LogHelper.logD(TAG,"mDLNA went away with unhandled events");
                return;
            }
            switch (msg.what){
                case DEVICE_CHANGE_EVENT:
                    if(mRemote.mDeviceChangeListener != null){
                        if(msg.arg1 == DeviceEvent.DEVICE_EVENT_ADDED){
                            mRemote.mDeviceChangeListener.onDeviceAdded((Device) msg.obj);
                        }else if(msg.arg1 == DeviceEvent.DEVICE_EVENT_REMOVED){
                            mRemote.mDeviceChangeListener.onDeviceRemoved((Device) msg.obj);
                        }
                    }
                    break;
                case ACTION_RESPONSE_EVENT:
                    if(mRemote.mMRActionResponseListener!=null){
                        switch (msg.arg1){
                            case ResponseEvent.ACTION_RESPONSE_OPEN:
                                mRemote.mMRActionResponseListener.onOpen(msg.arg2 == 0);
                                break;
                            case ResponseEvent.ACTION_RESPONSE_PLAY:
                                mRemote.mMRActionResponseListener.onPlay(msg.arg2 == 0);
                                break;
                            case ResponseEvent.ACTION_RESPONSE_PAUSE:
                                mRemote.mMRActionResponseListener.onPause(msg.arg2 == 0);
                                break;
                            case ResponseEvent.ACTION_RESPONSE_STOP:
                                mRemote.mMRActionResponseListener.onStop(msg.arg2 == 0);
                                break;
                            case ResponseEvent.ACTION_RESPONSE_SEEK:
                                mRemote.mMRActionResponseListener.onSeek(msg.arg2 == 0);
                                break;
                            case ResponseEvent.ACTION_RESPONSE_SET_MUTE:
                                mRemote.mMRActionResponseListener.onSetMute(msg.arg2 == 0);
                                break;
                            case ResponseEvent.ACTION_RESPONSE_GET_MUTE:
                                mRemote.mMRActionResponseListener.onGetMute(msg.arg2 == 0,  (Integer) msg.obj == 1);
                                break;
                            case ResponseEvent.ACTION_RESPONSE_SET_VOLUME:
                                mRemote.mMRActionResponseListener.onSetVolume(msg.arg2 == 0);
                                break;
                            case ResponseEvent.ACTION_RESPONSE_GET_VOLUME:
                                mRemote.mMRActionResponseListener.onGetVolume(msg.arg2 == 0, (Integer) msg.obj);
                                break;
                            case ResponseEvent.ACTION_RESPONSE_DURATION:
                                mRemote.mMRActionResponseListener.onGetDuration(msg.arg2==0, (Integer) msg.obj);
                                break;
                            case ResponseEvent.ACTION_RESPONSE_POSITION:
                                mRemote.mMRActionResponseListener.onGetPosition(msg.arg2==0, (Integer) msg.obj);
                                break;
                        }
                    }
                    break;
                case STATE_VARIABLE_CHANGE_EVENT:
                    if(mRemote.mMRStateVariablesChangedListener!=null){
                        switch (msg.arg1){
                            case StateEvent.PLAYBACK_STATE_CHANGE_PLAY:
                                mRemote.mMRStateVariablesChangedListener.onPlay();
                                break;
                            case StateEvent.PLAYBACK_STATE_CHANGE_PAUSE:
                                mRemote.mMRStateVariablesChangedListener.onPause();
                                break;
                            case StateEvent.PLAYBACK_STATE_CHANGE_STOP:
                                mRemote.mMRStateVariablesChangedListener.onStop();
                                break;
                            case StateEvent.PLAYBACK_STATE_CHANGE_MUTE:
                                mRemote.mMRStateVariablesChangedListener.onMuteChange(msg.arg2 == 1);
                                break;
                            case StateEvent.PLAYBACK_STATE_CHANGE_VOLUME:
                                mRemote.mMRStateVariablesChangedListener.onVolumeChange(msg.arg2);
                                break;
                            case StateEvent.PLAYBACK_STATE_CHANGE_DURATION:
                                mRemote.mMRStateVariablesChangedListener.onDurationChange(msg.arg2);
                                break;
                            case StateEvent.PLAYBACK_STATE_CHANGE_POSITION:
                                mRemote.mMRStateVariablesChangedListener.onPositionChange(msg.arg2);
                                break;
                        }
                    }
                    break;
                case ACTION_REQUEST_EVENT:
                    if(mRemote.mActionRequestListener!=null){
                        Bundle bundle=msg.getData();
                        String value=bundle.getString("value");
                        String metaData=bundle.getString("data");
                        switch (msg.arg1){
                            case RenderRequestEvent.MEDIA_RENDER_CTL_MSG_SET_AV_URL:
                                mRemote.mActionRequestListener.open(value,metaData);
                                break;
                            case RenderRequestEvent.MEDIA_RENDER_CTL_MSG_STOP:
                                mRemote.mActionRequestListener.stop();
                                break;
                            case RenderRequestEvent.MEDIA_RENDER_CTL_MSG_PLAY:
                                mRemote.mActionRequestListener.play();
                                break;
                            case RenderRequestEvent.MEDIA_RENDER_CTL_MSG_PAUSE:
                                mRemote.mActionRequestListener.pause();
                                break;
                            case RenderRequestEvent.MEDIA_RENDER_CTL_MSG_SEEK:
                                mRemote.mActionRequestListener.seekTo(value);
                                break;
                            case RenderRequestEvent.MEDIA_RENDER_CTL_MSG_SETVOLUME:
                                mRemote.mActionRequestListener.setVolume(value);
                                break;
                            case RenderRequestEvent.MEDIA_RENDER_CTL_MSG_SETMUTE:
                                mRemote.mActionRequestListener.setMute(value);
                                break;
                            case RenderRequestEvent.MEDIA_RENDER_CTL_MSG_PRE:
                                mRemote.mActionRequestListener.previous();
                                break;
                            case RenderRequestEvent.MEDIA_RENDER_CTL_MSG_NEXT:
                                mRemote.mActionRequestListener.next();
                                break;
                            case RenderRequestEvent.MEDIA_RENDER_CTL_MSG_SETVOLUMEDB:
                                mRemote.mActionRequestListener.setVolumeDB(value);
                                break;
                            case RenderRequestEvent.MEDIA_RENDER_CTL_MSG_GETVOLUMEDBRANGE:
                                mRemote.mActionRequestListener.getVolumeDBRange();
                                break;
                        }
                    }
                    break;
                case MEDIA_BROWSER_EVENT:
                    if(mRemote.mMSMediaBrowserListener!=null){
                        ArrayList<MediaObject> objectList = msg.getData().getParcelableArrayList("list");
                        mRemote.mMSMediaBrowserListener.onBrowserResult(msg.arg1,objectList);
                    }
                    break;
                case MEDIA_CHANGE_EVENT:
                    if(mRemote.mMRStateVariablesChangedListener!=null){
                        if(msg.arg1 == StateEvent.PLAYBACK_STATE_CHANGE_AV_TRANSPORT_URI){
                            mRemote.mMRStateVariablesChangedListener.onAVTransportURI(msg.obj == null?null:msg.obj.toString());
                        }else if(msg.arg1 == StateEvent.PLAYBACK_STATE_CHANGE_AV_TRANSPORT_URI_METADATA){
                            mRemote.mMRStateVariablesChangedListener.onAVTransportURIMetadata(msg.obj == null?null:msg.obj.toString());
                        }
                    }
                    break;
            }
        }
    }

    private class DeviceEvent{
        static final int DEVICE_EVENT_ADDED = 1;
        static final int DEVICE_EVENT_REMOVED= 2;
    }

    private class ResponseEvent{
        static final int ACTION_RESPONSE_OPEN 		= 1;  //设置播放URL命令的结果事件
        static final int ACTION_RESPONSE_PLAY 		= 2;  //播放开始命令的结果事件
        static final int ACTION_RESPONSE_PAUSE 	    = 3;  //播放暂停命令的结果事件
        static final int ACTION_RESPONSE_STOP 		= 4;  //播放停止命令的结果事件
        static final int ACTION_RESPONSE_SEEK 		= 5;  //播放进度命令的结果事件
        static final int ACTION_RESPONSE_SET_MUTE 	= 6;  //静音设置命令的结果事件
        static final int ACTION_RESPONSE_GET_MUTE 	= 7;  //获得静音设置命令的结果事件
        static final int ACTION_RESPONSE_SET_VOLUME 	= 8;  //音量设置命令的结果事件
        static final int ACTION_RESPONSE_GET_VOLUME 	= 9;  //获得音量设置命令的结果事件
        static final int ACTION_RESPONSE_DURATION 	= 10; //获得视频总时长命令的结果事件
        static final int ACTION_RESPONSE_POSITION 	= 11; //获得当前时间位置命令的结果事件
    }

    class  RenderRequestEvent{
        static final int MEDIA_RENDER_CTL_MSG_SET_AV_URL =0;
        static final int MEDIA_RENDER_CTL_MSG_STOP =1;
        static final int MEDIA_RENDER_CTL_MSG_PLAY =2;
        static final int MEDIA_RENDER_CTL_MSG_PAUSE =3;
        static final int MEDIA_RENDER_CTL_MSG_SEEK =4;
        static final int MEDIA_RENDER_CTL_MSG_SETVOLUME =5;
        static final int MEDIA_RENDER_CTL_MSG_SETMUTE =6;
        static final int MEDIA_RENDER_CTL_MSG_PRE =7;
        static final int MEDIA_RENDER_CTL_MSG_NEXT =8;
        static final int MEDIA_RENDER_CTL_MSG_SETVOLUMEDB =9;
        static final int MEDIA_RENDER_CTL_MSG_GETVOLUMEDBRANGE =10;
        static final int MEDIA_RENDER_CTL_MSG_DURATION =11;
        static final int MEDIA_RENDER_CTL_MSG_POSITION =12;
    }

    private class StateEvent{
        static final int PLAYBACK_STATE_CHANGE_AV_TRANSPORT_URI=1;
        static final int PLAYBACK_STATE_CHANGE_PLAY =2;
        static final int PLAYBACK_STATE_CHANGE_PAUSE =3;
        static final int PLAYBACK_STATE_CHANGE_STOP =4;
        static final int PLAYBACK_STATE_CHANGE_MUTE =5;
        static final int PLAYBACK_STATE_CHANGE_VOLUME =6;
        static final int PLAYBACK_STATE_CHANGE_DURATION =7;
        static final int PLAYBACK_STATE_CHANGE_POSITION =8;
        static final int PLAYBACK_STATE_CHANGE_AV_TRANSPORT_URI_METADATA=9;
    }
}
