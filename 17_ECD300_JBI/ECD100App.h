// ECD100App.h : main header file for the ECD100APP application
//

#if !defined(AFX_ECD100APP_H__893D4B04_B63D_4161_BE0D_DFA2A940ED34__INCLUDED_)
#define AFX_ECD100APP_H__893D4B04_B63D_4161_BE0D_DFA2A940ED34__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CECD100AppApp:
// See ECD100App.cpp for the implementation of this class
//

class CECD100AppApp : public CWinApp
{
public:
	CECD100AppApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CECD100AppApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CECD100AppApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ECD100APP_H__893D4B04_B63D_4161_BE0D_DFA2A940ED34__INCLUDED_)
