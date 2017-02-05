#include "stdafx.h"
#include "RemoteDevice.h"

/*RemoteDevice is a abstract of Remote Device.*/

/*used to receive message from AdapterManager.*/
OYM_VOID OYM_RemoteDevice::Run()
{
	MESSAGE* msg;
	OYM_PUINT8	buf;
	OYM_UINT16	size;

	while (1) {
		if (mMessage.size() != 0) //get msg from message queue
		{
			msg = mMessage.front();
			buf = (PUINT8)msg->data;
			size = (UINT16)msg->length;
			LOGDEBUG("received message 0x%x \n", msg->event);
			
			switch (mState) 
			{
				case OYM_DEVICE_STATE_IDLE:
					LOGDEBUG("OYM_DEVICE_STATE_IDLE state received message \n");
					IdleStateProcessMessage((OYM_DEVICE_EVENT)msg->event, buf, size);
					break;
				case OYM_DEVICE_STATE_W4CONN:
					LOGDEBUG("OYM_DEVICE_STATE_W4CONN state received message \n");

					W4ConnStateProcessMessage((OYM_DEVICE_EVENT)msg->event, buf, size);
					break;
				case OYM_DEVICE_STATE_W4SECU:
					LOGDEBUG("OYM_DEVICE_STATE_W4SECU state received message \n");
					for (OYM_UINT16 i = 0; i < size; i++)
					{
						LOGDEBUG("the data of [%d]th bytes is 0x%02x \n", i, buf[i]);
					}
					W4SecuStateProcessMessage((OYM_DEVICE_EVENT)msg->event, buf, size);
					break;
				case OYM_DEVICE_STATE_GATT_PRI_SVC:
					LOGDEBUG("OYM_DEVICE_STATE_GATT_PRI_SVC state received message \n");
					W4GattPriSvcStateProcessMessage((OYM_DEVICE_EVENT)msg->event, buf, size);
					break;
				case OYM_DEVICE_STATE_GATT_INC_SVC:
					LOGDEBUG("OYM_DEVICE_STATE_GATT_INC_SVC state received message \n");
					W4GattIncSvcStateProcessMessage((OYM_DEVICE_EVENT)msg->event, buf, size);
					break;
				case OYM_DEVICE_STATE_GATT_DIS_CHARC:
					LOGDEBUG("OYM_DEVICE_STATE_GATT_DIS_CHARC state received message \n");
					W4GattDisCharcStateProcessMessage((OYM_DEVICE_EVENT)msg->event, buf, size);
					break;
				case OYM_DEVICE_STATE_GATT_READ_CHARC_VALUE:
					LOGDEBUG("OYM_DEVICE_STATE_GATT_READ_CHARC_VALUE state received message \n");
					W4GattReadCharcValueStateProcessMessage((OYM_DEVICE_EVENT)msg->event, buf, size);
					break;
			}
			mMessage.pop_front();
		}
		else 
		{
			Sleep(50);
		}
	}
}

OYM_RemoteDevice::~OYM_RemoteDevice()
{
	delete mLog;
}

OYM_RemoteDevice::OYM_RemoteDevice(OYM_NPI_Interface* minterface, BLE_DEVICE address, OYM_CallBack* callback)
{
	mInterface = minterface;
	mCallback = callback;
	mAddrType = address.addr_type;
	memcpy(mAddr, address.addr, BT_ADDRESS_SIZE);
	memset(mDevName, 0, BLE_DEVICE_NAME_LENGTH);
	memcpy(mDevName, address.dev_name, BT_ADDRESS_SIZE);
	mLog = new OYM_Log(MODUAL_TAG_RD, sizeof(MODUAL_TAG_RD));


	/*create the thread to process message.*/
	mThread = new CThread(this);
	mThread->Start();
	mThread->Join(100);
	mThreadID = mThread->GetThreadID(); 

	mState = OYM_DEVICE_STATE_IDLE;
}

OYM_STATUS OYM_RemoteDevice::Connect()
{
	OYM_STATUS result = OYM_FAIL;
	if (mInterface != NULL)
	{
		LOGDEBUG("Connect... \n");
		result = mInterface->Connect(mAddr, mAddrType);
		//if (result == OYM_SUCCESS)
		{
			mState = OYM_DEVICE_STATE_W4CONN;
		}
	}
		
	return result;
}

OYM_STATUS OYM_RemoteDevice::IdleStateProcessMessage(OYM_DEVICE_EVENT event, OYM_PUINT8 data, OYM_UINT16 length)
{
	OYM_STATUS result = OYM_FAIL;
	LOGDEBUG("IdleStateProcessMessage process Message = %d \n", event);
	switch (event)
	{
		case OYM_DEVICE_EVENT_DEVICE_CONNECTED:

		break;

		case OYM_DEVICE_STATE_W4CONN:

		break;

		default:
			break;
	}
	return result;
}

