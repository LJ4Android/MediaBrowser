package com.lj.dlnacontroller.api;

import android.os.Build;
import android.os.Environment;
import android.text.TextUtils;

import com.lj.dlnacontroller.main.LogHelper;

/**
 * Created by wh on 2017/12/13.
 */

public class DigitalMediaServer {

    private final String TAG="DMS";

    private int NPT_SUCCESS=0;

    private String path= Environment.getExternalStorageDirectory().getAbsolutePath();
    private String friendly_name=Build.MODEL+"_DMS";
    private String uuid= Build.SERIAL+"_dms";
    private MediaRemote mRemote;

    public DigitalMediaServer(String path,String friendly_name,String uuid)throws InstantiationException{
        mRemote=MediaRemote.getInstance();
        if(!TextUtils.isEmpty(path)){
            this.path=path;
        }
        if(!TextUtils.isEmpty(friendly_name)){
            this.friendly_name=friendly_name;
        }
        if(!TextUtils.isEmpty(uuid)){
            this.uuid=uuid;
        }
    }

    public boolean start(){
        LogHelper.logD(TAG,"path: "+path);
        LogHelper.logD(TAG,"friendly_name: "+friendly_name);
        LogHelper.logD(TAG,"uuid: "+uuid);
        return mRemote.startMediaServer(path,friendly_name,uuid)==NPT_SUCCESS;
    }

    public boolean stop(){
        return mRemote.stopMediaServer() == NPT_SUCCESS;
    }
}
