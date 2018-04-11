/*
** Copyright 2016, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/


/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "PltMicroMediaServer.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <android/log.h>

NPT_SET_LOCAL_LOGGER("platinum.tests.micromediaserver")



/*----------------------------------------------------------------------
|   PLT_MicroMediaController::PLT_MicroMediaController
+---------------------------------------------------------------------*/
PLT_MicroMediaServer::PLT_MicroMediaServer(PLT_FileMediaServer* mediaserver)
{
    mMediaServer = mediaserver;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::PLT_MicroMediaController
+---------------------------------------------------------------------*/
PLT_MicroMediaServer::~PLT_MicroMediaServer()
{
    NPT_LOG_INFO("destructor");
}


/*----------------------------------------------------------------------
| add by linhuaji 2014.06.26
| PLT_MicroMediaServer::RapidTransmitVideo
+---------------------------------------------------------------------*/
void
PLT_MicroMediaServer::RapidTransmitVideo(const char *filename)
{
    PLT_Service *service = NULL;
    NPT_LOG_INFO_1("RapidTransmitVideo filename %s" , filename);
	
    mMediaServer->FindServiceById("urn:upnp-org:serviceId:ContentDirectory", service);
    
    if(service != NULL) {
	service->SetStateVariable("RapidTransmitVideoFileName", filename);	
    } else {
        NPT_LOG_INFO("RapidTransmitVideo service \"urn:schemas-upnp-org:service:ContentDirectory:1\" not found");
    }
    return;		
}

NPT_Result
PLT_MicroMediaServer::getServerRootUri(char *uri){
    return mMediaServer->getServerRootUri(uri);
}


/* end */