OYM_STATUS OYM_RemoteDevice::W4ConnStateProcessMessage(OYM_DEVICE_EVENT event, OYM_PUINT8 data, OYM_UINT16 length)
{
	OYM_STATUS result = OYM_FAIL;
	OYM_STATUS status;
	LOGDEBUG("W4ConnStateProcessMessage process Message = %d \n", event);
	switch (event)
	{
		case OYM_DEVICE_EVENT_DEVICE_CONNECTED:
			LOGDEBUG("OYM_DEVICE_EVENT_DEVICE_CONNECTED with length = %d\n", length);
			for (OYM_UINT16 i = 0; i < length; i++)
			{
				LOGDEBUG("the data of [%d]th bytes is 0x%02x \n", i, data[i]);
			}
			mHandle = data[EVENT_DEVICE_CCONNECTED_CONN_HANDLE_OFFSET] + (data[EVENT_DEVICE_CCONNECTED_CONN_HANDLE_OFFSET+1] << 8);
			LOGDEBUG("mHandle = 0x%02x \n", mHandle);
			mConnRole = (OYM_DEVICE_ROLE)data[EVENT_DEVICE_CCONNECTED_ROLE_OFFSET];
			mConnInternal = data[EVENT_DEVICE_CCONNECTED_CONN_INTERVAL_OFFSET] + (data[EVENT_DEVICE_CCONNECTED_CONN_INTERVAL_OFFSET+1] << 8);
			mSlaveLatency = data[EVENT_DEVICE_CCONNECTED_SLAVE_LATENCY_OFFSET] + (data[EVENT_DEVICE_CCONNECTED_SLAVE_LATENCY_OFFSET+1] << 8);
			mConnTimeout = data[EVENT_DEVICE_CCONNECTED_CONN_TIMEOUT_OFFSET] + (data[EVENT_DEVICE_CCONNECTED_CONN_TIMEOUT_OFFSET+1] << 8);
			mClockAccuracy = data[EVENT_DEVICE_CCONNECTED_CLOCK_ACCURACY_OFFSET];
			
			LOGDEBUG("mConnRole = 0x%02x \n", mConnRole);
			LOGDEBUG("mConnInternal = 0x%02x \n", mConnInternal);
			LOGDEBUG("mSlaveLatency = 0x%02x \n", mSlaveLatency);
			LOGDEBUG("mConnTimeout = 0x%02x \n", mConnTimeout);
			LOGDEBUG("mClockAccuracy = 0x%02x \n", mClockAccuracy);

			/*start to authentication automatic if we do not have the link key of remote device*/
			/*check whether we have the link key, if not, start to authenticate, to do...*/
			status = mInterface->Authenticate(mHandle);
			if (status == OYM_SUCCESS)
			{
				mInterface->ExchangeMTUsize(mHandle, 150);
				mState = OYM_DEVICE_STATE_W4SECU;
			}
			break;

		case OYM_DEVICE_EVENT_SLAVE_SECURY_REQUEST:
			LOGDEBUG("OYM_DEVICE_EVENT_SLAVE_SECURY_REQUEST with length = %d\n", length);

			status = data[EVENT_SLAVE_SECURY_REQUEST_STATUS_OFFSET];
			if (status != OYM_SUCCESS)
			{
				LOGDEBUG("OYM_DEVICE_EVENT_SLAVE_SECURY_REQUEST with error status = %d\n", status);
				return result;
			}

			if (0 == memcmp(data + EVENT_SLAVE_SECURY_REQUEST_ADDRESS_OFFSET, mAddr, BT_ADDRESS_SIZE))
			{
				OYM_UINT8 auth_type = data[EVENT_SLAVE_SECURY_REQUEST_AUTH_TYPE_OFFSET];
				if (auth_type == 1)
				{
					LOGDEBUG("---------->mInterface->Authenticate start....\n");
					mInterface->Authenticate(mHandle);
					mState = OYM_DEVICE_STATE_W4SECU;
					LOGDEBUG("---------->mInterface->Authenticate end....\n");
				}
				else
				{
					LOGDEBUG("---------->mInterface->Authenticate not called....\n");
				}
			}
			else
			{
				LOGDEBUG("address is not patch");
			}
			break;

		case OYM_DEVICE_EVENT_AUTH_COMPLETE:
		case OYM_DEVICE_EVENT_BOND_COMPLETE:
			LOGDEBUG("W4ConnStateProcessMessage with error message!!!");
			break;
		
		default:
			break;
	}
	return result;
}

