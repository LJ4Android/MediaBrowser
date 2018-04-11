package com.lj.dlnacontroller.api;

import java.util.ArrayList;

/**
 * Created by wh on 2017/12/13.
 */

public class DigitalMediaController{

    private int NPT_SUCCESS=0;

    private MediaRemote mRemote;

    public DigitalMediaController()throws InstantiationException{
        mRemote=MediaRemote.getInstance();
    }

    public boolean start(){
        setup();
        return mRemote.startMediaController() == NPT_SUCCESS;
    }

    public boolean stop(){
        return mRemote.stopMediaController() == NPT_SUCCESS;
    }

    private void setup(){
        mRemote.setOnMRStateVariablesChangedListener(stateVariablesChangedListener);
        mRemote.setOnMRActionResponseListener(actionResponseListener);
        mRemote.setOnDeviceChangeListener(deviceChangeListener);
        mRemote.setOnMSMediaBrowserListener(mediaBrowserListener);
    }

    public void release(){
        mRemote.setOnDeviceChangeListener(null);
        mRemote.setOnMRActionResponseListener(null);
        mRemote.setOnMRStateVariablesChangedListener(null);
        mOnMRActionResponseListener=null;
        mOnDeviceChangeListener=null;
        mOnMRStateVariablesChangedListener=null;
    }

    public int setDMR(String uuid){
        return mRemote.setDMR(uuid);
    };

    public String getDMR() throws IllegalStateException{
        return mRemote.getDMR();
    };

    public int setDMS(String uuid){
        return mRemote.setDMS(uuid);
    };

    public String getDMS() throws IllegalStateException{
        return mRemote.getDMS();
    };

    public int DoBrowse(String object_id, boolean metadata){
        return mRemote.DoBrowse(object_id,metadata);
    }
    /**
     * 操作命令：设置播放URL
     * 参数: url 播放文件地址 ； didl 文件信息
     *  返回值：结果异步通知
     */
    public void cmdOpen(String url, String didl) throws IllegalStateException, IllegalArgumentException,
            RuntimeException{
        mRemote.open(url,didl);
    };

    /**
     * 操作命令：开始播放
     * 参数:
     * 返回值：结果异步通知
     */
    public void cmdPlay() throws IllegalStateException{
        mRemote.play();
    };

    /**
     * 操作命令：停止播放
     * 参数:
     * 返回值：结果异步通知
     */
    public void cmdPause() throws IllegalStateException{
        mRemote.pause();
    };

    /**
     * 操作命令：播放进度
     * 参数: msec 单位毫秒
     * 返回值：结果异步通知
     */
    public void cmdSeek(int msec) throws IllegalStateException{
        mRemote.seek(msec);
    };

    /**
     * 操作命令：播放停止
     * 参数:
     * 返回值：结果异步通知
     */
    public void cmdStop() throws IllegalStateException{
        mRemote.stop();
    };

    /**
     * 操作命令：设备静音控制
     * 参数: mute true 静音； false有音
     * 返回值：结果异步通知
     */
    public void cmdSetMute(boolean mute) throws IllegalStateException{
        mRemote.setMute(mute);
    };

    /**
     * 操作命令：获得当前静音设置
     * 参数:
     * 返回值：结果异步通知
     */
    public void cmdGetMute() throws IllegalStateException{
        mRemote.getMute();
    };

    /**
     * 操作命令：获得设备最小音量
     * 参数:
     * 返回值：最小音量 -1 失败; >=0 期望值
     */
    public int cmdGetVolumeMin() throws IllegalStateException{
        return mRemote.getVolumeMin();
    };

    /**
     * 操作命令：获得设备最大音量
     * 参数:
     * 返回值：最大音量 -1 失败; >0 期望值
     */
    public int cmdGetVolumeMax() throws IllegalStateException{
        return mRemote.getVolumeMax();
    };

    /**
     * 操作命令：设置设备音量
     * 参数: vol音量大小 范围在最小音量和最大音量之间
     * 返回值：
     */
    public void cmdSetVolume(int vol) throws IllegalStateException{
        mRemote.setVolume(vol);
    };

    /**
     * 操作命令：获得当前设备音量设置
     * 参数:
     * 返回值：结果异步通知
     */
    public void cmdGetVolume() throws IllegalStateException{
        mRemote.getVolume();
    };

    /**
     * 操作命令：获得当前播放文件的总时长
     * 参数:
     * 返回值：结果异步通知
     */
    public void cmdGetDuration() throws IllegalStateException{
        mRemote.getDuration();
    };

    /**
     * 操作命令：获得当前播放时间位置
     * 参数:
     * 返回值：结果异步通知
     */
    public void cmdGetPosition() throws IllegalStateException{
        mRemote.getPosition();
    };

