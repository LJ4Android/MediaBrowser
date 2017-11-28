//
// Created by wh on 2017/7/25.
//

#include "PltMicroMediaRenderer.h"

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, "sinadlna_jni", __VA_ARGS__))

void PLT_MicroMediaRenderer::startMediaRenderer(const char *friendly_name) {
    LOGI("PLT_MicroMediaRenderer::startMediaRenderer");
    PLT_DeviceHostReference device(
            new PLT_MediaRenderer(friendly_name, false, "e6572b54-f3c7-2d91-2fb5-b757f2537e21"));
    LOGI("PLT_DeviceHostReference");
    this->upnp = new PLT_UPnP();
    LOGI("new PLT_UPnP");
    this->upnp->AddDevice(device);
    LOGI("upnp->AddDevice");
    this->upnp->Start();
    LOGI("upnp->Start");
}

void PLT_MicroMediaRenderer::stopMediaRenderer() {
    if (NULL != this->upnp)
        this->upnp->Stop();
}
