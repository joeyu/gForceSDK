#pragma once


// CmdTab3_Dlg �Ի���

class CmdTab3_Dlg : public CDialogEx
{
	DECLARE_DYNAMIC(CmdTab3_Dlg)

public:
	CmdTab3_Dlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CmdTab3_Dlg();

// �Ի�������
	enum { IDD = IDD_TAB3_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
};