OYM_STATUS OYM_RemoteDevice::W4SecuStateProcessMessage(OYM_DEVICE_EVENT event, OYM_PUINT8 data, OYM_UINT16 length)
{
	OYM_STATUS result = OYM_FAIL;
	OYM_STATUS status;
	LOGDEBUG("W4SecuStateProcessMessage process Message = %d \n", event);
	switch (event)
	{
		case OYM_DEVICE_EVENT_SLAVE_SECURY_REQUEST:
			LOGDEBUG("OYM_DEVICE_EVENT_SLAVE_SECURY_REQUEST with length = %d\n", length);
			LOGDEBUG("OYM_DEVICE_EVENT_SLAVE_SECURY_REQUEST skip it--------> \n");
#if 0
			LOGDEBUG("OYM_DEVICE_EVENT_SLAVE_SECURY_REQUEST with length = %d\n", length);

			status = data[EVENT_SLAVE_SECURY_REQUEST_STATUS_OFFSET];
			if (status != OYM_SUCCESS)
			{
				LOGDEBUG("OYM_DEVICE_EVENT_SLAVE_SECURY_REQUEST with error status = %d\n", status);
				return result;
			}

			if (0 == memcmp(data + EVENT_SLAVE_SECURY_REQUEST_ADDRESS_OFFSET, mAddr, BT_ADDRESS_SIZE))
			{
				OYM_UINT8 auth_type = data[EVENT_SLAVE_SECURY_REQUEST_AUTH_TYPE_OFFSET];
				if (auth_type == 1)
				{
					LOGDEBUG("---------->mInterface->Authenticate start....\n");
					mInterface->Authenticate(mHandle);
					LOGDEBUG("---------->mInterface->Authenticate end....\n");
				}
				else
				{
					LOGDEBUG("---------->mInterface->Authenticate not called....\n");
				}
			}
			else
			{
				LOGDEBUG("address is not patch");
			}
#endif
			break;

		case OYM_DEVICE_EVENT_AUTH_COMPLETE:
		{
			LOGDEBUG("OYM_DEVICE_EVENT_AUTH_COMPLETE with length = %d\n", length);
			for (OYM_UINT16 i = 0; i < length; i++)
			{
				LOGDEBUG("the data of [%d]th bytes is 0x%02x \n", i, data[i]);
			}

			if (data[EVENT_AUTH_COMPLETE_STATUS_OFFSET] != OYM_SUCCESS)
			{
				LOGDEBUG("OYM_DEVICE_EVENT_AUTH_COMPLETE with error!! \n");
				return OYM_FAIL;
			}
			mKeySize = data[EVENT_AUTH_COMPLETE_KEYSIZE_OFFSET];
			LOGDEBUG("OYM_DEVICE_EVENT_AUTH_COMPLETE with keysize = %d!! \n ", mKeySize);
			/*need to check the length of keysize.*/
			memcpy(mLTK, data + EVENT_AUTH_COMPLETE_LTK_OFFSET, mKeySize);
			for (OYM_UINT16 i = 0; i < mKeySize; i++)
			{
				LOGDEBUG("the mLTK of [%d]th bytes is 0x%02x \n", i, mLTK[i]);
			}
			mDIV = data[EVENT_AUTH_COMPLETE_DIV_OFFSET] + (data[EVENT_AUTH_COMPLETE_DIV_OFFSET + 1] << 8);
			LOGDEBUG("OYM_DEVICE_EVENT_AUTH_COMPLETE with mDIV = 0x%02x!! \n ", mDIV);
			memcpy(mRAND, data + EVENT_AUTH_COMPLETE_RAND_OFFSET, 8);
			for (OYM_UINT16 i = 0; i < mKeySize; i++)
			{
				LOGDEBUG("the mRAND of [%d]th bytes is 0x%02x \n", i, mRAND[i]);
			}

			LOGDEBUG("---------->mInterface->Bond start....\n");
			mInterface->Bond(mHandle, mLTK, mDIV, mRAND, mKeySize);
			LOGDEBUG("---------->mInterface->Bond end....\n");
			break;
		}

		case OYM_DEVICE_EVENT_BOND_COMPLETE:
			LOGDEBUG("OYM_DEVICE_EVENT_AUTH_COMPLETE with length = %d\n", length);

			status = data[EVENT_BOND_COMPLETE_STATUS_OFFSET];
			if (status == OYM_SUCCESS)
			{
				LOGDEBUG("OYM_DEVICE_EVENT_BOND_COMPLETE successful!!!\n");
				//start to discovery all primary service.
				if (OYM_SUCCESS == mInterface->DiscoveryAllPrimaryService(mHandle))
				{
					LOGDEBUG("Gatt start to discovery all primary service! \n");
					mState = OYM_DEVICE_STATE_GATT_PRI_SVC;
				}
				else
				{
					//fail to discovery all primary service, disconnect connection
				}
			}
			else if (status == 0x06) //key missing
			{
				LOGDEBUG("OYM_DEVICE_EVENT_BOND_COMPLETE fail with status = %d!!!\n", status);
				//bond again.
				mInterface->Authenticate(mHandle);
			}
			
			break;

		case OYM_DEVICE_EVENT_LINK_PARA_UPDATE:
			LOGDEBUG("OYM_DEVICE_EVENT_LINK_PARA_UPDATE with length = %d\n", length);
			status = data[EVENT_BOND_COMPLETE_STATUS_OFFSET];
			if (status == OYM_SUCCESS)
			{
				mConnInternal = data[EVENT_LINK_PARA_UPDATE_INTERVEL_OFFSET] + (data[EVENT_LINK_PARA_UPDATE_INTERVEL_OFFSET] << 8);
				mSlaveLatency = data[EVENT_LINK_PARA_UPDATE_LATENCY_OFFSET] + (data[EVENT_LINK_PARA_UPDATE_LATENCY_OFFSET+1] << 8);
				mConnTimeout = data[EVENT_LINK_PARA_UPDATE_TIMEOUT_OFFSET] + (data[EVENT_LINK_PARA_UPDATE_TIMEOUT_OFFSET+1] << 8);
				LOGDEBUG("mConnInternal = 0x%02x \n", mConnInternal);
				LOGDEBUG("mSlaveLatency = 0x%02x \n", mSlaveLatency);
				LOGDEBUG("mConnTimeout = 0x%02x \n", mConnTimeout);
			}
			break;

		default:
			break;
	}
	return result;
}

