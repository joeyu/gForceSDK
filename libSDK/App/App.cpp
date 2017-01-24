// App.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"

#include <OYM_NIF.h>
#include <DiscoveryService.h>
#include "AdapterManager.h"
FILE* inputfile = NULL;
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
	//inputfile = fopen("rawdata.txt", "a");
	while (1) {
		Sleep(5000);
		printf("main thread!\n");
	};
}


void ProcessGforceData(OYM_PUINT8 data, OYM_UINT16 length)
{
	//add data to process gForce rawdata
	char buf[1000];
	char* ptr = buf;
	OYM_INT total = 1000;
	OYM_INT offset = 0;
	OYM_INT totaloffset = 0;
	for (int i = 0; i < length; i++)
	{
		offset = sprintf_s((char*)ptr + totaloffset, total - totaloffset, "%02x ", data[i]);
		totaloffset = totaloffset + offset;
		//ptr += 2;
	}
	buf[totaloffset] = '\n';
	totaloffset += 1;
	errno_t err = fopen_s(&inputfile,"rawdata.txt", "a");
	if (err == 0)
	{
		fwrite(buf, sizeof(OYM_UINT8),totaloffset , inputfile);
		fclose(inputfile);
	}
	OutputDebugString(L"processGforceRawData \n ");
}