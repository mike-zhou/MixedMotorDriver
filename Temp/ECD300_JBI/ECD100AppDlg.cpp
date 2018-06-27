// ECD100AppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ECD100App.h"
#include "ECD100AppDlg.h"
#include "afxdlgs.h"
#include "ui_string.h"
#include "source\stub_implement.h"

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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CECD100AppDlg message handlers
UINT jamThreadWrapper( LPVOID pParam )
{
    return pDlg->jamThread(pParam);   // thread completed successfully
}

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

	GUID guid = {0xF66AC7A1, 0x11AA, 0x4a42, 0x92, 0xAA, 0x1F, 0xAA, 0x77, 0xF3, 0xE6, 0xB5};
	int deviceCount;
	int endPointCount;
	int i;
	

	pUsbDevice=new CCyUSBDevice(NULL, guid);
	
	pEp0=NULL;
	pEp1=NULL;
	pEp2=NULL;
	pEp4=NULL;
	pEp6=NULL;
	pEp8=NULL;
	if(NULL!=pUsbDevice)
	{
		deviceCount=pUsbDevice->DeviceCount();
		endPointCount=pUsbDevice->EndPointCount();

		if(0==deviceCount)
			MessageBox(NO_ECD_CONNECTED, APPLICATION_TITLE, MB_OK|MB_ICONERROR);

		for(i=0;i<endPointCount;i++)
		{
			switch(pUsbDevice->EndPoints[i]->Address)
			{
			case 0:
				pEp0=(CCyControlEndPoint *)(pUsbDevice->EndPoints[i]);
				break;
			case 1:
				pEp1=(CCyBulkEndPoint *)(pUsbDevice->EndPoints[i]);
				break;
			case 2:
				pEp2=(CCyBulkEndPoint *)(pUsbDevice->EndPoints[i]);
				break;
			case 4:
				pEp4=(CCyBulkEndPoint *)(pUsbDevice->EndPoints[i]);
				break;
			case 0x86:
				pEp6=(CCyBulkEndPoint *)(pUsbDevice->EndPoints[i]);
				break;
			case 0x88:
				pEp8=(CCyBulkEndPoint *)(pUsbDevice->EndPoints[i]);
				break;
			default:
				break;
			}
		}
	}
	else
	{
		MessageBox(NO_ECD_DRIVER_INSTALLED, APPLICATION_TITLE, MB_OK|MB_ICONERROR);
	}
	
	pCommandBuffer=(unsigned char *)malloc(COMMAND_MAX*COMMAND_LENGTH);

	m_actionLock=new CSemaphore (0, 1);
	m_exitLock=new CSemaphore(0, 1);
	AfxBeginThread(jamThreadWrapper, NULL);

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

	currentCommandIndex=0;
	
	m_jamEvent=JAM_EVENT_PROGRAM;
	m_actionLock->Unlock();
}

BOOL CECD100AppDlg::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	if(NULL==pUsbDevice)
	{
		delete pUsbDevice;
	}
	
	return CDialog::DestroyWindow();
}

int CECD100AppDlg::executeJam(unsigned char *action)
{
	CFile file;

	if(file.Open((LPCTSTR)m_jamFilePath, CFile::modeRead))
	{
		unsigned char * pJamBuffer;
		
		pJamBuffer=(unsigned char *)malloc(file.GetLength());
		if(NULL!=pJamBuffer)
		{
			if(file.GetLength()!=file.Read(pJamBuffer, file.GetLength()))
			{
				MessageBox(ERROR_WHEN_READING_JAM_FILE, APPLICATION_TITLE, MB_OK|MB_ICONERROR);
			}
			else
			{
				jp_main(pJamBuffer, file.GetLength(), action, 1);
			}
			free(pJamBuffer);
		}
		file.Close();
	}
	else
	{
		MessageBox(FILE_CAN_NOT_BE_OPENED_FOR_READ, APPLICATION_TITLE, MB_OK|MB_ICONERROR);
	}

	return 0;
}

void CECD100AppDlg::OnVerify() 
{
	// TODO: Add your control notification handler code here
	CString empty ("");
	
	//clear the information.
	m_dataExchange=empty;
	UpdateData(FALSE);

	currentCommandIndex=0;
	
	m_jamEvent=JAM_EVENT_VERIFY;
	m_actionLock->Unlock();
}

