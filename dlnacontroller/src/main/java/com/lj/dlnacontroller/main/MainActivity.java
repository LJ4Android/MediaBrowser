package com.lj.dlnacontroller.main;

import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Environment;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.MediaController;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.VideoView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import com.lj.dlnacontroller.R;
import com.lj.dlnacontroller.api.Device;
import com.lj.dlnacontroller.api.DigitalMediaController;
import com.lj.dlnacontroller.api.DigitalMediaRenderer;
import com.lj.dlnacontroller.api.DigitalMediaServer;
import com.lj.dlnacontroller.api.MediaObject;

import java.io.File;
import java.util.ArrayList;

public class MainActivity extends AppCompatActivity {

    private static String TAG= "MainActivity";
    private DigitalMediaController mDMC;
    private DigitalMediaRenderer mDMR;
    private DigitalMediaServer mDMS;
    private VideoView videoView;
    private boolean isSetup;
    private boolean isStartDMR;
    private boolean isStartDMS;
    private String path= Environment.getExternalStorageDirectory().getAbsolutePath();
    private String videoPath= Environment.getExternalStorageDirectory().getAbsolutePath()+ File.separator+"a.mp4";
    WifiAdmin wifiAdmin;
    WifiManager.MulticastLock multicastLock;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        LogHelper.logD(TAG,"onCreate");
        multicastLock=openWifiBrocast(this);
        videoView= findViewById(R.id.video);
        videoView.setMediaController(new MediaController(this));
        try {
            mDMR=new DigitalMediaRenderer(null,null);
            mDMR.setOnMRActionRequestListener(mActionRequestListener);
            mDMS=new DigitalMediaServer(path,null,null);
            mDMC=new DigitalMediaController();
            mDMC.setOnDeviceChangeListener(mDeviceChangeListener);
            //mDMC.setOnMRActionResponseListener(mActionResponseListener);
            mDMC.setOnMRStateVariablesChangedListener(mStateVariablesChangedListener);
            mDMC.setOnMSMediaBrowserListener(mMediaBrowserListener);
        } catch (InstantiationException e) {
            e.printStackTrace();
        }
        wifiAdmin=new WifiAdmin(this);
    }
    public static WifiManager.MulticastLock openWifiBrocast(Context context){
        WifiManager wifiManager=(WifiManager)context.getApplicationContext().getSystemService(Context.WIFI_SERVICE);
        if(wifiManager==null)return null;
        WifiManager.MulticastLock multicastLock=wifiManager.createMulticastLock("MediaRender");
        if (multicastLock != null){
            LogHelper.logD(TAG,"multicastLock");
            multicastLock.acquire();
        }
        return multicastLock;
    }

    @Override
    protected void onStop() {
        super.onStop();
        wifiAdmin.release();
    }

    @Override
    protected void onDestroy() {
        mDMC.release();
        mDMC=null;
        mDMR.release();
        mDMR=null;
        mDMS=null;
        if(multicastLock!=null)
            multicastLock.release();
        super.onDestroy();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        wifiAdmin.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        wifiAdmin.onRequestPermissionsResult(requestCode,permissions,grantResults);
    }

    public void createGroup(View v){
        wifiAdmin.createGroup();
    }

    public void connectGroup(View v){
        wifiAdmin.connectGroup();
    }

    public void startPlatinum(View v){
        Button bt= (Button) v;
        if(isSetup){
            mDMC.stop();
            isSetup=false;
            bt.setText("搜索设备");
        }else {
            mDMC.start();
            isSetup=true;
            bt.setText("关闭搜索");
        }
    }

    public void switchDMR(View v){
        Button bt= (Button) v;
        if(isStartDMR){
            try {
                mDMR.stop();
            }catch (Exception e){
                Log.d("info",""+e.getMessage());
            }
            isStartDMR=false;
            bt.setText("打开DMR");
        }else {
            try {
                mDMR.start();
            }catch (Exception e){
                Log.d("info",""+e.getMessage());
            }
            isStartDMR=true;
            bt.setText("关闭DMR");
        }
    }

    public void switchDMS(View v){
        Button bt= (Button) v;
        if(isStartDMS){
            try {
                mDMS.stop();
            }catch (Exception e){
                Log.d("info",""+e.getMessage());
            }
            isStartDMS=false;
            bt.setText("打开DMS");
        }else {
            try {
                mDMS.start();
            }catch (Exception e){
                Log.d("info",""+e.getMessage());
            }
            isStartDMS=true;
            bt.setText("关闭DMS");
        }
    }

    public void open(View v){
        mDMC.cmdOpen(videoPath,"ff");
    }

    public void play(View v){
        mDMC.cmdPlay();
    }

    public void pause(View v){
        mDMC.cmdPause();
    }

    public void stop(View v){
        mDMC.cmdStop();
    }

    private DigitalMediaRenderer.OnMRActionRequestListener mActionRequestListener=new DigitalMediaRenderer.OnMRActionRequestListener() {
        @Override
        public void open(String uri, String metaData) {
            mDMR.responseStop();
            videoView.setVideoPath(uri);
            videoView.start();
            if(mDMR.responsePlay())
                Toast.makeText(MainActivity.this,"responsePlay:"+uri,Toast.LENGTH_SHORT).show();
        }

        private int position=0;
        @Override
        public void stop() {
            videoView.stopPlayback();
            if(mDMR.responseStop()){
                Toast.makeText(MainActivity.this,"responseStop",Toast.LENGTH_SHORT).show();
            }
            mDMR.responseOpen(videoPath+position,"ffff"+position);
            ++position;
        }

        @Override
        public void play() {
            videoView.start();
            if(mDMR.responsePlay())
                Toast.makeText(MainActivity.this,"responsePlay",Toast.LENGTH_SHORT).show();
        }

        @Override
        public void pause() {
            videoView.pause();
            if(mDMR.responsePause())
                Toast.makeText(MainActivity.this,"responsePause",Toast.LENGTH_SHORT).show();
        }

        @Override
        public void seekTo(int position) {

        }

        @Override
        public void previous() {

        }

        @Override
        public void next() {

        }

        @Override
        public void setVolume(int volume) {

        }

        @Override
        public void setMute(boolean mute) {

        }

        @Override
        public void setVolumeDB(int volumeDB) {

        }

        @Override
        public void getVolumeDBRange() {

        }
    };

    private DigitalMediaController.OnDeviceChangeListener mDeviceChangeListener=new DigitalMediaController.OnDeviceChangeListener() {
        @Override
        public void onDeviceAdded(Device device) {
            if(device == null) return;
            Toast.makeText(MainActivity.this,"Added: "+device.getName(),Toast.LENGTH_SHORT).show();
            if(Device.TYPE_DMR.equals(device.getType())){
                mDMC.setDMR(device.getUUID());
            }
            if(Device.TYPE_DMS.equals(device.getType())){
                mDMC.setDMS(device.getUUID());
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        mDMC.DoBrowse("0",false);
                    }
                }).start();
            }
        }

        @Override
        public void onDeviceRemoved(Device device) {
            if (device!=null)
                Toast.makeText(MainActivity.this,"Removed: "+device.getName(),Toast.LENGTH_SHORT).show();
        }
    };

    private DigitalMediaController.OnMSMediaBrowserListener mMediaBrowserListener =new DigitalMediaController.OnMSMediaBrowserListener() {
        @Override
        public void onBrowserResult(int result, ArrayList<MediaObject> objectList) {
            Toast.makeText(MainActivity.this,"onBrowserResult: "+objectList,Toast.LENGTH_SHORT).show();
            if(objectList ==null)return;
            StringBuilder builder=new StringBuilder("");
            for (MediaObject mediaObject:objectList) {
                builder.append(mediaObject.getUri()).append(",");
                String title = mediaObject.getTitle();
                if(!TextUtils.isEmpty(title) && title.equals("a")){
                    String path = mediaObject.getUri();
                    if(!TextUtils.isEmpty(path)){
                        mDMC.cmdOpen(path,mediaObject.getDidl());
                    }
                }
            }
            Toast.makeText(MainActivity.this,builder.toString(),Toast.LENGTH_LONG).show();
        }
    };

    private DigitalMediaController.OnMRActionResponseListener mActionResponseListener=new DigitalMediaController.OnMRActionResponseListener() {
        @Override
        public void onOpen(boolean result) {
            Toast.makeText(MainActivity.this,"onOpen:"+result,Toast.LENGTH_SHORT).show();
        }

        @Override
        public void onPlay(boolean result) {
            Toast.makeText(MainActivity.this,"onPlay:"+result,Toast.LENGTH_SHORT).show();
        }

        @Override
        public void onPause(boolean result) {
            Toast.makeText(MainActivity.this,"onPause:"+result,Toast.LENGTH_SHORT).show();
        }

        @Override
        public void onStop(boolean result) {
            Toast.makeText(MainActivity.this,"onStop:"+result,Toast.LENGTH_SHORT).show();
        }

        @Override
        public void onSeek(boolean result) {

        }

        @Override
        public void onSetMute(boolean result) {

        }

        @Override
        public void onGetMute(boolean result, boolean mute) {

        }

        @Override
        public void onSetVolume(boolean result) {

        }

        @Override
        public void onGetVolume(boolean result, int vol) {

        }

        @Override
        public void onGetDuration(boolean result, int msec) {

        }

        @Override
        public void onGetPosition(boolean result, int msec) {

        }
    };

    private DigitalMediaController.OnMRStateVariablesChangedListener mStateVariablesChangedListener=new DigitalMediaController.OnMRStateVariablesChangedListener() {

        @Override
        public void onAVTransportURI(String uri) {
            Toast.makeText(MainActivity.this,"onAVTransportURI:"+uri,Toast.LENGTH_SHORT).show();
        }

        @Override
        public void onAVTransportURIMetadata(String metaData) {
            Toast.makeText(MainActivity.this,"onAVTransportURIMetadata:"+metaData,Toast.LENGTH_SHORT).show();
        }

        @Override
        public void onPlay() {
            Toast.makeText(MainActivity.this,"onPlay:",Toast.LENGTH_SHORT).show();
        }

        @Override
        public void onPause() {
            Toast.makeText(MainActivity.this,"onPause:",Toast.LENGTH_SHORT).show();
        }

        @Override
        public void onStop() {
            Toast.makeText(MainActivity.this,"onStop:",Toast.LENGTH_SHORT).show();
        }

        @Override
        public void onMuteChange(boolean mute) {

        }

        @Override
        public void onVolumeChange(int vol) {

        }

        @Override
        public void onDurationChange(int seconds) {

        }

        @Override
        public void onPositionChange(int seconds) {

        }
    };
}
