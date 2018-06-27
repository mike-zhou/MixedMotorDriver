// ECD100AppDlg.h : header file
//

#if !defined(AFX_ECD100APPDLG_H__AA6DBE88_43D2_4696_88EC_E5399175E18B__INCLUDED_)
#define AFX_ECD100APPDLG_H__AA6DBE88_43D2_4696_88EC_E5399175E18B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "cyapi.h"
#include "afxmt.h"

#define __PROXY_TAG_JTAG		1
#define __PROXY_TAG_REGISTER	2

#define COMMAND_MAX 100
#define COMMAND_LENGTH 5

/*
The following structure is sent to ECD100 or ECD200 to notify the jtag access:
{
	unsigned char tag;
	unsigned char length;	//the msb of this byte indicate whether the next is the higher part of the length.
							//must be 3 in this case.
	unsigned char command;	//if 0x01, only TMS and TDI of the jtag need to be clocked out; 
							//if 0x03, the TDO of jtag need to be read before TMS and TDI are clocked out.
	unsigned char tms;
	unsigned char tdi;
}
If TDO of jtag need to be read, the ECD100 or ECD200 should reply the host with following structure when requested:
{
	unsigned char tag;
	unsigned char length;	//0x01
	unsigned char tdo;
}

*/

enum JamActionEvent
{
	JAM_EVENT_PROGRAM,
	JAM_EVENT_VERIFY,
	JAM_EVENT_EXIT
};

/////////////////////////////////////////////////////////////////////////////
// CECD100AppDlg dialog

class CECD100AppDlg : public CDialog
{
// Construction
public:
	void printString(char * pString);
	CECD100AppDlg(CWnd* pParent = NULL);	// standard constructor

    	int jtagIO(int tms, int tdi, int bReadTdo);
    	int jtagDelay(int milliSecond);
	UINT jamThread(LPVOID pParam);

	int jtagInit(void);
	
	unsigned long internalRam[2*1024];//8K bytes
	unsigned long internalRamIndex;
	unsigned long externalRam[32*1024];//128K bytes
	unsigned long externalRamIndex;
	unsigned char jamBuffer[128*1024];//128K bytes.
	
	void * internalRamMalloc(unsigned long size);
	unsigned long externalRamMalloc(unsigned long size);
	unsigned char getJamByte(unsigned long index);

// Dialog Data
	//{{AFX_DATA(CECD100AppDlg)
	enum { IDD = IDD_ECD100APP_DIALOG };
	CString	m_jamFilePath;
	CString	m_dataExchange;
	long	m_port;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CECD100AppDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CECD100AppDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnFindJamFile();
	afx_msg void OnProgram();
	afx_msg void OnVerify();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HANDLE m_rs232Handle;
	CSemaphore * m_exitLock;
	CSemaphore * m_actionLock;

	enum JamActionEvent m_jamEvent;


	int executeJam(unsigned char * action);

	CCyControlEndPoint * pEp0;
	CCyBulkEndPoint * pEp1, * pEp2, *pEp4, * pEp6, *pEp8;
	CCyUSBDevice * pUsbDevice;

    unsigned char * pCommandBuffer;
    int currentCommandIndex;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ECD100APPDLG_H__AA6DBE88_43D2_4696_88EC_E5399175E18B__INCLUDED_)
