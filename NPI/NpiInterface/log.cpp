#include"stdafx.h"
#include"com.h"
#include"log.h"
#include"npi_cmd.h"
#include"npi_evt.h"


Log::Log(DWORD com_id, DWORD evt_id)
{
	comThreadID = com_id;
	evtThreadID = evt_id;
#ifdef MFC_GUI
	cf.cbSize = sizeof(CHARFORMAT2);
#endif
}

Log::~Log()
{
}

void Log::Run()
{
	MSG     msg;
	PUINT8  buf;
	UINT8   size;
	while (1) {
		if (GetMessage(&msg, 0, 0, 0)) { //get msg from message queue
			if ((msg.message >= LOG_MSG) && (msg.message <= LOG_MSG + 0xFF)) {
				buf = (PUINT8)msg.wParam;
				size = (UINT8)msg.lParam;
				switch (buf[0]) {
				case HCI_CMD_PACKET:
					Analyze_TX(buf, size);
					break;
				case HCI_EVENT_PACKET:
					Analyze_RX(buf, size);
					break;
				default:
					Log_printf(L"LOG Err\n");
					break;
				}
			} else {
				Log_printf(L"Log:Err msg type=%4X !\n", msg.message);
			}
		}
	}
}

void Log::Analyze_TX(PUINT8 buf, UINT8 size)
{
	sCMD* pCmd = (sCMD*)buf;
	SYSTEMTIME sys;
	UINT8 i = 0;

#ifdef MFC_GUI
	theApp.m_logView->m_richEdit.GetSelectionCharFormat(cf);
	cf.dwMask = CFM_COLOR | CFM_SIZE;// | CFM_BOLD | CFM_BACKCOLOR;
	cf.dwEffects &= ~CFE_AUTOCOLOR;
	cf.crTextColor = RGB(0, 255, 0);//green
	cf.yHeight = 200;
#endif
	GetLocalTime(&sys);
	Log_printf(L"<Tx> -");
	Log_printf(L"%02d:%02d:%02d.%03d\n", sys.wHour, sys.wMinute, sys.wSecond,
	           sys.wMilliseconds);
	Log_printf(L"-Type			: 0x%02X\n", pCmd->type);
	Log_printf(L"-opcode		: 0x%04X\n", pCmd->opcode);
	Log_printf(L"Data Lenth		: 0x%02X\n", pCmd->len);

	Sprintf_DumpTx(buf, size);
	delete buf;
}

void Log::Analyze_RX(PUINT8 buf, UINT8 size)
{
	SYSTEMTIME sys;
	UINT8 i = 0;
	UINT16 event;
	sNpiEvt* pEvt = (sNpiEvt*)buf;

#ifdef MFC_GUI
	theApp.m_logView->m_richEdit.GetSelectionCharFormat(cf);
	cf.dwMask = CFM_COLOR | CFM_SIZE;// | CFM_BOLD | CFM_BACKCOLOR;
	cf.dwEffects &= ~CFE_AUTOCOLOR;
	cf.crTextColor = RGB(0, 0, 255);//Blue
	cf.yHeight = 200;
#endif

	GetLocalTime(&sys);
	Log_printf(L"<Rx> -");
	Log_printf(L"%02d:%02d:%02d.%03d\n", sys.wHour, sys.wMinute, sys.wSecond,
	           sys.wMilliseconds);
	Log_printf(L"-Type			: 0x%02X\n", pEvt->type);
	Log_printf(L"-EventCode		: 0x%02X\n", pEvt->event_code);
	Log_printf(L"-Data Length		: 0x%02X\n", pEvt->len);

	if (pEvt->event_code == HCI_LE_EVENT_CODE) {
		Analyze_LE_Hci_Event(buf, size);
	} else {
		event = pEvt->event_lo + (pEvt->event_hi << 8);
		Log_printf(L"Event			: 0x%04X\n", event);
		Log_printf(L"Status			: 0x%02X\n", pEvt->status);

		switch (event & 0xFF80) {
		case HCI_EXT_LL_EVENT:
			Analyze_LL_Event(buf, size);
			break;
		case HCI_EXT_L2CAP_EVENT:
			Analyze_L2CAP_Event(buf, size);
			break;
		case HCI_EXT_ATT_EVENT:
			Analyze_ATT_Event(buf, size);
			break;
		case HCI_EXT_GATT_EVENT:
			Analyze_GATT_Event(buf, size);
			break;
		case HCI_EXT_GAP_EVENT:
			Analyze_GAP_Event(buf, size);
			break;
		default:
			break;
		}
	}
}

void Log::Sprintf_HexData(PUINT8 pBuf, INT16 len)
{
	INT16 i;
	for (i = 0; i < len - 1; i++) {
		Log_printf(L"%02X:", *(pBuf + i));
	}
	Log_printf(L"%02X\n", *(pBuf + i));
}