OYM_STATUS OYM_RemoteDevice::W4GattPriSvcStateProcessMessage(OYM_DEVICE_EVENT event, OYM_PUINT8 data, OYM_UINT16 length)
{
	OYM_STATUS result = OYM_FAIL;
	OYM_UINT8 status;
	LOGDEBUG("W4GattPriSvcStateProcessMessage process Message = %d \n", event);
	switch (event)
	{
	case OYM_DEVICE_EVENT_SLAVE_SECURY_REQUEST:
	case OYM_DEVICE_EVENT_AUTH_COMPLETE:
	case OYM_DEVICE_EVENT_BOND_COMPLETE:
		LOGDEBUG("W4GattPriSvcStateProcessMessage process error Message = %d \n", event);
		break;

	case OYM_DEVICE_EVENT_LINK_PARA_UPDATE:
		LOGDEBUG("OYM_DEVICE_EVENT_LINK_PARA_UPDATE with length = %d\n", length);
		status = data[EVENT_BOND_COMPLETE_STATUS_OFFSET];
		if (status == OYM_SUCCESS)
		{
			mConnInternal = data[EVENT_LINK_PARA_UPDATE_INTERVEL_OFFSET] + (data[EVENT_LINK_PARA_UPDATE_INTERVEL_OFFSET] << 8);
			mSlaveLatency = data[EVENT_LINK_PARA_UPDATE_LATENCY_OFFSET] + (data[EVENT_LINK_PARA_UPDATE_LATENCY_OFFSET + 1] << 8);
			mConnTimeout = data[EVENT_LINK_PARA_UPDATE_TIMEOUT_OFFSET] + (data[EVENT_LINK_PARA_UPDATE_TIMEOUT_OFFSET + 1] << 8);
			LOGDEBUG("mConnInternal = 0x%02x \n", mConnInternal);
			LOGDEBUG("mSlaveLatency = 0x%02x \n", mSlaveLatency);
			LOGDEBUG("mConnTimeout = 0x%02x \n", mConnTimeout);
		}
		break;

	case OYM_DEVICE_EVENT_ATT_READ_BY_GRP_TYPE_MSG:
	{
		LOGDEBUG("OYM_DEVICE_EVENT_ATT_READ_BY_GRP_TYPE_MSG with length = %d\n", length);
		OYM_UINT8 status = data[0];
		OYM_UINT8 length = data[3];
		OYM_UINT8 pair_len = data[4];
		LOGDEBUG("OYM_DEVICE_EVENT_ATT_READ_BY_GRP_TYPE_MSG with pair_len = %d\n", pair_len);
		OYM_UINT8 num_svc = 1;
		if (pair_len != 0)
		{
			num_svc = (length - 1) / pair_len;
		}
		OYM_PUINT8 p = data + 5;

		LOGDEBUG("OYM_DEVICE_EVENT_ATT_READ_BY_GRP_TYPE_MSG with status = %d\n", status);
		if (0x1A == status)
		{
			LOGDEBUG("OYM_DEVICE_EVENT_ATT_READ_BY_GRP_TYPE_MSG completeed with service number is %d! \n", mService.mNumOfPrivateService);
			//change state to discovery included service and characteristic.
			OYM_PRISERVICE* service = mService.FindPriSvcbyIndex(mService.mCurrentPriService);
			if (NULL != service)
			{
				LOGDEBUG("start to find the included service of %dth service. \n", mService.mCurrentPriService);
				mInterface->DiscoveryIncludedPrimaryService(mHandle, service->mStartHandle, service->mEndHandle);
				mState = OYM_DEVICE_STATE_GATT_INC_SVC;
			}
			else
			{
				LOGDEBUG("mService.mCurrentPriService = %d is not available. \n", mService.mCurrentPriService);
			}
		}
		else if (0 == status)
		{
			if ((length - 1) % pair_len == 0)
			{
				for (OYM_UINT8 i = 0; i < num_svc; i++)
				{
					mService.AddPriSvcIntoService(p, pair_len);
					p += pair_len;
				}
			}
			else
			{
				LOGDEBUG("OYM_DEVICE_EVENT_ATT_READ_BY_GRP_TYPE_MSG with wrong length = %d, pair_len = %d\n", length, pair_len);
			}
		}
		else
		{
			LOGERROR("OYM_DEVICE_EVENT_ATT_READ_BY_GRP_TYPE_MSG with error status! %d\n");
		}
	}
	break;

	default:
		break;
	}
	return result;
}

OYM_STATUS OYM_RemoteDevice::W4GattIncSvcStateProcessMessage(OYM_DEVICE_EVENT event, OYM_PUINT8 data, OYM_UINT16 length)
{
	OYM_STATUS result = OYM_FAIL;
	OYM_UINT8 status = data[OYM_ATT_READ_BY_TYPE_RES_STATUS_OFFSET];
	OYM_UINT8 pdu_len;
	OYM_UINT8 pair_len;
	OYM_UINT8 num_svc;
	LOGDEBUG("W4GattIncSvcStateProcessMessage process Message = %d \n", event);
	switch (event)
	{
		case OYM_DEVICE_EVENT_SLAVE_SECURY_REQUEST:
		case OYM_DEVICE_EVENT_AUTH_COMPLETE:
		case OYM_DEVICE_EVENT_BOND_COMPLETE:
		case OYM_DEVICE_EVENT_ATT_READ_BY_GRP_TYPE_MSG:
			LOGDEBUG("W4GattIncSvcStateProcessMessage process error Message = %d \n", event);
			break;

		case OYM_DEVICE_EVENT_LINK_PARA_UPDATE:
			LOGDEBUG("OYM_DEVICE_EVENT_LINK_PARA_UPDATE with length = %d\n", length);
			status = data[EVENT_BOND_COMPLETE_STATUS_OFFSET];
			if (status == OYM_SUCCESS)
			{
				mConnInternal = data[EVENT_LINK_PARA_UPDATE_INTERVEL_OFFSET] + (data[EVENT_LINK_PARA_UPDATE_INTERVEL_OFFSET] << 8);
				mSlaveLatency = data[EVENT_LINK_PARA_UPDATE_LATENCY_OFFSET] + (data[EVENT_LINK_PARA_UPDATE_LATENCY_OFFSET + 1] << 8);
				mConnTimeout = data[EVENT_LINK_PARA_UPDATE_TIMEOUT_OFFSET] + (data[EVENT_LINK_PARA_UPDATE_TIMEOUT_OFFSET + 1] << 8);
				LOGDEBUG("mConnInternal = 0x%02x \n", mConnInternal);
				LOGDEBUG("mSlaveLatency = 0x%02x \n", mSlaveLatency);
				LOGDEBUG("mConnTimeout = 0x%02x \n", mConnTimeout);
			}
			break;

		case OYM_DEVICE_EVENT_ATT_ERROR_MSG:
		{
			//get error code
			OYM_UINT8 error_code = data[OYM_ATT_ERROR_MSG_ERROR_CODE_OFFSET];
			if (OYM_ERROR_CODE_ATTR_NOT_FOUND == error_code)
			{
				//no included service found, start to discovery charactistic.
				this->StartDiscoveryCharacteristic();
			}
			break;
		}	
		case OYM_DEVICE_EVENT_ATT_READ_BY_TYPE_MSG:
		{
			LOGDEBUG("OYM_DEVICE_EVENT_ATT_READ_BY_TYPE_MSG with length = %d\n, status = 0x%02x", length, status);
			if (OYM_ATT_PROCEDURE_SUCCESS == status)
			{
				pdu_len = data[OYM_ATT_READ_BY_TYPE_RES_PDU_LEN_OFFSET]; //get pdu length from raw data
				pair_len = data[OYM_ATT_READ_BY_TYPE_RES_PAIR_LEN_OFFSET]; //get characteristic struct length from raw data
				OYM_PUINT8 p = data + 5;
	
				if (pair_len != 0)
				{
					num_svc = (pdu_len - 1) / pair_len;
				}
				else
				{
					LOGERROR("pair_len error!!!");
				}

				OYM_PRISERVICE* service = mService.FindPriSvcbyIndex(mService.mCurrentPriService);
				if ((pdu_len - 1) % pair_len == 0)
				{
					for (OYM_UINT8 i = 0; i < num_svc; i++)
					{
						service->AddIncSvcIntoPriService(p, pair_len);
						p += pair_len;
					}
				}
				else
				{
					LOGDEBUG("OYM_DEVICE_EVENT_ATT_READ_BY_TYPE_MSG with wrong length = %d, pair_len = %d\n", length, pair_len);
				}
				//UUID handle = data[OYM_ATT_READ_BY_TYPE_RES_START_HANDLE_OFFSET] + data[OYM_ATT_READ_BY_TYPE_RES_HANDLE_OFFSET + 1] << 8;
			}
			else if (OYM_ATT_PRODECURE_COMPLETE == status)
			{
				//included service find completeed.
				this->StartDiscoveryCharacteristic();
			}
			else
			{
				LOGERROR("included service end with error status!!!");
				//ERROR handle to do...
			}
		}
		default:
			break;
	}
	return result;
}

