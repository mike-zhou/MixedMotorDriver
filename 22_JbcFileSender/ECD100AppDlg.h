// ECD100AppDlg.h : header file
//

#if !defined(AFX_ECD100APPDLG_H__AA6DBE88_43D2_4696_88EC_E5399175E18B__INCLUDED_)
#define AFX_ECD100APPDLG_H__AA6DBE88_43D2_4696_88EC_E5399175E18B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "afxmt.h"

/////////////////////////////////////////////////////////////////////////////
// CECD100AppDlg dialog

class CECD100AppDlg : public CDialog
{
// Construction
public:
	void printString(char * pString);
	CECD100AppDlg(CWnd* pParent = NULL);	// standard constructor


// Dialog Data
	//{{AFX_DATA(CECD100AppDlg)
	enum { IDD = IDD_ECD100APP_DIALOG };
	CString	m_jamFilePath;
	CString	m_dataExchange;
	int		m_port;
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
	afx_msg void OnChangePort();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL isHexDigit(unsigned char c);
	HANDLE m_rs232Handle;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ECD100APPDLG_H__AA6DBE88_43D2_4696_88EC_E5399175E18B__INCLUDED_)
