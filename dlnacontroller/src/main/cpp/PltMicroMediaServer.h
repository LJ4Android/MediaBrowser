//
// Created by wh on 2017/7/27.
//

#ifndef PLATINUMKIT_PLTMICROMEDIASERVER_H
#define PLATINUMKIT_PLTMICROMEDIASERVER_H

#include "Platinum.h"
#include <android/log.h>

class PLT_MicroMediaServer{

public:
    PLT_MicroMediaServer(){};
    ~PLT_MicroMediaServer(){};

    void startMediaServer(const char *path,const char* friendly_name);
    void stopMediaServer();

private:
    PLT_UPnP* uPnP;
    PLT_MediaServer* plt_mediaServer;

};

#endif //PLATINUMKIT_PLTMICROMEDIASERVER_H
