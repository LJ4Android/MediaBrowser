//
// Created by wh on 2017/7/25.
//

#ifndef PLATINUMKIT_PLTMICROMEDIARENDERER_H
#define PLATINUMKIT_PLTMICROMEDIARENDERER_H

#include "Platinum.h"
#include <android/log.h>

class PLT_MicroMediaRenderer{

public:
    PLT_MicroMediaRenderer(){};
    ~PLT_MicroMediaRenderer(){};
    void startMediaRenderer(const char* friendly_name);
    void stopMediaRenderer();

private:
    PLT_UPnP *upnp;
};


#endif //PLATINUMKIT_PLTMICROMEDIARENDERER_H
