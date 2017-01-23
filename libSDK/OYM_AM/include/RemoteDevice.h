#ifndef __REMOTEDEVICE_H__
#define __REMOTEDEVICE_H__

#include "oym_types.h"
#include "OYM_Log.h"
#include <OYM_Callback.h>
#include <OYM_NIF.h>
#include "GattClient.h"

#define MODUAL_TAG_RD "Remote Device"

typedef struct
{
	OYM_UINT16 event;
	OYM_UINT16 length;
	OYM_UINT8 data[200];
}MESSAGE;

typedef enum
{
	OYM_DEVICE_STATE_IDLE = 0,
	OYM_DEVICE_STATE_W4CONN = 1,
	OYM_DEVICE_STATE_W4SECU = 2,
	OYM_DEVICE_STATE_GATT_PRI_SVC = 3,
	OYM_DEVICE_STATE_GATT_INC_SVC = 4,
	OYM_DEVICE_STATE_GATT_DIS_CHARC = 5,
	OYM_DEVICE_STATE_GATT_READ_CHARC_VALUE = 6,
} OYM_DEVICE_STATE;

typedef enum
{
	OYM_DEVICE_EVENT_DEVICE_CONNECTED = WM_USER + 0x500,
	OYM_DEVICE_EVENT_SLAVE_SECURY_REQUEST = WM_USER + 0x501,
	OYM_DEVICE_EVENT_AUTH_COMPLETE = WM_USER + 0x502,
	OYM_DEVICE_EVENT_BOND_COMPLETE = WM_USER + 0x503,
	OYM_DEVICE_EVENT_LINK_PARA_UPDATE = WM_USER + 0x504,
	OYM_DEVICE_EVENT_ATT_READ_BY_GRP_TYPE_MSG = WM_USER + 0x505,
	OYM_DEVICE_EVENT_ATT_READ_BY_TYPE_MSG = WM_USER + 0x506,
	OYM_DEVICE_EVENT_ATT_READ_RESP_MSG = WM_USER + 0x507,
	OYM_DEVICE_EVENT_ATT_ERROR_MSG = WM_USER + 0x508,
	OYM_DEVICE_EVENT_ATT_READ_BY_INFO_MSG = WM_USER + 0x509,
	OYM_DEVICE_EVENT_ATT_WRITE_REPONSE = WM_USER + 0x50A,
	OYM_DEVICE_EVENT_INVALID = WM_USER + 0x599,
} OYM_DEVICE_EVENT;

typedef enum
{
	OYM_DEVICE_ROLE_CENTRAL = 0,
	OYM_DEVICE_ROLE_PERIPHERAL = 1,
}OYM_DEVICE_ROLE;

#define EVENT_DEVICE_CCONNECTED_CONN_HANDLE_OFFSET 8
#define EVENT_DEVICE_CCONNECTED_ROLE_OFFSET (EVENT_DEVICE_CCONNECTED_CONN_HANDLE_OFFSET + 2)
#define EVENT_DEVICE_CCONNECTED_CONN_INTERVAL_OFFSET (EVENT_DEVICE_CCONNECTED_ROLE_OFFSET + 1)
#define EVENT_DEVICE_CCONNECTED_SLAVE_LATENCY_OFFSET (EVENT_DEVICE_CCONNECTED_CONN_INTERVAL_OFFSET + 2)
#define EVENT_DEVICE_CCONNECTED_CONN_TIMEOUT_OFFSET (EVENT_DEVICE_CCONNECTED_SLAVE_LATENCY_OFFSET + 2)
#define EVENT_DEVICE_CCONNECTED_CLOCK_ACCURACY_OFFSET (EVENT_DEVICE_CCONNECTED_CONN_TIMEOUT_OFFSET + 2)

#define EVENT_SLAVE_SECURY_REQUEST_STATUS_OFFSET 0
#define EVENT_SLAVE_SECURY_REQUEST_HANDLE_OFFSET (EVENT_SLAVE_SECURY_REQUEST_STATUS_OFFSET + 1)
#define EVENT_SLAVE_SECURY_REQUEST_ADDRESS_OFFSET (EVENT_SLAVE_SECURY_REQUEST_HANDLE_OFFSET + 2)
#define EVENT_SLAVE_SECURY_REQUEST_AUTH_TYPE_OFFSET (EVENT_SLAVE_SECURY_REQUEST_ADDRESS_OFFSET + 6)

