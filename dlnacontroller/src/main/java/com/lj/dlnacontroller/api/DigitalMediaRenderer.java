package com.lj.dlnacontroller.api;

import android.os.Build;
import android.text.TextUtils;

/**
 * Created by wh on 2017/12/13.
 */

public class DigitalMediaRenderer {
    private int NPT_SUCCESS=0;

    private String friendly_name= Build.MODEL+"_DMR";
    private String uuid=Build.SERIAL+"_dmr";
    private MediaRemote mRemote;

    public DigitalMediaRenderer(String friendly_name,String uuid)throws InstantiationException{
        mRemote=MediaRemote.getInstance();
        if(!TextUtils.isEmpty(friendly_name)){
            this.friendly_name=friendly_name;
        }
        if(!TextUtils.isEmpty(uuid)){
            this.uuid=uuid;
        }
    }

    public boolean start(){
       setup();
        return mRemote.startMediaRender(friendly_name,uuid) == NPT_SUCCESS;
    }

    public boolean stop(){
        return mRemote.stopMediaRender() == NPT_SUCCESS;
    }

    private void setup(){
        mRemote.setOnMRActionRequestListener(mrActionRequestListener);
    }

    public void  release(){
        mRemote.setOnMRActionRequestListener(null);
        mOnMRActionRequestListener=null;
    }

    public boolean responseOpen(String uri, String metaData){
        return mRemote.responseMRGenaEvent(MediaRemote.RenderRequestEvent.MEDIA_RENDER_CTL_MSG_SET_AV_URL,uri,metaData) == NPT_SUCCESS;
    }
    public boolean responsePlay(){
        return mRemote.responseMRGenaEvent(MediaRemote.RenderRequestEvent.MEDIA_RENDER_CTL_MSG_PLAY,"","") == NPT_SUCCESS;
    }
    public boolean responsePause(){
        return mRemote.responseMRGenaEvent(MediaRemote.RenderRequestEvent.MEDIA_RENDER_CTL_MSG_PAUSE,"","") == NPT_SUCCESS;
    }
    public boolean responseStop(){
        return mRemote.responseMRGenaEvent(MediaRemote.RenderRequestEvent.MEDIA_RENDER_CTL_MSG_STOP,"","") == NPT_SUCCESS;
    }
    public boolean responseMuteChange(boolean mute){
        return mRemote.responseMRGenaEvent(MediaRemote.RenderRequestEvent.MEDIA_RENDER_CTL_MSG_SETMUTE,"","") == NPT_SUCCESS;
    }
    public boolean responseVolumeChange(int vol){
        return mRemote.responseMRGenaEvent(MediaRemote.RenderRequestEvent.MEDIA_RENDER_CTL_MSG_SETVOLUME,"","") == NPT_SUCCESS;
    }
    public boolean responseDurationChange(int duration){
        return mRemote.responseMRGenaEvent(MediaRemote.RenderRequestEvent.MEDIA_RENDER_CTL_MSG_DURATION,"","") == NPT_SUCCESS;
    }
    public boolean responsePositionChange(int position){
        return mRemote.responseMRGenaEvent(MediaRemote.RenderRequestEvent.MEDIA_RENDER_CTL_MSG_POSITION,"","") == NPT_SUCCESS;
    }

    private OnMRActionRequestListener mOnMRActionRequestListener;
    public void setOnMRActionRequestListener(OnMRActionRequestListener l){
        this.mOnMRActionRequestListener=l;
    }
    public interface OnMRActionRequestListener {
        void open(String uri, String metaData) ;

        void stop() ;

        void play() ;

        void pause();

        void seekTo(int position);

        void previous();

        void next();

        void setVolume(int volume) ;

        void setMute(boolean mute);

        void setVolumeDB(int volumeDB) ;

        void getVolumeDBRange() ;
    }

    private MediaRemote.OnMRActionRequestListener mrActionRequestListener=new MediaRemote.OnMRActionRequestListener() {
        @Override
        public void open(String uri, String metaData) {
            if(mOnMRActionRequestListener != null)
                mOnMRActionRequestListener.open(uri,metaData);
        }

        @Override
        public void stop() {
            if(mOnMRActionRequestListener != null)
                mOnMRActionRequestListener.stop();
        }

        @Override
        public void play() {
            if(mOnMRActionRequestListener != null)
                mOnMRActionRequestListener.play();
        }

        @Override
        public void pause() {
            if(mOnMRActionRequestListener != null)
                mOnMRActionRequestListener.pause();
        }

        @Override
        public void seekTo(String position) {
            if(mOnMRActionRequestListener != null)
                mOnMRActionRequestListener.seekTo(100);
        }

        @Override
        public void previous() {
            if(mOnMRActionRequestListener != null)
                mOnMRActionRequestListener.previous();
        }

        @Override
        public void next() {
            if(mOnMRActionRequestListener != null)
                mOnMRActionRequestListener.next();
        }

        @Override
        public void setVolume(String volume) {
            if(mOnMRActionRequestListener != null)
                mOnMRActionRequestListener.setVolume(10);
        }

        @Override
        public void setMute(String mute) {
            if(mOnMRActionRequestListener != null)
                mOnMRActionRequestListener.setMute(true);
        }

        @Override
        public void setVolumeDB(String volumeDB) {
            if(mOnMRActionRequestListener != null)
                mOnMRActionRequestListener.setVolumeDB(10);
        }

        @Override
        public void getVolumeDBRange() {
            if(mOnMRActionRequestListener != null)
                mOnMRActionRequestListener.getVolumeDBRange();
        }
    };
}
