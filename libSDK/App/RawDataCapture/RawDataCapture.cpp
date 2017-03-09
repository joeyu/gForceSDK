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
#include "npi_queue.h"
#include "Thread.h"
#include "WebSocket.h"

class WebSocketClientRunnable : public Runnable {
public:
	WebSocketClientRunnable(const WCHAR *server, INTERNET_PORT port) 
		: m_server(server), 
		m_port(port), 
		m_state(UNINITIALIZED){}
	~WebSocketClientRunnable();

	enum STATE {
		UNINITIALIZED,
		READY_TO_RUN,
		RUNNING,
		PAUSED
	};

	int	Init();
	void	Deinit();
	void	Run();
	int	PutData(char *data_buf, size_t size);
	STATE	GetState() const { return m_state; }
private:
	struct Data {
		size_t	size;
		char	buf[1];
	};

	GForceQueue<Data *, 256>	m_sendQueue; // buffer of data to be sent.
	WebSocket			m_webSocket;
	const WCHAR			*m_server;
	const INTERNET_PORT		m_port;
	volatile STATE			m_state;
};

WebSocketClientRunnable::~WebSocketClientRunnable() {
	Deinit();
}

int WebSocketClientRunnable::Init() {
	if (m_state != UNINITIALIZED) {
		return 0;
	}
	if (0 != m_webSocket.Open(m_server, m_port)) {
		printf("[ERROR] WebSocket::Open failed\n");
		return -1;
	}
	m_state = READY_TO_RUN;
	//printf("[INFO] Connected to WebSocket server [%s:%d] successfully.\n", m_server, m_port);
	return 0;
}

void WebSocketClientRunnable::Deinit() {
	m_webSocket.Close();
	m_state = UNINITIALIZED;
}

void WebSocketClientRunnable::Run() {
	if (m_state != READY_TO_RUN) {
		return;
	}
	m_state = RUNNING;

	// Open the connection.
	// Signal the server of the client role.
	const char source_role[] = "source";
	if (0 != m_webSocket.Send(WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE, (void *)source_role, strlen(source_role))) {
		//printf("[ERROR] WebSocket::Send\n");
		goto QUIT;
	}
	while (true) {
		Data *data = m_sendQueue.Pop();
		if (0 != m_webSocket.Send(WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE, data->buf, data->size)) {
			printf("[ERROR] WebSocket::Send\n");
			goto QUIT;
		}
		delete data; // this is what the consumer must do
	}
QUIT:
	m_state = READY_TO_RUN;
}

int WebSocketClientRunnable::PutData(char *data_buf, size_t length) {
	if (m_state != RUNNING) {
		return 0;
	}
	Data * data = (Data *)new char[(long)&(((Data *)0)->buf) + length];
	if (NULL == data) {
		return -1;
	}
	memcpy(data->buf, data_buf, length);
	data->size = length;
	m_sendQueue.Push(data);

	return 0;
}


//using namespace System;
#define EMGDATA_EVENT_TYPE_INDEX	6	// minor data index in received notify data
#define EMGDATA_LENGTH_INDEX		7	// for protocol ,lenght index,the value of length inclue emg data and package id data
#define EMGDATA_CRC_INDEX		8	// crc data index, crc  for emg data and package id 
#define EMGDATA_PACKAGEID_INDEX		9	// package id index in received notify data
#define EMGDATA_VALIDDATA_INDEX		10	// valid emg data index

// valid data is emgdata + head data + packageid
#define EMGDATA_LENGTH				128  // emg data length
#define EMGDATA_HEAD_LENGTH			3	// head data length
#define EMGDATA_PACKAGEID_LENGTH		1   // package id length

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

WCHAR *g_server = L"127.0.0.1";
unsigned short g_port = 8888;
WebSocketClientRunnable g_webSocketClientRunnalbe(g_server, g_port);

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
	cout << "*****************************************************************\n";
