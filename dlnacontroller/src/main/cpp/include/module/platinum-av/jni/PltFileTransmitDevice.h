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

#ifndef _PLT_FILE_TRANSMIT_DEVICE_H_
#define _PLT_FILE_TRANSMIT_DEVICE_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "PltDeviceHost.h"

/*----------------------------------------------------------------------
|   PLT_FileTransmitDeviceDelegate
+---------------------------------------------------------------------*/
class PLT_FileTransmitDeviceDelegate
{
public:
    virtual ~PLT_FileTransmitDeviceDelegate() {}

    // ConnectionManager
    //virtual NPT_Result OnGetCurrentConnectionInfo(PLT_ActionReference& action) = 0;

    // AVTransport
    virtual NPT_Result OnCancel(PLT_ActionReference& action) = 0;
    virtual NPT_Result OnFinish(PLT_ActionReference& action) = 0;
    virtual NPT_Result OnTransmitFile(PLT_ActionReference& action) = 0;
    virtual NPT_Result OnGetProgress(PLT_ActionReference& action) = 0;
};

/*----------------------------------------------------------------------
|   PLT_MediaRenderer
+---------------------------------------------------------------------*/
class PLT_FileTransmitDevice : public PLT_DeviceHost
{
public:
    PLT_FileTransmitDevice(const char*  friendly_name,
                      bool         show_ip = false,
                      const char*  uuid = NULL,
                      unsigned int port = 0,
                      bool         port_rebind = false);
    // methods
    virtual void SetDelegate(PLT_FileTransmitDeviceDelegate* delegate) { m_Delegate = delegate; }

    // PLT_DeviceHost methods
    virtual NPT_Result SetupServices();
    virtual NPT_Result OnAction(PLT_ActionReference&          action, 
                                const PLT_HttpRequestContext& context);

protected:
    virtual ~PLT_FileTransmitDevice();

    // PLT_FileTransmitDeviceInterface methods
    virtual NPT_Result OnCancel(PLT_ActionReference& action);
    virtual NPT_Result OnFinish(PLT_ActionReference& action);
    virtual NPT_Result OnTransmitFile(PLT_ActionReference& action);
	virtual NPT_Result OnGetProgress(PLT_ActionReference& action);

private:
    PLT_FileTransmitDeviceDelegate* m_Delegate;
};

#endif /* _PLT_FILE_TRANSMIT_DEVICE_H_ */