void Log::Sprintf_DumpRx(PUINT8 pBuf, INT16 len)
{
	INT16 i;
#ifdef MFC_GUI
	theApp.m_logView->m_richEdit.GetSelectionCharFormat(cf);
	cf.dwMask = CFM_COLOR | CFM_SIZE;// | CFM_BOLD | CFM_BACKCOLOR;
	cf.dwEffects &= ~CFE_AUTOCOLOR;
	cf.crTextColor = RGB(255, 0, 0);//red
	cf.yHeight = 200;
#endif
	Log_printf(L"dump(Rx):\n");
	for (i = 0; i < len; i++) {
		Log_printf(L"%02X ", *(pBuf + i));
	}
	Log_printf(L"\n");
	Log_printf(L"----------------------------------------\n");
}
void Log::Sprintf_DumpTx(PUINT8 pBuf, INT16 len)
{
	INT16 i;

#ifdef MFC_GUI
	theApp.m_logView->m_richEdit.GetSelectionCharFormat(cf);
	cf.dwMask = CFM_COLOR | CFM_SIZE;// | CFM_BOLD | CFM_BACKCOLOR;
	cf.dwEffects &= ~CFE_AUTOCOLOR;
	cf.crTextColor = RGB(255, 0, 0);//red
	cf.yHeight = 200;
#endif

	Log_printf(L"dump(Tx):\n");
	for (i = 0; i < len; i++) {
		Log_printf(L"%02X ", *(pBuf + i));
	}
	Log_printf(L"\n");
	Log_printf(L"----------------------------------------\n");
}

void Log::Analyze_LL_Event(PUINT8 buf, UINT8 size)
{
	BOOL del_flag = TRUE;
	sNpiEvt* pEvt = (sNpiEvt*)buf;
	UINT16 event = pEvt->event_lo + (pEvt->event_hi << 8);
	UINT16 opCode = pEvt->data[0] + (pEvt->data[1] << 8);

	switch (event) {
	case HCI_EXT_SET_RX_GAIN_EVENT:
	case HCI_EXT_SET_TX_POWER_EVENT:
	case HCI_EXT_CLK_DIVIDE_ON_HALT_EVENT:
	case HCI_EXT_DECLARE_NV_USAGE_EVENT:
	case HCI_EXT_SET_LOCAL_SUPPORTED_FEATURES_EVENT:
	case HCI_EXT_SET_FAST_TX_RESP_TIME_EVENT:
	case HCI_EXT_MODEM_TEST_TX_EVENT:
	case HCI_EXT_MODEM_HOP_TEST_TX_EVENT:
	case HCI_EXT_MODEM_TEST_RX_EVENT:
	case HCI_EXT_END_MODEM_TEST_EVENT:
	case HCI_EXT_SET_BDADDR_EVENT:
	case HCI_EXT_SET_SCA_EVENT:
	case HCI_EXT_SET_FREQ_TUNE_EVENT:
	case HCI_EXT_SAVE_FREQ_TUNE_EVENT:
	case HCI_EXT_SET_MAX_DTM_TX_POWER_EVENT:
	case HCI_EXT_MAP_PM_IO_PORT_EVENT:
	case HCI_EXT_DISCONNECT_IMMED_EVENT:
	case HCI_EXT_PER_EVENT:
	case HCI_EXT_EXTEND_RF_RANGE_EVENT:
	case HCI_EXT_HALT_DURING_RF_EVENT:
	case HCI_EXT_OVERRIDE_SL_EVENT:
	case HCI_EXT_BUILD_REVISION_EVENT:
	case HCI_EXT_DELAY_SLEEP_EVENT:
	case HCI_EXT_RESET_SYSTEM_EVENT:
	case HCI_EXT_OVERLAPPED_PROCESSING_EVENT:
	case HCI_EXT_NUM_COMPLETED_PKTS_LIMIT_EVENT:
	case HCI_EXT_GET_CONNECTION_INFO_EVENT:
		Log_printf(L"OpCode			: 0x%04X\n", opCode);
		Sprintf_DumpRx(buf, size);
		/*send status message to com-thread*/
		while (!PostThreadMessage(evtThreadID, HCI_STATUS_MSG, pEvt->status, 0)) {
			LogW(L"Post RX_STATUS_MSG Err:%d\n", GetLastError());
			Sleep(500);
		}
		/*delete heap buffer ?*/
		delete buf;
		break;

	case HCI_EXT_DECRYPT_EVENT:
		Log_printf(L"Decrypted Data		: ");
		Sprintf_HexData((PUINT8)pEvt->data, pEvt->len - 3);
		Sprintf_DumpRx(buf, size);
		/*send event message to evt-thread and delete heap in that thread*/
		while (!PostThreadMessage(evtThreadID, HCI_DECRYPT_MSG, (WPARAM)&pEvt->status,
		                          pEvt->len - 2)) {
			LogW(L"Post HCI_DECRYPT_MSG Err:%d\n", GetLastError());
			Sleep(500);
		}
		break;
	default:
		break;

	}
}

