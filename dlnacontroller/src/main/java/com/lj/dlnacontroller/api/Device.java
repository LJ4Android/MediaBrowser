package com.lj.dlnacontroller.api;

/**
 * Created by wh on 2017/12/11.
 */

public class Device {

    public static final String TYPE_DMS="urn:schemas-upnp-org:device:MediaServer:1";
    public static final String TYPE_DMR="urn:schemas-upnp-org:device:MediaRenderer:1";

    private String uuid;
    private String name;
    private String type;

    public Device(String uuid,String name,String type){
        this.uuid=uuid;
        this.name=name;
        this.type=type;
    }

    public String getName() {
        return name;
    }

    public String getType() {
        return type;
    }

    public String getUUID() {
        return uuid;
    }

    @Override
    public String toString() {
        return "Device["+"uuid:"+uuid+",name:"+name+",type:"+type+"]";
    }
}
