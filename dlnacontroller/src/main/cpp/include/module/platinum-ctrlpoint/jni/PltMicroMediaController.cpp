/*****************************************************************
|
|   Platinum - Miccro Media Controller
|
| Copyright (c) 2004-2010, Plutinosoft, LLC.
| All rights reserved.
| http://www.plutinosoft.com
|
| This program is free software; you can redistribute it and/or
| modify it under the terms of the GNU General Public License
| as published by the Free Software Foundation; either version 2
| of the License, or (at your option) any later version.
|
| OEMs, ISVs, VARs and other distributors that combine and 
| distribute commercially licensed software with Platinum software
| and do not wish to distribute the source code for the commercially
| licensed software under version 2, or (at your option) any later
| version, of the GNU General Public License (the "GPL") must enter
| into a commercial license agreement with Plutinosoft, LLC.
| licensing@plutinosoft.com
| 
| This program is distributed in the hope that it will be useful,
| but WITHOUT ANY WARRANTY; without even the implied warranty of
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
| GNU General Public License for more details.
|
| You should have received a copy of the GNU General Public License
| along with this program; see the file LICENSE.txt. If not, write to
| the Free Software Foundation, Inc., 
| 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
| http://www.gnu.org/licenses/gpl-2.0.html
|
****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "PltMicroMediaController.h"
#include "PltLeaks.h"
#include "PltDownloader.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <android/log.h>

NPT_SET_LOCAL_LOGGER("platinum.tests.micromediacontroller")

/* added by lihaihua */
DeviceCallback::DeviceCallback()
    :   mTest(0)
{
}

