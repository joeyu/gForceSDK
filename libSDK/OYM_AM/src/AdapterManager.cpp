#include "stdafx.h"
#include "AdapterManager.h"
#include "RemoteDevice.h"

OYM_AdapterManager::OYM_AdapterManager() :OYM_CallBack(ADAPTER_MANAGER_EVENT)
{
	mInterface = new OYM_NPI_Interface;
	mLog = new OYM_Log(MODUAL_TAG_AM, sizeof(MODUAL_TAG_AM));
	mDS = new OYM_Discovery_Service(mInterface, this);
}

OYM_AdapterManager::~OYM_AdapterManager()
{
	delete mLog;
	delete mDS;
}

OYM_STATUS OYM_AdapterManager::Init()
{
	OYM_STATUS result = OYM_FAIL;

	if (mInterface != NULL)
	{
		result = mInterface->Init();
	}
	else
	{
		return result;
	}
	
	if (result == OYM_SUCCESS)
	{
		//register to receive event form NIF
		mInterface->RegisterCallback(this);
		mInterface->InitDevice();
	}
	
	if (mDS != NULL)
	{
		result = mDS->Init();
	}

	return result;
}

OYM_STATUS OYM_AdapterManager::Deinit()
{
	return OYM_SUCCESS;
}

OYM_STATUS OYM_AdapterManager::StartScan()
{
	if (mDS != NULL)
	{
		//start to scan
		return mDS->StartScan();
	}
	else
	{
		return OYM_FAIL;
	}
}

OYM_STATUS OYM_AdapterManager::StopScan()
{
	if (mDS != NULL)
	{
		//start to scan
		return mDS->StopScan();
	}
	else
	{
		return OYM_FAIL;
	}
}

OYM_STATUS OYM_AdapterManager::Connect(OYM_PUINT8 addr, UINT8 addr_type)
{
	OYM_STATUS result = OYM_FAIL;
	if (mInterface != NULL)
	{
		result = mInterface->Connect(addr, addr_type);
	}
	return result;
}

OYM_STATUS OYM_AdapterManager::OnDeviceFound(BLE_DEVICE new_device)
{
	OYM_STATUS result = OYM_SUCCESS;
	LOGDEBUG("OnNewDeviceFound... \n");
	OYM_RemoteDevice *device = new OYM_RemoteDevice(mInterface, new_device, this);
	mAvailabeDevice.push_front(device);
	
	return result;
}

OYM_STATUS OYM_AdapterManager::OnScanFinished()
{
	OYM_STATUS result = OYM_FAIL;
	OYM_RemoteDevice *device;
	LOGDEBUG("OnScanFinished... \n");
	LOGDEBUG("found device number is %d \n", mAvailabeDevice.size());
	if (mAvailabeDevice.size() != 0)
	{
		device = mAvailabeDevice.front();
		result = device->Connect();
	}
	return result;
}

OYM_STATUS OYM_AdapterManager::OnConnect(OYM_PUINT8 data, OYM_UINT16 length)
{
	OYM_STATUS result = OYM_FAIL;
	list<OYM_RemoteDevice*>::iterator ii;
	OYM_UINT8 status = data[0];
	if (status != OYM_SUCCESS)
	{
		LOGDEBUG("connected with error status = %d \n", status);
		return OYM_FAIL;
	}
	OYM_UINT8 addr_type = data[1];
	LOGDEBUG("LOGDEBUG with length = %d \n" , length);
	for (OYM_UINT16 i = 0; i < length; i++)
	{
		LOGDEBUG("the data of [%d]th bytes is 0x%02x \n", i, data[i]);
	}
	LOGDEBUG("mAvailabeDevice.size() = %d \n", mAvailabeDevice.size());
	
	for (ii = mAvailabeDevice.begin(); ii != mAvailabeDevice.end(); ii++)
	{
		
		LOGDEBUG("mAvailabeDevice.size() = %d \n", mAvailabeDevice.size());
		if (((*ii)->mAddrType == addr_type) && memcmp((*ii)->mAddr, data + 2, BT_ADDRESS_SIZE) == 0)
		{
			(*ii)->ProcessMessage(OYM_DEVICE_EVENT_DEVICE_CONNECTED, data, length);
		}
	}

	return result;
}

