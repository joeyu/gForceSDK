#ifndef __DISCOVERYSERVICE_H_HH
#define __DISCOVERYSERVICE_H_HH

#include <oym_types.h>
#include <OYM_NIF.h>
#include <OYM_log.h>
#include <OYM_Callback.h>
//#include "AdapterManager.h"

#define LE_CONN_ADV			0
#define LE_SCAN_RESPONSE    4

#define LE_ADV_EVENT_TYPE_OFFSET   1
#define LE_ADV_ADDRESS_TYPE_OFFSET (LE_ADV_EVENT_TYPE_OFFSET+1)
#define LE_ADV_ADDRESS_OFFSET      (LE_ADV_ADDRESS_TYPE_OFFSET+1)
#define LE_ADV_RSSI_OFFSET         (LE_ADV_ADDRESS_OFFSET+6)
#define LE_ADV_DATA_LEN_OFFSET     (LE_ADV_RSSI_OFFSET+1)
#define LE_ADV_DATA_OFFSET         (LE_ADV_DATA_LEN_OFFSET+1)

//the threshlold can be adjust.
#define LE_ADV_RSSI_THRESHOLD 0xC8

//define return value for adv checking
#define LE_ADV_PROCESS_OK			 0 //can been report to upper level for new device
#define LE_ADV_UNDISCOVERABEL		 1 //the adv is either general discoverabel or limited discoverabel
#define LE_ADV_WAIT_FOR_DEVICE_NAME  2 //report new device to upper level until device name is availabel, whcih may exist in scan reponse

#define BLE_AD_TYPE_FLAG			0x01
#define BLE_AD_TYPE_16SRV_PART      0x02
#define BLE_AD_TYPE_NAME_CMPL       0x09
#define BLE_AD_TYPE_APPEARANCE      0x19

#define BLE_LIMIT_DISC_FLAG         (0x01 << 0)
#define BLE_GEN_DISC_FLAG           (0x01 << 1)

#define DEVICE_NAME_PATTERN_LENGTH 9
const char DEVICE_NAME_PATTERN[DEVICE_NAME_PATTERN_LENGTH] = { 0x67, 0x46, 0x6F, 0x72, 0x63, 0x65, 0x56, 0x45, 0x4e };

#define DISCOVERY_SERVICE_EVENT_MASK (EVENT_MASK_GAP_DEVICE_INFO_MSG | EVENT_MASK_GAP_DEVICE_DISCOVERY_MSG)
	
#define MODUAL_TAG_DS "DiscoveryService"

class OYM_Discovery_Service : public OYM_CallBack
{
public:
	OYM_Discovery_Service(OYM_NPI_Interface* minterface, OYM_CallBack* mcallback);
	//OYM_Discovery_Service(OYM_NPI_Interface* minterface);
	~OYM_Discovery_Service();

	OYM_STATUS Init();
	OYM_STATUS Deinit();

	OYM_STATUS CheckAdvData(OYM_PUINT8 data, OYM_UINT16 length);
	OYM_BOOL GetDeviceName(OYM_PCHAR result, OYM_PUINT8 data, OYM_UINT16 length);
	OYM_STATUS OnScanResult(OYM_PUINT8 data, OYM_UINT16 length);
	OYM_STATUS OnScanFinished();

	OYM_STATUS OnEvent(OYM_UINT32 event, OYM_PUINT8 data, OYM_UINT16 length)
	{
		return OYM_FAIL;
	}

	OYM_STATUS OnDeviceFound(BLE_DEVICE new_device)
	{
		return OYM_FAIL;
	}

	OYM_STATUS OnConnect(OYM_PUINT8 data, OYM_UINT16 length)
	{
		return OYM_FAIL;
	}

	OYM_STATUS StartScan();
	OYM_STATUS StopScan();

private:
	OYM_NPI_Interface *mInterface;
	OYM_BOOL mNeedScan;
	//OYM_AdapterManager* mAdapterManager;

	//fix Log name to mLog!!
	OYM_Log *mLog;

	list<BLE_DEVICE> mAvalbalDevice;
	OYM_CallBack* mCallback;
};
#endif