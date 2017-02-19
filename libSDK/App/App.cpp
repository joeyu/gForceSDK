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
FILE* inputfile = NULL;
HANDLE g_NotifySuccessed;
char str_filename[100];
CRITICAL_SECTION mutex;
void ProcessGforceData(OYM_PUINT8 pData, OYM_UINT16 length);

int _tmain(int charc, char* argv[]) {
	OYM_STATUS status;
	UINT8 comNum;
	g_NotifySuccessed = CreateEvent(NULL, TRUE, FALSE, NULL);
	InitializeCriticalSection(&mutex);
	memset(str_filename, 3, 100);
	printf("Please Enter COM number:");
	scanf_s("%u", &comNum);

	OYM_AdapterManager* am = new OYM_AdapterManager();
	status = am->Init(comNum);
	if (!OYM_SUCCEEDED)
	{
		return OYM_FAIL;
	}	
	am->RegistGforceData(ProcessGforceData);
	while (1)
	{
		status = am->StartScan();
		if (!OYM_SUCCEEDED)
		{
			printf("main thread! status = %d\n", status);
			return OYM_FAIL;
		}
		OYM_UINT8  num = am->WaitForScanFinished();  //wait for scan finished ,the max waitfor time is 10s
		if (num > 0)
		{
			break;  // find avaliable gforce
		}
	}  
	WaitForSingleObject(g_NotifySuccessed, INFINITE);  // wait for gforce notify data
	while (1) {
		Sleep(100);
		printf("***************************************Help Command***************************************\n");
		while (1)
		{
			printf("\nAfter gForce is worn on forearm properly, please enter Y:");
			char confirmdata = _getch();
			if (confirmdata == 'y' || confirmdata == 'Y')
			{
				printf("Y\n");
				break;
			}
			else {
				if (confirmdata == 'X')
				{
					return 1;
				}
			}
			
		}
		getchar();
		while (1)
		{		
			printf("Please Enter filename:");
			gets_s(str_filename, sizeof(str_filename)-1);
			FILE* tmpHandle;
			errno_t errn = fopen_s(&tmpHandle, str_filename, "ab");
			if (errn != 0) {
					printf("file name is wrong!Please enter a new file name\n");
			} else {
					fclose(tmpHandle);
					EnterCriticalSection(&mutex);
					errno_t err = fopen_s(&inputfile, str_filename, "ab");
					if (err != 0)
					{
						printf("open file failure***********************************************************\n");
					}
					LeaveCriticalSection(&mutex);
					break;
			}	
		}
		while (1)
		{
			OYM_INT getNum = _getch();
			if (getNum == 'Z'){
				printf("--------please wait five seconds!\n");
				Sleep(5000);
				printf("--------It is ok!\n");
				EnterCriticalSection(&mutex);
				fclose(inputfile);
				inputfile = NULL;
				LeaveCriticalSection(&mutex);
				break;
			}
			else if (getNum == 'X'){
				return 1;
			}
		}
	}
	return 1;
}


void ProcessGforceData(OYM_PUINT8 data, OYM_UINT16 length)
{
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

		printf("*           gForce Connect Succeed!!!!!!          *\n");
		SetEvent(g_NotifySuccessed);          //set event to notify main 
	} else {
		s_lostPackage = (data[EMGDATA_PACKAGEID_INDEX] + 256 - s_packageId - 1) % 256 + s_lostPackage;
		if (data[EMGDATA_PACKAGEID_INDEX] !=((s_packageId +1) % 256))
		{
			printf("!!!!!!!!!!!!lost package:%u\n",s_lostPackage);
		}
		s_packageId = data[EMGDATA_PACKAGEID_INDEX];
	}

		//float LostRate = ((s_lostPackage == 0) ? 0 : ((float)s_lostPackage / (float)s_ReceivePackageNum));
		//OYM_INT total = 1000;
		//OYM_INT offset = 0;
		//OYM_INT totaloffset = 0;
		//
		//for (int i = EMGDATA_INDEX; i < length; i++)
		//{
		//	offset = sprintf_s((char*)ptr + totaloffset, total - totaloffset, "%c", data[i]);
		//	totaloffset = totaloffset + offset;
		//}
			EnterCriticalSection(&mutex);
			if (inputfile)
			{
				OYM_INT writeLen = fwrite(&data[EMGDATA_INDEX], sizeof(OYM_UINT8), length - EMGDATA_INDEX, inputfile);
				if (writeLen != length - EMGDATA_INDEX){
					printf("some data can not be writed in file!!!!!!!!!!!!!!!!!!!!!!\n");
				}
			}
			LeaveCriticalSection(&mutex);	
//		printf("collecting EMG data:%d,   Lost package number:%u,    Total package number:%u,    Lost package Rate:%f.....\n", data[8], s_lostPackage, s_ReceivePackageNum, LostRate);
}