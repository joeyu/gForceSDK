#pragma once


// CTopFrame ���

class CTopFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CTopFrame)
	CSplitterWnd m_split;
protected:
	CTopFrame();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~CTopFrame();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
};


