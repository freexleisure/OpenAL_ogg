
// ApplicationDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Application.h"
#include "ApplicationDlg.h"
#include "afxdialogex.h"

#include <chrono>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CApplicationDlg 对话框



CApplicationDlg::CApplicationDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CApplicationDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CApplicationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER1, m_slider);
}

BEGIN_MESSAGE_MAP(CApplicationDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CApplicationDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CApplicationDlg::OnBnClickedButton2)
	ON_WM_VSCROLL()
END_MESSAGE_MAP()


// CApplicationDlg 消息处理程序

BOOL CApplicationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	AudioInterfaceTest();
	m_slider.SetRange(0,100);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CApplicationDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CApplicationDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CApplicationDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CApplicationDlg::AudioInterfaceTest()
{
	char FilePath[MAX_PATH] = {0};
	GetModuleFileName(NULL,FilePath,MAX_PATH);
#ifdef _DEBUG
	char *pPos = strstr(FilePath,"Debug");
	*pPos = 0;
#else
	char *pPos = strstr(FilePath,"Release");
	*pPos = 0;
#endif
	srand(time(0));
	
	//strcat(FilePath,"bling.ogg");
	//strcat(FilePath,"cAudioTheme1.ogg");
	//strcat(FilePath,"cAudioTheme2.ogg");
	char File1[MAX_PATH]={0};
	char File2[MAX_PATH]={0};
	strcpy(File1,FilePath);
	strcpy(File2,FilePath);

	strcat(File1,"愿得一人心.ogg");
	strcat(File2,"cAudioTheme2.ogg");
	
	std::chrono::system_clock::time_point pt = std::chrono::system_clock::now();

	CAudioInterfaceManage::getInstance()->OpenAl_Preload_Audio("cAudioTheme2.ogg",File2);
	CAudioInterfaceManage::getInstance()->OpenAl_Set_Audio_Volume("cAudioTheme2.ogg",2.0f);
	CAudioInterfaceManage::getInstance()->OpenAl_Preload_Audio("愿得一人心.ogg",File1);
	auto tCosTime= std::chrono::system_clock::now()-pt;
	TRACE1("cost time %d",tCosTime.count());

	SetTimer(100,5000,NULL);
	//CAudioInterfaceManage::getInstance()->OpenAl_Preload_Audio("愿得一人心.ogg",File1);
}

void CApplicationDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == 100)
	{
		CAudioInterfaceManage::getInstance()->OpenAl_Set_Listener_Position(rand()%100,rand()%100,rand()%100);
		CAudioInterfaceManage::getInstance()->OpenAl_Set_Listener_Velocity(rand()%100,rand()%100,rand()%100);
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CApplicationDlg::OnBnClickedButton1()
{
	CAudioInterfaceManage::getInstance()->OpenAl_Play_Audio_3d("愿得一人心.ogg",0,0,0,0);
	CAudioInterfaceManage::getInstance()->OpenAl_Play_Audio_3d("cAudioTheme2.ogg",0,0,0,0,true);
}


void CApplicationDlg::OnBnClickedButton2()
{
	CAudioInterfaceManage::getInstance()->OpenAl_Pause_Audio("愿得一人心.ogg");
}

void CApplicationDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CAudioInterfaceManage::getInstance()->OpenAl_Set_Audio_Volume("cAudioTheme2.ogg",((float)m_slider.GetPos())/100);
	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}