void Log::Analyze_L2CAP_Event(PUINT8 buf, UINT8 size)
{
	BOOL del_flag = TRUE;
	sNpiEvt* pEvt = (sNpiEvt*)buf;
	UINT16 event = BUILD_UINT16(pEvt->event_lo, pEvt->event_hi);
	UINT16 msg_type = 0;
	sL2capCmdRejEvt* pcmd_rej;
	switch (event) {
	case L2CAP_COMMAND_REJECT_EVT:
		pcmd_rej = (sL2capCmdRejEvt*)&pEvt->status;
		Log_printf(L"Handle			: 0x%04X\n", BUILD_UINT16(pcmd_rej->handle_lo,
		                pcmd_rej->handle_hi));
		Log_printf(L"RejectReason		: 0x%04X\n", BUILD_UINT16(pcmd_rej->reason_lo,
		                pcmd_rej->reason_hi));
		msg_type = L2CAP_COMMAND_REJECT_MSG;
		break;
	case L2CAP_DISCONNECT_RSP_EVT:
		msg_type = L2CAP_DISCONNECT_RSP_MSG;
		break;
	case L2CAP_INFO_RSP_EVT:
		msg_type = L2CAP_INFO_RSP_MSG;
		break;
	case L2CAP_PARAM_UPDATE_RSP_EVT:
		msg_type = L2CAP_PARAM_UPDATE_RSP_MSG;
		break;
	case L2CAP_CONNECT_RSP:
		msg_type = L2CAP_CONNECT_RSP_MSG;
		break;
	case L2CAP_CHANNEL_ESTABLISHED_EVT:
		msg_type = L2CAP_CHANNEL_ESTABLISHED_MSG;
		break;
	case L2CAP_CHANNEL_TERMINATED_EVT:
		msg_type = L2CAP_CHANNEL_TERMINATED_MSG;
		break;
	case L2CAP_OUT_OF_CREDIT_EVT:
		msg_type = L2CAP_OUT_OF_CREDIT_MSG;
		break;
	case L2CAP_PEER_CREDIT_THRESHOLD_EVT:
		msg_type = L2CAP_PEER_CREDIT_THRESHOLD_MSG;
		break;
	case L2CAP_SEND_SDU_DONE_EVT:
		msg_type = L2CAP_SEND_SDU_DONE_MSG;
		break;
	case L2CAP_DATA_EVT:
		msg_type = L2CAP_DATA_MSG;
		break;
	default:
		break;
	}
	Sprintf_DumpRx(buf, size);
	if (msg_type != 0) {
		while (!PostThreadMessage(evtThreadID, msg_type, (WPARAM)&pEvt->status,
		                          pEvt->len - 2)) {
			LogW(L"Post L2CAP_MSG Err:%d, Type:0x%x\n", GetLastError(), msg_type);
			Sleep(500);
		}
	}

}

