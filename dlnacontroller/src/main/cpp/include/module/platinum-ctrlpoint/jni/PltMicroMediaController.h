/*****************************************************************
|
|   Platinum - Micro Media Controller
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

#ifndef _MICRO_MEDIA_CONTROLLER_H_
#define _MICRO_MEDIA_CONTROLLER_H_

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
|   definitions
+---------------------------------------------------------------------*/
typedef NPT_Map<NPT_String, NPT_String>              PLT_StringMap;
typedef NPT_Lock<PLT_StringMap>                      PLT_LockStringMap;
typedef NPT_Map<NPT_String, NPT_String>::Entry       PLT_StringMapEntry;

/* added by lihaihua */
typedef struct {
    char name[256];
    char uuid[64];
} DeviceInfo;

typedef struct {
    char title[256];
    char path[256];
    char upnpclass[64];
    long long size;
    int duration;
    int filetype;
} ItemInfo;

typedef struct {
    char name[256];
} FileInfo;

#define MEDIATYPE_IMAGE 0
#define MEDIATYPE_AUDIO 1
#define MEDIATYPE_VIDEO 2

#define MEDIA_CTRL_MSG_BASE 0X100
#define MEDIA_CTRL_PLAY (MEDIA_CTRL_MSG_BASE + 0)
#define MEDIA_CTRL_PAUSE (MEDIA_CTRL_MSG_BASE + 1)
#define MEDIA_CTRL_RESUME (MEDIA_CTRL_MSG_BASE + 2)
#define MEDIA_CTRL_SEEK (MEDIA_CTRL_MSG_BASE + 3)
#define MEDIA_CTRL_STOP (MEDIA_CTRL_MSG_BASE + 4)
#define MEDIA_CTRL_MUTE (MEDIA_CTRL_MSG_BASE + 5)
#define MEDIA_CTRL_SETVOLUME (MEDIA_CTRL_MSG_BASE + 6)
#define MEDIA_CTRL_GETVOLUME (MEDIA_CTRL_MSG_BASE + 7)
#define MEDIA_CTRL_POSITION (MEDIA_CTRL_MSG_BASE + 8)
#define MEDIA_CTRL_GETMUTE (MEDIA_CTRL_MSG_BASE + 9)
/* add by linhuaji, 2014.07.01 */
#define MEDIA_CTRL_SETVOLUMEDB (MEDIA_CTRL_MSG_BASE + 10)
#define MEDIA_CTRL_GETVOLUMEDB (MEDIA_CTRL_MSG_BASE + 11)
/* end by linhuaji */

/* end */

/*----------------------------------------------------------------------
|   PLT_MediaItemIDFinder
+---------------------------------------------------------------------*/
class PLT_MediaItemIDFinder
{
public:
    // methods
    PLT_MediaItemIDFinder(const char* object_id) : m_ObjectID(object_id) {}

    bool operator()(const PLT_MediaObject* const & item) const {
        return item->m_ObjectID.Compare(m_ObjectID, true) ? false : true;
    }

private:
    // members
    NPT_String m_ObjectID;
};

/* added by lihaihua */
class DeviceCallback
{
public:
    DeviceCallback();
    virtual ~DeviceCallback();
    virtual int onDlnaDeviceAdded(char* path, char *uuid, int type) { return 0; }
    virtual int onDlnaDeviceRemoved(char* path, char *uuid, int type) { return 0; }
    virtual int onDlnaGetPositionInfo(int res, long duration, long relTime) { return 0; }
    virtual int onDlnaGetTransportInfo(int res, char *state) { return 0; }
    virtual int onDlnaGetVolumeInfo(int res, long volume) { return 0; }
    virtual int onDlnaGetMuteInfo(int res, bool mute) { return 0; }
    virtual int onDlnaMediaControl(int res, int cmd) { return 0; }
    virtual int onDlnaDmrStatusChanged(char* name, char *uuid) { return 0; }
    /* add by linhuaji, 2014.07.01 */
    virtual int onDlnaGetVolumeDBInfo(int res, long volumedb) { return 0; }
    virtual int onDlnaGetVolumeDBRangeInfo(int res, long min_volume, long max_volume) { return 0; }
    virtual int onRapidTransmitVideo(const char *filename){return 0;}
	/* end by linhuaji */
private:
    uint32_t mTest;
};
/* end */

