// ECD100AppDlg.h : header file
//

#if !defined(AFX_ECD100APPDLG_H__AA6DBE88_43D2_4696_88EC_E5399175E18B__INCLUDED_)
#define AFX_ECD100APPDLG_H__AA6DBE88_43D2_4696_88EC_E5399175E18B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CECD100AppDlg dialog

class CECD100AppDlg : public CDialog
{
// Construction
public:
	CECD100AppDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CECD100AppDlg)
	enum { IDD = IDD_ECD100APP_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CECD100AppDlg)
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
	afx_msg void OnProbe();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ECD100APPDLG_H__AA6DBE88_43D2_4696_88EC_E5399175E18B__INCLUDED_)