#define EVENT_AUTH_COMPLETE_STATUS_OFFSET 0
#define EVENT_AUTH_COMPLETE_KEYSIZE_OFFSET (EVENT_AUTH_COMPLETE_STATUS_OFFSET + 5)
#define EVENT_AUTH_COMPLETE_LTK_OFFSET 34
#define EVENT_AUTH_COMPLETE_DIV_OFFSET 50
#define EVENT_AUTH_COMPLETE_RAND_OFFSET (EVENT_AUTH_COMPLETE_DIV_OFFSET+2)

#define EVENT_BOND_COMPLETE_STATUS_OFFSET 0

#define EVENT_LINK_PARA_UPDATE_STATUS_OFFSET 0
#define EVENT_LINK_PARA_UPDATE_INTERVEL_OFFSET 3
#define EVENT_LINK_PARA_UPDATE_LATENCY_OFFSET (EVENT_LINK_PARA_UPDATE_INTERVEL_OFFSET + 2)
#define EVENT_LINK_PARA_UPDATE_TIMEOUT_OFFSET (EVENT_LINK_PARA_UPDATE_LATENCY_OFFSET + 2)

class OYM_RemoteDevice:public Runnable
//class OYM_RemoteDevice
{
public:
	OYM_RemoteDevice(OYM_NPI_Interface* minterface, BLE_DEVICE address, OYM_CallBack* callback);
	~OYM_RemoteDevice();
	OYM_VOID Run();
	OYM_ULONG GetThreadId();
	OYM_UINT16 GetHandle();

	OYM_STATUS Connect();
	OYM_STATUS ProcessMessage(OYM_DEVICE_EVENT event, OYM_PUINT8 data, OYM_UINT16 length);
	OYM_STATUS IdleStateProcessMessage(OYM_DEVICE_EVENT event, OYM_PUINT8 data, OYM_UINT16 length);
	OYM_STATUS W4ConnStateProcessMessage(OYM_DEVICE_EVENT event, OYM_PUINT8 data, OYM_UINT16 length); 
	OYM_STATUS W4SecuStateProcessMessage(OYM_DEVICE_EVENT event, OYM_PUINT8 data, OYM_UINT16 length);
	OYM_STATUS W4GattPriSvcStateProcessMessage(OYM_DEVICE_EVENT event, OYM_PUINT8 data, OYM_UINT16 length);
	OYM_STATUS W4GattIncSvcStateProcessMessage(OYM_DEVICE_EVENT event, OYM_PUINT8 data, OYM_UINT16 length);
	OYM_STATUS W4GattDisCharcStateProcessMessage(OYM_DEVICE_EVENT event, OYM_PUINT8 data, OYM_UINT16 length);
	OYM_STATUS W4GattReadCharcValueStateProcessMessage(OYM_DEVICE_EVENT event, OYM_PUINT8 data, OYM_UINT16 length);

	OYM_STATUS StartDiscoveryCharacteristic();
	OYM_STATUS NextCharacterateristic();
	OYM_STATUS DiscoveryDescriptor();

	OYM_STATUS WriteClientCharacteristicConfiguration();
	
	OYM_UINT8  mAddrType;
	OYM_UINT8  mAddr[BT_ADDRESS_SIZE];
private:
	OYM_CHAR   mDevName[BLE_DEVICE_NAME_LENGTH];
	OYM_UINT16 mHandle; //connection handle
	OYM_DEVICE_ROLE mConnRole;
	OYM_UINT16 mConnInternal;  //connection interval
	OYM_UINT16 mSlaveLatency;
	OYM_UINT16 mConnTimeout;
	OYM_UINT8 mClockAccuracy;

	OYM_UINT8 mLTK[16];
	OYM_UINT16 mDIV;
	OYM_UINT8 mRAND[8];
	OYM_UINT8 mKeySize;

	OYM_CallBack* mCallback;
	OYM_NPI_Interface *mInterface;
	OYM_DEVICE_STATE mState;
	OYM_Log* mLog;

	CThread* mThread;
	OYM_ULONG mThreadID;

	list<MESSAGE*> mMessage;

	OYM_Service mService;
};
#endif