// App.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "conio.h"
#include <stdio.h>
#include <OYM_NIF.h>
#include <DiscoveryService.h>
#include "AdapterManager.h"
#include "dos.h"

//using namespace System;
#define EMGDATA_MINOR_TYPE_INDEX	6  // minor data index in received notify data
#define EMGDATA_LENGTH_INDEX		7	// for protocol ,lenght index,the value of length inclue emg data and package id data
#define EMGDATA_CRC_INDEX			8	// crc data index, crc  for emg data and package id 
#define EMGDATA_PACKAGEID_INDEX		9	// package id index in received notify data
#define EMGDATA_VALIDDATA_INDEX		10  //valid emg data index

// valid data is emgdata + head data + packageid
#define EMGDATA_LENGTH				128  // emg data length
#define EMGDATA_HEAD_LENGTH			3	// head data length
#define EMGDATA_PACKAGEID_LENGTH	1   // package id length

#define SYSTEMTIME_LENGTH			8
#define FILE_NAME_LENGTH			100

// this software is run in rawdata mode,capture correct emg raw data
#define GFORCERAWDATAMODE	1
// this software is run in test mode,gforce send data is 0x00~0x7f,use to test gforce data transmission
#define GFORCETESTMODE		0

HANDLE g_DataAvailable;
CRITICAL_SECTION g_CriticalSection;
char g_filename[FILE_NAME_LENGTH];
FILE* g_file = NULL;
size_t g_SingleFileRecordedBytes;
bool g_PrintSingleFileRecordedBytesPreface;

static void ProcessGforceData(OYM_PUINT8 pData, OYM_UINT16 length);
static void PrintSingleFileRecordedBytes(int n);
static UINT8 CheckSum(UINT8* data, UINT8 length);
static ULARGE_INTEGER GetFileTime(void);


int _tmain(int charc, char* argv[]) {
	OYM_STATUS status;
	UINT8 comNum;

	g_DataAvailable = CreateEvent(NULL, TRUE, FALSE, NULL);
	InitializeCriticalSection(&g_CriticalSection);
#if GFORCETESTMODE
	cout<<"*****************************************************************\n";
	cout<<"*                                                               *\n";
	cout<<"*             This version is running in Test Mode              *\n";
	cout<<"*                                                               *\n";
	cout<<"*****************************************************************\n";
#endif
	cout << "===== gForce raw data capturing utility Ver0.1 =====\n\n"
	     << "Please enter COM number:";
	scanf_s("%u", &comNum);
	OYM_AdapterManager* am = new OYM_AdapterManager();
	status = am->Init(comNum);
	if (!OYM_SUCCEEDED)
	{
		return -2;
	}	
	am->RegistGforceData(ProcessGforceData);
	OYM_UINT8 gForce_count;
	do {
		status = am->StartScan();
		if (!OYM_SUCCEEDED)
		{
			cout << "[ERROR] OYM_AdapterManager::StartScanFailed\n";
			return -2;
		}
		gForce_count = am->WaitForScanFinished();  //wait for scan finished, 10s to timeout.
	} while (gForce_count == 0);

	getchar();  // clean the stdin buffer
	WaitForSingleObject(g_DataAvailable, INFINITE);  // wait for gforce notify data
	bool exit = false;
	do {
		Sleep(100);
		cout << "\n"
		     << "After putting on the gForce properly, please enter the name of the file for recording your EMG data:\n";
		FILE *tmp_file = NULL;
		while (1)
		{		
			gets_s(g_filename, sizeof(g_filename) - 1);
			errno_t err = fopen_s(&tmp_file, g_filename, "ab");
			if (err != 0) {
				cout << "Bad file name, please enter a new one:\n";
				continue;
			}
			break;
		}
		printf("\nPressing any key will start to record your EMG data to file '%s'.\n\n", g_filename);
		_getch();
		g_SingleFileRecordedBytes = 0; //Reset the count for the new file
		g_PrintSingleFileRecordedBytesPreface = true;
		g_file = tmp_file;
		printf("Recording to file '%s' started...\n\n", g_filename);
		cout << "During recording, pressing 'Z' will close the file and promt you to open another file to record, and pressing 'X' will also exit the program.\n\n";
		while (1)
		{
			int in_c = _getch();
			if (in_c == 'Z' || in_c == 'z' || in_c == 'X' || in_c == 'x') {
				// Exit writing to the current file...

				// It takes a couple of seconds to flush some 'delayed' data.
				cout << "Exiting.......\n"
				     << "Please wait 1 second for buffered data......\n";
				g_PrintSingleFileRecordedBytesPreface = true;
				Sleep(1000);

				EnterCriticalSection(&g_CriticalSection);
				fclose(g_file);
				g_file = NULL;
				LeaveCriticalSection(&g_CriticalSection);
				printf("\nFile '%s' has been saved successfully :-) \n\n", g_filename);

				if (in_c == 'X' || in_c == 'x') { // exit the program
					exit = true;
				}
				break;
			}
		}
	} while (!exit);

	CloseHandle(g_DataAvailable);
	return 0;
}


