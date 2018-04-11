//
// Created by wh on 2017/7/25.
//

#ifndef PLATINUMKIT_PLTMICROMEDIARENDERER_H
#define PLATINUMKIT_PLTMICROMEDIARENDERER_H

#include "Platinum.h"
#include <android/log.h>
#include "MediaRendererListener.h"

class PLT_MicroMediaRenderer : public PLT_MediaRendererDelegate{

public:
    PLT_MicroMediaRenderer(const char *friendly_name,const char *uuid);
    ~PLT_MicroMediaRenderer(){};
    NPT_Result startMediaRenderer();
    NPT_Result stopMediaRenderer();
    bool IsRunning();

    void setMediaRendererListener(MediaRendererListener* l){this->listener=l;}

    //Response
    NPT_Result ResponseGenaEvent(int cmd,const char *value,const char *data);

    // ConnectionManager
    NPT_Result OnGetCurrentConnectionInfo(PLT_ActionReference& action);

    // AVTransport
    NPT_Result OnNext(PLT_ActionReference& action);
    NPT_Result OnPause(PLT_ActionReference& action);
    NPT_Result OnPlay(PLT_ActionReference& action);
    NPT_Result OnPrevious(PLT_ActionReference& action);
    NPT_Result OnSeek(PLT_ActionReference& action);
    NPT_Result OnStop(PLT_ActionReference& action);
    NPT_Result OnSetAVTransportURI(PLT_ActionReference& action);
    NPT_Result OnSetPlayMode(PLT_ActionReference& action);

    // RenderingControl
    NPT_Result OnSetVolume(PLT_ActionReference& action);
    NPT_Result OnSetVolumeDB(PLT_ActionReference& action);
    NPT_Result OnGetVolumeDBRange(PLT_ActionReference& action);
    NPT_Result OnSetMute(PLT_ActionReference& action);

private:
    PLT_UPnP *upnp;
    MediaRendererListener* listener;
    PLT_MediaRenderer* renderer;
};


#endif //PLATINUMKIT_PLTMICROMEDIARENDERER_H
