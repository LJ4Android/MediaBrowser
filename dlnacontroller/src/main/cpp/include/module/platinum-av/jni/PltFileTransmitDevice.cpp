/*****************************************************************
|
|   Platinum - File Transmint Device
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
#include "Neptune.h"
#include "PltFileTransmitDevice.h"
#include "PltService.h"

NPT_SET_LOCAL_LOGGER("platinum.file.transmit")

/*----------------------------------------------------------------------
|   external references
+---------------------------------------------------------------------*/
extern NPT_UInt8 RDR_ConnectionManagerSCPD[];
extern NPT_UInt8 RDR_AVTransportSCPD[];
extern const char* SCPDXML_FileTransmitControl;

/*----------------------------------------------------------------------
|   PLT_FileTransmitDevice::PLT_MediaRenderer
+---------------------------------------------------------------------*/
PLT_FileTransmitDevice::PLT_FileTransmitDevice(const char*  friendly_name, 
                                     bool         show_ip     /* = false */, 
                                     const char*  uuid        /* = NULL */, 
                                     unsigned int port        /* = 0 */,
                                     bool         port_rebind /* = false */) :	
    PLT_DeviceHost("/", 
                   uuid, 
                   "urn:schemas-upnp-org:device:FileTransmitDevice:1", 
                   friendly_name, 
                   show_ip, 
                   port, 
                   port_rebind),
    m_Delegate(NULL)
{
    m_ModelDescription = "Plutinosoft File Transmit Device";
    m_ModelName        = "File Transmint Device";
    m_ModelURL         = "http://www.plutinosoft.com/platinum";
}

/*----------------------------------------------------------------------
|   PLT_FileTransmitDevice::~PLT_MediaRenderer
+---------------------------------------------------------------------*/
PLT_FileTransmitDevice::~PLT_FileTransmitDevice()
{
}

