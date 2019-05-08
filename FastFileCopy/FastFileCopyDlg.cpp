// FastFileCopyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FastFileCopy.h"
#include "FastFileCopyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CFastFileCopyDlg dialog

CFastFileCopyDlg::CFastFileCopyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFastFileCopyDlg::IDD, pParent)
	, m_source(_T(""))
	, m_dest(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFastFileCopyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_source);
	DDX_Text(pDX, IDC_EDIT2, m_dest);
	DDX_Control(pDX, IDC_PROGRESS1, m_part1);
	DDX_Control(pDX, IDC_PROGRESS2, m_part2);
	DDX_Control(pDX, IDC_PROGRESS3, m_part3);
	DDX_Control(pDX, IDC_PROGRESS4, m_part4);
	DDX_Control(pDX, IDC_PROGRESS5, m_part5);
}

BEGIN_MESSAGE_MAP(CFastFileCopyDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON2, &CFastFileCopyDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON1, &CFastFileCopyDlg::OnBnClickedButton1)
	ON_BN_CLICKED(ID_START, &CFastFileCopyDlg::OnBnClickedStart)
END_MESSAGE_MAP()

// CFastFileCopyDlg message handlers
void CFastFileCopyDlg::OnTimer(UINT_PTR nIDEvent)
{
	UpdateData(TRUE);

	m_part1.SetPos(g_data.m_downloadstatus[0]);
	m_part2.SetPos(g_data.m_downloadstatus[1]);
	m_part3.SetPos(g_data.m_downloadstatus[2]);
	m_part4.SetPos(g_data.m_downloadstatus[3]);
	m_part5.SetPos(g_data.m_downloadstatus[4]);

	UpdateData(FALSE);
}

BOOL CFastFileCopyDlg::OnInitDialog()
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
	SetTimer(1234, 10, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFastFileCopyDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CFastFileCopyDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFastFileCopyDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

CString GetFileName(LPCTSTR filtername, LPCTSTR filtervalue, LPCTSTR defaultextension, BOOL forOpen)
{
	OPENFILENAME OpenFileName;

	//buffer for storing the file name
	TCHAR szFile[MAX_PATH] = _T("\0");
    StringCchCopy(szFile,MAX_PATH,_T(""));

	if(!filtername || !filtervalue)
	{
		filtername = _T("All Files (*.*)");
		filtervalue = _T("*.*");
	}

	TCHAR *filter = NULL;
	//create a NULL character seperated strings for defining the filters
	size_t filtername_len = 0;
	size_t filtervalue_len = 0;
	StringCchLength(filtername, STRSAFE_MAX_CCH, &filtername_len);
	StringCchLength(filtervalue, STRSAFE_MAX_CCH, &filtervalue_len);

	size_t size = filtername_len + filtervalue_len + 3;
	filter = new TCHAR[size];

	StringCchCopy(&(filter[0]), size, filtername);
	filter[filtername_len] = '\0';

	StringCchCopy(&(filter[filtername_len + 1]), size, filtervalue);
	filter[size - 2] = '\0';
	filter[size - 1] = '\0';

	OpenFileName.lStructSize       = sizeof(OPENFILENAME);
    OpenFileName.hwndOwner         = NULL;
    OpenFileName.hInstance         = NULL;
    OpenFileName.lpstrFilter       = filter;
    OpenFileName.lpstrCustomFilter = NULL;
    OpenFileName.nMaxCustFilter    = 0;
    OpenFileName.nFilterIndex      = 1;
    OpenFileName.lpstrFile         = szFile;
    OpenFileName.nMaxFile          = MAX_PATH;
    OpenFileName.lpstrFileTitle    = NULL;
    OpenFileName.nMaxFileTitle     = 0;
    OpenFileName.lpstrInitialDir   = NULL;
	OpenFileName.lpstrTitle        = _T("Select File");
    OpenFileName.nFileOffset       = 0;
    OpenFileName.nFileExtension    = 0;
    OpenFileName.lpstrDefExt       = defaultextension;
    OpenFileName.lCustData         = NULL;
	OpenFileName.lpfnHook 		   = NULL;
	OpenFileName.lpTemplateName    = NULL;
    OpenFileName.Flags             = OFN_LONGNAMES | OFN_SHOWHELP | OFN_EXPLORER;

	CString filename = _T("");
	if(forOpen)
	{
		OpenFileName.Flags |=  OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

		if(GetOpenFileName(&OpenFileName))
			filename = OpenFileName.lpstrFile;
	}
	else
	{
		OpenFileName.Flags |=  OFN_OVERWRITEPROMPT;

		if(GetSaveFileName(&OpenFileName))
			filename = OpenFileName.lpstrFile;
	}

	if(filter)
		delete filter;
	
	return filename;
}

void CFastFileCopyDlg::OnBnClickedButton2()
{
	CString val = GetFileName(NULL, NULL, NULL, TRUE);
	m_source = val;
	UpdateData(FALSE);
}

void CFastFileCopyDlg::OnBnClickedButton1()
{
	CString val = GetFileName(NULL, NULL, NULL, FALSE);
	m_dest = val;
	UpdateData(FALSE);
}

void FireCopy(CString src, CString dest);
void CFastFileCopyDlg::OnBnClickedStart()
{
	UpdateData();
	FireCopy(m_source, m_dest);
}
