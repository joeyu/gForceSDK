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
#define EMGDATA_PACKAGEID_INDEX		8
#define EMGDATA_INDEX				9
#define FILE_NAME_LENGTH			100
HANDLE g_DataAvailable;
CRITICAL_SECTION g_CriticalSection;
FILE* g_file = NULL;

static void ProcessGforceData(OYM_PUINT8 pData, OYM_UINT16 length);

int _tmain(int charc, char* argv[]) {
	OYM_STATUS status;
	UINT8 comNum;
	char filename[FILE_NAME_LENGTH];

	g_DataAvailable = CreateEvent(NULL, TRUE, FALSE, NULL);
	InitializeCriticalSection(&g_CriticalSection);

	cout << "gForce raw data capturing utility\n"
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
		while (1)
		{		
			gets_s(filename, sizeof(filename)-1);
			errno_t err = fopen_s(&g_file, filename, "ab");
			if (err != 0) {
				cout << "Bad file name, please enter a new one:\n";
				continue;
			}
			break;
		}
		printf("\nPressing any key will start to record your EMG data to file '%s'.\n\n", filename);
		_getch();
		printf("Recording to file '%s' started...\n\n", filename);
		cout << "During recording, pressing 'Z' will close the file and promt you to open another file to record, and pressing 'X' will also exit the program.\n\n";
		while (1)
		{
			int in_c = _getch();
			if (in_c == 'Z' || in_c == 'z' || in_c == 'X' || in_c == 'x') {
				// Exit writing to the current file...

				// It takes a couple of seconds to flush some 'delayed' data.
				cout << "Exiting.......\n"
				     << "Please wait 1 second for buffered data...... ";
				Sleep(1000);

				EnterCriticalSection(&g_CriticalSection);
				fclose(g_file);
				g_file = NULL;
				LeaveCriticalSection(&g_CriticalSection);
				printf("File '%s' has been saved successfully :-) \n\n", filename);

				if (in_c == 'X' || in_c == 'x') { // exit the program
					exit = true;
				}
				break;
			}
		}
	} while (!exit);
	return 0;
}


void ProcessGforceData(OYM_PUINT8 data, OYM_UINT16 length)
{
	// If received emg data length is not equal to 137, something is wrong, and just drop it
	if (length != 137) {		
		cout << "[ERROR] Received bad EMG data!!!!\n";
		return;
	}
	//add data to process gForce rawdata
	static BOOL b_GetFirstPackage = FALSE;
	static OYM_UINT8 s_packageId = 0;
	static OYM_UINT32 s_ReceivePackageNum = 0;
	static OYM_UINT32 s_lostPackage = 0;
	
	s_ReceivePackageNum++;
	if (b_GetFirstPackage == FALSE)  // first time get into this function
	{
		b_GetFirstPackage = TRUE;
		s_packageId = data[EMGDATA_PACKAGEID_INDEX];  // get the first package id

		cout << "[INFO] Succeeded in connecting to gForce!\n";
		SetEvent(g_DataAvailable);          //set event to notify main 
	} else {
		s_lostPackage = (data[EMGDATA_PACKAGEID_INDEX] + 256 - s_packageId - 1) % 256 + s_lostPackage;
		if (data[EMGDATA_PACKAGEID_INDEX] !=((s_packageId +1) % 256))
		{
			float LostRate = ((float)(s_lostPackage * 10000 / s_ReceivePackageNum)) / 100;
			printf("[WARNING] Lost package: %d, lost package rate: %4f\n", s_lostPackage, LostRate);
		}
		s_packageId = data[EMGDATA_PACKAGEID_INDEX];
	}

	EnterCriticalSection(&g_CriticalSection);
	if (g_file)
	{
		// Write EMG data to file
		size_t writeLen = fwrite(&data[EMGDATA_INDEX], length - EMGDATA_INDEX, 1, g_file);
		if (writeLen != 1){
			cout << "[ERROR] Some data can't be written to the file!\n";
		}



	}
	LeaveCriticalSection(&g_CriticalSection);
}