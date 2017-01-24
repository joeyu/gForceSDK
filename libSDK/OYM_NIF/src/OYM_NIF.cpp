#include"stdafx.h"
#include "OYM_NIF.h"
#include <oym_types.h>
#define COM_PORT_NUM 18

/* OYM_NIF as a abstract of NPI_Interface, upper level get OYM_NIF instant, which is able send command to  
lower layer, register callback function to receiver event. the event is masked by eventmask.
*/
void OYM_NPI_Interface::Run()
{
	MSG	msg;
	OYM_PUINT8	buf;
	OYM_UINT16	size;
	LISTCALLBACK::iterator ii;
	OYM_UINT32 event = 0x00;

	while (1) {
		event = 0x00;
		if (GetMessage(&msg, 0, 0, 0)) //get msg from message queue
		{
			buf = (PUINT8)msg.wParam;
			size = (UINT16)msg.lParam;
			
			LOGDEBUG("received event 0x%x \n", msg.message);
			switch (msg.message) {
				case HCI_EXT_GAP_DEVICE_INIT_DONE_MSG: //0x0700
					LOGDEBUG("HCI_EXT_GAP_DEVICE_INIT_DONE_MSG \n");
					break;

				case HCI_EXT_GAP_DEVICE_DISCOVERY_MSG: //0x0701
					LOGDEBUG("HCI_EXT_GAP_DEVICE_DISCOVERY_MSG size of callback is %d\n", mCallback.size());
					for (ii = mCallback.begin(); ii != mCallback.end(); ii++)
					{
						OYM_UINT16 result = (((*ii)->GetEventMask()) & EVENT_MASK_GAP_DEVICE_DISCOVERY_MSG);
						LOGDEBUG("(*ii)->GetEventMask() = %d \n", (*ii)->GetEventMask());
						LOGDEBUG("result = %d \n", result);
						if (result != 0)
						{
							(*ii)->OnScanFinished();
						}
					}
					break;

				case HCI_EXT_GAP_DEVICE_INFO_MSG: //0x070D
					LOGDEBUG("HCI_EXT_GAP_DEVICE_INFO_MSG with size = %d, size of callback is %d\n", size, mCallback.size());
					for (ii = mCallback.begin(); ii != mCallback.end(); ii++)
					{
						OYM_UINT16 result = (((*ii)->GetEventMask()) & EVENT_MASK_GAP_DEVICE_INFO_MSG);
						LOGDEBUG("(*ii)->GetEventMask() = %d \n", (*ii)->GetEventMask());

						LOGDEBUG("result = %d \n", result);
						if (result != 0)
						{
							(*ii)->OnScanResult(buf, size);
						}
					}
					break;

				case HCI_EXT_GAP_LINK_ESTABLISHED_MSG: //0x0705
					LOGDEBUG("HCI_EXT_GAP_LINK_ESTABLISHED_MSG \n");
					for (ii = mCallback.begin(); ii != mCallback.end(); ii++)
					{
						OYM_UINT16 result = (((*ii)->GetEventMask()) & EVENT_MASK_GAP_LINK_ESTABLISHED_MSG);

						LOGDEBUG("result = %d \n", result);
						if (result != 0)
						{
							(*ii)->OnConnect(buf, size);
						}
					}
					break;

				case HCI_EXT_GAP_LINK_PARAM_UPDATE_MSG: //0x707
					LOGDEBUG("HCI_EXT_GAP_LINK_PARAM_UPDATE_MSG \n");
					event = EVENT_MASK_LINK_PARA_UPDATE_MSG;
					break;

				case HCI_EXT_GAP_AUTH_COMPLETE_MSG://0x070A
					LOGDEBUG("HCI_EXT_GAP_AUTH_COMPLETE_MSG \n");
					event = EVENT_MASK_AUTH_COMPLETE_MSG;
					break;

				case HCI_EXT_GAP_SLAVE_REQUESTED_SECURITY_MSG: //0x070C
					LOGDEBUG("HCI_EXT_GAP_SLAVE_REQUESTED_SECURITY_MSG \n");
					event = EVENT_MASK_SLAVE_REQUESTED_SECURITY_MSG;
					break;

				case HCI_EXT_GAP_BOND_COMPLETE_MSG:
					event = EVENT_MASK_BOND_COMPLETE_MSG;
					break;

				case HCI_GAP_STATUS_MSG: //0x404 Gap status msg event only sent to high level with error status
					LOGDEBUG("HCI_GAP_STATUS_MSG \n");
					event = EVENT_MASK_GAP_STATUS_MSG;
					break;

				case HCI_EXT_GAP_LINK_TERMINATED_MSG: //0x0706
					LOGDEBUG("HCI_EXT_GAP_LINK_TERMINATED_MSG \n");
					break;

				case ATT_READ_BY_GRP_TYPE_MSG: //0x0608
					LOGDEBUG("ATT_READ_BY_GRP_TYPE_MSG \n");
					event = EVENT_MASK_ATT_READ_BY_GRP_TYPE_MSG;
					break;

				case ATT_ERROR_MSG: //0x0600
					LOGDEBUG("ATT_ERROR_MSG \n");
					event = EVENT_MASK_ATT_ERROR_MSG;
					break;

				case ATT_FIND_INFO_MSG: //0x0602 characteristic descriptor
					LOGDEBUG("ATT_FIND_INFO_MSG \n");
					event = EVENT_MASK_ATT_READ_BY_INFO_MSG;
					break;
					
				case ATT_READ_BY_TYPE_MSG: //0x0604
					LOGDEBUG("ATT_READ_BY_TYPE_MSG \n");
					event = EVENT_MASK_ATT_READ_BY_TYPE_MSG;
					break;
				case ATT_READ_MSG: //0x605
					LOGDEBUG("ATT_READ_MSG \n");
					event = EVENT_MASK_ATT_READ_RESP_MSG;
					break;

				case ATT_HANDLE_VALUE_NOTI_MSG:
					LOGDEBUG("ATT_HANDLE_VALUE_NOTI_MSG \n");
					event = EVENT_MASK_ATT_NOTI_MSG;
					break;

				case ATT_WRITE_MSG:   // attWrite_response
					LOGDEBUG("ATT_WRITE_RESPONSE_MSG \n");
					event = EVENT_MASK_ATT_WRITE_RESPONSE;
					break;

				default:
					printf("NPI_EVT++++++++++++++++£ºErr msg type=%4X !\n", msg.message);
					break;
			}

			for (ii = mCallback.begin(); ii != mCallback.end(); ii++)
			{
				OYM_UINT16 result = (((*ii)->GetEventMask()) & event);

				LOGDEBUG("result = %d \n", result);
				if (result != 0)
				{
					(*ii)->OnEvent(event, buf, size);
				}
			}

		}
	}
}