/*----------------------------------------------------------------------
|   PLT_MicroMediaController
+---------------------------------------------------------------------*/
class PLT_MicroMediaController : public PLT_SyncMediaBrowser,
                                 public PLT_MediaController,
                                 public PLT_MediaControllerDelegate,
                                 public PLT_MediaContainerChangesListener
{
public:
    PLT_MicroMediaController(PLT_CtrlPointReference& ctrlPoint, DeviceCallback *callback);
    virtual ~PLT_MicroMediaController();

    void ProcessCommandLoop();

    // PLT_MediaBrowserDelegate methods
    bool OnMSAdded(PLT_DeviceDataReference& device);

    // PLT_MediaControllerDelegate methods
    bool OnMRAdded(PLT_DeviceDataReference& device);
    void OnMRRemoved(PLT_DeviceDataReference& device);
    void OnMRStateVariablesChanged(PLT_Service* /* service */, 
                                   NPT_List<PLT_StateVariable*>* /* vars */);
    
    // PLT_HttpClientTask method
    NPT_Result ProcessResponse(NPT_Result                    res,
                               const NPT_HttpRequest&        request,
                               const NPT_HttpRequestContext& context,
                               NPT_HttpResponse*             response);

    /* qiku add */
    //linhuaji add PLT_MediaContainerChangesListener method
    void OnContainerChanged(PLT_DeviceDataReference& device, 
                                    const char*              item_id, 
                                    const char*              update_id);
    void OnRapidTransmitFileNameChange(const char *filename);
    /* qiku end */

/* added by lihaihua */
    void ScanDevice(long mx);
    int GetDmsList(DeviceInfo list[]);
    int GetDmrList(DeviceInfo list[]);
    int SetDms(const char *dmsname);
    int SetDmr(const char *dmrname);
    void EntryDirectory(char *dir);
    int GetItemCount(void);
    int GetItemList(ItemInfo list[]);
    int GetFileItemCount(int flags);
    int GetFileList(int flags, FileInfo list[]);
    void GetPosInfo(void);
    void GetTransInfo(void);
    void GetVol(void);
    void SetVol(int vol);
    void GetMuteStatus(void);
    int GetDownloadUri(const char *uri, char *dluri, int flags);
    int CtrlPlay2(const char *uri, int flags);
    int CtrlPlay(const char *uri, int flags);
    int CtrlPlayOnline(const char *uri, const char *title, int flags);
    int CtrlSeek(const char *relTime);
    void CtrlPause(void);
    void CtrlResume(void);
    void CtrlStop(void);
    void CtrlMute(bool mute);

    void OnGetPositionInfoResult(NPT_Result res, PLT_DeviceDataReference& device, PLT_PositionInfo* info, void* userdata);
    void OnGetTransportInfoResult(NPT_Result res, PLT_DeviceDataReference& device, PLT_TransportInfo* info, void* userdata);
    void OnGetVolumeResult(NPT_Result res, PLT_DeviceDataReference& device, const char* channel, NPT_UInt32 volume, void* userdata);
    void OnGetMuteResult(NPT_Result res, PLT_DeviceDataReference& device, const char* channel, bool mute, void* userdata);
    void OnSetVolumeResult(NPT_Result res, PLT_DeviceDataReference& device, void* userdata);
    void OnPlayResult(NPT_Result res, PLT_DeviceDataReference& device, void* userdata);
    void OnSeekResult(NPT_Result res, PLT_DeviceDataReference& device, void* userdata);
    void OnPauseResult(NPT_Result res, PLT_DeviceDataReference& device, void* userdata);
    void OnStopResult(NPT_Result res, PLT_DeviceDataReference& device, void* userdata);
    void OnSetMuteResult(NPT_Result res, PLT_DeviceDataReference& device, void* userdata);
    /* add by linhuaji, 2014.07.01 */
	void GetVolumeDB(void);
    void SetVolumeDB(int voldb);
    void GetVolumeDBRange(void);	
    void OnGetVolumeDBResult(NPT_Result res, PLT_DeviceDataReference& device, const char* channel, NPT_UInt32 volumedb, void* userdata);
    void OnSetVolumeDBResult(NPT_Result res, PLT_DeviceDataReference& device, void* userdata);
    void OnGetVolumeDBRangeResult(NPT_Result res, PLT_DeviceDataReference& device, const char* channel, NPT_UInt32 min_volume, NPT_UInt32 max_volume, void* userdata);
	/* end by linhuaji */
/* end */
    
private:
    const char* ChooseIDFromTable(PLT_StringMap& table);
    void        PopDirectoryStackToRoot(void);
    NPT_Result  DoBrowse(const char* object_id = NULL, bool metdata = false);

    void        GetCurMediaServer(PLT_DeviceDataReference& server);
    void        GetCurMediaRenderer(PLT_DeviceDataReference& renderer);

    PLT_DeviceDataReference ChooseDevice(const NPT_Lock<PLT_DeviceMap>& deviceList);

    // Command Handlers
    void    HandleCmd_scan(const char* ip);
    void    HandleCmd_getms();
    void    HandleCmd_setms();
    void    HandleCmd_ls();
    void    HandleCmd_info();
    void    HandleCmd_cd(const char* command);
    void    HandleCmd_cdup();
    void    HandleCmd_pwd();
    void    HandleCmd_help();
    void    HandleCmd_getmr();
    void    HandleCmd_setmr();
    void    HandleCmd_download();
    void    HandleCmd_open();
    void    HandleCmd_play();
    void    HandleCmd_seek(const char* command);
    void    HandleCmd_stop();
    void    HandleCmd_mute();
    void    HandleCmd_unmute();

private:
    /* added by lihaihua */
    DeviceCallback *mCallback;
    bool mIsMsiDmr;
    /* end */

    /* Tables of known devices on the network.  These are updated via the
     * OnMSAddedRemoved and OnMRAddedRemoved callbacks.  Note that you should first lock
     * before accessing them using the NPT_Map::Lock function.
     */
    NPT_Lock<PLT_DeviceMap> m_MediaServers;
    NPT_Lock<PLT_DeviceMap> m_MediaRenderers;

    /* Currently selected media server as well as 
     * a lock.  If you ever want to hold both the m_CurMediaRendererLock lock and the 
     * m_CurMediaServerLock lock, make sure you grab the server lock first.
     */
    PLT_DeviceDataReference m_CurMediaServer;
    NPT_Mutex               m_CurMediaServerLock;

    /* Currently selected media renderer as well as 
     * a lock.  If you ever want to hold both the m_CurMediaRendererLock lock and the 
     * m_CurMediaServerLock lock, make sure you grab the server lock first.
     */
    PLT_DeviceDataReference m_CurMediaRenderer;
    NPT_Mutex               m_CurMediaRendererLock;

    /* Most recent results from a browse request.  The results come back in a 
     * callback instead of being returned to the calling function, so this 
     * variable is necessary in order to give the results back to the calling 
     * function.
     */
    PLT_MediaObjectListReference m_MostRecentBrowseResults;

    /* When browsing through the tree on a media server, this is the stack 
     * symbolizing the current position in the tree.  The contents of the 
     * stack are the object ID's of the nodes.  Note that the object id: "0" should
     * always be at the bottom of the stack.
     */
    NPT_Stack<NPT_String> m_CurBrowseDirectoryStack;

    /* Semaphore on which to block when waiting for a response from over
     * the network 
     */
    NPT_SharedVariable m_CallbackResponseSemaphore;
    
    /* Task Manager managing download tasks */
    PLT_TaskManager m_DownloadTaskManager;
};

#endif /* _MICRO_MEDIA_CONTROLLER_H_ */