void Log::Analyze_ATT_Event(PUINT8 buf, UINT8 size)
{
	BOOL del_flag = TRUE;
	sNpiEvt* pEvt = (sNpiEvt*)buf;
	UINT16 event = BUILD_UINT16(pEvt->event_lo, pEvt->event_hi);
	UINT16 i, msg_type = 0;
	UINT16 pduLen;
	UINT8 len, cnt;
	u_AttMsg* pAttMsg = (u_AttMsg*)&pEvt->status;
	switch (event) {
	case ATT_ERROR_EVENT:
		Log_printf(L"ReqOpCode		: 0x%02X\n", pAttMsg->errorRsp.reqOpcode);
		Log_printf(L"Handle			: 0x%04X\n", pAttMsg->errorRsp.handle);
		Log_printf(L"ErrorCode		: 0x%02X\n", pAttMsg->errorRsp.errCode);
		msg_type = ATT_ERROR_MSG;
		break;
	case ATT_EXCHANGE_MTU_EVENT:
		Log_printf(L"connHandle		: 0x%04X\n", pAttMsg->exchangeMTURsp.conHandle);
		Log_printf(L"PduLen			: 0x%02X\n", pAttMsg->exchangeMTURsp.pduLen);
		if (pAttMsg->exchangeMTURsp.pduLen > 0) {
			Log_printf(L"ServerRxMtu		: 0x%04X\n", pAttMsg->exchangeMTURsp.serverRxMTU);
		}
		msg_type = ATT_EXCHANGE_MTU_MSG;
		break;
	case ATT_FIND_INFO_EVENT:
		Log_printf(L"connHandle		: 0x%04X\n", pAttMsg->findInfoRsp.conHandle);
		Log_printf(L"PduLen			: 0x%02X\n", pAttMsg->findInfoRsp.pduLen);
		if (pAttMsg->findInfoRsp.pduLen > 0) {
			Log_printf(L"Format			: 0x%02X\n", pAttMsg->findInfoRsp.format);
			if (pAttMsg->findInfoRsp.format == 1) { //2-oct uuid
				for (i = 0; i < (pAttMsg->findInfoRsp.pduLen - 1) / sizeof(attHandleBtUUID_t);
				     i++) {
					Log_printf(L"Handle			: 0x%02X\n", pAttMsg->findInfoRsp.Info.uuid2[i].handle);
					Log_printf(L"UUID			: ");
					Sprintf_HexData(pAttMsg->findInfoRsp.Info.uuid2[i].uuid, ATT_BT_UUID_SIZE);
				}
			} else { //16-oct uuid
				for (i = 0; i < (pAttMsg->findInfoRsp.pduLen - 1) / sizeof(attHandleUUID_t);
				     i++) {
					Log_printf(L"Handle			: 0x%02X\n", pAttMsg->findInfoRsp.Info.uuid16[i].handle);
					Log_printf(L"UUID			: ");
					Sprintf_HexData(pAttMsg->findInfoRsp.Info.uuid16[i].uuid, ATT_UUID_SIZE);
				}
			}
		}
		msg_type = ATT_FIND_INFO_MSG;
		break;
	case ATT_FIND_BY_TYPE_VALUE_EVENT:
		Log_printf(L"connHandle		: 0x%04X\n", pAttMsg->findByTypeValueRsp.conHandle);
		pduLen = pAttMsg->findByTypeValueRsp.pduLen;
		Log_printf(L"PduLen			: 0x%02X\n", pduLen);
		if (pduLen > 0) {
			for (i = 0; i < (pduLen / 2); i++) {
				Log_printf(L"Handle			: 0x%02X\n", pAttMsg->findByTypeValueRsp.handle[i]);
			}
		}
		msg_type = ATT_FIND_BY_TYPE_VALUE_MSG;
		break;
	case ATT_READ_BY_TYPE_EVENT:
		pduLen = pAttMsg->readByTypeRsp.pduLen;
		Log_printf(L"connHandle		: 0x%04X\n", pAttMsg->readByTypeRsp.conHdl);
		Log_printf(L"PduLen			: 0x%02X\n", pduLen);
		if (pduLen > 0) {
			len = pAttMsg->readByTypeRsp.len;
			Log_printf(L"Length			: 0x%02X\n", len);
			cnt = (pduLen - 1) / len;//loop times
			for (i = 0; i < cnt; i++) {
				Log_printf(L"Handle			: 0x%04X\n", pAttMsg->readByTypeRsp.data[i * len]);
				Log_printf(L"Data			: ");
				Sprintf_HexData(&pAttMsg->readByTypeRsp.data[i * len + 2], len - 2);
			}
		}
		msg_type = ATT_READ_BY_TYPE_MSG;
		break;
	case ATT_READ_EVENT:
		pduLen = pAttMsg->readRsp.pduLen;
		Log_printf(L"connHandle		: 0x%04X\n", pAttMsg->readRsp.conHdl);
		Log_printf(L"PduLen			: 0x%02X\n", pduLen);
		if (pduLen > 0) {
			Log_printf(L"Value			: ");
			Sprintf_HexData(&pAttMsg->readRsp.value[0], pduLen);
		}
		msg_type = ATT_READ_MSG;
		break;
	case ATT_READ_BLOB_EVENT:
		msg_type = ATT_READ_BLOB_MSG;
		break;
	case ATT_READ_MULTI_EVENT:
		pduLen = pAttMsg->readMultiRsp.pduLen;
		Log_printf(L"connHandle		: 0x%04X\n", pAttMsg->readMultiRsp.conHdl);
		Log_printf(L"PduLen			: 0x%02X\n", pduLen);
		if (pduLen > 0) {
			Log_printf(L"Values			: ");
			Sprintf_HexData(&pAttMsg->readMultiRsp.value[0], pduLen);
		}
		msg_type = ATT_READ_MULTI_MSG;
		break;
	case ATT_READ_BY_GRP_TYPE_EVENT:
		pduLen = pAttMsg->readByGrpTypeRsp.pduLen;
		Log_printf(L"connHandle		: 0x%04X\n", pAttMsg->readByGrpTypeRsp.conHdl);
		Log_printf(L"PduLen			: 0x%02X\n", pduLen);
		if (pduLen > 0) {
			len = pAttMsg->readByGrpTypeRsp.len;
			Log_printf(L"Length			: 0x%02X\n", len);
			cnt = (pduLen - 1) / len;//loop times
			for (i = 0; i < cnt; i++) {
				Log_printf(L"AttrHandle		: 0x%04X\n", pAttMsg->readByGrpTypeRsp.data[i * len]);
				Log_printf(L"EndGrpHandle		: 0x%04X\n",
				           pAttMsg->readByGrpTypeRsp.data[i * len + 2]);
				Log_printf(L"Value			: ");
				Sprintf_HexData(&pAttMsg->readByGrpTypeRsp.data[i * len + 4], len - 4);
			}
		}
		msg_type = ATT_READ_BY_GRP_TYPE_MSG;
		break;
	case ATT_WRITE_EVENT:
		msg_type = ATT_WRITE_MSG;
		break;
	case ATT_PREPARE_WRITE_EVENT:
		msg_type = ATT_PREPARE_WRITE_MSG;
		break;
	case ATT_EXECUTE_WRITE_EVENT:
		msg_type = ATT_EXECUTE_WRITE_MSG;
		break;
	case ATT_HANDLE_VALUE_NOTI:                             // notify message
		msg_type = ATT_HANDLE_VALUE_NOTI_MSG;               
		break;
	case ATT_HANDLE_VALUE_IND:
		msg_type = ATT_HANDLE_VALUE_IND_MSG;
		break;
	case ATT_HANDLE_VALUE_CFM:
		msg_type = ATT_HANDLE_VALUE_CFM_MSG;
		break;
	case ATT_FLOW_CTRL_VIOLATED_EVENT:
		msg_type = ATT_FLOW_CTRL_VIOLATED_MSG;
		break;
	case ATT_MTU_UPDATED_EVENT:
		msg_type = ATT_MTU_UPDATED_MSG;
		break;
	default:
		break;
	}
	Sprintf_DumpRx(buf, size);
	if (msg_type != 0) {
		while (!PostThreadMessage(evtThreadID, msg_type, (WPARAM)&pEvt->status,
		                          pEvt->len - 2)) {
			LogW(L"Post ATT_MSG Err:%d, Ttpe:0x%x\n", GetLastError(), msg_type);
			Sleep(500);
		}
	}
}

