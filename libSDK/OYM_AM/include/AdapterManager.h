#ifndef __ADAPTERMANAGER_H__
#define __ADAPTERMANAGER_H__
#include "oym_types.h"
//#include "OYM_NIF.h"
//#include <OYM_Log.h>
#include <OYM_Callback.h>
//#include "DiscoveryService.h"
//#include "RemoteDevice.h"
#include "list"
using namespace std;
#define MODUAL_TAG_AM "AdaperManager"
//#define ADAPTERMANAGER_EVENT (EVENT_MASK_LINK_PARA_UPDATE_MSG | EVENT_MASK_AUTH_COMPLETE_MSG | EVENT_MASK_SLAVE_REQUESTED_SECURITY_MSG | EVENT_MASK_INTERNAL_DEVICE_FOUND | EVENT_MASK_INTERNAL_SCAN_FINISHED | EVENT_MASK_GAP_LINK_ESTABLISHED_MSG)


typedef void(*PTGFORCEDATA)(OYM_PUINT8 pData, OYM_UINT16 length);

class OYM_RemoteDevice;
class OYM_Discovery_Service;
class OYM_Log;
class OYM_NPI_Interface;

class OYM_AdapterManager: public OYM_CallBack
{
public:
	//OYM_AdapterManager(OYM_NPI_Interface* nif_interface);
	OYM_AdapterManager();
	~OYM_AdapterManager();

	OYM_STATUS Init();
	OYM_STATUS Init(OYM_UINT8 portNum);
	OYM_STATUS Deinit();
	OYM_STATUS StartScan();
	OYM_STATUS StopScan();
	OYM_STATUS OnDeviceFound(BLE_DEVICE new_device);
	OYM_STATUS Connect(OYM_PUINT8 addr, UINT8 addr_type);
	OYM_STATUS OnConnect(OYM_PUINT8 data, OYM_UINT16 length);
	OYM_UINT8	WaitForScanFinished();
	OYM_STATUS OnEvent(OYM_UINT32 event, OYM_PUINT8 data, OYM_UINT16 length);
	OYM_STATUS RegistGforceData(PTGFORCEDATA p_DataFun);
	OYM_STATUS OnScanResult(OYM_PUINT8 data, OYM_UINT16 length)
	{
		return OYM_FAIL;
	}

	OYM_STATUS OnScanFinished();

private:
	OYM_NPI_Interface* mInterface;
	OYM_Log* mLog;
	OYM_Discovery_Service* mDS;
	list<OYM_RemoteDevice*> mAvailabeDevice;
	PTGFORCEDATA mPTgForceDataFunction;
	//BOOL mScanFinishFlag;
	HANDLE mScanFinishEvent;

};
#endif