#endif
	CThread webSocketClientThread(&g_webSocketClientRunnalbe);

	cout << "========================= gForce EMG raw data capturing utility V0.2 ==========================\n\n";

	cout << "The EMG raw data captured from your arm will be stored in the files specified by you later.\n"
		"Furthermore, the data can also be real-time shown in your web browser as long as you set\n"
		"up a websocket server. To do so, please make sure that `node.js` is installed first, and then \n"
		"run `node WebSocketServer.js` in the command shell, and finally open file `WebSocketClient.html` \n"
		"in your browser to wait for raw data to be shown up.\n\n";

	cout << "Please press any key to continue if you're ready...\n\n";
	_getch();
	
	wprintf(L"Connecting to server %s:%d... ", g_server, g_port);
	status = g_webSocketClientRunnalbe.Init();
	if (!OYM_SUCCEEDED) {
		cout << "\n"
			"[WARNING] WebSocket client failed to connect to the server!\n"
			"          Captured raw data won't be shown up in your browser.\n\n";
	}
	status = webSocketClientThread.Start();

	cout << "connected successfully!\n"
		"\n"
		"Please make sure the gForce has been turned on, and then plug the gForce USB dongle \n"
		"and enter its corresponding serial COM number: ";
	scanf_s("%u", &comNum);
	OYM_AdapterManager* am = new OYM_AdapterManager();
	status = am->Init(comNum);
	if (!OYM_SUCCEEDED) {
		return -2;
	}
	am->RegistGforceData(ProcessGforceData);
	OYM_UINT8 gForce_count;
	do {
		status = am->StartScan();
		if (!OYM_SUCCEEDED) {
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
			"Please make sure the gForce has been put on properly, and then enter the name of \n"
			"the file for recording your EMG raw data: \n";
		FILE *tmp_file = NULL;
		while (1) {
			gets_s(g_filename, sizeof(g_filename) - 1);
			errno_t err = fopen_s(&tmp_file, g_filename, "ab");
			if (err != 0) {
				cout << "Bad file name, please enter a new one:\n";
				continue;
			}
			break;
		}
		printf("\nPressing any key will start to record your EMG data to file '%s'...\n\n", g_filename);
		_getch();
		g_SingleFileRecordedBytes = 0; //Reset the count for the new file
		g_PrintSingleFileRecordedBytesPreface = true;
		g_file = tmp_file;
		printf( "Recording to file '%s' started...\n\n", g_filename);
		cout << "During recording, pressing 'Z' will close the file and promt you to open another \n"
			"file to record, and pressing 'X' will also exit the program.\n\n";
		while (1) {
			int in_c = _getch();
			if (in_c == 'Z' || in_c == 'z' || in_c == 'X' || in_c == 'x') {
				// Exit writing to the current file...

				// It takes a couple of seconds to flush some 'delayed' data.
				cout << "Exiting.......\n"
					"Please wait 1 second for buffered data......\n";
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

	webSocketClientThread.Terminate(0);
	webSocketClientThread.Join(-1);

	CloseHandle(g_DataAvailable);
	return 0;
}


void ProcessGforceData(OYM_PUINT8 data, OYM_UINT16 length) {
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
	if (b_GetFirstPackage == FALSE) { // first time get into this function
		b_GetFirstPackage = TRUE;
		s_packageId = data[EMGDATA_PACKAGEID_INDEX];  // get the first package id

		cout << "[INFO] Succeeded in connecting to gForce!\n";
		SetEvent(g_DataAvailable);          //set event to notify main 
	} else {
		s_lostPackage = (data[EMGDATA_PACKAGEID_INDEX] + 256 - s_packageId - 1) % 256 + s_lostPackage;
		if (data[EMGDATA_PACKAGEID_INDEX] != ((s_packageId + 1) % 256)) {  // lost package
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
	g_webSocketClientRunnalbe.PutData((char *)&data[EMGDATA_VALIDDATA_INDEX], EMGDATA_LENGTH);

	EnterCriticalSection(&g_CriticalSection);
	if (g_file) {
		// Write EMG data to file
		UINT8 crcData = CheckSum(&data[EMGDATA_PACKAGEID_INDEX], 129);
		if (crcData == data[EMGDATA_CRC_INDEX])	{
			size_t toWrite = EMGDATA_LENGTH + EMGDATA_HEAD_LENGTH + EMGDATA_PACKAGEID_LENGTH;
			if (1 != fwrite(&data[EMGDATA_EVENT_TYPE_INDEX], toWrite, 1, g_file)) {
				cout << "[ERROR] Some data can't be written to the file!\n";
				g_PrintSingleFileRecordedBytesPreface = true;
			}
			else {
				// Append 8-byte timestamp to the end
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
UINT8 CheckSum(UINT8* data, UINT8 length) {
	UINT8 cs = 0;
	while (length--) {
		cs ^= *data++;
	}
	return cs;
}

//get systemtime and convert to ulonglong format
ULARGE_INTEGER GetFileTime(void) {
	SYSTEMTIME utcSystemTime;
	FILETIME utcFileTime;
	ULARGE_INTEGER time;
	GetSystemTime(&utcSystemTime);
	SystemTimeToFileTime(&utcSystemTime, &utcFileTime);
	time.HighPart = utcFileTime.dwHighDateTime;
	time.LowPart = utcFileTime.dwLowDateTime;
	return time;
}