OYM_STATUS OYM_RemoteDevice::W4GattDisCharcStateProcessMessage(OYM_DEVICE_EVENT event, OYM_PUINT8 data, OYM_UINT16 length)
{
	OYM_STATUS result = OYM_FAIL;
	OYM_UINT8 status = data[OYM_ATT_READ_BY_TYPE_RES_STATUS_OFFSET];
	OYM_UINT8 pdu_len;
	OYM_UINT8 pair_len;
	LOGDEBUG("W4GattDisCharcStateProcessMessage process Message = %d \n", event);
	switch (event)
	{
	case OYM_DEVICE_EVENT_SLAVE_SECURY_REQUEST:
	case OYM_DEVICE_EVENT_AUTH_COMPLETE:
	case OYM_DEVICE_EVENT_BOND_COMPLETE:
		LOGDEBUG("W4GattPriSvcStateProcessMessage process error Message = %d \n", event);
		break;

	case OYM_DEVICE_EVENT_LINK_PARA_UPDATE:
		LOGDEBUG("OYM_DEVICE_EVENT_LINK_PARA_UPDATE with length = %d\n", length);
		status = data[EVENT_BOND_COMPLETE_STATUS_OFFSET];
		if (status == OYM_SUCCESS)
		{
			mConnInternal = data[EVENT_LINK_PARA_UPDATE_INTERVEL_OFFSET] + (data[EVENT_LINK_PARA_UPDATE_INTERVEL_OFFSET] << 8);
			mSlaveLatency = data[EVENT_LINK_PARA_UPDATE_LATENCY_OFFSET] + (data[EVENT_LINK_PARA_UPDATE_LATENCY_OFFSET + 1] << 8);
			mConnTimeout = data[EVENT_LINK_PARA_UPDATE_TIMEOUT_OFFSET] + (data[EVENT_LINK_PARA_UPDATE_TIMEOUT_OFFSET + 1] << 8);
			LOGDEBUG("mConnInternal = 0x%02x \n", mConnInternal);
			LOGDEBUG("mSlaveLatency = 0x%02x \n", mSlaveLatency);
			LOGDEBUG("mConnTimeout = 0x%02x \n", mConnTimeout);
		}
		break;

	case OYM_DEVICE_EVENT_ATT_ERROR_MSG:
	{
		//get error code
		OYM_UINT8 error_code = data[OYM_ATT_ERROR_MSG_ERROR_CODE_OFFSET];
		if (OYM_ERROR_CODE_ATTR_NOT_FOUND == error_code)
		{
			//no included service found, start to discovery charactistic.
			OYM_PRISERVICE* service = mService.FindPriSvcbyIndex(mService.mCurrentPriService);
			if (NULL != service)
			{
				LOGDEBUG("start to find the characteristic of  %dth service. \n", mService.mCurrentPriService);
				//mInterface->DiscoveryCharacteristic(mHandle, service->mStartHandle, service->mEndHandle);
				//mState = OYM_DEVICE_STATE_GATT_DIS_CHARC;
			}
			else
			{
				LOGDEBUG("mService.mCurrentPriService = %d is not available. \n", mService.mCurrentPriService);
			}
		}
	}

	case OYM_DEVICE_EVENT_ATT_READ_BY_TYPE_MSG:
	{
		pdu_len = data[OYM_ATT_READ_BY_TYPE_RES_PDU_LEN_OFFSET]; //get pdu length from raw data
		pair_len = data[OYM_ATT_READ_BY_TYPE_RES_PAIR_LEN_OFFSET]; //get characteristic struct length from raw data
		OYM_PRISERVICE* service = mService.FindPriSvcbyIndex(mService.mCurrentPriService);
		LOGDEBUG("OYM_DEVICE_EVENT_ATT_READ_BY_TYPE_MSG with status = %d\n", status);
		if (OYM_ATT_PRODECURE_COMPLETE == status)
		{
			if (NULL != service)
			{
				LOGDEBUG("OYM_DEVICE_EVENT_ATT_READ_BY_TYPE_MSG completeed with characteristic number is %d! \n", service->mNumOfCharacteristic);

				LOGDEBUG("start to read value------>! \n");
				if (service->mNumOfCharacteristic != 0)
				{
					service->mCurrentCharateristic = 0;
					OYM_CHARACTERISTIC* characteristic = service->FindCharacteristicbyIndex(service->mCurrentCharateristic);
					if (characteristic != NULL)
					{
						if (characteristic->mProperty & OYM_ATT_PROPERTY_READ)
						{
							//characteristic is readable
							mInterface->ReadCharacteristicValue(mHandle, characteristic->mValueHandle);
							mState = OYM_DEVICE_STATE_GATT_READ_CHARC_VALUE;
						}
						else
						{
							//do not need to read characteristic value. start to read characteristic descriptor if need.
							this->DiscoveryDescriptor();
						}
						
					}	
				}
				else
				{
					LOGDEBUG("no characteristic found, change state to next priservice! \n");
				}
				
				//change state to read characteristic value.
				// to do
			}
			else
			{
				LOGDEBUG("mService.mCurrentPriService = %d is not available. \n", mService.mCurrentPriService);
			}
		}
		else if (OYM_ATT_PROCEDURE_SUCCESS == status)
		{
			OYM_UINT8 num_chars;
			OYM_PUINT8 p = data + 5;

			if (pair_len != 0)
			{
				num_chars = (pdu_len - 1) / pair_len;
			}
			else
			{
				LOGERROR("pair_len with error!!!");
			}
			
			if ((pdu_len - 1) % pair_len == 0)
			{
				for (OYM_UINT8 i = 0; i < num_chars; i++)
				{
					service->AddCharacterIntoPriService(p, pair_len);
					p += pair_len;
				}
			}
			else
			{
				LOGDEBUG("OYM_DEVICE_EVENT_ATT_READ_BY_GRP_TYPE_MSG with wrong length = %d, pair_len = %d\n", length, pair_len);
			}
		}
		else
		{
			LOGERROR("OYM_DEVICE_EVENT_ATT_READ_BY_GRP_TYPE_MSG with error status! %d\n");
		}
	}

	default:
		break;
	}
	return result;
}