OYM_NPI_Interface::OYM_NPI_Interface()
{
	mLog = new OYM_Log(MODUAL_TAG, sizeof(MODUAL_TAG));
	mCallback.clear();
}

OYM_STATUS OYM_NPI_Interface::Init()
{
	mCommand = new NPI_CMD(COM_PORT_NUM);
	if (mCommand == NULL)
	{
		return OYM_FAIL;
	}

	/*create the event thread to receive message from controller.*/
	mEvtThread = new CThread(this);
	mEvtThread->Start();
	mEvtThread->Join(100);
	mEvtThreadID = mEvtThread->GetThreadID();

	if (false == (mCommand->Connect(mEvtThreadID)))
	{
		delete mCommand;
		return OYM_FAIL;
	}

	return OYM_SUCCESS;
}

OYM_STATUS OYM_NPI_Interface::Deinit()
{
	//no need to close com port??
	delete mCommand;

	return OYM_SUCCESS;
}

OYM_STATUS OYM_NPI_Interface::InitDevice()
{
	OYM_STATUS result;

	gapRole_t gap_role;
	OYM_UINT8 irk[16] = { 0 };
	OYM_UINT8 csrk[16] = { 0 };
	OYM_UINT8 ltk[16] = { 0 };
	OYM_UINT8 rand[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	OYM_UINT8 sign_count = 1;
	gap_role.data = 0x08;
	if (OYM_TRUE == mCommand->GAP_DeviceInit((gapRole_t)gap_role, 5, irk, csrk, (OYM_PUINT8)&sign_count))
	{
		result = OYM_SUCCESS;
	}
	else
	{
		result = OYM_FAIL;
	}
	return result;
}

OYM_STATUS OYM_NPI_Interface::StartLEScan()
{
	OYM_STATUS result;
	OYM_BOOL status = OYM_FAIL;

	if (mCommand != NULL)
	{
		status = mCommand->GAP_DeviceDiscoveryRequest(DEVDISC_MODE_ALL, NPI_ENABLE, NPI_DISABLE);
	}
		

	return result = (status == OYM_TRUE) ? OYM_SUCCESS : OYM_FAIL;
}

OYM_STATUS OYM_NPI_Interface::StopLEScan()
{
	OYM_STATUS result;
	OYM_BOOL status = OYM_FAIL;

	if (mCommand != NULL)
	{
		status = mCommand->GAP_DeviceDiscoveryCancel();
	}
		
	return result = (status == OYM_TRUE) ? OYM_SUCCESS : OYM_FAIL;
}

OYM_STATUS OYM_NPI_Interface::Connect(OYM_PUINT8 addr, UINT8 addr_type)
{
	OYM_STATUS result;
	OYM_BOOL status = OYM_FAIL;

	if (mCommand != NULL)
	{
		mCommand->GAP_EstablishLinkRequest(NPI_DISABLE, NPI_DISABLE, (eGapAddrType)addr_type, addr);
	}

	return result = status;
}

OYM_STATUS OYM_NPI_Interface::RegisterCallback(OYM_CallBack *callback)
{
	LOGDEBUG("before register, num of callback is %d \n", mCallback.size());
	mCallback.push_front(callback);
	LOGDEBUG("after register, num of callback is %d \n", mCallback.size());

	return OYM_SUCCESS;
}

OYM_STATUS OYM_NPI_Interface::Authenticate(OYM_UINT16 handle)
{
	OYM_BOOL status = OYM_FAIL;

	sGapAuth auth;
	memset(&auth, 0, sizeof(auth));
	auth.sec_authReq.oper = 0x00;
	auth.sec_ioCaps = (eGapIOCaps)0x04;
	auth.sec_maxEncKeySize = 0x10;
	auth.sec_keyDist.oper = 0x77;
	auth.sec_oobFlag = NPI_FALSE;

	auth.pair_ioCaps = (eGapIOCaps)0x03;
	auth.pair_en = NPI_DISABLE;
	auth.pair_keyDist.oper = 0x77;
	auth.pair_maxEncKeySize = 0x10;
	auth.pair_authReq.oper = 0x01;
	auth.pair_oobFlag = NPI_FALSE;

	LOGDEBUG("#######Authenticate start....\n");
	status = mCommand->GAP_Authenticate(handle, &auth);
	LOGDEBUG("#######Authenticate end....\n");
	return (status == OYM_TRUE) ? OYM_SUCCESS : OYM_FAIL;
}

OYM_STATUS OYM_NPI_Interface::Bond(OYM_UINT16 handle, OYM_PUINT8 ltk, OYM_UINT16 div, OYM_PUINT8 rand, OYM_UINT8 ltk_size)
{
	OYM_BOOL status = OYM_FAIL;
	status = mCommand->GAP_Bond(handle, NPI_DISABLE, ltk, div, rand, ltk_size);

	return (status == OYM_TRUE) ? OYM_SUCCESS : OYM_FAIL;
}

OYM_STATUS OYM_NPI_Interface::DiscoveryAllPrimaryService(OYM_UINT16 handle)
{
	OYM_BOOL status = OYM_FAIL;
	status = mCommand->GATT_DiscAllPrimaryServices(handle);

	return (status == OYM_TRUE) ? OYM_SUCCESS : OYM_FAIL;
}

OYM_STATUS OYM_NPI_Interface::DiscoveryIncludedPrimaryService(OYM_UINT16 conn_handle, OYM_UINT16 start_handle, OYM_UINT16 end_handle)
{
	OYM_BOOL status = OYM_FAIL;
	status = mCommand->GATT_FindIncludedServices(conn_handle, start_handle, end_handle);
	return (status == OYM_TRUE) ? OYM_SUCCESS : OYM_FAIL;
}

OYM_STATUS OYM_NPI_Interface::DiscoveryCharacteristic(OYM_UINT16 conn_handle, OYM_UINT16 start_handle, OYM_UINT16 end_handle)
{
	OYM_BOOL status = OYM_FAIL;
	status = mCommand->GATT_DiscAllChar(conn_handle, start_handle, end_handle);
	return (status == OYM_TRUE) ? OYM_SUCCESS : OYM_FAIL;
}

OYM_STATUS OYM_NPI_Interface::ReadCharacteristicValue(OYM_UINT16 conn_handle, OYM_UINT16 att_handle)
{
	OYM_BOOL status = OYM_FAIL;
	status = mCommand->GATT_ReadCharVal(conn_handle, att_handle);
	return (status == OYM_TRUE) ? OYM_SUCCESS : OYM_FAIL;
}

OYM_STATUS OYM_NPI_Interface::FindCharacteristicDescriptor(OYM_UINT16 conn_handle, OYM_UINT16 start_handle, OYM_UINT16 end_handle)
{
	OYM_BOOL status = OYM_FAIL;
	status = mCommand->GATT_DiscAllCharDesc(conn_handle, start_handle, end_handle);
	return (status == OYM_TRUE) ? OYM_SUCCESS : OYM_FAIL;
}

OYM_STATUS OYM_NPI_Interface::WriteCharacVlaue(OYM_UINT16 conn_handle, OYM_UINT16 att_handle, OYM_PUINT8 data, OYM_UINT8 len)
{
	OYM_BOOL status = OYM_FAIL;
	status = mCommand->GATT_WriteCharValue(conn_handle, att_handle, data, len);
	return (status == OYM_TRUE) ? OYM_SUCCESS : OYM_FAIL;
}

OYM_STATUS OYM_NPI_Interface::ExchangeMTUsize(OYM_UINT16 conn_handle, OYM_UINT16 mtu_size)
{
	OYM_BOOL  status = OYM_FAIL;
	status = mCommand->GATT_ExchangeMTU(conn_handle, mtu_size);
	return (status == OYM_TRUE) ? OYM_SUCCESS : OYM_FAIL;
}