void ProcessGforceData(OYM_PUINT8 data, OYM_UINT16 length)
{
	// If received emg data length is not equal to 137, something is wrong, and just drop it
	if (length != 138) {		
		cout << "[ERROR] Received bad EMG data!!!!\n";
		return;
	}
	//add data to process gForce rawdata
	static BOOL b_GetFirstPackage = FALSE;
	static OYM_UINT8 s_packageId = 0;
	static OYM_UINT32 s_ReceivePackageNum = 0;
	static OYM_UINT32 s_lostPackage = 0;
#if GFORCETESTMODE
	bool  b_errordata = false;  // receive data is error
#endif
	s_ReceivePackageNum++;
	if (b_GetFirstPackage == FALSE)  // first time get into this function
	{
		b_GetFirstPackage = TRUE;
		s_packageId = data[EMGDATA_PACKAGEID_INDEX];  // get the first package id

		cout << "[INFO] Succeeded in connecting to gForce!\n";
		SetEvent(g_DataAvailable);          //set event to notify main 
	} else {
		s_lostPackage = (data[EMGDATA_PACKAGEID_INDEX] + 256 - s_packageId - 1) % 256 + s_lostPackage;
		if (data[EMGDATA_PACKAGEID_INDEX] !=((s_packageId +1) % 256))   // lost package
		{
			float LostRate = ((float)(s_lostPackage * 10000 / s_ReceivePackageNum)) / 100;
			printf("[WARNING] Lost package: %d, lost package rate: %4f\n", s_lostPackage, LostRate);
			g_PrintSingleFileRecordedBytesPreface = true;
		}
		s_packageId = data[EMGDATA_PACKAGEID_INDEX];
	}

#if GFORCETESTMODE
	UINT8* dataP = &data[EMGDATA_INDEX];
	for(unsigned int index =0; index<128; index++) {
		if (dataP[index]!=index) {
			printf("[ERROR] Receive data:%d ,Actual data:%d\n",dataP[index],index);
			b_errordata = true;
		}
	}
	if (b_errordata) {
		printf("---------------------------------------------------------------\n");
	}
#endif
	EnterCriticalSection(&g_CriticalSection);
	if (g_file)
	{
		// Write EMG data to file
		UINT8 crcData = CheckSum(&data[EMGDATA_PACKAGEID_INDEX], 129);
		if (crcData == data[EMGDATA_CRC_INDEX])
		{
			size_t toWrite = EMGDATA_LENGTH + EMGDATA_HEAD_LENGTH + EMGDATA_PACKAGEID_LENGTH;
			if (1 != fwrite(&data[EMGDATA_MINOR_TYPE_INDEX], toWrite, 1, g_file)){
				cout << "[ERROR] Some data can't be written to the file!\n";
				g_PrintSingleFileRecordedBytesPreface = true;
			}
			else {
				ULARGE_INTEGER ltime;
				ltime = GetFileTime();
				if (1 != fwrite(&ltime, sizeof(ltime), 1, g_file)) {
					cout << "[ERROR] system time can't be written to the file\n";
					g_PrintSingleFileRecordedBytesPreface = true;
				}
				g_SingleFileRecordedBytes = g_SingleFileRecordedBytes + toWrite + sizeof(ltime);                           
				PrintSingleFileRecordedBytes(g_SingleFileRecordedBytes);
			}
		}
		else {
			printf("[ERROR] CRC checksum error!\n");
		}
		
	}
	LeaveCriticalSection(&g_CriticalSection);
}


void PrintSingleFileRecordedBytes(int n) {
	if (g_PrintSingleFileRecordedBytesPreface) {
		printf("File %s's recorded bytes:         ", g_filename);
	}
	printf("\b\b\b\b\b\b\b\b%8d", n);
	g_PrintSingleFileRecordedBytesPreface = false;
}

//crc checksum
UINT8 CheckSum(UINT8* data, UINT8 length)
{
	UINT8 cs = 0;
	while (length--) {
		cs ^= *data++;
	}
	return cs;
}

//get systemtime and convert to ulonglong format
ULARGE_INTEGER GetFileTime(void)
{
	SYSTEMTIME utcSystemTime;
	FILETIME utcFileTime;
	ULARGE_INTEGER time;
	GetSystemTime(&utcSystemTime);
	SystemTimeToFileTime(&utcSystemTime, &utcFileTime);
	time.HighPart = utcFileTime.dwHighDateTime;
	time.LowPart = utcFileTime.dwLowDateTime;
	return time;
}