OYM_STATUS OYM_RemoteDevice::W4GattReadCharcValueStateProcessMessage(OYM_DEVICE_EVENT event, OYM_PUINT8 data, OYM_UINT16 length)
{
	OYM_STATUS result = OYM_FAIL;
	OYM_UINT8 status = data[OYM_ATT_READ_VALUE_RESP_STATUS_OFFSET];
	LOGDEBUG("W4GattReadCharcValueStateProcessMessage process Message = %d \n", event);
	switch (event)
	{
		case OYM_DEVICE_EVENT_SLAVE_SECURY_REQUEST:
		case OYM_DEVICE_EVENT_AUTH_COMPLETE:
		case OYM_DEVICE_EVENT_BOND_COMPLETE:
			LOGDEBUG("W4GattPriSvcStateProcessMessage process error Message = %d \n", event);
			break;

		case OYM_DEVICE_EVENT_LINK_PARA_UPDATE:
			LOGDEBUG("OYM_DEVICE_EVENT_LINK_PARA_UPDATE with length = %d\n", length);
			status = data[EVENT_BOND_COMPLETE_STATUS_OFFSET];
			if (status == OYM_SUCCESS)
			{
				mConnInternal = data[EVENT_LINK_PARA_UPDATE_INTERVEL_OFFSET] + (data[EVENT_LINK_PARA_UPDATE_INTERVEL_OFFSET] << 8);
				mSlaveLatency = data[EVENT_LINK_PARA_UPDATE_LATENCY_OFFSET] + (data[EVENT_LINK_PARA_UPDATE_LATENCY_OFFSET + 1] << 8);
				mConnTimeout = data[EVENT_LINK_PARA_UPDATE_TIMEOUT_OFFSET] + (data[EVENT_LINK_PARA_UPDATE_TIMEOUT_OFFSET + 1] << 8);
				LOGDEBUG("mConnInternal = 0x%02x \n", mConnInternal);
				LOGDEBUG("mSlaveLatency = 0x%02x \n", mSlaveLatency);
				LOGDEBUG("mConnTimeout = 0x%02x \n", mConnTimeout);
			}
			break;

		case OYM_DEVICE_EVENT_ATT_ERROR_MSG:
		{
			//get error code
			OYM_UINT8 error_code = data[OYM_ATT_ERROR_MSG_ERROR_CODE_OFFSET];
			LOGDEBUG("OYM_DEVICE_EVENT_ATT_ERROR_MSG with error code = %d \n", error_code);
			if (OYM_ERROR_CODE_ATTR_NOT_FOUND == error_code)
			{
				//no included service found, start to discovery charactistic.
				OYM_PRISERVICE* service = mService.FindPriSvcbyIndex(mService.mCurrentPriService);
				if (NULL != service)
				{
					LOGDEBUG("start to find the characteristic of  %dth service. \n", mService.mCurrentPriService);
					//mInterface->DiscoveryCharacteristic(mHandle, service->mStartHandle, service->mEndHandle);
					//mState = OYM_DEVICE_STATE_GATT_DIS_CHARC;
				}
				else
				{
					LOGDEBUG("mCurrentPriService = %d is not available. \n", mService.mCurrentPriService);
				}
			}
			else 
			{
				DiscoveryDescriptor();
			}
		}
		
		case OYM_DEVICE_EVENT_ATT_READ_BY_TYPE_MSG:
			break;
			
		/*characteristic value has been read.*/
		case OYM_DEVICE_EVENT_ATT_READ_RESP_MSG:
		{
			LOGDEBUG("OYM_DEVICE_EVENT_ATT_READ_RESP_MSG \n");
			OYM_PRISERVICE* service = mService.FindPriSvcbyIndex(mService.mCurrentPriService);
			if (service != NULL)
			{
				OYM_CHARACTERISTIC* characteristic = service->FindCharacteristicbyIndex(service->mCurrentCharateristic);
				LOGDEBUG("OYM_DEVICE_EVENT_ATT_READ_RESP_MSG success!!!\n");
				if (characteristic != NULL)
				{
					OYM_UINT8 length = data[OYM_ATT_READ_VALUE_RESP_LEN_OFFSET];
					LOGDEBUG("OYM_DEVICE_EVENT_ATT_READ_RESP_MSG with length = %d !!!\n", length);
					memcpy(characteristic->mAttriValue->mData, data + OYM_ATT_READ_VALUE_RESP_DATA_OFFSET, length);
					LOGDEBUG("OYM_DEVICE_EVENT_ATT_READ_RESP_MSG success!!!\n");
					
					this->DiscoveryDescriptor();
				}
				else
				{
					LOGERROR("OYM_DEVICE_EVENT_ATT_READ_RESP_MSG characteristic not found!!!\n");
				}
			}
			else
			{
				LOGERROR("OYM_DEVICE_EVENT_ATT_READ_RESP_MSG service not found!!!\n");
			}
			
		}
			break;
		case OYM_DEVICE_EVENT_ATT_READ_BY_INFO_MSG: //charecteristic descriptor
		{
			LOGDEBUG("OYM_DEVICE_EVENT_ATT_READ_BY_INFO_MSG with length %d\n", length);
			OYM_PRISERVICE* service = mService.FindPriSvcbyIndex(mService.mCurrentPriService);
			if (status == OYM_ATT_PROCEDURE_SUCCESS)
			{
				if (service != NULL)
				{
					OYM_CHARACTERISTIC* characteristic = service->FindCharacteristicbyIndex(service->mCurrentCharateristic);
					LOGDEBUG("OYM_DEVICE_EVENT_ATT_READ_BY_INFO_MSG success!!!\n");
					if (characteristic != NULL)
					{
						OYM_UINT8 len = data[3];
						OYM_UINT8 format = data[4];
						OYM_UINT8 num_char_des = 0;
						OYM_PUINT8 p = data + 5;
						/*16bit UUID*/
						if (format == 1)
						{
							if ((len - 1) % 4 == 0)
							{
								num_char_des = (len - 1) / 4;
								for (OYM_UINT8 i = 0; i< num_char_des; i++)
								{
									characteristic->AddDescriptorIntoCharacteristic(p, 4);
									LOGDEBUG("UUID of descriptor = 0x%02x!!!\n", (p[2] + (p[3] << 8) ));
									p += 4;
								}
							}
						}
						/*128bit UUID*/
						else if (format == 2)
						{
							if ((len - 1) % 18 == 0)
							{
								num_char_des = (len - 1) / 18;
								for (OYM_UINT8 i = 0; i< num_char_des; i++)
								{
									characteristic->AddDescriptorIntoCharacteristic(p, 18);
									p += 18;
								}
							}

						}
						else
						{
							LOGERROR("OYM_DEVICE_EVENT_ATT_READ_BY_INFO_MSG invalid format = %d !!!\n", format);
						}

						LOGDEBUG("descriptor number is %d !!!\n", characteristic->mNumOfDescriptor);
					}
					else
					{
						LOGERROR("OYM_DEVICE_EVENT_ATT_READ_RESP_MSG characteristic not found!!!\n");

					}
				}
				else
				{
					LOGERROR("OYM_DEVICE_EVENT_ATT_READ_BY_INFO_MSG service not found!!!\n");
				}
			}
			else if (status == OYM_ATT_PRODECURE_COMPLETE)
			{
				LOGDEBUG("descriptor discovery finished!!\n");
				NextCharacterateristic();
			}
			else
			{
				LOGERROR("OYM_DEVICE_EVENT_ATT_READ_BY_INFO_MSG with error status = %d!!!\n", status);
			}
		}
			break;
		case OYM_DEVICE_EVENT_ATT_WRITE_REPONSE:
		{
				
				break;
		}
		default:
			break;
	}
	return result;
}