/*----------------------------------------------------------------------
|   PLT_FileTransmitDevice::SetupServices
+---------------------------------------------------------------------*/
NPT_Result
PLT_FileTransmitDevice::SetupServices()
{
    PLT_Service* service;


    {
        /* File Transmit Control */
        service = new PLT_Service(
            this,
            "urn:schemas-upnp-org:service:FileTransmitControl:1", 
            "urn:upnp-org:serviceId:FileTransmitControl",
            "FileTransmitControl",
            "urn:schemas-upnp-org:metadata-1-0/RCS/");
        NPT_CHECK_FATAL(service->SetSCPDXML((const char*) SCPDXML_FileTransmitControl));
        NPT_CHECK_FATAL(AddService(service));
    }

    return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_FileTransmitDevice::OnAction
+---------------------------------------------------------------------*/
NPT_Result
PLT_FileTransmitDevice::OnAction(PLT_ActionReference&          action, 
                            const PLT_HttpRequestContext& context)
{
    NPT_COMPILER_UNUSED(context);

    /* parse the action name */
    NPT_String name = action->GetActionDesc().GetName();

    // since all actions take an instance ID and we only support 1 instance
    // verify that the Instance ID is 0 and return an error here now if not
    NPT_String serviceType = action->GetActionDesc().GetService()->GetServiceType();
	serviceType = action->GetActionDesc().GetService()->GetServiceType();
	if (serviceType.Compare("urn:schemas-upnp-org:service:FileTransmitControl:1", true) == 0) {
		if (NPT_FAILED(action->VerifyArgumentValue("InstanceID", "0"))) {
			action->SetError(702, "Not valid InstanceID");
			return NPT_FAILURE;
		}
	}


	/* Is it a File Transmit Service Action ? */
    if (name.Compare("StartTransmitFile", true) == 0) {
        if (NPT_FAILED(OnTransmitFile(action))){
            action->SetError(10000, "OnTransmitFile Failed");
            return NPT_FAILURE;
        }
        return NPT_SUCCESS;
    }
    if (name.Compare("CancelTransmitFile", true) == 0) {
        if (NPT_FAILED(OnCancel(action))){
            action->SetError(10001, "CancelTransmitFile Failed");
            return NPT_FAILURE;
        }
        return NPT_SUCCESS;
    }
    if (name.Compare("FinishTransmitFile", true) == 0) {
        NPT_LOG_INFO("debug");
        if (NPT_FAILED(OnFinish(action))){
            action->SetError(10002, "FinishTransmitFile Failed");
            return NPT_FAILURE;
        }
        return NPT_SUCCESS;
    }
    if (name.Compare("GetTransmitFileProgress", true) == 0) {
        if (NPT_FAILED(OnGetProgress(action))){
            action->SetError(10003, "GetTransmitFileProgress Failed");
            return NPT_FAILURE;
        }
        return NPT_SUCCESS;
    }

    // other actions rely on state variables
    NPT_CHECK_LABEL_WARNING(action->SetArgumentsOutFromStateVariable(), failure);
    return NPT_SUCCESS;

failure:
    action->SetError(401,"No Such Action.");
    return NPT_FAILURE;
}

/*----------------------------------------------------------------------
|   PLT_FileTransmitDevice::OnTransmitFile
+---------------------------------------------------------------------*/
NPT_Result
PLT_FileTransmitDevice::OnTransmitFile(PLT_ActionReference& action)
{
    if (m_Delegate) {
	NPT_LOG_INFO("OnTransmitFile");
        return m_Delegate->OnTransmitFile(action);
    }
    return NPT_ERROR_NOT_IMPLEMENTED;
}


/*----------------------------------------------------------------------
|   PLT_FileTransmitDevice::OnCancel
+---------------------------------------------------------------------*/
NPT_Result
PLT_FileTransmitDevice::OnCancel(PLT_ActionReference& action)
{
    if (m_Delegate) {
	NPT_LOG_INFO("OnCancel");
        return m_Delegate->OnCancel(action);
    }
    return NPT_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------
|   PLT_FileTransmitDevice::OnFinish
+---------------------------------------------------------------------*/
NPT_Result
PLT_FileTransmitDevice::OnFinish(PLT_ActionReference& action)
{
    if (m_Delegate) {
	NPT_LOG_INFO("OnFinish");
        return m_Delegate->OnFinish(action);
    }
    return NPT_ERROR_NOT_IMPLEMENTED;
}

/*----------------------------------------------------------------------
|   PLT_FileTransmitDevice::OnGetProgress
+---------------------------------------------------------------------*/
NPT_Result
PLT_FileTransmitDevice::OnGetProgress(PLT_ActionReference& action)
{
    if (m_Delegate) {
	NPT_LOG_INFO("OnGetProgress");
        return m_Delegate->OnGetProgress(action);
    }
    return NPT_ERROR_NOT_IMPLEMENTED;
}


const char* SCPDXML_FileTransmitControl =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    "<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">"
    "<specVersion>"
      "<major>1</major>"
      "<minor>0</minor>"
    "</specVersion>"
    "<actionList>"
      "<action>"
         "<name>StartTransmitFile</name>"
         "<argumentList>"
            "<argument>"
               "<name>InstanceID</name>"
               "<direction>in</direction>"
               "<relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>"
            "</argument>"
            "<argument>"
               "<name>Channel</name>"
               "<direction>in</direction>"
               "<relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>"
            "</argument>"
            "<argument>"
               "<name>FileName</name>"
               "<direction>in</direction>"
               "<relatedStateVariable>STR_VALUE_FileName</relatedStateVariable>"
            "</argument>"
            "<argument>"
               "<name>FileSize</name>"
               "<direction>in</direction>"
               "<relatedStateVariable>STR_VALUE_FileSize</relatedStateVariable>"
            "</argument>"
            "<argument>"
               "<name>SID</name>"
               "<direction>in</direction>"
               "<relatedStateVariable>STR_VALUE_Sessionid</relatedStateVariable>"
            "</argument>"
            "<argument>"
               "<name>From</name>"
               "<direction>in</direction>"
               "<relatedStateVariable>STR_VALUE_From</relatedStateVariable>"
            "</argument>"
         "</argumentList>"
      "</action>"
      "<action>"
         "<name>CancelTransmitFile</name>"
         "<argumentList>"
            "<argument>"
               "<name>InstanceID</name>"
               "<direction>in</direction>"
               "<relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>"
            "</argument>"
            "<argument>"
               "<name>Channel</name>"
               "<direction>in</direction>"
               "<relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>"
            "</argument>"
            "<argument>"
               "<name>SID</name>"
               "<direction>in</direction>"
               "<relatedStateVariable>STR_VALUE_Sessionid</relatedStateVariable>"
            "</argument>"            
         "</argumentList>"
      "</action>"
      "<action>"
         "<name>FinishTransmitFile</name>"
         "<argumentList>"
            "<argument>"
               "<name>InstanceID</name>"
               "<direction>in</direction>"
               "<relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>"
            "</argument>"
            "<argument>"
               "<name>Channel</name>"
               "<direction>in</direction>"
               "<relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>"
            "</argument>"
            "<argument>"
               "<name>SID</name>"
               "<direction>in</direction>"
               "<relatedStateVariable>STR_VALUE_Sessionid</relatedStateVariable>"
            "</argument>"
         "</argumentList>"
      "</action>"
      "<action>"
         "<name>GetTransmitFileProgress</name>"
         "<argumentList>"
            "<argument>"
               "<name>InstanceID</name>"
               "<direction>in</direction>"
               "<relatedStateVariable>A_ARG_TYPE_InstanceID</relatedStateVariable>"
            "</argument>"
            "<argument>"
               "<name>Channel</name>"
               "<direction>in</direction>"
               "<relatedStateVariable>A_ARG_TYPE_Channel</relatedStateVariable>"
            "</argument>"
            "<argument>"
               "<name>InSID</name>"
               "<direction>in</direction>"
               "<relatedStateVariable>STR_VALUE_Sessionid</relatedStateVariable>"
            "</argument>"
            "<argument>"
               "<name>OutSID</name>"
               "<direction>out</direction>"
               "<relatedStateVariable>STR_VALUE_Sessionid</relatedStateVariable>"
            "</argument>"
            "<argument>"
               "<name>CurrentTransmitProgress</name>"
               "<direction>out</direction>"
               "<relatedStateVariable>INT_VALUE_TransmitProgress</relatedStateVariable>"
            "</argument>"
         "</argumentList>"
      "</action>"
   "</actionList>"
   "<serviceStateTable>"
   	"<stateVariable sendEvents=\"no\">"
         "<name>A_ARG_TYPE_Channel</name>"
         "<dataType>string</dataType>"
         "<allowedValueList>"
            "<allowedValue>Master</allowedValue>"
            "<allowedValue>LF</allowedValue>"
            "<allowedValue>RF</allowedValue>"
         "</allowedValueList>"
      "</stateVariable>"
      "<stateVariable sendEvents=\"no\">"
         "<name>A_ARG_TYPE_InstanceID</name>"
         "<dataType>ui4</dataType>"
      "</stateVariable>"
      "<stateVariable sendEvents=\"no\">"
         "<name>STR_VALUE_FileName</name>"
         "<dataType>string</dataType>"
      "</stateVariable>"
      "<stateVariable sendEvents=\"no\">"
         "<name>STR_VALUE_FileSize</name>"
         "<dataType>string</dataType>"
      "</stateVariable>"
      "<stateVariable sendEvents=\"no\">"
         "<name>STR_VALUE_Sessionid</name>"
         "<dataType>string</dataType>"
      "</stateVariable>"
      "<stateVariable sendEvents=\"no\">"
         "<name>STR_VALUE_From</name>"
         "<dataType>string</dataType>"
      "</stateVariable>"
	  "<stateVariable sendEvents=\"no\">"
	     "<name>INT_VALUE_TransmitProgress</name>"
	     "<dataType>string</dataType>"
	  "</stateVariable>"
      "</serviceStateTable>"
"</scpd>";