int CECD100AppDlg::jtagIO(int tms, int tdi, int bReadTdo)
{
#if 0
	unsigned char buffer[5];
	int rc;
	long length;
	int tdo=0;
	int i;

	buffer[0]=__PROXY_TAG_JTAG;//tag
	buffer[1]=3;//length
	//command
	if(0==bReadTdo)
		buffer[2]=0x1;
	else
		buffer[2]=0x3;
	//tms
	if(0!=tms)
		buffer[3]=1;
	else
		buffer[3]=0;
	//tdi
	if(0!=tdi)
		buffer[4]=1;
	else
		buffer[4]=0;

	for(i=0;i<COMMAND_LENGTH;i++)
		pCommandBuffer[currentCommandIndex*COMMAND_LENGTH+i]=buffer[i];
	currentCommandIndex++;

	if(0!=bReadTdo)
	{
		length=COMMAND_LENGTH*currentCommandIndex;
		if(NULL!=pEp2)
		{
			rc=pEp2->XferData(pCommandBuffer, length);
		}
		if(NULL!=pEp6)
		{
			length=COMMAND_LENGTH;

			rc=pEp6->XferData(buffer, length);
			if(rc==true)
			{
				tdo=buffer[2];
			}
		}
		
		currentCommandIndex=0;
	}
	else if(COMMAND_MAX==currentCommandIndex)
	{
		length=COMMAND_LENGTH*currentCommandIndex;
		if(NULL!=pEp2)
		{
			rc=pEp2->XferData(pCommandBuffer, length);
		}
		currentCommandIndex=0;
	}

	if(JAM_EVENT_EXIT==m_jamEvent)
		ExitThread(0);
	
	return (tdo==0)?0:1;

#else
	
	unsigned char c=0;
	DWORD dwBytesWrite=1; 
	COMSTAT ComStat; 
	DWORD dwErrorFlags; 
	BOOL bWriteStat; 

	if(tms)
		c|=0x2;
	if(tdi)
		c|=0x1;
	if(bReadTdo)
		c|=0x80;

	//send command to device through serial port.
	ClearCommError(m_rs232Handle,&dwErrorFlags,&ComStat); 
	bWriteStat=WriteFile(m_rs232Handle,&c,dwBytesWrite,& dwBytesWrite,NULL);
	if(!bWriteStat)
	{
		MessageBox("Fail to write COM9");
		return 0;
	}

	c=0;
	if(bReadTdo)
	{
		DWORD wCount;
		BOOL bReadStat; 

		bReadStat=ReadFile(m_rs232Handle,&c,1,&wCount,NULL); 
		if(!bReadStat) 
		{ 
			MessageBox("Fail to read COM9"); 
			return 0; 
		}
	}

	if(c!=0)
		return 1;
	else
		return 0;

#endif
}

int CECD100AppDlg::jtagDelay(int milliSecond)
{
	long length;
	
	if(currentCommandIndex>0)
	{
		if(NULL!=pEp2)
		{
			length=COMMAND_LENGTH*currentCommandIndex;
			pEp2->XferData(pCommandBuffer, length);
			currentCommandIndex=0;
		}
	}

	if(JAM_EVENT_EXIT==m_jamEvent)
		ExitThread(0);
		
	Sleep(milliSecond);
	return 0;
}

int jtagInit(void)
{
    return 0;
}

int jtagRelease(void)
{
    return 0;
}

int jtagIo(int tms, int tdi, int bReadTdo)
{
	return pDlg->jtagIO(tms, tdi, bReadTdo);
}

int stubDelay(unsigned long microSeconds)
{
	unsigned long milliSecond;

	milliSecond=(microSeconds+999)/1000;
	pDlg->jtagDelay(milliSecond);
	
	return 0;
}

int stubSetClockFrequency(unsigned long maxFrequencyHz)
{
	char buffer[100];

	sprintf(buffer, "Maxium JTAG frequency: %d Hz\r\n", maxFrequencyHz);
	pDlg->printString(buffer);
	
	return 0;
}

void * stubMalloc(unsigned long size)
{
	void * address;

	address=malloc(size);
	
	return address;
}

void stubFree(void * address)
{
	if(NULL!=address)
		free(address);
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
	m_jamEvent=JAM_EVENT_EXIT;
	m_actionLock->Unlock();
	//wait for the exit of jamThread.
	Sleep(500);

	delete m_exitLock;
	delete m_actionLock;
	free(pCommandBuffer);
}

UINT CECD100AppDlg::jamThread(LPVOID pParam)
{
	for(;;)
	{
		m_actionLock->Lock();
		switch(m_jamEvent)
		{
			case JAM_EVENT_EXIT:
				m_exitLock->Unlock();
				return 0;
				break;
			case JAM_EVENT_PROGRAM:
			{
				unsigned char action[]={'P', 'R', 'O', 'G', 'R', 'A', 'M', '\0'};
				DCB dcb;

				//open com
				m_rs232Handle=CreateFile("COM9",
							GENERIC_READ|GENERIC_WRITE,
							0,
							NULL,
							OPEN_EXISTING,
							0,
							NULL);
				if(m_rs232Handle==(HANDLE)-1)
				{
					MessageBox("Cannot open COM9");
					continue;
				}
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

				executeJam(action);

				CloseHandle(m_rs232Handle);
				m_rs232Handle=(HANDLE)-1;
			}
				break;
			case JAM_EVENT_VERIFY:
			{
				unsigned char action[]={'V', 'E', 'R', 'I', 'F', 'Y', '\0'};
				DCB dcb;
				
				//open com
				m_rs232Handle=CreateFile("COM9",
							GENERIC_READ|GENERIC_WRITE,
							0,
							NULL,
							OPEN_EXISTING,
							0,
							NULL);
				if(m_rs232Handle==(HANDLE)-1)
				{
					MessageBox("Cannot open COM9");
					continue;
				}
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

				executeJam(action);

				CloseHandle(m_rs232Handle);
				m_rs232Handle=(HANDLE)-1;
			}
				break;
			default:
				break;	
		}
	}
}