OYM_ULONG OYM_RemoteDevice::GetThreadId()
{
	return mThreadID;
}

OYM_UINT16 OYM_RemoteDevice::GetHandle()
{
	return mHandle;
}

//revice message from AdapterManager and queue message to a list.
OYM_STATUS OYM_RemoteDevice::ProcessMessage(OYM_DEVICE_EVENT event, OYM_PUINT8 data, OYM_UINT16 length)
{
	LOGDEBUG("----->ProcessMessage\n");
	MESSAGE *message = new MESSAGE;
	message->event = event;
	message->length = length;
	memcpy(message->data, data, length);

	mMessage.push_back(message);
	return OYM_SUCCESS;
}

OYM_STATUS OYM_RemoteDevice::NextCharacterateristic()
{
	LOGERROR("start to Next characteristic!!!\n");
	OYM_PRISERVICE* service = mService.FindPriSvcbyIndex(mService.mCurrentPriService);
	OYM_CHARACTERISTIC* characteristic = service->FindCharacteristicbyIndex(service->mCurrentCharateristic);

	if (service->mCurrentCharateristic < (service->mNumOfCharacteristic - 1))
	{
		service->mCurrentCharateristic++;
		/*find the next characteristic*/
		characteristic = service->FindCharacteristicbyIndex(service->mCurrentCharateristic);
		if (characteristic != NULL)
		{
			mInterface->ReadCharacteristicValue(mHandle, characteristic->mValueHandle);
			mState = OYM_DEVICE_STATE_GATT_READ_CHARC_VALUE;
		}
		else
		{
			LOGERROR("find charcteristic error!!!\n");
		}
	}
	/*no more chracteristic exist, move to next private service if exist*/
	else if ((service = mService.GetNextAvailablePriSvc()) != NULL)
	{
		LOGDEBUG("start to discovery %dth primary service!!!\n", mService.mCurrentPriService);
		mInterface->DiscoveryIncludedPrimaryService(mHandle, service->mStartHandle, service->mEndHandle);
		mState = OYM_DEVICE_STATE_GATT_INC_SVC;
		return OYM_SUCCESS;
	}
	/*no more characteristic and no more priamry service exist.*/
	else
	{
		LOGDEBUG("222no more primary service exist, service discovery process has been finished!!!\n");
		LOGDEBUG("start to write CCC value!!!\n");
		WriteClientCharacteristicConfiguration();
	}

	return OYM_SUCCESS;
}

