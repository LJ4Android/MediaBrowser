package com.lj.dlnacontroller.main;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.lj.dlnacontroller.R;
import com.lj.dlnacontroller.api.SinaDLNA;

public class MainActivity extends AppCompatActivity {

    private SinaDLNA mSinaDLNA;
    private TextView text_devices;
    private boolean isSetup;
    private boolean isStartDMR;
    private boolean isStartDMS;
    private String path= Environment.getExternalStorageDirectory().getAbsolutePath();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        text_devices= (TextView) findViewById(R.id.text_devices);
        mSinaDLNA=new SinaDLNA(this);
        mSinaDLNA.setSinaDLNAListener(sinaDLNAListener);
    }

    public void startPlatinum(View v){
        Button bt= (Button) v;
        if(isSetup){
            mSinaDLNA.release();
            text_devices.setText("");
            isSetup=false;
            bt.setText("搜索设备");
        }else {
            mSinaDLNA.setup();
            isSetup=true;
            bt.setText("关闭搜索");
        }
    }

    public void switchDMR(View v){
        Button bt= (Button) v;
        if(isStartDMR){
            try {
                mSinaDLNA.stopMediaRender();
            }catch (Exception e){
                Log.d("info",""+e.getMessage());
            }
            isStartDMR=false;
            bt.setText("打开DMR");
        }else {
            try {
                mSinaDLNA.startMediaRender("Hello DMR");
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
                mSinaDLNA.stopMediaServer();
            }catch (Exception e){
                Log.d("info",""+e.getMessage());
            }
            isStartDMS=false;
            bt.setText("打开DMS");
        }else {
            try {
                mSinaDLNA.startMediaServer("Hello DMS","Hello DMS");
            }catch (Exception e){
                Log.d("info",""+e.getMessage());
            }
            isStartDMS=true;
            bt.setText("关闭DMS");
        }
    }

    private SinaDLNA.SinaDLNAListener sinaDLNAListener=new SinaDLNA.SinaDLNAListener() {
        @Override
        public void onMediaRenderAdded(String uuid, String name) {
            String device="[ name: "+name+",uuid: "+uuid+" ]";
            text_devices.setText(device);
        }

        @Override
        public void onMediaRenderRemoved(String uuid, String name) {

        }

        @Override
        public void onMediaRenderStateChanged(String name, String value) {

        }

        @Override
        public void onOpen(int result) {

        }

        @Override
        public void onPlay(int result) {

        }

        @Override
        public void onPause(int result) {

        }

        @Override
        public void onStop(int result) {

        }

        @Override
        public void onSeek(int result) {

        }

        @Override
        public void onSetMute(int result) {

        }

        @Override
        public void onGetMute(int result, boolean mute) {

        }

        @Override
        public void onSetVolume(int result) {

        }

        @Override
        public void onGetVolume(int result, int vol) {

        }

        @Override
        public void onGetDuration(int result, int msec) {

        }

        @Override
        public void onGetPosition(int result, int msec) {

        }
    };
}
