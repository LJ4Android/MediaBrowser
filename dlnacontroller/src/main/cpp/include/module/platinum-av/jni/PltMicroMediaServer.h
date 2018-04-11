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


#ifndef _MICRO_MEDIA_SERVER_H_
#define _MICRO_MEDIA_SERVER_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Platinum.h"
#include "PltMediaServer.h"
#include "PltSyncMediaBrowser.h"
#include "PltMediaController.h"
#include "NptMap.h"
#include "NptStack.h"


/*----------------------------------------------------------------------
|   PLT_MicroMediaServer
+---------------------------------------------------------------------*/
class PLT_MicroMediaServer
{
public:
    PLT_MicroMediaServer(PLT_FileMediaServer* mediaserver);
    virtual ~PLT_MicroMediaServer();
    void RapidTransmitVideo(const char *filename);
    NPT_Result getServerRootUri(char *uri);
	
private:
    PLT_FileMediaServer *mMediaServer;
 
};

#endif /* _MICRO_MEDIA_SERVER_H_ */

