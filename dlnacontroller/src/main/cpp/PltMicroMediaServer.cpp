//
// Created by wh on 2017/7/27.
//

#include "PltMicroMediaServer.h"

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, "sinadlna_jni", __VA_ARGS__))


void PLT_MicroMediaServer::startMediaServer(const char *path,const char *friendly_name) {
  LOGI("startMediaServer");
  PLT_FileMediaServerDelegate* delegate=new PLT_FileMediaServerDelegate("/", path);
  plt_mediaServer=new PLT_MediaServer(
          friendly_name,
          false,
          "e6572b54-f3c7-2d91-2fb5-b757f2537e21");
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
  this->uPnP->Start();
  LOGI("Start");
}

void PLT_MicroMediaServer::stopMediaServer() {
  if(NULL != this->uPnP)
    this->uPnP->Stop();
}