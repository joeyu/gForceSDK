#ifndef __OYM_NIF_H__
#define __OYM_NIF_H__

#include <npi_cmd.h>
#include <npi_evt.h>
#include <oym_types.h>
#include <OYM_Callback.h>
#include <OYM_Log.h>
#include <iostream>
#include <list>
using namespace std;

#define MODUAL_TAG "Native Interface"
//list<int> mCallback;
typedef list<OYM_CallBack *> LISTCALLBACK;

class OYM_NPI_Interface:public Runnable
{
public:
	OYM_VOID Run();
	OYM_NPI_Interface();
	OYM_STATUS Init();
	OYM_STATUS Deinit();

	OYM_STATUS InitDevice();
	OYM_STATUS StartLEScan();
	OYM_STATUS StopLEScan();
	OYM_STATUS Connect(OYM_PUINT8 addr, UINT8 addr_type);
	
	OYM_STATUS Authenticate(OYM_UINT16 handle);
	OYM_STATUS Bond(OYM_UINT16 handle, OYM_PUINT8 ltk, OYM_UINT16 div, OYM_PUINT8 rand, OYM_UINT8 ltk_size);
	
	OYM_STATUS DiscoveryAllPrimaryService(OYM_UINT16 handle);
	OYM_STATUS DiscoveryIncludedPrimaryService(OYM_UINT16 conn_handle, OYM_UINT16 start_handle, OYM_UINT16 end_handle);
	OYM_STATUS DiscoveryCharacteristic(OYM_UINT16 conn_handle, OYM_UINT16 start_handle, OYM_UINT16 end_handle);
	OYM_STATUS ReadCharacteristicValue(OYM_UINT16 conn_handle, OYM_UINT16 att_handle);
	OYM_STATUS FindCharacteristicDescriptor(OYM_UINT16 conn_handle, OYM_UINT16 start_handle, OYM_UINT16 end_handle);
	OYM_STATUS WriteCharacVlaue(OYM_UINT16 conn_handle, OYM_UINT16 att_handle, OYM_PUINT8 data, OYM_UINT8 len);
	OYM_STATUS ExchangeMTUsize(OYM_UINT16 conn_handle, OYM_UINT16 mtu_size);
	//call this function to register callback function to process event.
	OYM_STATUS RegisterCallback(OYM_CallBack *callback);
	
private:
	NPI_CMD* mCommand;
	LISTCALLBACK mCallback;
	OYM_ULONG mEvtThreadID;
	CThread *mEvtThread;

	//fix Log name to mLog!!
	OYM_Log *mLog;
};

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

#define EVENT_MASK_GAP_STATUS_MSG				BIT0
#define EVENT_MASK_GAP_DEVICE_INFO_MSG			BIT1
#define EVENT_MASK_GAP_DEVICE_DISCOVERY_MSG		BIT2
#define EVENT_MASK_GAP_LINK_ESTABLISHED_MSG		BIT3
#define EVENT_MASK_GAP_LINK_TERMINATED_MSG		BIT4
#define EVENT_MASK_SLAVE_REQUESTED_SECURITY_MSG BIT5
#define EVENT_MASK_AUTH_COMPLETE_MSG			BIT6
#define EVENT_MASK_BOND_COMPLETE_MSG			BIT7

#define EVENT_MASK_LINK_PARA_UPDATE_MSG			(BIT0 << 8)
#define EVENT_MASK_ATT_NOTI_MSG					(BIT2 << 8)
#define EVENT_MASK_ATT_READ_BY_GRP_TYPE_MSG		(BIT3 << 8)
#define EVENT_MASK_ATT_READ_BY_TYPE_MSG			(BIT4 << 8)
#define EVENT_MASK_ATT_READ_BY_INFO_MSG			(BIT5 << 8)
#define EVENT_MASK_ATT_ERROR_MSG				(BIT6 << 8)
#define EVENT_MASK_ATT_READ_RESP_MSG			(BIT7<< 8)

#define EVENT_MASK_INTERNAL_DEVICE_FOUND		(BIT0 << 16)
#define EVENT_MASK_INTERNAL_SCAN_FINISHED		(BIT1 << 16)
#define EVENT_MASK_ATT_WRITE_RESPONSE			(BIT2 << 16)
#endif