OYM_STATUS OYM_RemoteDevice::DiscoveryDescriptor()
{
	LOGDEBUG("start to DiscoveryDescriptor!!!\n");
	OYM_PRISERVICE* service = mService.FindPriSvcbyIndex(mService.mCurrentPriService);
	OYM_CHARACTERISTIC* characteristic = service->FindCharacteristicbyIndex(service->mCurrentCharateristic);
	OYM_UINT16 start_handle = (characteristic->mValueHandle + 1);
	OYM_UINT16 end_handle;
	/*if next charcteristic exist, end handle is the start handle of next charcteristic*/
	if (service->mCurrentCharateristic < (service->mNumOfCharacteristic - 1))
	{
		OYM_CHARACTERISTIC* next_characteristic = service->FindCharacteristicbyIndex(service->mCurrentCharateristic + 1);
		end_handle = next_characteristic->mHandle - 1;
	}
	/*if next charcteristic do not exist, but next private service exist, end handle is the start handle of next private service*/
	else if (mService.mCurrentPriService < (mService.mNumOfPrivateService - 1))
	{
		OYM_PRISERVICE* next_service = mService.FindPriSvcbyIndex(mService.mCurrentPriService + 1);
		end_handle = next_service->mStartHandle - 1;
	}
	else
		/*else, this characteristic is the last one.*/
	{
		end_handle = 0xFFFF;
	}
	LOGDEBUG("start_handle = %d, end_handle = %d !!!\n", start_handle, end_handle);

	if (end_handle >= start_handle)
	{
		LOGDEBUG("start to find charcteristic descriptor!!!\n");
		mInterface->FindCharacteristicDescriptor(mHandle, start_handle, end_handle);
		mState = OYM_DEVICE_STATE_GATT_READ_CHARC_VALUE;
	}
	else
	{
		LOGERROR("OYM_DEVICE_EVENT_ATT_READ_RESP_MSG no characteristic descriptor exist, move to next characteristic!!!\n");
		if (service->mCurrentCharateristic < (service->mNumOfCharacteristic - 1))
		{
			service->mCurrentCharateristic++;
			/*find the next characteristic*/
			characteristic = service->FindCharacteristicbyIndex(service->mCurrentCharateristic);
			if (characteristic != NULL)
			{
				mInterface->ReadCharacteristicValue(mHandle, characteristic->mValueHandle);
				mState = OYM_DEVICE_STATE_GATT_READ_CHARC_VALUE;
			}
			else
			{
				LOGERROR("find charcteristic error!!!\n");
			}
		}
		/*no more chracteristic exist, move to next private service if exist*/
		else if ((service = mService.GetNextAvailablePriSvc()) != NULL)
		{
			LOGDEBUG("start to discovery %dth primary service!!!\n", mService.mCurrentPriService);
			mInterface->DiscoveryIncludedPrimaryService(mHandle, service->mStartHandle, service->mEndHandle);
			mState = OYM_DEVICE_STATE_GATT_INC_SVC;
			return OYM_SUCCESS;
		}
		/*no more characteristic and no more priamry service exist.*/
		else
		{
			LOGDEBUG("111no more primary service exist, service discovery process has been finished!!!\n");
			LOGDEBUG("start to write CCC value!!!\n");
			WriteClientCharacteristicConfiguration();
		}
	}

	return OYM_SUCCESS;
}

OYM_STATUS OYM_RemoteDevice::StartDiscoveryCharacteristic()
{
	OYM_STATUS result;
	OYM_PRISERVICE* service = mService.FindPriSvcbyIndex(mService.mCurrentPriService);
	if (NULL != service)
	{
		LOGDEBUG("start to find the characteristic of  %dth service. \n", mService.mCurrentPriService);
		if (mInterface != NULL)
		{
			result = mInterface->DiscoveryCharacteristic(mHandle, service->mStartHandle, service->mEndHandle);
			if (result == OYM_SUCCESS)
			{
				mState = OYM_DEVICE_STATE_GATT_DIS_CHARC;
			}
		}
		else
		{
			//error handling.
		}
	}
	else
	{
		LOGDEBUG("mService.mCurrentPriService = %d is not available. \n", mService.mCurrentPriService);
	}

	return result;
}

OYM_STATUS OYM_RemoteDevice::WriteClientCharacteristicConfiguration()
{
	OYM_PRISERVICE* service;
	OYM_CHARACTERISTIC* characteristic;
	OYM_CHAR_DESCRIPTOR* descriptor;
	OYM_UUID uuid;
	OYM_UINT8 data[2] = {0x01, 0x00};
	uuid.type = OYM_UUID_16;
	uuid.value.uuid16 = 0x2902;

	mService.mCurrentPriService = 0;
	while (mService.mCurrentPriService < mService.mNumOfPrivateService)
	{
		service = mService.FindPriSvcbyIndex(mService.mCurrentPriService);
		service->mCurrentCharateristic = 0;
		LOGDEBUG("%d characteristic in %d th primary service. \n", service->mNumOfCharacteristic, mService.mCurrentPriService);
		while (service->mCurrentCharateristic < service->mNumOfCharacteristic)
		{
			characteristic = service->FindCharacteristicbyIndex(service->mCurrentCharateristic);
			LOGDEBUG("%d th characteristic finding... with descriptor num = %d\n", service->mCurrentCharateristic, characteristic->mNumOfDescriptor);
			descriptor = characteristic->FindDescriptorByUUID(uuid);
			if (descriptor != NULL)
			{
				LOGDEBUG("found!! handle is 0x%04x \n", descriptor->mHandle);
				if (descriptor->mHandle == 0x0039) {
					LOGDEBUG("found!! handle is 0x%04x,start write CCC in 0x%04x \n", descriptor->mHandle);
					mInterface->WriteCharacVlaue(mHandle, descriptor->mHandle, data, 2);
				}
				
			}
			service->mCurrentCharateristic++;
		}
		mService.mCurrentPriService++;
	}

	return OYM_SUCCESS;
}
