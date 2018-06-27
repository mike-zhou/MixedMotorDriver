// ECD100AppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ECD100App.h"
#include "ECD100AppDlg.h"
#include "afxdlgs.h"
#include "ui_string.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern 	CECD100AppDlg * pDlg;



/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CECD100AppDlg dialog

CECD100AppDlg::CECD100AppDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CECD100AppDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CECD100AppDlg)
	m_jamFilePath = _T("");
	m_dataExchange = _T("");
	m_port = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CECD100AppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CECD100AppDlg)
	DDX_Text(pDX, IDC_JamFilePath, m_jamFilePath);
	DDX_Text(pDX, IDC_DataExchange, m_dataExchange);
	DDX_Text(pDX, IDC_Port, m_port);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CECD100AppDlg, CDialog)
	//{{AFX_MSG_MAP(CECD100AppDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_FindJamFile, OnFindJamFile)
	ON_BN_CLICKED(IDC_Program, OnProgram)
	ON_BN_CLICKED(IDC_Verify, OnVerify)
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_Port, OnChangePort)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CECD100AppDlg message handlers

BOOL CECD100AppDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_jamFilePath="";
	UpdateData(FALSE);
	
	this->SetWindowText(APPLICATION_TITLE);

	m_rs232Handle=(HANDLE)-1;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CECD100AppDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CECD100AppDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CECD100AppDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CECD100AppDlg::OnFindJamFile() 
{
	CFileDialog fileDialog(TRUE);
	char * filter;

	filter=JAM_FILE_FILTER;
	fileDialog.m_ofn.lpstrTitle=FIND_JAM_FILE_TO_OPEN;
	fileDialog.m_ofn.lpstrFilter=filter;
	if(IDOK==fileDialog.DoModal())
	{
		m_jamFilePath=fileDialog.GetPathName();
		UpdateData(false);
	}
	
}


void CECD100AppDlg::OnProgram() 
{
	// TODO: Add your control notification handler code here
	CString empty ("");
	
	//clear the information.
	m_dataExchange=empty;
	UpdateData(FALSE);
	
	OnVerify();
}

BOOL CECD100AppDlg::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class

	return CDialog::DestroyWindow();
}


void CECD100AppDlg::OnVerify() 
{
	// TODO: Add your control notification handler code here
	CFile file;
	unsigned long fileLength;
 	unsigned char * pJamBuffer;

	DCB dcb;
	CString empty ("");
	CString comPort("");
	
	unsigned char bufferTx[128];
	unsigned char bufferRx[128];

	m_dataExchange=empty;
	UpdateData(FALSE);

 	if(file.Open((LPCTSTR)m_jamFilePath, CFile::modeRead))
 	{
		fileLength=file.GetLength();
 		pJamBuffer=(unsigned char *)malloc(file.GetLength());
 		if(NULL!=pJamBuffer)
 		{
 			if(file.GetLength()!=file.Read(pJamBuffer, file.GetLength()))
 			{
 				MessageBox(ERROR_WHEN_READING_JAM_FILE, APPLICATION_TITLE, MB_OK|MB_ICONERROR);
 				file.Close();
				return;
 			}
 		}
 		file.Close();
 	}
 	else
 	{
 		MessageBox(FILE_CAN_NOT_BE_OPENED_FOR_READ, APPLICATION_TITLE, MB_OK|MB_ICONERROR);
		return;
 	}

	comPort.Format("%s%d", "COM", m_port);
	//open com
	m_rs232Handle=CreateFile((LPCTSTR)comPort,
							GENERIC_READ|GENERIC_WRITE,
							0,
							NULL,
							OPEN_EXISTING,
							0,
							NULL);
	if(m_rs232Handle==(HANDLE)-1)
	{
		CString str;
		
		str.Format("Cannot open ");
		m_dataExchange=str+comPort;
		UpdateData(FALSE);

		str.Format("Cannot open COM%d", m_port);
		MessageBox((LPCTSTR)str);
	}
	else
	{
		unsigned long fileIndex;

		//set read and write buffer size.
		SetupComm(m_rs232Handle,1024,1024);
		//configure com
		GetCommState(m_rs232Handle,&dcb); 
		dcb.BaudRate=115200;
		dcb.ByteSize=8;
		dcb.Parity=NOPARITY;
		dcb.StopBits=TWOSTOPBITS;
		SetCommState(m_rs232Handle,&dcb); 
		//clear input and output buffer.
		PurgeComm(m_rs232Handle,PURGE_TXCLEAR|PURGE_RXCLEAR); 
		
		{
			CString info ("Start sending data...\r\n");
			m_dataExchange=info;
			UpdateData(FALSE);
		}
		
		for(fileIndex=0; fileIndex<fileLength; fileIndex++)
		{
				//send this line to ECD300
				{
					BOOL bWriteStat;
					DWORD dwBytesWrite=1;
					COMSTAT ComStat; 
					DWORD dwErrorFlags; 

					ClearCommError(m_rs232Handle,&dwErrorFlags,&ComStat); 
					bWriteStat=WriteFile(m_rs232Handle,pJamBuffer+fileIndex,dwBytesWrite,&dwBytesWrite,NULL);
					if(!bWriteStat)
					{
						MessageBox("Fail to write serial port");
						break;
					}
					if(dwBytesWrite!=1)
					{
						MessageBox("Cannot send data to serial port");
						break;
					}
				}
		}
				

		{
			CString info ("Data transfer finished...\r\n");
			m_dataExchange=m_dataExchange+info;
			UpdateData(FALSE);
		}

		CloseHandle(m_rs232Handle);
	}

 	free(pJamBuffer);

}

