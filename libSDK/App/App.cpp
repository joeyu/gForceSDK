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
FILE* inputfile = NULL;
BOOL b_file = FALSE;
BOOL b_information = FALSE;
char str_filename[100];
void ProcessGforceData(OYM_PUINT8 pData, OYM_UINT16 length);

int _tmain(int charc, char* argv[]) {
	OYM_STATUS status;
	OYM_AdapterManager* am = new OYM_AdapterManager();
	status = am->Init();
	if (!OYM_SUCCEEDED)
	{
		return OYM_FAIL;
	}
	am->RegistGforceData(ProcessGforceData);
	status = am->StartScan();
	if (!OYM_SUCCEEDED)
	{
		return OYM_FAIL;
		printf("main thread! status = %d\n", status);
	}
	while (!b_information);  //wait for notify data message
	while (1) {
		Sleep(100);
		printf("***************************************Help Command***************************************\n");
		while (1)
		{
			printf("\nAfter gForce is worn on forearm properly, please enter Y:");
			char confirmdata = _getch();
			if (confirmdata == 'y' || confirmdata == 'Y')
			{
				break;
			}
		}
		
		printf("\nPlease Enter filename:");
		gets_s(str_filename);
		b_file = TRUE;
		while (1)
		{
			if (_getch() == 'Z')
				break;
		}
		b_file = FALSE;
		memset(str_filename, '\0', 100);
		//Sleep(5000);
		//printf("main thread!\n");
	}
}


void ProcessGforceData(OYM_PUINT8 data, OYM_UINT16 length)
{
	//add data to process gForce rawdata
	static BOOL b_GetFirstPackage = FALSE;
	static OYM_UINT8 s_packageId = 0;
	static OYM_UINT32 s_ReceivePackageNum = 0;
	static OYM_UINT32 lostPackage = 0;
	s_ReceivePackageNum++;
	if (b_GetFirstPackage == FALSE)
	{
		b_GetFirstPackage = TRUE;
		s_packageId = data[EMGDATA_PACKAGEID_INDEX];
	} else{
		lostPackage = (data[EMGDATA_PACKAGEID_INDEX] + 256 - s_packageId -1) % 256  + lostPackage;
		s_packageId = data[EMGDATA_PACKAGEID_INDEX];
	}
	float LostRate = ((lostPackage == 0) ? 0 : ((float)lostPackage / (float)s_ReceivePackageNum));
	if (b_file)
	{
		char buf[1000];
		char* ptr = buf;
		OYM_INT total = 1000;
		OYM_INT offset = 0;
		OYM_INT totaloffset = 0;
		for (int i = EMGDATA_INDEX; i < length; i++)
		{
			offset = sprintf_s((char*)ptr + totaloffset, total - totaloffset, "%c", data[i]);
			totaloffset = totaloffset + offset;
		}
		errno_t err = fopen_s(&inputfile, str_filename, "a");
		if (err == 0)
		{
			fwrite(buf, sizeof(OYM_UINT8), totaloffset, inputfile);
			fclose(inputfile);
		}
		printf("collecting EMG data:%d,   Lost package number:%u,    Total package number:%u,    Lost package Rate:%f.....\n", data[8], lostPackage,s_ReceivePackageNum, LostRate);
		//OutputDebugString(L"processGforceRawData \n ");
	}
	else if(b_information == FALSE){
		b_information = TRUE;
		printf("*                                  gForce is connect success!                                       *\n");
	}
}