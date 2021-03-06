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
#include "NptMap.h"
#include "NptStack.h"
#include "MediaControllerListener.h"
#include <android/log.h>

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, "sinadlna_jni", __VA_ARGS__))
/*----------------------------------------------------------------------
|   definitions
+---------------------------------------------------------------------*/
typedef NPT_Map<NPT_String, NPT_String>              PLT_StringMap;
typedef NPT_Lock<PLT_StringMap>                      PLT_LockStringMap;
typedef NPT_Map<NPT_String, NPT_String>::Entry       PLT_StringMapEntry;

//class SinaDLNAListener;

/*----------------------------------------------------------------------
|   PLT_MicroMediaController
+---------------------------------------------------------------------*/
class PLT_MicroMediaController :
                                 //public PLT_MediaController,
                                 public PLT_MediaControllerDelegate,
								 public PLT_MediaBrowserDelegate
{
public:
    PLT_MicroMediaController();
    virtual ~PLT_MicroMediaController();

	NPT_Result startMediaController();
	NPT_Result stopMediaController();
	//listener
	void 	setMediaControllerListener(MediaControllerListener* listener){this->m_mr_listener=listener;}

	//control devices
	void    setDMR(const char *uuid); //set currenr mr by uuid
	const char*  getDMR(); //get current mr uuid
	void    setDMS(const char *uuid);
	const char*  getDMS();

	//server controller
	void        PopDirectoryStackToRoot(void);
	NPT_Result  DoBrowse(const char* object_id = NULL, bool metadata = false);

    //render controller
	void    open(const char* uri, const char* didl);
    void    play();
	void 	pause();
    void    seek(const char* time);  //time format 00:06:38
    void    stop();
	void    setMute(bool yes);
    void    getMute();
    void    setVolume(int vol);
    void    getVolume();
	void    getVolumeRange(int& volMin, int& volMax);
	void    getMediaInfo();
    void    getPositionInfo();
	void OnMRStateVariablesChanged(PLT_Service* service,
								   NPT_List<PLT_StateVariable*>* vars);
	//server change
	bool OnMSAdded(PLT_DeviceDataReference& /* device */);

    void OnMSRemoved(PLT_DeviceDataReference& /* device */);

    void OnMSStateVariablesChanged(
            PLT_Service*                  service,
            NPT_List<PLT_StateVariable*>* vars);

    // ContentDirectory
    void OnBrowseResult(
            NPT_Result               res,
            PLT_DeviceDataReference& device,
            PLT_BrowseInfo*          info,
            void*                    userdata);

    void OnSearchResult(
            NPT_Result               res,
            PLT_DeviceDataReference& device,
            PLT_BrowseInfo*          info,
            void*                    userdata);

    // PLT_MediaControllerDelegate methods
    bool OnMRAdded(PLT_DeviceDataReference& device);

    void OnMRRemoved(PLT_DeviceDataReference& device);

	// AVTransport
	void OnGetMediaInfoResult(
        NPT_Result               res,
        PLT_DeviceDataReference& device,
        PLT_MediaInfo*           info ,
        void*                    userdata);

    void OnGetPositionInfoResult(
        NPT_Result               res ,
        PLT_DeviceDataReference& device ,
        PLT_PositionInfo*        info ,
        void*                    userdata );

	void OnSetAVTransportURIResult(
        NPT_Result               res ,
        PLT_DeviceDataReference& device ,
        void*                    userdata);


	void OnPlayResult(
        NPT_Result               res ,
        PLT_DeviceDataReference& device ,
        void*                    userdata);

    void OnPauseResult(
        NPT_Result               res ,
        PLT_DeviceDataReference& device,
        void*                     userdata );


    void OnSeekResult(
        NPT_Result               res ,
        PLT_DeviceDataReference& device,
        void*                    userdata);


	void OnStopResult(
        NPT_Result               res ,
        PLT_DeviceDataReference& device ,
        void*                    userdata);

	// RenderingControl
    void OnSetMuteResult(
        NPT_Result               res,
        PLT_DeviceDataReference& device,
        void*                    userdata);

    void OnGetMuteResult(
        NPT_Result               res ,
        PLT_DeviceDataReference& device ,
        const char*              channel,
        bool                     mute,
        void*                    userdata) ;

	void OnSetVolumeResult(
        NPT_Result               res ,
        PLT_DeviceDataReference& device,
        void*                    userdata);

	void OnGetVolumeResult(
        NPT_Result               res,
        PLT_DeviceDataReference& device,
		const char*              channel,
    	NPT_UInt32				 volume ,
	    void*                    userdata);


private:
    void        GetCurMediaRenderer(PLT_DeviceDataReference& renderer);
	void        GetCurMediaServer(PLT_DeviceDataReference& server);
private:
    /* Tables of known devices on the network.  These are updated via the
     * OnMRAddedRemoved callbacks.  Note that you should first lock
     * before accessing them using the NPT_Map::Lock function.
     */
    NPT_Lock<PLT_DeviceMap> m_Devices;

    /* Currently selected media renderer as well as 
     * a lock.  If you ever want to hold both the m_CurMediaRendererLock lock and the 
     * m_CurMediaServerLock lock, make sure you grab the server lock first.
     */
	PLT_DeviceDataReference m_CurMediaRenderer;
	NPT_Mutex               m_CurMediaRendererLock;

    /* Currently selected media server as well as
     * a lock.  If you ever want to hold both the m_CurMediaRendererLock lock and the
     * m_CurMediaServerLock lock, make sure you grab the server lock first.
     */
    PLT_DeviceDataReference m_CurMediaServer;
    NPT_Mutex               m_CurMediaServerLock;

	/* When browsing through the tree on a media server, this is the stack
     * symbolizing the current position in the tree.  The contents of the
     * stack are the object ID's of the nodes.  Note that the object id: "0" should
     * always be at the bottom of the stack.
     */
	NPT_Stack<NPT_String> m_CurBrowseDirectoryStack;

    /* Most recent results from a browse request.  The results come back in a
     * callback instead of being returned to the calling function, so this
     * variable is necessary in order to give the results back to the calling
     * function.
     */
    PLT_MediaObjectListReference m_MostRecentBrowseResults;

	MediaControllerListener*	m_mr_listener;
	NPT_Mutex               m_mr_listener_lock;

	PLT_MediaController* plt_mediaController;
	PLT_SyncMediaBrowser* plt_syncMediaBrowser;

	PLT_UPnP* uPnP;
	PLT_CtrlPointReference ctrlPoint;

	//FIX BUG
	int m_TrackDuration; //unit msec
	int m_MediaDuration; //unit msec
};

#endif 