void stubPrintString(char * pString)
{
	pDlg->printString(pString);
}



void CECD100AppDlg::printString(char *pString)
{
	CEdit * pDataExchange;

	CString str (pString);
	CString newLine ("\r\n");

	m_dataExchange+=str;
	m_dataExchange+=newLine;

	pDataExchange=(CEdit *)GetDlgItem(IDC_DataExchange);
	if(pDataExchange)
	{
		pDataExchange->SetWindowText(m_dataExchange);
		pDataExchange->LineScroll(100);
	}
}

void CECD100AppDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
}

//DEL UINT CECD100AppDlg::jamThread(LPVOID pParam)
//DEL {
//DEL 	for(;;)
//DEL 	{
//DEL 		m_actionLock->Lock();
//DEL 		switch(m_jamEvent)
//DEL 		{
//DEL 			case JAM_EVENT_EXIT:
//DEL 				m_exitLock->Unlock();
//DEL 				return 0;
//DEL 				break;
//DEL 			case JAM_EVENT_PROGRAM:
//DEL 			{
//DEL 				DCB dcb;
//DEL 
//DEL 				//open com
//DEL 				m_rs232Handle=CreateFile("COM9",
//DEL 							GENERIC_READ|GENERIC_WRITE,
//DEL 							0,
//DEL 							NULL,
//DEL 							OPEN_EXISTING,
//DEL 							0,
//DEL 							NULL);
//DEL 				if(m_rs232Handle==(HANDLE)-1)
//DEL 				{
//DEL 					MessageBox("Cannot open COM9");
//DEL 					continue;
//DEL 				}
//DEL 				//set read and write buffer size.
//DEL 				SetupComm(m_rs232Handle,1024,1024);
//DEL 				//configure com
//DEL 				GetCommState(m_rs232Handle,&dcb); 
//DEL 				dcb.BaudRate=115200;
//DEL 				dcb.ByteSize=8;
//DEL 				dcb.Parity=NOPARITY;
//DEL 				dcb.StopBits=TWOSTOPBITS;
//DEL 				SetCommState(m_rs232Handle,&dcb); 
//DEL 				//clear input and output buffer.
//DEL 				PurgeComm(m_rs232Handle,PURGE_TXCLEAR|PURGE_RXCLEAR); 				
//DEL 
//DEL 				//executeJam(action);
//DEL 
//DEL 				CloseHandle(m_rs232Handle);
//DEL 				m_rs232Handle=(HANDLE)-1;
//DEL 			}
//DEL 				break;
//DEL 			case JAM_EVENT_VERIFY:
//DEL 			{
//DEL 				DCB dcb;
//DEL 				
//DEL 				//open com
//DEL 				m_rs232Handle=CreateFile("COM9",
//DEL 							GENERIC_READ|GENERIC_WRITE,
//DEL 							0,
//DEL 							NULL,
//DEL 							OPEN_EXISTING,
//DEL 							0,
//DEL 							NULL);
//DEL 				if(m_rs232Handle==(HANDLE)-1)
//DEL 				{
//DEL 					MessageBox("Cannot open COM9");
//DEL 					continue;
//DEL 				}
//DEL 				//set read and write buffer size.
//DEL 				SetupComm(m_rs232Handle,1024,1024);
//DEL 				//configure com
//DEL 				GetCommState(m_rs232Handle,&dcb); 
//DEL 				dcb.BaudRate=115200;
//DEL 				dcb.ByteSize=8;
//DEL 				dcb.Parity=NOPARITY;
//DEL 				dcb.StopBits=TWOSTOPBITS;
//DEL 				SetCommState(m_rs232Handle,&dcb); 
//DEL 				//clear input and output buffer.
//DEL 				PurgeComm(m_rs232Handle,PURGE_TXCLEAR|PURGE_RXCLEAR); 				
//DEL 
//DEL 				//executeJam(action);
//DEL 
//DEL 				CloseHandle(m_rs232Handle);
//DEL 				m_rs232Handle=(HANDLE)-1;
//DEL 			}
//DEL 				break;
//DEL 			default:
//DEL 				break;	
//DEL 		}
//DEL 	}
//DEL }

BOOL CECD100AppDlg::isHexDigit(unsigned char c)
{
	if((c>='0')&&(c<='9'))
		return TRUE;
	else if((c>='A')&&(c<='F'))
		return TRUE;
	else if((c>='a')&&(c<='f'))
		return TRUE;
	else
		return FALSE;
}

void CECD100AppDlg::OnChangePort() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}
