#include"stdafx.h"

#include"npi_tl.h"
#include "npi_cmd.h"
/*---------*/
/*TX THREAD
/*---------*/
NPI_TX::NPI_TX(HANDLE hdl, DWORD logThread)
{
	m_com = hdl;
	m_log = logThread;
}

NPI_TX::~NPI_TX()
{
}

void NPI_TX::Run(void)
{
	enum {
		WR_TYPE_STATE,
		WR_OPCODE_STATE,
		WR_LEN_STATE,
		WR_OTHERS
	};
	UINT8 state = WR_TYPE_STATE;
	UINT8 buf[MAX_TX_SIZE];
	INT16 i;
	INT8 totalLen;
	DWORD dwBytesWritten = 0;
	PUINT8 pBuf = NULL;
	UINT8 s_Buf[255] = { 0 };
	OVERLAPPED osWrite;
	memset(&osWrite, 0, sizeof(OVERLAPPED));
	osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	while (1) {
		switch (state) {
		case WR_TYPE_STATE:
			buf[0] = m_cmdQue.Pop();
			state = WR_OPCODE_STATE;
			break;
		case WR_OPCODE_STATE:
			buf[1] = m_cmdQue.Pop();
			buf[2] = m_cmdQue.Pop();
			state = WR_LEN_STATE;
			break;
		case WR_LEN_STATE:
			buf[3] = m_cmdQue.Pop();
			totalLen = buf[3] + CMD_HEAD_LEN;
			pBuf = new UINT8[totalLen];
			if (pBuf) {
				pBuf[0] = buf[0];
				pBuf[1] = buf[1];
				pBuf[2] = buf[2];
				pBuf[3] = buf[3];
				state = WR_OTHERS;
			} else {
				state = WR_TYPE_STATE;
			}
			break;
		case WR_OTHERS:
			for (i = 0; i < buf[3]; i++) {
				pBuf[4 + i] = m_cmdQue.Pop();
			}

			if (!WriteFile(m_com, pBuf, totalLen, &dwBytesWritten, &osWrite)) {
				if (GetLastError() == ERROR_IO_PENDING) {
					//GetOverlappedResult(m_com, &osWrite, &dwBytesWritten, true);
				}

			}
			while (!PostThreadMessage(m_log, GetLogMsg(), (WPARAM)pBuf, totalLen)) {
				LogE(L"Post LOG_MSG Err:%d\n", GetLastError());
				Sleep(500);
			}
			//delete []pBuf;
			state = WR_TYPE_STATE;
			break;
		default:
			break;
		}
	}
}

GForceQueue<UINT8, CMD_BUF_SIZE>* NPI_TX::Get_Queue(void)
{
	return &m_cmdQue;
}


/*---------*/
/*RX THREAD
/*---------*/

NPI_RX::NPI_RX(HANDLE hdl, DWORD logThread)
{
	m_com = hdl;
	m_log = logThread;
}

NPI_RX::~NPI_RX()
{
}

void NPI_RX::Run(void)
{

	OVERLAPPED osWait;
	memset(&osWait, 0, sizeof(OVERLAPPED));
	osWait.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	DWORD dwEvtMask;

	while (1) {
		if (WaitCommEvent(m_com, &dwEvtMask, &osWait)) {
			if (dwEvtMask & EV_RXCHAR) {
				if (!this->readBlock()) {
					LogE(L"Read Block£¡");
				}
			}
		} else {
			if (GetLastError() == ERROR_IO_PENDING) {
				WaitForSingleObject(osWait.hEvent, INFINITE);
				if (dwEvtMask & EV_RXCHAR) {
					if (!this->readBlock()) {
						LogE(L"Read Block£¡");
					}
				}
			}
		}
	}


	while (1) {

	}
}

DWORD NPI_RX::readBlock(void)
{
	enum {
		READ_TYPE_STATE,
		READ_EVT_CODE,
		READ_DATA_LEN,
		READ_OTHERS
	};
	PUINT8 pBuf = NULL;
	DWORD nLenOut = 0;
	UINT8 state = READ_TYPE_STATE;
	UINT8 msgLen = 0;
	UINT8 type;
	UINT8 evtCode;
	UINT8 dataLen;
	DWORD offset;
	DWORD remaining;

	OVERLAPPED osRead;
	memset(&osRead, 0, sizeof(OVERLAPPED));
	osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	while (1) {
		switch (state) {
		case READ_TYPE_STATE:
			if (ReadFile(m_com, &type, 1, &nLenOut, &osRead)) {
				if (nLenOut) {
					state = READ_EVT_CODE;
				}
			} else {
				if (GetLastError() == ERROR_IO_PENDING) {
					GetOverlappedResult(m_com, &osRead, &nLenOut, true);
					if (nLenOut) {
						state = READ_EVT_CODE;
					}
				}
			}
			break;
		case READ_EVT_CODE:
			if (ReadFile(m_com, &evtCode, 1, &nLenOut, &osRead)) {
				if (nLenOut) {
					state = READ_DATA_LEN;
				}
			} else {
				if (GetLastError() == ERROR_IO_PENDING) {
					GetOverlappedResult(m_com, &osRead, &nLenOut, true);
					if (nLenOut) {
						state = READ_DATA_LEN;
					}
				}
			}
			break;
		case READ_DATA_LEN:
			if (ReadFile(m_com, &dataLen, 1, &nLenOut, &osRead)) {
				if (nLenOut) {
					state = READ_OTHERS;
					pBuf = new UINT8[dataLen + EVT_HEADER_LEN];
					pBuf[0] = type;
					pBuf[1] = evtCode;
					pBuf[2] = dataLen;
				}
			} else {
				if (GetLastError() == ERROR_IO_PENDING) {
					GetOverlappedResult(m_com, &osRead, &nLenOut, true);
					if (nLenOut) {
						state = READ_OTHERS;
						pBuf = new UINT8[dataLen + EVT_HEADER_LEN];
						pBuf[0] = type;
						pBuf[1] = evtCode;
						pBuf[2] = dataLen;
					}
				}
			}
			break;
		case READ_OTHERS:
			remaining = dataLen;
			offset = EVT_HEADER_LEN;
			while (remaining > 0) {
				if (ReadFile(m_com, pBuf + offset, remaining, &nLenOut, &osRead)) {
					remaining -= nLenOut;
					offset += nLenOut;
				} else {
					if (GetLastError() == ERROR_IO_PENDING) {
						GetOverlappedResult(m_com, &osRead, &nLenOut, true);
						if (nLenOut) {
							remaining -= nLenOut;
							offset += nLenOut;
						}
					}
				}
			}
			while (!PostThreadMessage(m_log, GetLogMsg(), (WPARAM)pBuf,
			                          dataLen + EVT_HEADER_LEN)) {
				LogW(L"Post LOG_MSG Err:%d\n", GetLastError());
				Sleep(500);
			}
			state = READ_TYPE_STATE;
			//delete []pBuf;
			//return dataLen + EVT_HEADER_LEN;
			break;
		default:
			::LogE(L"Read File Err!\n");
			break;
		}
	}
}