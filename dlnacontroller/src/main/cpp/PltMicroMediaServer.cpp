//
// Created by wh on 2017/7/27.
//

#include "PltMicroMediaServer.h"

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, "sinadlna_jni", __VA_ARGS__))

PLT_MicroMediaServer::PLT_MicroMediaServer(const char *path, const char *friendly_name,
                                           const char *uuid) {
  PLT_FileMediaServerDelegate* delegate=new PLT_FileMediaServerDelegate("/", path);
  plt_mediaServer=new PLT_MediaServer(
          friendly_name,
          false,
          uuid);
  plt_mediaServer->SetDelegate(delegate);
  PLT_DeviceHostReference device(plt_mediaServer);
  LOGI("new PLT_MediaServer");
  device->m_ModelDescription = "Platinum File Media Server";
  device->m_ModelURL = "http://www.plutinosoft.com/";
  device->m_ModelNumber = "1.0";
  device->m_ModelName = "Platinum File Media Server";
  device->m_Manufacturer = "Plutinosoft";
  device->m_ManufacturerURL = "http://www.plutinosoft.com/";

  this->uPnP=new PLT_UPnP();
  LOGI("new PLT_UPnP");
  this->uPnP->AddDevice(device);
  LOGI("AddDevice");
}

NPT_Result PLT_MicroMediaServer::startMediaServer() {
  LOGI("startMediaServer");
  int ret= NPT_FAILURE;
  if(this->uPnP){
    ret=this->uPnP->Start();
  }
  return ret;
}

NPT_Result PLT_MicroMediaServer::stopMediaServer() {
  int ret= NPT_FAILURE;
  if (this->uPnP){
    if(this->uPnP->IsRunning()){
      ret=this->uPnP->Stop();
    }
    delete this->uPnP;
    this->uPnP=NULL;
  }
  if(plt_mediaServer){
    plt_mediaServer=NULL;
  }
  return ret;
}