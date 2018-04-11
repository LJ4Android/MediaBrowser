/*****************************************************************
|
|   Platinum - File Transmit Controler (File Transmit Control Point)
|
| Copyright (c) 2013-2014, 
| All rights reserved.
|
****************************************************************/

#ifndef _PLT_FILE_TRANMIT_CONTROLLER_H_
#define _PLT_FILE_TRANMIT_CONTROLLER_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "PltCtrlPoint.h"
#include "PltMediaItem.h"

/*----------------------------------------------------------------------
|   Defines
+---------------------------------------------------------------------*/



/*----------------------------------------------------------------------
|   PLT_MediaControllerDelegate
+---------------------------------------------------------------------*/
class PLT_FileTransmitControllerDelegate
{
public:
    virtual ~PLT_FileTransmitControllerDelegate() {}

    virtual bool OnFTDAdded(PLT_DeviceDataReference& /* device */) { return true; }
    virtual void OnFTDRemoved(PLT_DeviceDataReference& /* device */) {}
    virtual void OnFTDStateVariablesChanged(PLT_Service*                  /* service */, 
                                           NPT_List<PLT_StateVariable*>* /* vars */) {}

    // DowlodControl
    virtual void OnStartTransmitFileResult(
        NPT_Result               /* res */, 
        PLT_DeviceDataReference& /* device */,
        const char*          /* sid */, 
        const char*          /* filename */, 
        void*                    /* userdata */) {}

    virtual void OnCancelTransmitFileResult(
        NPT_Result               /* res */, 
        PLT_DeviceDataReference& /* device */,
        const char*  /* sid */,
        void*                    /* userdata */) {}

    virtual void OnFinishTransmitFileResult(
        NPT_Result               /* res */,
        PLT_DeviceDataReference& /* device */,
        const char*           /* sid */,
        void*                    /* userdata */) {}

    virtual void OnGetTransmitFileProgressResult(
        NPT_Result               /* res */,
        PLT_DeviceDataReference& /* device */,
        const char*           /* sid */,
        const char*           /* progress */,
        void*                    /* userdata */) {}

};

/*----------------------------------------------------------------------
|   PLT_MediaController
+---------------------------------------------------------------------*/
class PLT_FileTransmitController : public PLT_CtrlPointListener
{
public:
    PLT_FileTransmitController(PLT_CtrlPointReference&      ctrl_point, 
                        PLT_FileTransmitControllerDelegate* delegate = NULL);
    virtual ~PLT_FileTransmitController();

    // public methods
    virtual void SetDelegate(PLT_FileTransmitControllerDelegate* delegate) {
        m_Delegate = delegate;
    }

    // PLT_CtrlPointListener methods
    virtual NPT_Result OnDeviceAdded(PLT_DeviceDataReference& device);
    virtual NPT_Result OnDeviceRemoved(PLT_DeviceDataReference& device);
    virtual NPT_Result OnActionResponse(NPT_Result res, PLT_ActionReference& action, void* userdata);
    virtual NPT_Result OnEventNotify(PLT_Service* service, NPT_List<PLT_StateVariable*>* vars);

    // DowlodControl
    NPT_Result startTransmitFile(PLT_DeviceDataReference&  device, 
										  NPT_UInt32				instance_id, 
										  const char*				channel,
										  NPT_String 				session_id, 
										  NPT_String 				filename, 
										  NPT_String 				filesize, 
										  NPT_String 				from,
										  void* 					userdata) ;
    NPT_Result cancelTransmitFile(PLT_DeviceDataReference&  device, 
										  NPT_UInt32				instance_id, 
										  const char*				channel,
										  NPT_String 				session_id, 
										  void* 					userdata) ;
    NPT_Result finishTransmitFile(PLT_DeviceDataReference&  device, 
										  NPT_UInt32				instance_id, 
										  const char*				channel,
										  NPT_String 				session_id, 
										  void* 					userdata) ;
    NPT_Result getTransmitFileProgress(PLT_DeviceDataReference&  device, 
										  NPT_UInt32				instance_id, 
										  const char*				channel,
										  NPT_String 				session_id, 
										  void* 					userdata) ;


private:
    NPT_Result InvokeActionWithInstance(PLT_ActionReference& action, NPT_UInt32 instance_id, void* userdata = NULL);
    NPT_Result FindFTDevice(const char* uuid, PLT_DeviceDataReference& device);
    NPT_Result OnStartTransmitFileResponse(NPT_Result res, PLT_DeviceDataReference& device, PLT_ActionReference& action, void* userdata);
    NPT_Result OnCancelTransmitFileResponse(NPT_Result res, PLT_DeviceDataReference& device, PLT_ActionReference& action, void* userdata);
    NPT_Result OnFinishTransmitFileResponse(NPT_Result res, PLT_DeviceDataReference& device, PLT_ActionReference& action, void* userdata);
    NPT_Result OnGetTransmitFileProgressResponse(NPT_Result res, PLT_DeviceDataReference& device, PLT_ActionReference& action, void* userdata);

private:
    PLT_CtrlPointReference                m_CtrlPoint;
    PLT_FileTransmitControllerDelegate*          m_Delegate;
    NPT_Lock<PLT_DeviceDataReferenceList> m_FileTransmitDevices;
};

typedef NPT_Reference<PLT_FileTransmitController> PLT_FileTransmitControllerReference;

#endif /* _PLT_FILE_TRANMIT_CONTROLLER_H_ */