OYM_STATUS OYM_AdapterManager::OnEvent(OYM_UINT32 event, OYM_PUINT8 data, OYM_UINT16 length)
{
	OYM_STATUS result = OYM_SUCCESS;
	OYM_DEVICE_EVENT message = OYM_DEVICE_EVENT_INVALID;
	list<OYM_RemoteDevice*>::iterator ii;
	OYM_UINT8 handle_offset;

	//for (OYM_UINT16 i = 0; i < length; i++)
	//{
	//	LOGDEBUG("the data of [%d]th bytes is 0x%02x \n", i, data[i]);
	//}
	switch(event)
	{
		case EVENT_MASK_ATT_READ_BY_GRP_TYPE_MSG:
		{
			message = OYM_DEVICE_EVENT_ATT_READ_BY_GRP_TYPE_MSG;
			handle_offset = 1;
			break;
		}

		case EVENT_MASK_ATT_READ_BY_TYPE_MSG:
		{
			message = OYM_DEVICE_EVENT_ATT_READ_BY_TYPE_MSG;
			handle_offset = 1;
			break;
		}

		case EVENT_MASK_ATT_ERROR_MSG:
		{
			message = OYM_DEVICE_EVENT_ATT_ERROR_MSG;
			handle_offset = 1;
			break;
		}

		case EVENT_MASK_ATT_READ_RESP_MSG:
		{
			message = OYM_DEVICE_EVENT_ATT_READ_RESP_MSG;
			handle_offset = 1;
			break;
		}

		case EVENT_MASK_ATT_READ_BY_INFO_MSG:
		{
			message = OYM_DEVICE_EVENT_ATT_READ_BY_INFO_MSG;
			handle_offset = 1;
			break;
		}

		case EVENT_MASK_ATT_NOTI_MSG:
		{
			LOGDEBUG("--------->notification received! \n");
			for (OYM_UINT16 i = 0; i < length; i++)
			{
				LOGDEBUG("the notificationdata of [%d]th bytes is 0x%02x \n", i, data[i]);
			}
			break;
		}

		case EVENT_MASK_SLAVE_REQUESTED_SECURITY_MSG:
		{
			message = OYM_DEVICE_EVENT_SLAVE_SECURY_REQUEST;
			handle_offset = 1;
			break;
		}

		case EVENT_MASK_AUTH_COMPLETE_MSG:
		{
			message = OYM_DEVICE_EVENT_AUTH_COMPLETE;
			handle_offset = 1;
			break;
		}

		case EVENT_MASK_BOND_COMPLETE_MSG:
		{
			message = OYM_DEVICE_EVENT_BOND_COMPLETE;
			handle_offset = 1;
			break;
		}

		case EVENT_MASK_LINK_PARA_UPDATE_MSG:
		{
			message = OYM_DEVICE_EVENT_LINK_PARA_UPDATE;
			handle_offset = 1;
			break;
		}
		case EVENT_MASK_ATT_WRITE_RESPONSE:
		{
			message = OYM_DEVICE_EVENT_ATT_WRITE_REPONSE;
			handle_offset = 1;
			break;
		}
		default:
			break;
	}

	if (message != OYM_DEVICE_EVENT_INVALID)
	{
		LOGDEBUG("OnEvent with length = %d \n", length);
		for (OYM_UINT16 i = 0; i < length; i++)
		{
			LOGDEBUG("the data of [%d]th bytes is 0x%02x \n", i, data[i]);
		}
		for (ii = mAvailabeDevice.begin(); ii != mAvailabeDevice.end(); ii++)
		{
			OYM_UINT16 handle = data[handle_offset] + (data[handle_offset + 1] << 8);
			if ((*ii)->GetHandle() == handle)
			{
				(*ii)->ProcessMessage(message, data, length);
			}
		}
	} 
	else 
	{
		if (event == EVENT_MASK_GAP_STATUS_MSG)
		{
			LOGDEBUG("EVENT_MASK_GAP_STATUS_MSG with length = %d \n", length);
			for (OYM_UINT16 i = 0; i < length; i++)
			{
				LOGDEBUG("the data of [%d]th bytes is 0x%02x \n", i, data[i]);
			}
		}
	}

	return result;
}