void Log::Analyze_GATT_Event(PUINT8 buf, UINT8 size)
{
	/*nothing*/
}

void Log::Analyze_GAP_Event(PUINT8 buf, UINT8 size)
{
	BOOL del_flag = FALSE;
	sNpiEvt* pEvt = (sNpiEvt*)buf;
	UINT16 event = BUILD_UINT16(pEvt->event_lo, pEvt->event_hi);
	UINT16 opCode = BUILD_UINT16(pEvt->data[0], pEvt->data[1]);
	UINT16 msg_type = 0;
	UINT8 i;

	u_GapMsg* gapMsg = (u_GapMsg*)&pEvt->status;
	switch (event) {
	case HCI_EXT_GAP_CMD_STATUS_EVENT:
		Log_printf(L"OpCode		: 0x%04X\n", opCode);
		Log_printf(L"Data Length		: 0x%02X\n", pEvt->data[2]);
		/*send event message to evt-thread*/
		msg_type = HCI_GAP_STATUS_MSG;
		break;
	case HCI_EXT_GAP_DEVICE_INIT_DONE_EVENT:
		Log_printf(L"DevAddr		: ");
		Sprintf_HexData(gapMsg->InitDone_Evt.devAddr, BLE_ADDR_LEN);
		Log_printf(L"DataPktLen		: 0x%04X\n", gapMsg->InitDone_Evt.dataPktLen);
		Log_printf(L"NumDataPkts		: 0x%02X\n", gapMsg->InitDone_Evt.numDataPkts);
		Log_printf(L"IRK			: ");
		Sprintf_HexData(gapMsg->InitDone_Evt.irk, IRK_LEN);
		Log_printf(L"CSRK			: ");
		Sprintf_HexData(gapMsg->InitDone_Evt.csrk, CSRK_LEN);
		msg_type = HCI_EXT_GAP_DEVICE_INIT_DONE_MSG;
		break;
	case HCI_EXT_GAP_DEVICE_DISCOVERY_EVENT:
		Log_printf(L"NumDevs		: 0x%02X\n", gapMsg->DiscDone_Evt.numDevs);
		for (i = 0; i < gapMsg->DiscDone_Evt.numDevs; i++) {
			Log_printf(L"Device #%d\n", i);
			Log_printf(L"EventType		: 0x%02X\n", gapMsg->DiscDone_Evt.DevList[i].eventType);
			Log_printf(L"AddrType		: 0x%02X\n", gapMsg->DiscDone_Evt.DevList[i].addrType);
			Log_printf(L"Addr			: ");
			Sprintf_HexData(gapMsg->DiscDone_Evt.DevList[i].addr, BLE_ADDR_LEN);
		}
		msg_type = HCI_EXT_GAP_DEVICE_DISCOVERY_MSG;
		break;
	case HCI_EXT_GAP_ADV_DATA_UPDATE_DONE_EVENT:
		Log_printf(L"AdType			: 0x%02X\n", pEvt->data[0]);
		msg_type = HCI_EXT_GAP_ADV_DATA_UPDATE_DONE_MSG;
		break;
	case HCI_EXT_GAP_MAKE_DISCOVERABLE_DONE_EVENT:
	case HCI_EXT_GAP_END_DISCOVERABLE_DONE_EVENT:
		del_flag = TRUE;
		break;
	case HCI_EXT_GAP_LINK_ESTABLISHED_EVENT:
		Log_printf(L"DevAddrType		: 0x%02X\n", gapMsg->EstLink_Evt.devAddrType);
		Log_printf(L"DevAddr			: ");
		Sprintf_HexData(gapMsg->EstLink_Evt.devAddr, BLE_ADDR_LEN);
		Log_printf(L"ConnHandle		: 0x%04X\n", gapMsg->EstLink_Evt.connectionHandle);
		Log_printf(L"ConnRole		: 0x%02X\n", gapMsg->EstLink_Evt.connRole);
		Log_printf(L"ConnInterval		: 0x%04X\n", gapMsg->EstLink_Evt.connInterval);
		Log_printf(L"ConnLatency		: 0x%04X\n", gapMsg->EstLink_Evt.connLatency);
		Log_printf(L"ConnTimeout		: 0x%04X\n", gapMsg->EstLink_Evt.connTimeout);
		msg_type = HCI_EXT_GAP_LINK_ESTABLISHED_MSG;
		break;
	case HCI_EXT_GAP_LINK_TERMINATED_EVENT:
		Log_printf(L"ConnHandle		: 0x%04X\n", gapMsg->TerLink_Evt.connectionHandle);
		Log_printf(L"Reason			: 0x%02X\n", gapMsg->TerLink_Evt.reason);
		msg_type = HCI_EXT_GAP_LINK_TERMINATED_MSG;
		break;
	case HCI_EXT_GAP_LINK_PARAM_UPDATE_EVENT:
		Log_printf(L"ConnHandle		: 0x%04X\n", BUILD_UINT16(pEvt->data[0],
		                pEvt->data[1]));
		Log_printf(L"ConnInterval		: 0x%04X\n", BUILD_UINT16(pEvt->data[2],
		                pEvt->data[3]));
		Log_printf(L"ConnLatency		: 0x%04X\n", BUILD_UINT16(pEvt->data[4],
		                pEvt->data[5]));
		Log_printf(L"ConnTimeout		: 0x%04X\n", BUILD_UINT16(pEvt->data[6],
		                pEvt->data[7]));
		msg_type = HCI_EXT_GAP_LINK_PARAM_UPDATE_MSG;
		break;
	case HCI_EXT_GAP_RANDOM_ADDR_CHANGED_EVENT:
		Log_printf(L"AddrType		: 0x%02X\n", gapMsg->RanAddr_Evt.addrType);
		Log_printf(L"RandAddr		: ");
		Sprintf_HexData(gapMsg->RanAddr_Evt.newRandomAddr, BLE_ADDR_LEN);
		msg_type = HCI_EXT_GAP_RANDOM_ADDR_CHANGED_MSG;
		break;
	case HCI_EXT_GAP_SIGNATURE_UPDATED_EVENT:
		Log_printf(L"AddrType		: 0x%02X\n", gapMsg->SigUpdate_Evt.addrType);
		Log_printf(L"DevAddr		: ");
		Sprintf_HexData(gapMsg->SigUpdate_Evt.devAddr, BLE_ADDR_LEN);
		Log_printf(L"SignCounter		: 0x%08X\n", gapMsg->SigUpdate_Evt.signCounter);
		msg_type = HCI_EXT_GAP_SIGNATURE_UPDATED_MSG;
		break;
	case HCI_EXT_GAP_AUTH_COMPLETE_EVENT:
		Log_printf(L"ConnHandle		: 0x%04X\n", gapMsg->AuthComp_Evt.connectionHandle);
		Log_printf(L"AuthState		: 0x%02X\n", gapMsg->AuthComp_Evt.authState);
		msg_type = HCI_EXT_GAP_AUTH_COMPLETE_MSG;
		break;
	case HCI_EXT_GAP_PASSKEY_NEEDED_EVENT:
		Log_printf(L"DevAddr		: ");
		Sprintf_HexData(gapMsg->PassKeyNeed_Evt.deviceAddr, BLE_ADDR_LEN);
		Log_printf(L"ConnHandle		: 0x%04X\n", gapMsg->PassKeyNeed_Evt.connectionHandle);
		Log_printf(L"UIInput		: 0x%02X\n", gapMsg->PassKeyNeed_Evt.uiInputs);
		Log_printf(L"UIOutput		: 0x%02X\n", gapMsg->PassKeyNeed_Evt.uiOutputs);
		msg_type = HCI_EXT_GAP_PASSKEY_NEEDED_MSG;
		break;
	case HCI_EXT_GAP_SLAVE_REQUESTED_SECURITY_EVENT:
		Log_printf(L"ConnHandle		: 0x%04X\n", gapMsg->SlaveSec_Evt.connectionHandle);
		Log_printf(L"DevAddr		: ");
		Sprintf_HexData(gapMsg->SlaveSec_Evt.deviceAddr, BLE_ADDR_LEN);
		Log_printf(L"AuthReq		: 0x%02X\n", gapMsg->SlaveSec_Evt.authReq);
		msg_type = HCI_EXT_GAP_SLAVE_REQUESTED_SECURITY_MSG;
		break;
	case HCI_EXT_GAP_DEVICE_INFO_EVENT:
		Log_printf(L"EventType		: 0x%02X\n", gapMsg->DevInfo_Evt.eventType);
		Log_printf(L"AddrType		: 0x%02X\n", gapMsg->DevInfo_Evt.addrType);
		Log_printf(L"DevAddr		: ");
		Sprintf_HexData(gapMsg->DevInfo_Evt.addr, BLE_ADDR_LEN);
		Log_printf(L"RSSI			: 0x%02X\n", gapMsg->DevInfo_Evt.rssi);
		Log_printf(L"DataLen			: 0x%02X\n", gapMsg->DevInfo_Evt.dataLen);
		msg_type = HCI_EXT_GAP_DEVICE_INFO_MSG;
		break;
	case HCI_EXT_GAP_BOND_COMPLETE_EVENT:
		Log_printf(L"ConnHandle		: 0x%04X\n", BUILD_UINT16(pEvt->data[0],
		                pEvt->data[1]));
		msg_type = HCI_EXT_GAP_BOND_COMPLETE_MSG;
		break;
	case HCI_EXT_GAP_PAIRING_REQ_EVENT:
		Log_printf(L"ConnHandle		: 0x%04X\n", gapMsg->PairReq_Evt.connectionHandle);
		Log_printf(L"IOCap		: 0x%02X\n", gapMsg->PairReq_Evt.pairReq.ioCap);
		Log_printf(L"OOBFlag		: 0x%02X\n", gapMsg->PairReq_Evt.pairReq.oobDataFlag);
		Log_printf(L"AuthReq		: 0x%02X\n", gapMsg->PairReq_Evt.pairReq.authReq);
		Log_printf(L"MaxEncKeySize	: 0x%02X\n",
		           gapMsg->PairReq_Evt.pairReq.maxEncKeySize);
		Log_printf(L"KeyDist		: 0x%02X\n", gapMsg->PairReq_Evt.pairReq.keyDist);
		msg_type = HCI_EXT_GAP_PAIRING_REQ_MSG;
		break;
	default:
		break;
	}
	Sprintf_DumpRx(buf, size);
	if (msg_type != 0) {
		/*send event message to evt-thread*/
		while (!PostThreadMessage(evtThreadID, msg_type, (WPARAM)&pEvt->status,
		                          pEvt->len - 2)) {
			LogW(L"Post GAP EVENT Err:%d, Type:0x%x\n", GetLastError(), msg_type);
			Sleep(500);
		}
	}
	if (del_flag) {
		delete buf;
	}
}

