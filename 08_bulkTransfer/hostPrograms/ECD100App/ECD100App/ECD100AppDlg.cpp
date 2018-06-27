// ECD100AppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ECD100App.h"
#include "ECD100AppDlg.h"

#include "cyapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CECD100AppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CECD100AppDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CECD100AppDlg, CDialog)
	//{{AFX_MSG_MAP(CECD100AppDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_PROBE, OnProbe)
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

void CECD100AppDlg::OnProbe() 
{
	// TODO: Add your control notification handler code here
	CCyUSBDevice * pDevice;
	GUID guid = {0xF66AC7A1, 0x11AA, 0x4a42, 0x92, 0xAA, 0x1F, 0xAA, 0x77, 0xF3, 0xE6, 0xB5};
	int deviceCount;
	int endPointCount;
	int i;
	CCyBulkEndPoint * pBulkEp;
	bool rc;
	unsigned char buffer[4];

	pDevice=new CCyUSBDevice(NULL, guid);
	
	deviceCount=pDevice->DeviceCount();
	endPointCount=pDevice->EndPointCount();
	//Write EP1
	pBulkEp=NULL;
	for(i=0;i<endPointCount;i++)
	{
		if(pDevice->EndPoints[i]->Address==0x1)
		{
			pBulkEp=(CCyBulkEndPoint *)(pDevice->EndPoints[i]);
			break;
		}
	}
	if(NULL!=pBulkEp)
	{
		long length;

		buffer[0]=1;
		buffer[1]=2;
		buffer[2]=3;
		buffer[3]=4;
		length=4;
		rc=pBulkEp->XferData(buffer, length);
	}
	
	//read EP3
	pBulkEp=NULL;
	for(i=0;i<endPointCount;i++)
	{
		if(pDevice->EndPoints[i]->Address==0x83)
		{
			pBulkEp=(CCyBulkEndPoint *)(pDevice->EndPoints[i]);
			break;
		}
	}
	if(NULL!=pBulkEp)
	{
		long length;

		buffer[0]=0;
		buffer[1]=0;
		buffer[2]=0;
		buffer[3]=0;
		length=4;
		rc=pBulkEp->XferData(buffer, length);

	}

	delete pDevice;
}
