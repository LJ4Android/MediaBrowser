//
// Created by wh on 2017/7/27.
//

#ifndef PLATINUMKIT_PLTMICROMEDIASERVER_H
#define PLATINUMKIT_PLTMICROMEDIASERVER_H

#include "Platinum.h"
#include <android/log.h>

class PLT_MicroMediaServer{

public:
    PLT_MicroMediaServer(const char *path,const char* friendly_name,const char *uuid);
    ~PLT_MicroMediaServer(){};

    NPT_Result startMediaServer();
    NPT_Result stopMediaServer();

private:
    PLT_UPnP* uPnP;
    PLT_MediaServer* plt_mediaServer;
};

#endif //PLATINUMKIT_PLTMICROMEDIASERVER_H