void Log::Analyze_LE_Hci_Event(PUINT8 buf, UINT8 size)
{
	BOOL del_flag = FALSE;
	sHciEvt* pEvt = (sHciEvt*)buf;
	UINT16 opCode = BUILD_UINT16(pEvt->op_lo, pEvt->op_hi);
	UINT16 msg_type = 0;
	Log_printf(L"Packets			: 0x%02X\n", pEvt->packets);
	Log_printf(L"OpCode			: 0x%04X\n", opCode);
	Log_printf(L"Status			: 0x%02X\n", pEvt->status);
	switch (opCode) {
	case HCI_LE_CLEAR_WHITE_LIST:
	case HCI_LE_ADD_WHITE_LIST:
	case HCI_LE_REMOVE_WHITE_LIST:
	case HCI_LE_RECEIVER_TEST:
	case HCI_LE_TRANSMITTER_TEST:
		/*delete heap buffer ?*/
		del_flag = TRUE;
		break;
	case HCI_READ_LOCAL_SUPPORTED_FEATURES:
		Log_printf(L"Feature Set		: ");
		Sprintf_HexData(pEvt->data, FEATURES_LEN);
		msg_type = HCI_READ_LOCAL_SUPPORT_FEAT_MSG;
		break;
	case HCI_READ_BDADDR:
		Log_printf(L"BD Addr			: ");
		Sprintf_HexData(pEvt->data, BLE_ADDR_LEN);
		msg_type = HCI_READ_BDADDR_MSG;
		break;
	case HCI_READ_RSSI:
		Log_printf(L"Handle			: 0x%04X\n", BUILD_UINT16(pEvt->data[0], pEvt->data[1]));
		Log_printf(L"RSSI			: 0x%02X\n", pEvt->data[2]);
		msg_type = HCI_READ_RSSI_MSG;
		break;
	case HCI_LE_SET_EVENT_MASK:
		msg_type = HCI_LE_SET_EVENT_MASK_MSG;
		break;
	case HCI_LE_READ_LOCAL_SUPPORTED_FEATURES:
		Log_printf(L"Feature Set		: ");
		Sprintf_HexData(pEvt->data, FEATURES_LEN);
		msg_type = HCI_LE_READ_LOCAL_SUPPORT_FEAT_MSG;
		break;
	case HCI_LE_READ_WHITE_LIST_SIZE:
		Log_printf(L"White List Size		: 0x%02X\n", pEvt->data[0]);
		msg_type = HCI_LE_READ_WHITE_LIST_SIZE_MSG;
		break;
	case HCI_LE_CONNECTION_UPDATE:
		msg_type = HCI_LE_CONNECTION_UPDATE_MSG;
		break;
	case HCI_LE_TEST_END:
		Log_printf(L"Num Of Packets		: 0x%04X\n", BUILD_UINT16(pEvt->data[0],
		                pEvt->data[1]));
		msg_type = HCI_LE_TEST_END_MSG;
		break;
	case HCI_LE_REMOTE_CONN_PARAM_REQ_REPLY:
		msg_type = HCI_LE_REM_CONN_PARAM_REQ_REP_MSG;
		break;
	case HCI_LE_REMOTE_CONN_PARAM_REQ_NEG_REPLY:
		msg_type = HCI_LE_REM_CONN_PARAM_REQ_NEG_REP_MSG;
		break;
	default:
		break;
	}

	Sprintf_DumpRx(buf, size);
	/*Message to user*/
	if (msg_type != 0) {
		/*send event message to evt-thread*/
		while (!PostThreadMessage(evtThreadID, msg_type, (WPARAM)&pEvt->status,
		                          pEvt->len - 3)) {
			LogW(L"Post HCI LE EVENT Err:%d, Type:0x%x\n", GetLastError(), msg_type);
			Sleep(500);
		}
	}
	if (del_flag)  {
		delete buf;
	}
}

