1. ��װ��AndroidStudio�汾��2.3

2. ��Ҫ�޸�ÿ��project��local.properties�ļ�������sdk��ndk����ʵ��װ·����
	sdk.dir=C\:\\Users\\yy220\\AppData\\Local\\Android\\Sdk
	ndk.dir=C\:\\Users\\yy220\\AppData\\Local\\Android\\android-ndk-r14b
	
3. \script\android\BLELibrary\���project����֮�󣬻���������lib��
	\BLELibrary\app\build\intermediates\ndkBuild\debug\obj\local\xxx\libble-lib.so����ļ�����jni(c++�����ɵ�so�ļ���
	�����˸���ƽ̨�¿ɵ��õ�so, ���Ӧ��source code ��\BLELibrary\app\src\main\cppĿ¼�¡�
	
	\script\android\BLELibrary\blelibrary\build\outputs\aar\blelibrary-debug.aar����java�ļ���lib.
	���Ӧ��source code��\BLELibrary\blelibrary\src\main\java\com\oym\blelibraryĿ¼�£���Щjava�ļ������android��׼��API��

4. �ȱ���BLELibrary project���ٽ����ɵ�so�ļ�������һ��test project�С�
	test project����so��aar�ļ�������һ��apk����������ʹ�á�
	��blelibrary-debug.aar����\test\app\libs�ļ����С�
	����Ӧƽ̨��libble-lib.so����\test\app\src\main\jniLibs\�¶�Ӧƽ̨���ļ����¡�
	\test\app\build.gradle�ļ���������apk���յ�����ƽ̨��
		externalNativeBuild {
            ndkBuild {
                cppFlags ""
                cFlags  "-std=c++11"
                abiFilters "x86"
            }
        }
	�����ģ��������android x86,ѡ��x86,
	�������ʵ�ֻ���ѡ��armeabiƽ̨
	\test\app\src\main\cpp\Android.mk�ļ�Ҫ�������ö�Ӧƽ̨��so�ļ���
	LOCAL_PATH := $(call my-dir)

	include $(CLEAR_VARS)
	LOCAL_MODULE := ble-lib
	LOCAL_SRC_FILES := $(LOCAL_PATH)/../jniLibs/x86/libble-lib.so
	#LOCAL_SRC_FILES := $(LOCAL_PATH)/../jniLibs/armeabi/libble-lib.so
	include $(PREBUILT_SHARED_LIBRARY)

	include $(CLEAR_VARS)
	
5. ���һ��˳���Ļ�������\test\app\build\outputs\apk\����һ��apk�ļ����ܹ�����ʵ��android���������С�

6. �ϲ�sdk��Ҫ���õ�api��\BLELibrary\app\src\main\inc\AdapterManagerInterface.h
	�ӿ���ʽ��win7 + NPI dongle�Ľӿ���һ���ġ�
	��������api��ʱ��֧�֡�
	//��Ϊandroid java�㣬û�нӿ����þ����conn_interval, slave_latence, super_TO�Ȳ�����
	virtual GF_STATUS ConnectionParameterUpdate(GF_UINT16 conn_handle, GF_UINT16 conn_interval_min, GF_UINT16 conn_interval_max, GF_UINT16 slave_latence, GF_UINT16 supervision_timeout) = 0;

	//��Ϊandroid java�㣬���ᱩ¶attribute_handle������������ʱ��֧��ͨ�����ֽӿڶ�дCharacteristic��
	virtual GF_STATUS WriteCharacteristic(GF_UINT16 conn_handle, GF_UINT16 attribute_handle, GF_UINT8 data_length, GF_PUINT8 data) = 0;
	virtual GF_STATUS ReadCharacteristic(GF_UINT16 conn_handle, GF_UINT16 attribute_handle) = 0;


	#ifndef __ADAPTERMANAGERINTERFACE_H__
	#define __ADAPTERMANAGERINTERFACE_H__
	#include "GFBLETypes.h"

	class GF_CClientCallback;
	class GF_CAdapterManager;

	class GF_CAdapterManagerInterface
	{
	public:
		static GF_CAdapterManagerInterface* GetInstance();
		virtual GF_STATUS Init(GF_UINT8 com_num, GF_UINT8 log_type) = 0;
		virtual GF_STATUS Deinit() = 0;
		virtual GF_STATUS StartScan(GF_UINT8 RSSI_Threshold) = 0;
		virtual GF_STATUS StopScan() = 0;
		virtual GF_STATUS Connect(GF_PUINT8 addr, GF_UINT8 addr_type, GF_BOOL is_direct_conn) = 0;
		virtual GF_STATUS CancelConnect(GF_PUINT8 addr, GF_UINT8 addr_type) = 0;
		virtual GF_STATUS Disconnect(GF_UINT16 handle) = 0;
		virtual GF_STATUS RegisterClientCallback(GF_CClientCallback* callback) = 0;
		virtual GF_STATUS UnregisterClientCallback() = 0;

		virtual GF_STATUS ConfigMtuSize(GF_UINT16 conn_handle, GF_UINT16 MTU_Size) = 0;
		virtual GF_STATUS ConnectionParameterUpdate(GF_UINT16 conn_handle, GF_UINT16 conn_interval_min, GF_UINT16 conn_interval_max, GF_UINT16 slave_latence, GF_UINT16 supervision_timeout) = 0;
		virtual GF_STATUS WriteCharacteristic(GF_UINT16 conn_handle, GF_UINT16 attribute_handle, GF_UINT8 data_length, GF_PUINT8 data) = 0;
		virtual GF_STATUS ReadCharacteristic(GF_UINT16 conn_handle, GF_UINT16 attribute_handle) = 0;

		virtual GF_HubState GetHubState() = 0;
		virtual GF_UINT8 GetConnectedDeviceNum() = 0;
		/*connected_device is output result*/
		virtual GF_STATUS GetConnectedDeviceByIndex(GF_UINT8 index, GF_ConnectedDevice* connected_device) = 0;
	};
	#endif

	callback�Ľӿڶ�����\BLELibrary\app\src\main\inc\ClientCallbackInterface.h�ļ��С�
		�������ӿ���ʱ������á�
		//ԭ��ͬ�ϣ�android��֧�����ָ�ʽ�Ľӿڣ�������ͳһ������
		virtual void onConnectionParmeterUpdated(GF_STATUS status, GF_UINT16 handle, GF_UINT16 conn_int, GF_UINT16 superTO, GF_UINT16 slavelatency) = 0;
		virtual void onCharacteristicValueRead(GF_STATUS status, GF_UINT16 handle, GF_UINT8 length, GF_PUINT8 data) = 0;
		
		//android����Ҫ���callback��
		virtual void onComDestory() = 0;

	#ifndef ICLIENTCALLBADK
	#define ICLIENTCALLBADK
	#include "GFBLETypes.h"

	class GF_CClientCallback
	{
	public:
		virtual void onScanResult(GF_BLEDevice* device) = 0;
		virtual void onScanFinished() = 0;
		virtual void onDeviceConnected(GF_STATUS status, GF_ConnectedDevice *device) = 0;
		virtual void onDeviceDisconnected(GF_STATUS status, GF_ConnectedDevice *device, GF_UINT8 reason) = 0;

		virtual void onMTUSizeChanged(GF_STATUS status, GF_UINT16 handle, GF_UINT16 mtu_size) = 0;
		virtual void onConnectionParmeterUpdated(GF_STATUS status, GF_UINT16 handle, GF_UINT16 conn_int, GF_UINT16 superTO, GF_UINT16 slavelatency) = 0;
		virtual void onCharacteristicValueRead(GF_STATUS status, GF_UINT16 handle, GF_UINT8 length, GF_PUINT8 data) = 0;

		/*Notification format: data length(1 byte N) + data(N Bytes)*/
		virtual void onNotificationReceived(GF_UINT16 handle, GF_UINT8 length, GF_PUINT8 data) = 0;

		virtual void onComDestory() = 0;
	};
	#endif
	