    private MediaRemote.OnMRActionResponseListener actionResponseListener=new MediaRemote.OnMRActionResponseListener() {
        @Override
        public void onOpen(boolean result) {
            if(mOnMRActionResponseListener!=null)
                mOnMRActionResponseListener.onOpen(result);
        }

        @Override
        public void onPlay(boolean result) {
            if(mOnMRActionResponseListener!=null)
                mOnMRActionResponseListener.onPlay(result);
        }

        @Override
        public void onPause(boolean result) {
            if(mOnMRActionResponseListener!=null)
                mOnMRActionResponseListener.onPause(result);
        }

        @Override
        public void onStop(boolean result) {
            if(mOnMRActionResponseListener!=null)
                mOnMRActionResponseListener.onStop(result);
        }

        @Override
        public void onSeek(boolean result) {
            if(mOnMRActionResponseListener!=null)
                mOnMRActionResponseListener.onSeek(result);
        }

        @Override
        public void onSetMute(boolean result) {
            if(mOnMRActionResponseListener!=null)
                mOnMRActionResponseListener.onSetMute(result);
        }

        @Override
        public void onGetMute(boolean result, boolean mute) {
            if(mOnMRActionResponseListener!=null)
                mOnMRActionResponseListener.onGetMute(result,mute);
        }

        @Override
        public void onSetVolume(boolean result) {
            if(mOnMRActionResponseListener!=null)
                mOnMRActionResponseListener.onSetVolume(result);
        }

        @Override
        public void onGetVolume(boolean result, int vol) {
            if(mOnMRActionResponseListener!=null)
                mOnMRActionResponseListener.onGetVolume(result,vol);
        }

        @Override
        public void onGetDuration(boolean result, int msec) {
            if(mOnMRActionResponseListener!=null)
                mOnMRActionResponseListener.onGetDuration(result,msec);
        }

        @Override
        public void onGetPosition(boolean result, int msec) {
            if(mOnMRActionResponseListener!=null)
                mOnMRActionResponseListener.onGetPosition(result,msec);
        }
    };

    private MediaRemote.OnMRStateVariablesChangedListener stateVariablesChangedListener=new MediaRemote.OnMRStateVariablesChangedListener() {

        @Override
        public void onAVTransportURI(String uri) {
            if(mOnMRStateVariablesChangedListener!=null)
                mOnMRStateVariablesChangedListener.onAVTransportURI(uri);
        }

        @Override
        public void onAVTransportURIMetadata(String metaData) {
            if(mOnMRStateVariablesChangedListener!=null)
                mOnMRStateVariablesChangedListener.onAVTransportURIMetadata(metaData);
        }

        @Override
        public void onPlay() {
            if(mOnMRStateVariablesChangedListener!=null)
                mOnMRStateVariablesChangedListener.onPlay();
        }

        @Override
        public void onPause() {
            if(mOnMRStateVariablesChangedListener!=null)
                mOnMRStateVariablesChangedListener.onPause();
        }

        @Override
        public void onStop() {
            if(mOnMRStateVariablesChangedListener!=null)
                mOnMRStateVariablesChangedListener.onStop();
        }

        @Override
        public void onMuteChange(boolean mute) {
            if(mOnMRStateVariablesChangedListener!=null)
                mOnMRStateVariablesChangedListener.onMuteChange(mute);
        }

        @Override
        public void onVolumeChange(int vol) {
            if(mOnMRStateVariablesChangedListener!=null)
                mOnMRStateVariablesChangedListener.onVolumeChange(vol);
        }

        @Override
        public void onDurationChange(int seconds) {
            if(mOnMRStateVariablesChangedListener!=null)
                mOnMRStateVariablesChangedListener.onDurationChange(seconds);
        }

        @Override
        public void onPositionChange(int seconds) {
            if(mOnMRStateVariablesChangedListener!=null)
                mOnMRStateVariablesChangedListener.onPositionChange(seconds);
        }
    };

    private MediaRemote.OnDeviceChangeListener deviceChangeListener=new MediaRemote.OnDeviceChangeListener() {
        @Override
        public void onDeviceAdded(Device device) {
            if(mOnDeviceChangeListener!=null)
                mOnDeviceChangeListener.onDeviceAdded(device);
        }

        @Override
        public void onDeviceRemoved(Device device) {
            if(mOnDeviceChangeListener!=null)
                mOnDeviceChangeListener.onDeviceRemoved(device);
        }
    };

    private MediaRemote.OnMSMediaBrowserListener mediaBrowserListener=new MediaRemote.OnMSMediaBrowserListener() {
        @Override
        public void onBrowserResult(int result, ArrayList<MediaObject> objectList) {
            if(mMediaBrowserListener!=null)
                mMediaBrowserListener.onBrowserResult(result,objectList);
        }
    };

    private OnMRActionResponseListener mOnMRActionResponseListener;
    @Deprecated
    // warning: "Deprecated member is still used"
    public void setOnMRActionResponseListener(OnMRActionResponseListener l){
        this.mOnMRActionResponseListener=l;
    }
    public interface OnMRActionResponseListener extends MediaRemote.OnMRActionResponseListener{}

    private OnDeviceChangeListener mOnDeviceChangeListener;
    public void setOnDeviceChangeListener(OnDeviceChangeListener l){
        this.mOnDeviceChangeListener=l;
    }
    public interface OnDeviceChangeListener extends MediaRemote.OnDeviceChangeListener{}

    private OnMRStateVariablesChangedListener mOnMRStateVariablesChangedListener;
    public void setOnMRStateVariablesChangedListener(OnMRStateVariablesChangedListener l){
        this.mOnMRStateVariablesChangedListener =l;
    }
    public interface OnMRStateVariablesChangedListener extends MediaRemote.OnMRStateVariablesChangedListener{}

    public interface OnMSMediaBrowserListener extends MediaRemote.OnMSMediaBrowserListener{}
    private OnMSMediaBrowserListener mMediaBrowserListener;
    public void setOnMSMediaBrowserListener(OnMSMediaBrowserListener l){
        this.mMediaBrowserListener=l;
    }
}