DeviceCallback::~DeviceCallback()
{
}
/* end */

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::PLT_MicroMediaController
+---------------------------------------------------------------------*/
PLT_MicroMediaController::PLT_MicroMediaController(PLT_CtrlPointReference& ctrlPoint, DeviceCallback *callback) :
    PLT_SyncMediaBrowser(ctrlPoint, false, this),
    PLT_MediaController(ctrlPoint)
{
    /* added by lihaihua */
    mCallback = callback;
    mIsMsiDmr = false;
    /* end */

    // create the stack that will be the directory where the
    // user is currently browsing. 
    // push the root directory onto the directory stack.
    m_CurBrowseDirectoryStack.Push("0");

    PLT_MediaController::SetDelegate(this);
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::PLT_MicroMediaController
+---------------------------------------------------------------------*/
PLT_MicroMediaController::~PLT_MicroMediaController()
{
    NPT_LOG_INFO("destructor");
    if(mCallback != NULL) {
        delete mCallback;
        mCallback = NULL;
    }
}

/*
*  Remove trailing white space from a string
*/
static void strchomp(char* str)
{
    if (!str) return;
    char* e = str+NPT_StringLength(str)-1;

    while (e >= str && *e) {
        if ((*e != ' ')  &&
            (*e != '\t') &&
            (*e != '\r') &&
            (*e != '\n'))
        {
            *(e+1) = '\0';
            break;
        }
        --e;
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::ChooseIDFromTable
+---------------------------------------------------------------------*/
/* 
 * Presents a list to the user, allows the user to choose one item.
 *
 * Parameters:
 *		PLT_StringMap: A map that contains the set of items from
 *                        which the user should choose.  The key should be a unique ID,
 *						 and the value should be a string describing the item. 
 *       returns a NPT_String with the unique ID. 
 */
const char*
PLT_MicroMediaController::ChooseIDFromTable(PLT_StringMap& table)
{
    printf("Select one of the following:\n");

    NPT_List<PLT_StringMapEntry*> entries = table.GetEntries();
    if (entries.GetItemCount() == 0) {
        printf("None available\n"); 
    } else {
        // display the list of entries
        NPT_List<PLT_StringMapEntry*>::Iterator entry = entries.GetFirstItem();
        int count = 0;
        while (entry) {
            printf("%d)\t%s (%s)\n", ++count, (const char*)(*entry)->GetValue(), (const char*)(*entry)->GetKey());
            ++entry;
        }

        int index, watchdog = 3;
        char buffer[1024];

        // wait for input
        while (watchdog > 0) {
            fgets(buffer, 1024, stdin);
            strchomp(buffer);

            if (1 != sscanf(buffer, "%d", &index)) {
                printf("Please enter a number\n");
            } else if (index < 0 || index > count)	{
                printf("Please choose one of the above, or 0 for none\n");
                watchdog--;
                index = 0;
            } else {	
                watchdog = 0;
            }
        }

        // find the entry back
        if (index != 0) {
            entry = entries.GetFirstItem();
            while (entry && --index) {
                ++entry;
            }
            if (entry) {
                return (*entry)->GetKey();
            }
        }
    }

    return NULL;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::PopDirectoryStackToRoot
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::PopDirectoryStackToRoot(void)
{
    NPT_String val;
    while (NPT_SUCCEEDED(m_CurBrowseDirectoryStack.Peek(val)) && val.Compare("0")) {
        m_CurBrowseDirectoryStack.Pop(val);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::OnContainerChanged
+---------------------------------------------------------------------*/
void PLT_MicroMediaController::OnContainerChanged(PLT_DeviceDataReference& device, 
                                    const char*              item_id, 
                                    const char*              update_id)
{
    return;
}

/*----------------------------------------------------------------------
|	PLT_MicroMediaController::OnRapidTransmitFileNameChange
+---------------------------------------------------------------------*/
void PLT_MicroMediaController::OnRapidTransmitFileNameChange(const char *filename)
{
    NPT_LOG_INFO_1("Transmit File Name = %s", filename);
    mCallback->onRapidTransmitVideo(filename);
    return;
}


/*----------------------------------------------------------------------
|   PLT_MicroMediaController::OnMSAdded
+---------------------------------------------------------------------*/
bool 
PLT_MicroMediaController::OnMSAdded(PLT_DeviceDataReference& device) 
{     
    // Issue special action upon discovering MediaConnect server
    PLT_Service* service;
    if (NPT_SUCCEEDED(device->FindServiceByType("urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:*", service))) {
        PLT_ActionReference action;
        PLT_SyncMediaBrowser::m_CtrlPoint->CreateAction(
            device, 
            "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1", 
            "IsAuthorized", 
            action);
        if (!action.IsNull()) {
            action->SetArgumentValue("DeviceID", "");
            PLT_SyncMediaBrowser::m_CtrlPoint->InvokeAction(action, 0);
        }

        PLT_SyncMediaBrowser::m_CtrlPoint->CreateAction(
            device, 
            "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1", 
            "IsValidated", 
            action);
        if (!action.IsNull()) {
            action->SetArgumentValue("DeviceID", "");
            PLT_SyncMediaBrowser::m_CtrlPoint->InvokeAction(action, 0);
        }
    }

    NPT_LOG_INFO_3("uuid = %s, name = %s, type = %s", (const char*)(device->GetUUID()),
        (const char*)(device->GetFriendlyName()), (const char*)(device->GetType()));

/* added by lihaihua */
    mCallback->onDlnaDeviceAdded((char*)(device->GetFriendlyName()), (char*)(device->GetUUID()), 0);
/* end */

    return true; 
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::OnMRAdded
+---------------------------------------------------------------------*/
bool
PLT_MicroMediaController::OnMRAdded(PLT_DeviceDataReference& device)
{
    NPT_String uuid = device->GetUUID();

    // test if it's a media renderer
    PLT_Service* service;
    if (NPT_SUCCEEDED(device->FindServiceByType("urn:schemas-upnp-org:service:AVTransport:*", service))) {
        NPT_AutoLock lock(m_MediaRenderers);
        m_MediaRenderers.Put(uuid, device);
    }
    
    NPT_LOG_INFO_3("uuid = %s, name = %s, type = %s", (const char*)(device->GetUUID()),
        (const char*)(device->GetFriendlyName()), (const char*)(device->GetType()));

/* added by lihaihua */
    mCallback->onDlnaDeviceAdded((char*)(device->GetFriendlyName()), (char*)(device->GetUUID()), 1);
/* end */

    return true;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::OnMRRemoved
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::OnMRRemoved(PLT_DeviceDataReference& device)
{
    NPT_String uuid = device->GetUUID();

    {
        NPT_AutoLock lock(m_MediaRenderers);
        m_MediaRenderers.Erase(uuid);
    }

    {
        NPT_AutoLock lock(m_CurMediaRendererLock);

        // if it's the currently selected one, we have to get rid of it
        if (!m_CurMediaRenderer.IsNull() && m_CurMediaRenderer == device) {
            m_CurMediaRenderer = NULL;
        }
    }

    NPT_LOG_INFO_3("uuid = %s, name = %s, type = %s", (const char*)(device->GetUUID()),
        (const char*)(device->GetFriendlyName()), (const char*)(device->GetType()));

/* added by lihaihua */
    mCallback->onDlnaDeviceRemoved((char*)(device->GetFriendlyName()), (char*)(device->GetUUID()), 1);
/* end */
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::OnMRStateVariablesChanged
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::OnMRStateVariablesChanged(PLT_Service*                  service,
                                                    NPT_List<PLT_StateVariable*>* vars)
{
    NPT_String uuid = service->GetDevice()->GetUUID();

    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (device.IsNull() || (device->GetUUID().Compare(uuid, true) != 0))
        return;

    NPT_List<PLT_StateVariable*>::Iterator var = vars->GetFirstItem();
    while (var) {
        NPT_LOG_INFO_4("Received state var \"%s:%s:%s\" changes: \"%s\"",
               (const char*)uuid,
               (const char*)service->GetServiceID(),
               (const char*)(*var)->GetName(),
               (const char*)(*var)->GetValue());
        /* qiku add */
        /* added callback, lihaihua, 2013.12.19 */
        if((mIsMsiDmr == false) || ((*var)->GetName().Compare("TransportState", true) != 0)
                || ((*var)->GetValue().Compare("NO_MEDIA_PRESENT", true) != 0))
            mCallback->onDlnaDmrStatusChanged((char*)(*var)->GetName(), (char*)(*var)->GetValue());
        /* qiku end */
        ++var;
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::ChooseIDGetCurMediaServerFromTable
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::GetCurMediaServer(PLT_DeviceDataReference& server)
{
    NPT_AutoLock lock(m_CurMediaServerLock);

    if (m_CurMediaServer.IsNull()) {
        printf("No server selected, select one with setms\n");
    } else {
        server = m_CurMediaServer;
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::GetCurMediaRenderer
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::GetCurMediaRenderer(PLT_DeviceDataReference& renderer)
{
    NPT_AutoLock lock(m_CurMediaRendererLock);

    if (m_CurMediaRenderer.IsNull()) {
        printf("No renderer selected, select one with setmr\n");
    } else {
        renderer = m_CurMediaRenderer;
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::DoBrowse
+---------------------------------------------------------------------*/
NPT_Result
PLT_MicroMediaController::DoBrowse(const char* object_id, /* = NULL */
                                   bool        metadata  /* = false */)
{
    NPT_Result res = NPT_FAILURE;
    PLT_DeviceDataReference device;
    
    GetCurMediaServer(device);
    if (!device.IsNull()) {
        NPT_String cur_object_id;
        m_CurBrowseDirectoryStack.Peek(cur_object_id);

        // send off the browse packet and block
        res = BrowseSync(
            device, 
            object_id?object_id:(const char*)cur_object_id, 
            m_MostRecentBrowseResults, 
            metadata);		
    }

    return res;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_getms
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_getms()
{
    PLT_DeviceDataReference device;
    GetCurMediaServer(device);
    if (!device.IsNull()) {
        printf("Current media server: %s\n", (const char*)device->GetFriendlyName());
    } else {
        // this output is taken care of by the GetCurMediaServer call
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_getmr
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_getmr()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        printf("Current media renderer: %s\n", (const char*)device->GetFriendlyName());
    } else {
        // this output is taken care of by the GetCurMediaRenderer call
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::ChooseDevice
+---------------------------------------------------------------------*/
PLT_DeviceDataReference
PLT_MicroMediaController::ChooseDevice(const NPT_Lock<PLT_DeviceMap>& deviceList)
{
    PLT_StringMap            namesTable;
    PLT_DeviceDataReference* result = NULL;
    NPT_String               chosenUUID;
    NPT_AutoLock             lock(m_MediaServers);

    // create a map with the device UDN -> device Name 
    const NPT_List<PLT_DeviceMapEntry*>& entries = deviceList.GetEntries();
    NPT_List<PLT_DeviceMapEntry*>::Iterator entry = entries.GetFirstItem();
    while (entry) {
        PLT_DeviceDataReference device = (*entry)->GetValue();
        NPT_String              name   = device->GetFriendlyName();
        namesTable.Put((*entry)->GetKey(), name);

        ++entry;
    }

    // ask user to choose
    chosenUUID = ChooseIDFromTable(namesTable);
    if (chosenUUID.GetLength()) {
        deviceList.Get(chosenUUID, result);
    }

    return result?*result:PLT_DeviceDataReference(); // return empty reference if not device was selected
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_setms
+---------------------------------------------------------------------*/
void 
PLT_MicroMediaController::HandleCmd_setms()
{
    NPT_AutoLock lock(m_CurMediaServerLock);

    PopDirectoryStackToRoot();
    m_CurMediaServer = ChooseDevice(GetMediaServersMap());
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_setmr
+---------------------------------------------------------------------*/
void 
PLT_MicroMediaController::HandleCmd_setmr()
{
    NPT_AutoLock lock(m_CurMediaRendererLock);

    m_CurMediaRenderer = ChooseDevice(m_MediaRenderers);
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_ls
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_ls()
{
    DoBrowse();

    if (!m_MostRecentBrowseResults.IsNull()) {
        printf("There were %d results\n", m_MostRecentBrowseResults->GetItemCount());

        NPT_List<PLT_MediaObject*>::Iterator item = m_MostRecentBrowseResults->GetFirstItem();
        while (item) {
            if ((*item)->IsContainer()) {
                printf("Container: %s (%s)\n", (*item)->m_Title.GetChars(), (*item)->m_ObjectID.GetChars());
            } else {
                printf("Item: %s (%s)\n", (*item)->m_Title.GetChars(), (*item)->m_ObjectID.GetChars());
            }
            ++item;
        }

        m_MostRecentBrowseResults = NULL;
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_info
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_info()
{
    NPT_String              object_id;
    PLT_StringMap           tracks;
    PLT_DeviceDataReference device;

    // issue a browse
    DoBrowse();

    if (!m_MostRecentBrowseResults.IsNull()) {
        // create a map item id -> item title
        NPT_List<PLT_MediaObject*>::Iterator item = m_MostRecentBrowseResults->GetFirstItem();
        while (item) {
            if (!(*item)->IsContainer()) {
                tracks.Put((*item)->m_ObjectID, (*item)->m_Title);
            }
            ++item;
        }

        // let the user choose which one
        object_id = ChooseIDFromTable(tracks);

        if (object_id.GetLength()) {
            // issue a browse with metadata
            DoBrowse(object_id, true);

            // look back for the PLT_MediaItem in the results
            PLT_MediaObject* track = NULL;
            if (!m_MostRecentBrowseResults.IsNull() &&
                NPT_SUCCEEDED(NPT_ContainerFind(*m_MostRecentBrowseResults, PLT_MediaItemIDFinder(object_id), track))) {

                // display info
                printf("Title: %s \n", track->m_Title.GetChars());
                printf("OjbectID: %s\n", track->m_ObjectID.GetChars());
                printf("Class: %s\n", track->m_ObjectClass.type.GetChars());
                printf("Creator: %s\n", track->m_Creator.GetChars());
                printf("Date: %s\n", track->m_Date.GetChars());
                for (NPT_List<PLT_AlbumArtInfo>::Iterator iter = track->m_ExtraInfo.album_arts.GetFirstItem();
                     iter;
                     iter++) {
                    printf("Art Uri: %s\n", (*iter).uri.GetChars());
                    printf("Art Uri DLNA Profile: %s\n", (*iter).dlna_profile.GetChars());
                }
                for (NPT_Cardinal i=0;i<track->m_Resources.GetItemCount(); i++) {
                    printf("\tResource[%d].uri: %s\n", i, track->m_Resources[i].m_Uri.GetChars());
                    printf("\tResource[%d].profile: %s\n", i, track->m_Resources[i].m_ProtocolInfo.ToString().GetChars());
                    printf("\tResource[%d].duration: %d\n", i, track->m_Resources[i].m_Duration);
                    printf("\tResource[%d].size: %d\n", i, (int)track->m_Resources[i].m_Size);
                    printf("\n");
                }
                printf("Didl: %s\n", (const char*)track->m_Didl);
            } else {
                printf("Couldn't find the track\n");
            }
        }

        m_MostRecentBrowseResults = NULL;
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_download
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_download()
{
    NPT_String              object_id;
    PLT_StringMap           tracks;
    PLT_DeviceDataReference device;
    
    // issue a browse
    DoBrowse();
    
    if (!m_MostRecentBrowseResults.IsNull()) {
        // create a map item id -> item title
        NPT_List<PLT_MediaObject*>::Iterator item = m_MostRecentBrowseResults->GetFirstItem();
        while (item) {
            if (!(*item)->IsContainer()) {
                tracks.Put((*item)->m_ObjectID, (*item)->m_Title);
            }
            ++item;
        }
        
        // let the user choose which one
        object_id = ChooseIDFromTable(tracks);
        
        if (object_id.GetLength()) {
            // issue a browse with metadata
            DoBrowse(object_id, true);
            
            // look back for the PLT_MediaItem in the results
            PLT_MediaObject* track = NULL;
            if (!m_MostRecentBrowseResults.IsNull() &&
                NPT_SUCCEEDED(NPT_ContainerFind(*m_MostRecentBrowseResults, PLT_MediaItemIDFinder(object_id), track))) {
                
                if (track->m_Resources.GetItemCount() > 0) {
                    printf("\tResource[0].uri: %s\n", track->m_Resources[0].m_Uri.GetChars());
                    printf("\n");
                    NPT_HttpUrl url(track->m_Resources[0].m_Uri.GetChars());
                    if (url.IsValid()) {
                        // Extract filename from URL
                        NPT_String filename = NPT_FilePath::BaseName(url.GetPath(true).GetChars(), false);
                        NPT_String extension = NPT_FilePath::FileExtension(url.GetPath(true).GetChars());
                        printf("Downloading %s%s\n", filename.GetChars(), extension.GetChars());
                        
                        for (int i=0; i<3; i++) {
                            NPT_String filepath = NPT_String::Format("%s_%d%s", filename.GetChars(), i, extension.GetChars());
                            
                            // Open file for writing
                            NPT_File file(filepath);
                            file.Open(NPT_FILE_OPEN_MODE_WRITE | NPT_FILE_OPEN_MODE_CREATE | NPT_FILE_OPEN_MODE_TRUNCATE);
                            NPT_OutputStreamReference output;
                            file.GetOutputStream(output);
                            
                            // trigger 3 download
                            PLT_Downloader* downloader = new PLT_Downloader(url, output);
                            NPT_TimeInterval delay(5.);
                            m_DownloadTaskManager.StartTask(downloader, &delay);
                        }
                    }
                } else {
                    printf("No resources found");
                }
            } else {
                printf("Couldn't find the track\n");
            }
        }
        
        m_MostRecentBrowseResults = NULL;
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_cd
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_cd(const char* command)
{
    NPT_String    newobject_id;
    PLT_StringMap containers;

    // if command has parameter, push it to stack and return
    NPT_String id = command;
    NPT_List<NPT_String> args = id.Split(" ");
    if (args.GetItemCount() >= 2) {
        args.Erase(args.GetFirstItem());
        id = NPT_String::Join(args, " ");
        m_CurBrowseDirectoryStack.Push(id);
        return;
    }

    // list current directory to let user choose
    DoBrowse();

    if (!m_MostRecentBrowseResults.IsNull()) {
        NPT_List<PLT_MediaObject*>::Iterator item = m_MostRecentBrowseResults->GetFirstItem();
        while (item) {
            if ((*item)->IsContainer()) {
                containers.Put((*item)->m_ObjectID, (*item)->m_Title);
            }
            ++item;
        }

        newobject_id = ChooseIDFromTable(containers);
        if (newobject_id.GetLength()) {
            m_CurBrowseDirectoryStack.Push(newobject_id);
        }

        m_MostRecentBrowseResults = NULL;
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_cdup
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_cdup()
{
    // we don't want to pop the root off now....
    NPT_String val;
    m_CurBrowseDirectoryStack.Peek(val);
    if (val.Compare("0")) {
        m_CurBrowseDirectoryStack.Pop(val);
    } else {
        printf("Already at root\n");
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_pwd
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_pwd()
{
    NPT_Stack<NPT_String> tempStack;
    NPT_String val;

    while (NPT_SUCCEEDED(m_CurBrowseDirectoryStack.Peek(val))) {
        m_CurBrowseDirectoryStack.Pop(val);
        tempStack.Push(val);
    }

    while (NPT_SUCCEEDED(tempStack.Peek(val))) {
        tempStack.Pop(val);
        printf("%s/", (const char*)val);
        m_CurBrowseDirectoryStack.Push(val);
    }
    printf("\n");
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_open
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_open()
{
    NPT_String              object_id;
    PLT_StringMap           tracks;
    PLT_DeviceDataReference device;

    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        // get the protocol info to try to see in advance if a track would play on the device

        // issue a browse
        DoBrowse();

        if (!m_MostRecentBrowseResults.IsNull()) {
            // create a map item id -> item title
            NPT_List<PLT_MediaObject*>::Iterator item = m_MostRecentBrowseResults->GetFirstItem();
            while (item) {
                if (!(*item)->IsContainer()) {
                    tracks.Put((*item)->m_ObjectID, (*item)->m_Title);
                }
                ++item;
            }

            // let the user choose which one
            object_id = ChooseIDFromTable(tracks);
            if (object_id.GetLength()) {
                // look back for the PLT_MediaItem in the results
                PLT_MediaObject* track = NULL;
                if (NPT_SUCCEEDED(NPT_ContainerFind(*m_MostRecentBrowseResults, PLT_MediaItemIDFinder(object_id), track))) {
                    if (track->m_Resources.GetItemCount() > 0) {
                        // look for best resource to use by matching each resource to a sink advertised by renderer
                        NPT_Cardinal resource_index = 0;
                        if (NPT_FAILED(FindBestResource(device, *track, resource_index))) {
                            printf("No matching resource\n");
                            return;
                        }

                        // invoke the setUri
                        printf("Issuing SetAVTransportURI with url=%s & didl=%s", 
                            (const char*)track->m_Resources[resource_index].m_Uri, 
                            (const char*)track->m_Didl);
                        SetAVTransportURI(device, 0, track->m_Resources[resource_index].m_Uri, track->m_Didl, NULL);
                    } else {
                        printf("Couldn't find the proper resource\n");
                    }

                } else {
                    printf("Couldn't find the track\n");
                }
            }

            m_MostRecentBrowseResults = NULL;
        }
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_play
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_play()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        Play(device, 0, "1", NULL);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_seek
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_seek(const char* command)
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        // remove first part of command ("seek")
        NPT_String target = command;
        NPT_List<NPT_String> args = target.Split(" ");
        if (args.GetItemCount() < 2) return;

        args.Erase(args.GetFirstItem());
        target = NPT_String::Join(args, " ");
        
        Seek(device, 0, (target.Find(":")!=-1)?"REL_TIME":"X_DLNA_REL_BYTE", target, NULL);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_stop
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_stop()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        Stop(device, 0, NULL);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_mute
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_mute()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        SetMute(device, 0, "Master", true, NULL);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_unmute
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_unmute()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        SetMute(device, 0, "Master", false, NULL);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::HandleCmd_help
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::HandleCmd_help()
{
    printf("\n\nNone of the commands take arguments.  The commands with a * \n");
    printf("signify ones that will prompt the user for more information once\n");
    printf("the command is called\n\n");
    printf("The available commands are:\n\n");
    printf(" quit    -   shutdown the Control Point\n");
    printf(" exit    -   same as quit\n");

    printf(" setms   - * select a media server to become the active media server\n");
    printf(" getms   -   print the friendly name of the active media server\n");
    printf(" ls      -   list the contents of the current directory on the active \n");
    printf("             media server\n");
    printf(" info    -   display media info\n");
    printf(" down    -   download media to current directory\n");
    printf(" cd      - * traverse down one level in the content tree on the active\n");
    printf("             media server\n");
    printf(" cd ..   -   traverse up one level in the content tree on the active\n");
    printf("             media server\n");
    printf(" pwd     -   print the path from the root to your current position in the \n");
    printf("             content tree on the active media server\n");
    printf(" setmr   - * select a media renderer to become the active media renderer\n");
    printf(" getmr   -   print the friendly name of the active media renderer\n");
    printf(" open    -   set the uri on the active media renderer\n");
    printf(" play    -   play the active uri on the active media renderer\n");
    printf(" stop    -   stop the active uri on the active media renderer\n");
    printf(" seek    -   issue a seek command\n");
    printf(" mute    -   mute the active media renderer\n");
    printf(" unmute  -   unmute the active media renderer\n");

    printf(" help    -   print this help message\n\n");
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::ProcessCommandLoop
+---------------------------------------------------------------------*/
void 
PLT_MicroMediaController::ProcessCommandLoop()
{
    char command[2048];
    bool abort = false;

    command[0] = '\0';
    while (!abort) {
        printf("command> ");
        fflush(stdout);
        fgets(command, 2048, stdin);
        strchomp(command);

        if (0 == strcmp(command, "quit") || 0 == strcmp(command, "exit")) {
            abort = true;
        } else if (0 == strcmp(command, "setms")) {
            HandleCmd_setms();
        } else if (0 == strcmp(command, "getms")) {
            HandleCmd_getms();
        } else if (0 == strncmp(command, "ls", 2)) {
            HandleCmd_ls();
        } else if (0 == strcmp(command, "info")) {
            HandleCmd_info();
        } else if (0 == strcmp(command, "down")) {
            HandleCmd_download();
        } else if (0 == strcmp(command, "cd")) {
            HandleCmd_cd(command);
        } else if (0 == strcmp(command, "cd ..")) {
            HandleCmd_cdup();
        } else if (0 == strcmp(command, "pwd")) {
            HandleCmd_pwd();
        } else if (0 == strcmp(command, "setmr")) {
            HandleCmd_setmr();
        } else if (0 == strcmp(command, "getmr")) {
            HandleCmd_getmr();
        } else if (0 == strcmp(command, "open")) {
            HandleCmd_open();
        } else if (0 == strcmp(command, "play")) {
            HandleCmd_play();
        } else if (0 == strcmp(command, "stop")) {
            HandleCmd_stop();
        } else if (0 == strncmp(command, "seek", 4)) {
            HandleCmd_seek(command);
        } else if (0 == strcmp(command, "mute")) {
            HandleCmd_mute();
        } else if (0 == strcmp(command, "unmute")) {
            HandleCmd_mute();
        } else if (0 == strcmp(command, "help")) {
            HandleCmd_help();
        } else if (0 == strcmp(command, "")) {
            // just prompt again
        } else {
            printf("Unrecognized command: %s\n", command);
            HandleCmd_help();
        }
    }
}

/* added by lihaihua */

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::ScanDevice
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::ScanDevice(long mx)
{
    // tell control point to perform extra broadcast discover every 6 secs
    // in case our device doesn't support multicast
    //ctrlPoint->Discover(NPT_HttpUrl("255.255.255.255", 1900, "*"), "upnp:rootdevice", 1, 30.0);
    //ctrlPoint->Discover(NPT_HttpUrl("239.255.255.250", 1900, "*"), "upnp:rootdevice", 1, 30.0);

    //NPT_LOG_INFO("Discover: 255.255.255.255:1900");
    //PLT_SyncMediaBrowser::m_CtrlPoint->Discover(NPT_HttpUrl("255.255.255.255", 1900, "*"), "upnp:rootdevice", mx, 0.0);
    //NPT_LOG_INFO("Discover: 239.255.255.250:1900 upnp:rootdevice");
    //PLT_SyncMediaBrowser::m_CtrlPoint->Discover(NPT_HttpUrl("239.255.255.250", 1900, "*"), "upnp:rootdevice", mx, 0.0);
    //NPT_LOG_INFO("Discover: 239.255.255.250:1900 ssdp:all");
    //PLT_SyncMediaBrowser::m_CtrlPoint->Discover(NPT_HttpUrl("239.255.255.250", 1900, "*"), "ssdp:all", mx, 0.0);

    NPT_LOG_INFO("Search: start");

    //PLT_SyncMediaBrowser::m_CtrlPoint->Search(NPT_HttpUrl("239.255.255.250", 1900, "*"), "urn:schemas-upnp-org:device:CE:1", mx, 0.0);
    PLT_SyncMediaBrowser::m_CtrlPoint->Search(NPT_HttpUrl("239.255.255.250", 1900, "*"), "urn:schemas-upnp-org:device:MediaServer:1", mx, 0.0);
    //PLT_SyncMediaBrowser::m_CtrlPoint->Search(NPT_HttpUrl("239.255.255.250", 1900, "*"), "urn:dmc-samsung-com:device:SyncServer:1", mx, 0.0);
    //PLT_SyncMediaBrowser::m_CtrlPoint->Search(NPT_HttpUrl("239.255.255.250", 1900, "*"), "urn:samsung.com:device:MainTVServer2:1", mx, 0.0);
    //PLT_SyncMediaBrowser::m_CtrlPoint->Search(NPT_HttpUrl("239.255.255.250", 1900, "*"), "urn:samsung.com:device:RemoteControlReceiver:1", mx, 0.0);
    PLT_SyncMediaBrowser::m_CtrlPoint->Search(NPT_HttpUrl("239.255.255.250", 1900, "*"), "urn:schemas-upnp-org:device:MediaRenderer:1", mx, 0.0);
    PLT_SyncMediaBrowser::m_CtrlPoint->Search(NPT_HttpUrl("239.255.255.250", 1900, "*"), "upnp:rootdevice", mx, 0.0);

    NPT_LOG_INFO("Search: end");
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::GetDmsList
+---------------------------------------------------------------------*/
int 
PLT_MicroMediaController::GetDmsList(DeviceInfo list[])
{
    NPT_AutoLock lock(m_CurMediaServerLock);

    int count = 0;
    const NPT_List<PLT_DeviceMapEntry*>& entries = GetMediaServersMap().GetEntries();
    NPT_List<PLT_DeviceMapEntry*>::Iterator entry = entries.GetFirstItem();

    while (entry) {
        PLT_DeviceDataReference device = (*entry)->GetValue();
        NPT_String              name   = device->GetFriendlyName();

        if(count < 10) {
            strcpy(list[count].name, (const char *)name);
            strcpy(list[count].uuid, (const char *)(*entry)->GetKey());
        }
	NPT_LOG_INFO_3("%d)%s (%s)", ++count, (const char *)name, (const char *)(*entry)->GetKey());

        ++entry;
    }

    return ((count > 10) ? 10 : count);
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::GetDmrList
+---------------------------------------------------------------------*/
int 
PLT_MicroMediaController::GetDmrList(DeviceInfo list[])
{
    NPT_AutoLock lock(m_CurMediaRendererLock);

    int count = 0;
    const NPT_List<PLT_DeviceMapEntry*>& entries = m_MediaRenderers.GetEntries();
    NPT_List<PLT_DeviceMapEntry*>::Iterator entry = entries.GetFirstItem();

    while (entry) {
        PLT_DeviceDataReference device = (*entry)->GetValue();
        NPT_String              name   = device->GetFriendlyName();

        if(count < 10) {
            strcpy(list[count].name, (const char *)name);
            strcpy(list[count].uuid, (const char *)(*entry)->GetKey());
        }
	NPT_LOG_INFO_3("%d)%s (%s)", ++count, (const char *)name, (const char *)(*entry)->GetKey());

        ++entry;
    }

    return ((count > 10) ? 10 : count);
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::SetDms
+---------------------------------------------------------------------*/
int 
PLT_MicroMediaController::SetDms(const char *dmsname)
{
    NPT_AutoLock lock(m_CurMediaServerLock);

    PopDirectoryStackToRoot();

    const NPT_List<PLT_DeviceMapEntry*>& entries = GetMediaServersMap().GetEntries();
    NPT_List<PLT_DeviceMapEntry*>::Iterator entry = entries.GetFirstItem();

    while (entry) {
        PLT_DeviceDataReference device = (*entry)->GetValue();
        NPT_String              name   = device->GetFriendlyName();
        //NPT_LOG_INFO_2("%s(name): %s(dmsname)", (const char *)name, dmsname);
        if(strcmp(name, dmsname) == 0) {
            NPT_LOG_INFO_2("set dms to %s (%s)", (const char *)name, (const char *)(*entry)->GetKey());
            m_CurMediaServer = device;
            return 0;
        }
        ++entry;
    }

    return -1;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::SetDmr
+---------------------------------------------------------------------*/
int 
PLT_MicroMediaController::SetDmr(const char *dmrname)
{
    NPT_AutoLock lock(m_CurMediaRendererLock);

    const NPT_List<PLT_DeviceMapEntry*>& entries = m_MediaRenderers.GetEntries();
    NPT_List<PLT_DeviceMapEntry*>::Iterator entry = entries.GetFirstItem();

    while (entry) {
        PLT_DeviceDataReference device = (*entry)->GetValue();
        NPT_String              name   = device->GetFriendlyName();
        if(strcmp(name, dmrname) == 0) {
            NPT_LOG_INFO_2("set dmr to %s (%s)", (const char *)name, (const char *)(*entry)->GetKey());
            m_CurMediaRenderer = device;

            NPT_String model = device->GetModelDescription();
            if((!model.IsEmpty()) && (model.Compare("MSI MediaRenderer", true) == 0)) {
                NPT_LOG_INFO("Dmr model: MSI MediaRenderer");
                mIsMsiDmr = true;
            } else {
                mIsMsiDmr = false;
            }
            return 0;
        }
        ++entry;
    }

    return -1;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::EntryDirectory
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::EntryDirectory(char *dir)
{
    NPT_String id = dir;
    if(strcmp(dir, "..") == 0) {
        // we don't want to pop the root off now....
        NPT_String val;
        m_CurBrowseDirectoryStack.Peek(val);
        if (val.Compare("0")) {
            m_CurBrowseDirectoryStack.Pop(val);
        } else {
            NPT_LOG_INFO("Already at root\n");
        }
    } else {
        m_CurBrowseDirectoryStack.Push(id);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::GetItemCount
+---------------------------------------------------------------------*/
int
PLT_MicroMediaController::GetItemCount()
{
    int count = 0;

    DoBrowse();

    if (!m_MostRecentBrowseResults.IsNull()) {
        count = m_MostRecentBrowseResults->GetItemCount();
        NPT_LOG_INFO_1("There were %d results", count);
        m_MostRecentBrowseResults = NULL;
    }

    return count;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::GetItemList
+---------------------------------------------------------------------*/
int
PLT_MicroMediaController::GetItemList(ItemInfo list[])
{
    int count = 0;

    DoBrowse();

    if (!m_MostRecentBrowseResults.IsNull()) {
        NPT_LOG_INFO_1("There were %d results", m_MostRecentBrowseResults->GetItemCount());

        NPT_List<PLT_MediaObject*>::Iterator item = m_MostRecentBrowseResults->GetFirstItem();
        while (item) {
            if ((*item)->IsContainer()) {
                NPT_LOG_INFO_2("Container: %s (%s)", (*item)->m_Title.GetChars(), (*item)->m_ObjectID.GetChars());
                strcpy(list[count].title, (*item)->m_Title.GetChars());
                strcpy(list[count].path, (*item)->m_ObjectID.GetChars());
                strcpy(list[count].upnpclass, (*item)->m_ObjectClass.type.GetChars());
                list[count].size = 0;
                list[count].duration = 0;
                list[count].filetype = 0; 
            } else {
                NPT_LOG_INFO_2("Item: %s (%s)", (*item)->m_Title.GetChars(), (*item)->m_ObjectID.GetChars());
                //strcpy(list[count].title, NPT_FilePath::BaseName((*item)->m_ObjectID.GetChars()).GetChars());
                strcpy(list[count].title, (*item)->m_Title.GetChars());
                strcpy(list[count].path, (*item)->m_ObjectID.GetChars());
                strcpy(list[count].upnpclass, (*item)->m_ObjectClass.type.GetChars());
                list[count].size = (long long)(*item)->m_Resources[0].m_Size;
                list[count].duration = (*item)->m_Resources[0].m_Duration;
                list[count].filetype = 1; 
            }
            ++count;
            ++item;
        }

        m_MostRecentBrowseResults = NULL;
    }

    return count;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::GetFileItemCount
+---------------------------------------------------------------------*/
int
PLT_MicroMediaController::GetFileItemCount(int flags)
{
    int count = 0;

    PopDirectoryStackToRoot();

    if(flags == MEDIATYPE_IMAGE)
        m_CurBrowseDirectoryStack.Push("0/Image");
    else if(flags == MEDIATYPE_AUDIO)
        m_CurBrowseDirectoryStack.Push("0/Audio");
    else
        m_CurBrowseDirectoryStack.Push("0/Video");

    DoBrowse();

    if (!m_MostRecentBrowseResults.IsNull()) {
        NPT_LOG_INFO_1("There were %d results", m_MostRecentBrowseResults->GetItemCount());
        count = m_MostRecentBrowseResults->GetItemCount();
        m_MostRecentBrowseResults = NULL;
    }

    return count;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::GetFileList
+---------------------------------------------------------------------*/
int
PLT_MicroMediaController::GetFileList(int flags, FileInfo list[])
{
    int count = 0;

    PopDirectoryStackToRoot();

    if(flags == MEDIATYPE_IMAGE)
        m_CurBrowseDirectoryStack.Push("0/Image");
    else if(flags == MEDIATYPE_AUDIO)
        m_CurBrowseDirectoryStack.Push("0/Audio");
    else
        m_CurBrowseDirectoryStack.Push("0/Video");

    DoBrowse();

    if (!m_MostRecentBrowseResults.IsNull()) {
        NPT_LOG_INFO_1("There were %d results", m_MostRecentBrowseResults->GetItemCount());

        NPT_List<PLT_MediaObject*>::Iterator item = m_MostRecentBrowseResults->GetFirstItem();
        while (item) {
            if ((*item)->IsContainer()) {
                NPT_LOG_INFO_2("Container: %s (%s)", (*item)->m_Title.GetChars(), (*item)->m_ObjectID.GetChars());
            } else {
                NPT_LOG_INFO_2("Item: %s (%s)", (*item)->m_Title.GetChars(), (*item)->m_ObjectID.GetChars());
                strcpy(list[count].name, NPT_FilePath::BaseName((*item)->m_ObjectID.GetChars()).GetChars());
                count++;
            }
            ++item;
        }

        m_MostRecentBrowseResults = NULL;
    }

    return count;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::GetPosInfo
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::GetPosInfo()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        GetPositionInfo(device, 0, NULL);
    }
}

void PLT_MicroMediaController::OnGetPositionInfoResult(
    NPT_Result               res,
    PLT_DeviceDataReference& device,
    PLT_PositionInfo*        info,
    void*                    userdata) 
{
    if(res == NPT_SUCCESS) {
        //NPT_LOG_INFO_2("NPT_SUCCESS: Duration = %lld, RelTime = %lld", info->track_duration.ToSeconds(), info->rel_time.ToSeconds());
        mCallback->onDlnaGetPositionInfo(0, info->track_duration.ToSeconds(), info->rel_time.ToSeconds());
    } else {
        NPT_LOG_INFO("NPT_FAILURE");
        mCallback->onDlnaGetPositionInfo(-1, 0, 0);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::GetTransInfo
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::GetTransInfo()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        GetTransportInfo(device, 0, NULL);
    }
}

void PLT_MicroMediaController::OnGetTransportInfoResult(
    NPT_Result               res,
    PLT_DeviceDataReference& device,
    PLT_TransportInfo*        info,
    void*                    userdata) 
{
    if(res == NPT_SUCCESS) {
        if((mIsMsiDmr == true) && (info->cur_transport_state.Compare("NO_MEDIA_PRESENT" ,true) == 0))
            mCallback->onDlnaGetTransportInfo(0, (char *)"PLAYING");
        else
            mCallback->onDlnaGetTransportInfo(0, (char *)info->cur_transport_state);
    } else {
        NPT_LOG_INFO("NPT_FAILURE");
        mCallback->onDlnaGetTransportInfo(-1, (char *)"Failed");
    }
}

/*----------------------------------------------------------------------
| PLT_MicroMediaController::GetVolumeDBRange
| add by linhuaji 2014.07.01
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::GetVolumeDBRange()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);

    if (!device.IsNull()) {
        GetVolumeDBRangeInfo(device, 0, "Master", NULL);
    }
}

void PLT_MicroMediaController::OnGetVolumeDBRangeResult(
    NPT_Result               res,
    PLT_DeviceDataReference& device,
    const char*              channel,
    NPT_UInt32               min_volume,
    NPT_UInt32               max_volume,
    void*                    userdata) 
{
    if(res == NPT_SUCCESS) {
        NPT_LOG_INFO_3("GetVolumeDBRange NPT_SUCCESS: %s min_Volume = %d max_volume = %d", 
			channel, min_volume,max_volume);
        mCallback->onDlnaGetVolumeDBRangeInfo(0, min_volume, max_volume);
    } else {
        NPT_LOG_INFO("GetVolumeDBRange NPT_FAILURE");
        mCallback->onDlnaGetVolumeDBRangeInfo(-1, 0, 0);
    }
}

/*----------------------------------------------------------------------
| PLT_MicroMediaController::SetVolumeDB
| add by linhuaji 2014.07.01
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::SetVolumeDB(int voldb)
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);

    if (!device.IsNull()) {
        SetVolumeDBInfo(device, 0, "Master", voldb, NULL);
    }
}

void PLT_MicroMediaController::OnSetVolumeDBResult(
	NPT_Result				 res,
	PLT_DeviceDataReference& device,
	void*					 userdata) 
{
	if(res == NPT_SUCCESS) {
		NPT_LOG_INFO("NPT_SUCCESS");
		mCallback->onDlnaMediaControl(0, MEDIA_CTRL_SETVOLUMEDB);
	} else {
		NPT_LOG_INFO("NPT_FAILURE");
		mCallback->onDlnaMediaControl(-1, MEDIA_CTRL_SETVOLUMEDB);
	}
}

/*----------------------------------------------------------------------
| PLT_MicroMediaController::GetVolumeDB
| add by linhuaji 2014.07.01
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::GetVolumeDB()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);

    if (!device.IsNull()) {
        GetVolumeDBInfo(device, 0, "Master", NULL);
    }
}

void PLT_MicroMediaController::OnGetVolumeDBResult(
    NPT_Result               res,
    PLT_DeviceDataReference& device,
    const char*              channel,
    NPT_UInt32               volumedb,
    void*                    userdata) 
{
    if(res == NPT_SUCCESS) {
        NPT_LOG_INFO_2("GetVolumeDB NPT_SUCCESS: %s VolumDB = %d", 
			channel, volumedb);
        mCallback->onDlnaGetVolumeDBInfo(0, volumedb);
    } else {
        NPT_LOG_INFO("GetVolumeDB NPT_FAILURE");
        mCallback->onDlnaGetVolumeDBInfo(-1, 0);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::GetVol
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::GetVol()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);

    if (!device.IsNull()) {
        GetVolume(device, 0, "Master", NULL);
    }
}

void PLT_MicroMediaController::OnGetVolumeResult(
    NPT_Result               res,
    PLT_DeviceDataReference& device,
    const char*              channel,
    NPT_UInt32               volume,
    void*                    userdata) 
{
    if(res == NPT_SUCCESS) {
        NPT_LOG_INFO_2("NPT_SUCCESS: %s Volume = %d", channel, volume);
        mCallback->onDlnaGetVolumeInfo(0, volume);
    } else {
        NPT_LOG_INFO("NPT_FAILURE");
        mCallback->onDlnaGetVolumeInfo(-1, 0);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::SetVol
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::SetVol(int vol)
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);

    if (!device.IsNull()) {
        SetVolume(device, 0, "Master", vol, NULL);
    }
}

void PLT_MicroMediaController::OnSetVolumeResult(
    NPT_Result               res,
    PLT_DeviceDataReference& device,
    void*                    userdata) 
{
    if(res == NPT_SUCCESS) {
        NPT_LOG_INFO("NPT_SUCCESS");
        mCallback->onDlnaMediaControl(0, MEDIA_CTRL_SETVOLUME);
    } else {
        NPT_LOG_INFO("NPT_FAILURE");
        mCallback->onDlnaMediaControl(-1, MEDIA_CTRL_SETVOLUME);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::GetMute
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::GetMuteStatus()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);

    if (!device.IsNull()) {
        GetMute(device, 0, "Master", NULL);
    }
}

void PLT_MicroMediaController::OnGetMuteResult(
    NPT_Result               res,
    PLT_DeviceDataReference& device,
    const char*              channel,
    bool                     mute,
    void*                    userdata) 
{
    if(res == NPT_SUCCESS) {
        NPT_LOG_INFO_2("NPT_SUCCESS: %s mute = %d", channel, mute);
        mCallback->onDlnaGetMuteInfo(0, mute);
    } else {
        NPT_LOG_INFO("NPT_FAILURE");
        mCallback->onDlnaGetMuteInfo(-1, false);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::GetDownloadUri
+---------------------------------------------------------------------*/
int
PLT_MicroMediaController::GetDownloadUri(const char *uri, char *dluri, int flags)
{
    PLT_DeviceDataReference device;
    
    // issue a browse
    DoBrowse();
    
    if (!m_MostRecentBrowseResults.IsNull()) {
        // create a map item id -> item title
        NPT_List<PLT_MediaObject*>::Iterator item = m_MostRecentBrowseResults->GetFirstItem();
        while (item) {
            if (!(*item)->IsContainer()) {
                if(strcmp(uri, (*item)->m_ObjectID.GetChars()) == 0) {
	            NPT_LOG_INFO_1("Find file: %s", uri);
                    break;
                }
            }
            ++item;
        }
        
        if(item == 0) {
	    NPT_LOG_INFO("get uri failed: item is 0");
            m_MostRecentBrowseResults = NULL;
            return -1;
        }

        // let the user choose which one
        if ((*item)->m_ObjectID.GetLength()) {
            // look back for the PLT_MediaItem in the results
            PLT_MediaObject* track = NULL;
            if (NPT_SUCCEEDED(NPT_ContainerFind(*m_MostRecentBrowseResults, PLT_MediaItemIDFinder((*item)->m_ObjectID), track))) {
                if (track->m_Resources.GetItemCount() > 0) {
                    NPT_LOG_INFO_1("Download Resource[0].uri: %s", track->m_Resources[0].m_Uri.GetChars());
                    strcpy(dluri, (char *)track->m_Resources[0].m_Uri.GetChars());
                    m_MostRecentBrowseResults = NULL;
                    return 0;
                } else {
                    NPT_LOG_WARNING("No resources found");
                }
            } else {
                NPT_LOG_WARNING("Couldn't find the track\n");
            }
        }
        m_MostRecentBrowseResults = NULL;
    }
    
    return -1;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::CtrlPlay2
+---------------------------------------------------------------------*/
int
PLT_MicroMediaController::CtrlPlay2(const char *uri, int flags)
{
/* open */
    PLT_DeviceDataReference device;

    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        // get the protocol info to try to see in advance if a track would play on the device
	NPT_LOG_INFO_1("CtrlPlay2 Render Device found: %s", (char*)device->GetFriendlyName());
        // issue a browse
        DoBrowse();

        if (!m_MostRecentBrowseResults.IsNull()) {
            // create a map item id -> item title
            NPT_List<PLT_MediaObject*>::Iterator item = m_MostRecentBrowseResults->GetFirstItem();
            while (item) {
                if (!(*item)->IsContainer()) {
                    if(strcmp(uri, (*item)->m_ObjectID.GetChars()) == 0) {
	                NPT_LOG_INFO_1("Find file: %s", uri);
                        break;
                    }
	            //NPT_LOG_INFO_2("%s: %s", uri, (*item)->m_ObjectID.GetChars());
                }
                ++item;
            }

            if(item == 0) {
	        NPT_LOG_INFO_1("play failed: item is 0, uri: %s", uri);
                m_MostRecentBrowseResults = NULL;
                return -1;
            }

            // let the user choose which one
            if ((*item)->m_ObjectID.GetLength()) {
                // look back for the PLT_MediaItem in the results
                PLT_MediaObject* track = NULL;
                if (NPT_SUCCEEDED(NPT_ContainerFind(*m_MostRecentBrowseResults, PLT_MediaItemIDFinder((*item)->m_ObjectID), track))) {
                    if (track->m_Resources.GetItemCount() > 0) {
                        // look for best resource to use by matching each resource to a sink advertised by renderer
                        NPT_Cardinal resource_index = 0;
                        if (NPT_FAILED(FindBestResource(device, *track, resource_index))) {
                            NPT_LOG_WARNING("No matching resource");
                        /* just for debug: commented by lihaihua */
                            //return -1;
                        /* end */
                        }

                        if(strcmp(track->m_ObjectClass.type.GetChars(), "object.item") == 0) {
	                    NPT_LOG_INFO("Change object class");
                            if(flags == MEDIATYPE_IMAGE) {
                                track->m_Didl.Replace("object.item", "object.item.imageItem.photo");
                            } else if(flags == MEDIATYPE_AUDIO) {
                                track->m_Didl.Replace("object.item", "object.item.audioItem.musicTrack");
                            } else {
                                track->m_Didl.Replace("object.item", "object.item.videoItem");
                            }
                        }

                        // invoke the setUri
                        NPT_LOG_INFO_2("Issuing SetAVTransportURI with url=%s & didl=%s", 
                            (const char*)track->m_Resources[resource_index].m_Uri, 
                            (const char*)track->m_Didl);

                        SetAVTransportURI(device, 0, track->m_Resources[resource_index].m_Uri, track->m_Didl, NULL);
                        m_MostRecentBrowseResults = NULL;

                        NPT_System::Sleep(NPT_TimeInterval(0.3f));

                        /* play */
	                NPT_LOG_INFO_1("Play file: %s", uri);
                        Play(device, 0, "1", NULL);

                        #if 0
                        NPT_System::Sleep(NPT_TimeInterval(10.0f));
                        GetPosInfo();

                        NPT_System::Sleep(NPT_TimeInterval(10.0f));
                        GetPosInfo();
                        #endif

                        return 0;
                    } else {
                        NPT_LOG_WARNING("Couldn't find the proper resource");
                    }

                } else {
                    NPT_LOG_WARNING("Couldn't find the track");
                }
            }
            m_MostRecentBrowseResults = NULL;
        }
    }

    return -1;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::CtrlPlay
+---------------------------------------------------------------------*/
int
PLT_MicroMediaController::CtrlPlay(const char *uri, int flags)
{
/* open */
    PLT_DeviceDataReference device;

    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        // get the protocol info to try to see in advance if a track would play on the device

        PopDirectoryStackToRoot();

        NPT_String filename = NPT_FilePath::BaseName(uri, true);
        if(flags == MEDIATYPE_IMAGE) {
            m_CurBrowseDirectoryStack.Push("0/Image");
            filename = "0/Image/" + filename;
        } else if(flags == MEDIATYPE_AUDIO) {
            m_CurBrowseDirectoryStack.Push("0/Audio");
            filename = "0/Audio/" + filename;
        } else {
            m_CurBrowseDirectoryStack.Push("0/Video");
            filename = "0/Video/" + filename;
        }
        NPT_LOG_INFO_1("filename: %s", (const char *)filename);

        // issue a browse
        DoBrowse();

        if (!m_MostRecentBrowseResults.IsNull()) {
            // create a map item id -> item title
            NPT_List<PLT_MediaObject*>::Iterator item = m_MostRecentBrowseResults->GetFirstItem();
            while (item) {
                if (!(*item)->IsContainer()) {
                    if(strcmp(filename, (*item)->m_ObjectID.GetChars()) == 0) {
	                NPT_LOG_INFO_1("Find file: %s", uri);
                        break;
                    }
                }
                ++item;
            }

            if(item == 0) {
	        NPT_LOG_INFO("play failed: item is 0");
                m_MostRecentBrowseResults = NULL;
                return -1;
            }

            // let the user choose which one
            if ((*item)->m_ObjectID.GetLength()) {
                // look back for the PLT_MediaItem in the results
                PLT_MediaObject* track = NULL;
                if (NPT_SUCCEEDED(NPT_ContainerFind(*m_MostRecentBrowseResults, PLT_MediaItemIDFinder((*item)->m_ObjectID), track))) {
                    if (track->m_Resources.GetItemCount() > 0) {
                        // look for best resource to use by matching each resource to a sink advertised by renderer
                        NPT_Cardinal resource_index = 0;
                        if (NPT_FAILED(FindBestResource(device, *track, resource_index))) {
                            NPT_LOG_WARNING("No matching resource");
                        /* just for debug: commented by lihaihua */
                            //return -1;
                        /* end */
                        }

                        if(strcmp(track->m_ObjectClass.type.GetChars(), "object.item") == 0) {
	                    NPT_LOG_INFO("Change object class");
                            if(flags == MEDIATYPE_IMAGE) {
                                track->m_Didl.Replace("object.item", "object.item.imageItem.photo");
                            } else if(flags == MEDIATYPE_AUDIO) {
                                track->m_Didl.Replace("object.item", "object.item.audioItem.musicTrack");
                            } else {
                                track->m_Didl.Replace("object.item", "object.item.videoItem");
                            }
                        }

                        // invoke the setUri
                        NPT_LOG_INFO_2("Issuing SetAVTransportURI with url=%s & didl=%s", 
                            (const char*)track->m_Resources[resource_index].m_Uri, 
                            (const char*)track->m_Didl);

                        SetAVTransportURI(device, 0, track->m_Resources[resource_index].m_Uri, track->m_Didl, NULL);
                        m_MostRecentBrowseResults = NULL;

                        NPT_System::Sleep(NPT_TimeInterval(0.3f));

                        /* play */
	                NPT_LOG_INFO_1("Play file: %s", uri);
                        Play(device, 0, "1", NULL);

                        #if 0
                        NPT_System::Sleep(NPT_TimeInterval(10.0f));
                        GetPosInfo();

                        NPT_System::Sleep(NPT_TimeInterval(10.0f));
                        GetPosInfo();
                        #endif

                        return 0;
                    } else {
                        NPT_LOG_WARNING("Couldn't find the proper resource");
                    }

                } else {
                    NPT_LOG_WARNING("Couldn't find the track");
                }
            }
            m_MostRecentBrowseResults = NULL;
        }
    }

    return -1;
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::CtrlPlayOnline
+---------------------------------------------------------------------*/
int
PLT_MicroMediaController::CtrlPlayOnline(const char *uri, const char *title, int flags)
{
/* open */
    PLT_DeviceDataReference device;

    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        // invoke the setUri
        char didl[1024];
        char upnpClass[64];

        if(flags == MEDIATYPE_IMAGE)
            strcpy(upnpClass, "object.item.imageItem.photo");
        else if(flags == MEDIATYPE_AUDIO)
            strcpy(upnpClass, "object.item.audioItem.musicTrack");
        else
            strcpy(upnpClass, "object.item.videoItem");

        //snprintf(didl, 1024, "<DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns:dlna=\"urn:schemas-dlna-org:metadata-1-0/\"><item id=\"0/1\" parentID=\"0\" restricted=\"1\"><dc:title>%s</dc:title><upnp:class>%s</upnp:class></item></DIDL-Lite>", title, upnpClass);
        snprintf(didl, 1024, "%s<item id=\"0/1\" parentID=\"0\" restricted=\"1\"><dc:title>%s</dc:title><upnp:class>%s</upnp:class></item>%s", didl_header, title, upnpClass, didl_footer);

        NPT_LOG_INFO_2("Issuing SetAVTransportURI with url=%s & didl=%s", uri, didl);
        SetAVTransportURI(device, 0, uri, didl, NULL);

        m_MostRecentBrowseResults = NULL;

        NPT_System::Sleep(NPT_TimeInterval(0.3f));

        /* play */
	NPT_LOG_INFO_2("Play online uri: %s, title: %s", uri, title);
        Play(device, 0, "1", NULL);

        return 0;
    }

    return -1;
}

void PLT_MicroMediaController::OnPlayResult(
    NPT_Result               res,
    PLT_DeviceDataReference& device,
    void*                    userdata) 
{
    if(res == NPT_SUCCESS) {
        NPT_LOG_INFO("NPT_SUCCESS");
        mCallback->onDlnaMediaControl(0, MEDIA_CTRL_PLAY);
    } else {
        NPT_LOG_INFO("NPT_FAILURE");
        mCallback->onDlnaMediaControl(-1, MEDIA_CTRL_PLAY);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::CtrlSeek
+---------------------------------------------------------------------*/
int
PLT_MicroMediaController::CtrlSeek(const char *relTime)
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        // remove first part of command ("seek")
        NPT_String target = relTime;
        
        if (target.Find(":") != -1) {
            return Seek(device, 0, "REL_TIME", target, NULL);
        } else {
            NPT_LOG_WARNING("Wrong seek mode, please use format XX:XX:XX");
        }
    }

    return -1;
}

void PLT_MicroMediaController::OnSeekResult(
    NPT_Result               res,
    PLT_DeviceDataReference& device,
    void*                    userdata) 
{
    if(res == NPT_SUCCESS) {
        NPT_LOG_INFO("NPT_SUCCESS");
        mCallback->onDlnaMediaControl(0, MEDIA_CTRL_SEEK);
    } else {
        NPT_LOG_INFO("NPT_FAILURE");
        mCallback->onDlnaMediaControl(-1, MEDIA_CTRL_SEEK);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::CtrlPause
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::CtrlPause()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        Pause(device, 0, NULL);
    }
}

void PLT_MicroMediaController::OnPauseResult(
    NPT_Result               res,
    PLT_DeviceDataReference& device,
    void*                    userdata) 
{
    if(res == NPT_SUCCESS) {
        NPT_LOG_INFO("NPT_SUCCESS");
        mCallback->onDlnaMediaControl(0, MEDIA_CTRL_PAUSE);
    } else {
        NPT_LOG_INFO("NPT_FAILURE");
        mCallback->onDlnaMediaControl(-1, MEDIA_CTRL_PAUSE);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::CtrlResume
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::CtrlResume()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        Play(device, 0, "1", NULL);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::CtrlStop
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::CtrlStop()
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        Stop(device, 0, NULL);
    }
}

void PLT_MicroMediaController::OnStopResult(
    NPT_Result               res,
    PLT_DeviceDataReference& device,
    void*                    userdata) 
{
    if(res == NPT_SUCCESS) {
        NPT_LOG_INFO("NPT_SUCCESS");
        mCallback->onDlnaMediaControl(0, MEDIA_CTRL_STOP);
    } else {
        NPT_LOG_INFO("NPT_FAILURE");
        mCallback->onDlnaMediaControl(-1, MEDIA_CTRL_STOP);
    }
}

/*----------------------------------------------------------------------
|   PLT_MicroMediaController::CtrlMute
+---------------------------------------------------------------------*/
void
PLT_MicroMediaController::CtrlMute(bool mute)
{
    PLT_DeviceDataReference device;
    GetCurMediaRenderer(device);
    if (!device.IsNull()) {
        SetMute(device, 0, "Master", mute, NULL);
    }
}

void PLT_MicroMediaController::OnSetMuteResult(
    NPT_Result               res,
    PLT_DeviceDataReference& device,
    void*                    userdata) 
{
    if(res == NPT_SUCCESS) {
        NPT_LOG_INFO("NPT_SUCCESS");
        mCallback->onDlnaMediaControl(0, MEDIA_CTRL_MUTE);
    } else {
        NPT_LOG_INFO("NPT_FAILURE");
        mCallback->onDlnaMediaControl(-1, MEDIA_CTRL_MUTE);
    }
}

/* end */