size_t Log::Log_printf(const wchar_t* fmt, ...)
{
	va_list argp;
	const size_t BUF_SIZE = 512;
	WCHAR buf[BUF_SIZE];
	size_t len = 0;

	va_start(argp, fmt);

	len += vswprintf_s(buf + len, BUF_SIZE - len, fmt, argp);
#ifdef STD_OUT
	fwprintf_s(stdout, buf);
#endif
#ifdef MFC_GUI
	theApp.m_logView->m_richEdit.SetSel(-1, -1);
	theApp.m_logView->m_richEdit.SetSelectionCharFormat(cf);
	theApp.m_logView->m_richEdit.ReplaceSel((LPCTSTR)buf);
#endif
	va_end(argp);

	return len;
}

size_t LogE(const wchar_t* fmt, ...)
{
	va_list argp;
	const size_t BUF_SIZE = 512;
	WCHAR buf[BUF_SIZE];
	size_t len;

	va_start(argp, fmt);

	len = swprintf_s(buf, BUF_SIZE, L"ERROR[%d]: ", GetLastError());
	len += vswprintf_s(buf + len, BUF_SIZE - len, fmt, argp);
#ifdef STD_OUT
	fwprintf_s(stderr, buf);
#endif
#ifdef MFC_GUI
	AfxMessageBox(buf);
#endif

	va_end(argp);

	return len;
}

