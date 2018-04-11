/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Neptune.h"
#include "PltFileTransmitControler.h"
#include "PltDidl.h"
#include "PltDeviceData.h"
#include "PltUtilities.h"

NPT_SET_LOCAL_LOGGER("platinum.file.transmit.controller")

/*----------------------------------------------------------------------
|   PLT_FileTransmitController::PLT_FileTransmitController
+---------------------------------------------------------------------*/
PLT_FileTransmitController::PLT_FileTransmitController(PLT_CtrlPointReference&      ctrl_point, 
                                         PLT_FileTransmitControllerDelegate* delegate /* = NULL */) :
    m_CtrlPoint(ctrl_point),
    m_Delegate(delegate)
{
    m_CtrlPoint->AddListener(this);
}

/*----------------------------------------------------------------------
|   PLT_FileTransmitController::~PLT_FileTransmitController
+---------------------------------------------------------------------*/
PLT_FileTransmitController::~PLT_FileTransmitController()
{
    m_CtrlPoint->RemoveListener(this);
}

/*----------------------------------------------------------------------
|   PLT_FileTransmitController::OnDeviceAdded
+---------------------------------------------------------------------*/
NPT_Result
PLT_FileTransmitController::OnDeviceAdded(PLT_DeviceDataReference& device)
{
    // verify the device implements the function we need
    PLT_Service* serviceFTC = NULL;
    PLT_Service* serviceCMR;
	PLT_Service* serviceRC;
    NPT_String   type;
    NPT_LOG_FINE("----------------------Device Found1");
    
    if (!device->GetType().StartsWith("urn:schemas-upnp-org:device:FileTransmitDevice"))
        return NPT_FAILURE;
    NPT_LOG_FINE("----------------------Device Found");

	type = "urn:schemas-upnp-org:service:FileTransmitControl:*";
    if (NPT_FAILED(device->FindServiceByType(type, serviceFTC))) {
        NPT_LOG_FINE_1("Service %s not found", (const char*)type);
        return NPT_FAILURE;
    } else {
        // in case it's a newer upnp implementation, force to 1
        serviceFTC->ForceVersion(1);
    }

    {
        NPT_AutoLock lock(m_FileTransmitDevices);

        PLT_DeviceDataReference data;
        NPT_String uuid = device->GetUUID();
        
        // is it a new device?
        if (NPT_SUCCEEDED(NPT_ContainerFind(m_FileTransmitDevices, 
                                            PLT_DeviceDataFinder(uuid), data))) {
            NPT_LOG_WARNING_1("Device (%s) is already in our list!", (const char*)uuid);
            return NPT_FAILURE;
        }

        NPT_LOG_FINE_1("Device Found: %s", (const char*)*device);

        m_FileTransmitDevices.Add(device);
    }
	
    if (m_Delegate && m_Delegate->OnFTDAdded(device)) {
        // subscribe to services eventing only if delegate wants it
        //comment by now
       // if (serviceFTC) m_CtrlPoint->Subscribe(serviceFTC); 
    }

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_FileTransmitController::OnDeviceRemoved
+---------------------------------------------------------------------*/
NPT_Result 
PLT_FileTransmitController::OnDeviceRemoved(PLT_DeviceDataReference& device)
{   
    if (!device->GetType().StartsWith("urn:schemas-upnp-org:device:FileTransmitDevice"))
        return NPT_FAILURE;

    {
        NPT_AutoLock lock(m_FileTransmitDevices);

        // only release if we have kept it around
        PLT_DeviceDataReference data;
        NPT_String uuid = device->GetUUID();

        // Have we seen that device?
        if (NPT_FAILED(NPT_ContainerFind(m_FileTransmitDevices, PLT_DeviceDataFinder(uuid), data))) {
            NPT_LOG_WARNING_1("Device (%s) not found in our list!", (const char*)uuid);
            return NPT_FAILURE;
        }

        NPT_LOG_FINE_1("Device Removed: %s", (const char*)*device);

        m_FileTransmitDevices.Remove(device);
    }

    if (m_Delegate) {
        m_Delegate->OnFTDRemoved(device);
    }
    
    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_FileTransmitController::FindFTDevice
+---------------------------------------------------------------------*/
NPT_Result
PLT_FileTransmitController::FindFTDevice(const char* uuid, PLT_DeviceDataReference& device)
{
    NPT_AutoLock lock(m_FileTransmitDevices);

    if (NPT_FAILED(NPT_ContainerFind(m_FileTransmitDevices, PLT_DeviceDataFinder(uuid), device))) {
        NPT_LOG_FINE_1("Device (%s) not found in our list of renderers", (const char*)uuid);
        return NPT_FAILURE;
    }

    return NPT_SUCCESS;
}


/*----------------------------------------------------------------------
|   PLT_FileTransmitController::InvokeActionWithInstance
+---------------------------------------------------------------------*/
NPT_Result 
PLT_FileTransmitController::InvokeActionWithInstance(PLT_ActionReference& action,
                                              NPT_UInt32           instance_id,
                                              void*                userdata)
{
    // Set the object id
    NPT_CHECK_SEVERE(action->SetArgumentValue(
        "InstanceID", 
        NPT_String::FromInteger(instance_id)));

    // set the arguments on the action, this will check the argument values
    return m_CtrlPoint->InvokeAction(action, userdata);
}

/*----------------------------------------------------------------------
| add by linhuaji 2014.06.26
|   PLT_FileTransmitController::startTransmitFile
+---------------------------------------------------------------------*/
NPT_Result PLT_FileTransmitController::startTransmitFile(PLT_DeviceDataReference&  device, 
										  NPT_UInt32				instance_id, 
										  const char*				channel,
										  NPT_String 				session_id, 
										  NPT_String 				filename, 
										  NPT_String                          filesize,
										  NPT_String 				from, 
										  void* 					userdata) 
{

	PLT_ActionReference action;
	NPT_CHECK_SEVERE(m_CtrlPoint->CreateAction(
		device, 
		"urn:schemas-upnp-org:service:FileTransmitControl:1", 
		"StartTransmitFile", 
		action));

		// set the channel
	if (NPT_FAILED(action->SetArgumentValue("Channel", channel))) {
		NPT_LOG_INFO("Channel NPT_ERROR_INVALID_PARAMETERS");
		return NPT_ERROR_INVALID_PARAMETERS;
	}

	if (NPT_FAILED(action->SetArgumentValue("SID", session_id))) {
		NPT_LOG_INFO("SID NPT_ERROR_INVALID_PARAMETERS");
		return NPT_ERROR_INVALID_PARAMETERS;
	}

	if (NPT_FAILED(action->SetArgumentValue("FileName", filename))) {
		NPT_LOG_INFO("FileName NPT_ERROR_INVALID_PARAMETERS");
		return NPT_ERROR_INVALID_PARAMETERS;
	}

	if (NPT_FAILED(action->SetArgumentValue("FileSize", filesize))) {
		NPT_LOG_INFO("FileSize NPT_ERROR_INVALID_PARAMETERS");
		return NPT_ERROR_INVALID_PARAMETERS;
	}

	if (NPT_FAILED(action->SetArgumentValue("From", from))) {
		NPT_LOG_INFO("From NPT_ERROR_INVALID_PARAMETERS");
		return NPT_ERROR_INVALID_PARAMETERS;
	}

	NPT_LOG_INFO_3("PLT_FileTransmitController startTransmitFile: sid: %s, filename: %s, from: %s ", (char *)session_id, (char *)filename,(char*)from);

	return InvokeActionWithInstance(action, instance_id, userdata);
}

/*----------------------------------------------------------------------
| add by linhuaji 2014.06.26
|   PLT_FileTransmitController::cancelTransmitFile
+---------------------------------------------------------------------*/
NPT_Result PLT_FileTransmitController::cancelTransmitFile(PLT_DeviceDataReference&  device, 
										  NPT_UInt32				instance_id, 
										  const char*				channel,
										  NPT_String 				session_id, 
										  void* 					userdata) 
{

	PLT_ActionReference action;
	NPT_CHECK_SEVERE(m_CtrlPoint->CreateAction(
		device, 
		"urn:schemas-upnp-org:service:FileTransmitControl:1", 
		"CancelTransmitFile", 
		action));

		// set the channel
	if (NPT_FAILED(action->SetArgumentValue("Channel", channel))) {
		return NPT_ERROR_INVALID_PARAMETERS;
	}

	if (NPT_FAILED(action->SetArgumentValue("SID", session_id))) {
		return NPT_ERROR_INVALID_PARAMETERS;
	}

	return InvokeActionWithInstance(action, instance_id, userdata);
}

/*----------------------------------------------------------------------
| add by linhuaji 2014.06.26
|   PLT_FileTransmitController::finishTransmitFile
+---------------------------------------------------------------------*/
NPT_Result PLT_FileTransmitController::finishTransmitFile(PLT_DeviceDataReference&  device, 
										  NPT_UInt32				instance_id, 
										  const char*				channel,
										  NPT_String 				session_id, 
										  void* 					userdata) 
{

	PLT_ActionReference action;
	NPT_CHECK_SEVERE(m_CtrlPoint->CreateAction(
		device, 
		"urn:schemas-upnp-org:service:FileTransmitControl:1", 
		"FinishTransmitFile", 
		action));

		// set the channel
	if (NPT_FAILED(action->SetArgumentValue("Channel", channel))) {
		return NPT_ERROR_INVALID_PARAMETERS;
	}

	if (NPT_FAILED(action->SetArgumentValue("SID", session_id))) {
		return NPT_ERROR_INVALID_PARAMETERS;
	}
	NPT_LOG_INFO("debug");	
	return InvokeActionWithInstance(action, instance_id, userdata);
}

/*----------------------------------------------------------------------
| add by linhuaji 2014.06.26
|   PLT_FileTransmitController::getTransmitFileProgress
+---------------------------------------------------------------------*/
NPT_Result PLT_FileTransmitController::getTransmitFileProgress(PLT_DeviceDataReference&  device, 
										  NPT_UInt32				instance_id, 
										  const char*				channel,
										  NPT_String 				session_id, 
										  void* 					userdata) 
{

	PLT_ActionReference action;
	NPT_CHECK_SEVERE(m_CtrlPoint->CreateAction(
		device, 
		"urn:schemas-upnp-org:service:FileTransmitControl:1", 
		"GetTransmitFileProgress", 
		action));

		// set the channel
	if (NPT_FAILED(action->SetArgumentValue("Channel", channel))) {
		return NPT_ERROR_INVALID_PARAMETERS;
	}

	if (NPT_FAILED(action->SetArgumentValue("InSID", session_id))) {
		return NPT_ERROR_INVALID_PARAMETERS;
	}

	//out parameter
	//if (NPT_FAILED(action->SetArgumentValue("CurrentTransmitProgress", "0"))) {
		//return NPT_ERROR_INVALID_PARAMETERS;
	//}

	return InvokeActionWithInstance(action, instance_id, userdata);
}

/*----------------------------------------------------------------------
|   PLT_FileTransmitController::OnActionResponse
+---------------------------------------------------------------------*/
NPT_Result
PLT_FileTransmitController::OnActionResponse(NPT_Result           res, 
                                      PLT_ActionReference& action, 
                                      void*                userdata)
{
    if (m_Delegate == NULL) return NPT_SUCCESS;

    PLT_DeviceDataReference device;
    NPT_String uuid = action->GetActionDesc().GetService()->GetDevice()->GetUUID();
           
    /* extract action name */
    NPT_String actionName = action->GetActionDesc().GetName();

    NPT_LOG_INFO_2("##### actionName: %s, uuid: %s #####", (char *)actionName, (char *)uuid);

    /* AVTransport response ? */
    if (actionName.Compare("StartTransmitFile", true) == 0) {
        if (NPT_FAILED(FindFTDevice(uuid, device))) res = NPT_FAILURE;
        return OnStartTransmitFileResponse(res, device, action, userdata);
    }
    else if (actionName.Compare("CancelTransmitFile", true) == 0) {
        if (NPT_FAILED(FindFTDevice(uuid, device))) res = NPT_FAILURE;
        return OnCancelTransmitFileResponse(res, device, action, userdata);
    }
    else if (actionName.Compare("FinishTransmitFile", true) == 0) {
        if (NPT_FAILED(FindFTDevice(uuid, device))) res = NPT_FAILURE;
        return OnFinishTransmitFileResponse(res, device, action, userdata);
    }
    else if (actionName.Compare("GetTransmitFileProgress", true) == 0) {
        if (NPT_FAILED(FindFTDevice(uuid, device))) res = NPT_FAILURE;
        return OnGetTransmitFileProgressResponse(res, device, action, userdata);
    }

    return NPT_SUCCESS;
}


/*----------------------------------------------------------------------
|   PLT_FileTransmitController::OnStartTransmitFileResponse
+---------------------------------------------------------------------*/
NPT_Result
PLT_FileTransmitController::OnStartTransmitFileResponse(NPT_Result               res, 
                                            PLT_DeviceDataReference& device, 
                                            PLT_ActionReference&     action, 
                                            void*                    userdata)
{
    NPT_String      sid;
    NPT_String      filename;

    NPT_LOG_INFO("#####OnStartTransmitFileResponse#####");    

    if (NPT_FAILED(action->GetArgumentValue("SID", sid))) {
        goto bad_action;
    }

    if (NPT_FAILED(action->GetArgumentValue("FileName", filename))) {
        goto bad_action;
    }

    if (NPT_FAILED(res) || action->GetErrorCode() != 0) {
        goto bad_action;
    }

    m_Delegate->OnStartTransmitFileResult(NPT_SUCCESS, device, (char*)sid, (char*)filename, userdata);
    return NPT_SUCCESS;

bad_action:
    m_Delegate->OnStartTransmitFileResult(NPT_FAILURE, device, (char*)sid, NULL, userdata);
    return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   PLT_FileTransmitController::OnCancelTransmitFileResponse
+---------------------------------------------------------------------*/
NPT_Result
PLT_FileTransmitController::OnCancelTransmitFileResponse(NPT_Result               res, 
                                               PLT_DeviceDataReference& device, 
                                               PLT_ActionReference&     action, 
                                               void*                    userdata)
{
    NPT_String       sid;

    NPT_LOG_INFO("#####OnCancelTransmitFileResponse#####");

    if (NPT_FAILED(action->GetArgumentValue("SID", sid))) {
        goto bad_action;
    }

    if (NPT_FAILED(res) || action->GetErrorCode() != 0) {
        goto bad_action;
    }    

    m_Delegate->OnCancelTransmitFileResult(NPT_SUCCESS, device, (char*)sid, userdata);
    return NPT_SUCCESS;

bad_action:
    m_Delegate->OnCancelTransmitFileResult(NPT_FAILURE, device, (char*)sid, userdata);
    return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   PLT_FileTransmitController::OnFinishTransmitFileResponse
+---------------------------------------------------------------------*/
NPT_Result
PLT_FileTransmitController::OnFinishTransmitFileResponse(NPT_Result               res, 
                                                PLT_DeviceDataReference& device, 
                                                PLT_ActionReference&     action, 
                                                void*                    userdata)
{
    NPT_String       sid;

    NPT_LOG_INFO("#####OnFinishTransmitFileResponse#####");

    if (NPT_FAILED(action->GetArgumentValue("SID", sid))) {
        goto bad_action;
    }

    if (NPT_FAILED(res) || action->GetErrorCode() != 0) {
        goto bad_action;
    }

    m_Delegate->OnFinishTransmitFileResult(NPT_SUCCESS, device, (char*)sid, userdata);
    return NPT_SUCCESS;

bad_action:
    m_Delegate->OnFinishTransmitFileResult(NPT_FAILURE, device, (char*)sid, userdata);
    return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   PLT_FileTransmitController::OnGetTransmitFileProgressResponse
+---------------------------------------------------------------------*/
NPT_Result
PLT_FileTransmitController::OnGetTransmitFileProgressResponse(NPT_Result               res, 
                                                    PLT_DeviceDataReference& device, 
                                                    PLT_ActionReference&     action, 
                                                    void*                    userdata)
{
    NPT_String       sid;
    NPT_String       progress;

    NPT_LOG_INFO("#####OnGetTransmitFileProgressResponse#####");

    if (NPT_FAILED(res) || action->GetErrorCode() != 0) {
        goto bad_action;
    }

    if (NPT_FAILED(action->GetArgumentValue("OutSID", sid))) {
	NPT_LOG_INFO("GetArgumentValue OutSID Failed");
        goto bad_action;
    }    
	
    if (NPT_FAILED(action->GetArgumentValue("CurrentTransmitProgress", progress))) {
	NPT_LOG_INFO("GetArgumentValue CurrentTransmitProgress Failed");
       goto bad_action;
    }    
    NPT_LOG_INFO_1("GetArgumentValue CurrentTransmitProgress: %s", (char*)progress);
    m_Delegate->OnGetTransmitFileProgressResult(NPT_SUCCESS, device, (char*)sid, (char*)progress, userdata);
    return NPT_SUCCESS;

bad_action:
    m_Delegate->OnGetTransmitFileProgressResult(NPT_FAILURE, device, NULL, NULL, userdata);
    return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   PLT_FileTransmitController::OnEventNotify
+---------------------------------------------------------------------*/
NPT_Result
PLT_FileTransmitController::OnEventNotify(PLT_Service*                  service, 
                                   NPT_List<PLT_StateVariable*>* vars)
{   
    if (!service->GetDevice()->GetType().StartsWith("urn:schemas-upnp-org:device:FileTransmitControl"))
        return NPT_FAILURE;

    if (!m_Delegate) return NPT_SUCCESS;

    /* make sure device associated to service is still around */
    PLT_DeviceDataReference data;
    NPT_CHECK_WARNING(FindFTDevice(service->GetDevice()->GetUUID(), data));
    
    m_Delegate->OnFTDStateVariablesChanged(service, vars);
    return NPT_SUCCESS;
}