size_t LogW(const wchar_t* fmt, ...)
{
	va_list argp;
	const size_t BUF_SIZE = 512;
	WCHAR buf[BUF_SIZE];
	size_t len;

	va_start(argp, fmt);

	len = swprintf_s(buf, BUF_SIZE, L"WARNING: ");
	len += vswprintf_s(buf + len, BUF_SIZE - len, fmt, argp);
#ifdef STD_OUT
	fwprintf_s(stdout, buf);
#endif
#ifdef MFC_GUI
	AfxMessageBox(buf);
#endif

	va_end(argp);

	return len;
}

size_t LogI(const wchar_t* fmt, ...)
{
	va_list argp;
	const size_t BUF_SIZE = 512;
	WCHAR buf[BUF_SIZE];
	size_t len;

	va_start(argp, fmt);

	len = swprintf_s(buf, BUF_SIZE, L"INFO: ");
	len += vswprintf_s(buf + len, BUF_SIZE - len, fmt, argp);
#ifdef STD_OUT
	fwprintf_s(stdout, buf);
#endif
#ifdef MFC_GUI
	AfxMessageBox(buf);
#endif

	va_end(argp);

	return len;
}

UINT16 GetLogMsg(void)
{
	static UINT16 msg = LOG_MSG;
	if (msg++ == (LOG_MSG + 0xFF)) {
		msg = LOG_MSG;
	}
	return msg;
}
