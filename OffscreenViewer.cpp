#include <afxwin.h>
#include <afxcmn.h>

#include "Shlobj.h"
#include "Shlwapi.h"
#include "string.h"

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <stdio.h>

#include <iostream>


#include "opencv2/opencv.hpp"
#include "opencvlib.h"
#include "EnhanceImage.h"


#include "PTGDE/CPTGDE.h"

#pragma comment(lib,"PTGDEd.lib")



#pragma comment( lib, "gdiplus" )

using namespace std;

#include "OffscreenViewer.h"

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")



#define IDC_LBLSERVERNAME				1
#define IDC_LBLUSERNAME					2
#define	IDC_LBLPASSWORD					3
#define IDC_LBLMEDIACHANNELS			4
#define IDC_EDTSERVERNAME				5
#define IDC_EDTUSERNAME					6
#define	IDC_EDTPASSWORD					7
#define IDC_BTNCONNECT					8
#define IDC_BTNDISCONNECT				9
#define IDC_CONNECTPROGRESS				10
#define IDC_LBMEDIACHANNELS				11
#define IDC_PNLVIEWER					12
#define IDC_BTNLIVE						13
#define IDC_BTNSTOP						14
#define IDC_BTNFORWARD					15
#define IDC_BTNBACKWARD					16
#define IDC_BTNBOD						17
#define IDC_BTNEOD						18
#define IDC_BTNEXPORTSINGLEPICTURE		19
#define IDC_CBSCALETOSIZE				20
#define IDC_CBDOCUSTOMDRAW				21
#define IDC_CBDISPLAYTEXT				22
#define IDC_BTNFRAMEDISTANCE			23
#define IDC_EDTFRAMEDISTANCE			24
#define IDC_LBLMS						25

#define IDC_ENABLEPHAWK					26
#define IDC_BTNRADIOFOG					27
#define	IDC_BTNRADIOLOWLIGHT			28
#define	IDC_BTNRADIORAIN				29
#define IDC_BTNRADIOSNOW				30


using namespace Gng;

int phawkflag = 0;

string phawkFilterFlag = "";

// callback function for connect progress display
bool __stdcall ConnectProgressCB(void* Instance, unsigned __int32 Progress, unsigned __int32 MaxProgress)
{
	if(Instance == NULL)
		return TRUE;

	// calling the callback method of the main window object instance
	CMainWin* MainWin = (CMainWin*)Instance;
	return MainWin->ConnectProgress(Progress, MaxProgress);
}

// callback is called from offscreen viewer object before picture is decompressed, callback allows custom draw over the image
bool __stdcall CustomDrawCallBackExCB(void* Instance,
                                      HDC ViewerDC,                      // DC for drawing
                                      const TRect& ClientRect,           // Rect of rawing area
                                      const TRect& SrcRect,              // Rect of original image buffer
                                      const TPicData& PicData,           // Picture Information
                                      const TViewerStatus& ViewerStatus, // Viewer status
                                      const TEventData* MscEventData)    // pointer to a TEventData structure (only for Multiscope, if image is event image) DO NOT SAVE!
                                                                         // return value: TRUE : draw and update backbuffer
                                                                         //               FALSE: don't change backbuffer
                                                                         // to clear the backbuffer :
                                                                         // do not paint anything and return TRUE
{
	if(Instance == NULL)
		return TRUE;

	// calling the callback method of the main window object instance
	CMainWin* MainWin = (CMainWin*)Instance;
	return MainWin->CustomDrawCallBackEx(ViewerDC, ClientRect, SrcRect, PicData, ViewerStatus, MscEventData);
}

// callback is called from offscreen viewer object after picture has been decompressed
bool __stdcall NewOffscreenImageCallbackCB(void* Instance,
                                           const HGngDecompBuffer OffscreenBufferHandle,	// Handle to the DecompBuffer object..
                                           const TRect& SrcRect,							// Rect of decompressed buffer
                                           const TPicData& PicData,						    // Picture Information
                                           const TViewerStatus& ViewerStatus,				// Viewer status
                                           const TEventData* MscEventData)					// return value defines if user was able to handle image
{
	if(Instance == NULL)
		return TRUE;

	// calling the callback method of the main window object instance
	CMainWin* MainWin = (CMainWin*)Instance;
	return MainWin->NewOffscreenImageCallback(OffscreenBufferHandle, SrcRect, PicData, ViewerStatus, MscEventData);
}

// callback is called from offscreen viewer object before picture is decompressed
bool __stdcall NewOffscreenImageAcceptCallbackCB(void* Instance,
                                                 HGngDecompBuffer& OffscreenBufferHandle,  // Buffer to decompress, default buffer can be overridden..
											     const TPicData& PicData,                  // Picture Information
                                                 const TViewerStatus& ViewerStatus,        // Viewer status
                                                 const TEventData* MscEventData)           // return value defines if user wants the image to be decompressed
{
	if(Instance == NULL)
		return TRUE;

	// calling the callback method of the main window object instance
	CMainWin* MainWin = (CMainWin*)Instance;
	return MainWin->NewOffscreenImageAcceptCallback(OffscreenBufferHandle, PicData, ViewerStatus, MscEventData);
}



CMainWin::CMainWin()
{
	GdiplusStartupInput gdiplusStartupInput;
	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Create(NULL, L"Gng SDK Example \"VS2013CPP_OffscreenViewer\"");

	

}

CMainWin::~CMainWin()
{
    // deinitialize the GngMediaPlayer-DLL
    GMPDeInitializeMediaPlayerDLL();

	GdiplusShutdown(gdiplusToken);

    CoUninitialize();

	
}

afx_msg int CMainWin::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CFrameWnd::OnCreate(lpCreateStruct);
	
    // initialize the GngMediaPlayer-DLL with the window handle of the application main form
    GMPInitializeMediaPlayerDLL(NULL);

	lblServername.Create(L"Servername", WS_CHILD|WS_VISIBLE, CRect(8,24,88,37), this, IDC_LBLSERVERNAME);
	lblUsername.Create(L"Username", WS_CHILD|WS_VISIBLE, CRect(8,56,88,69), this, IDC_LBLUSERNAME);
	lblPassword.Create(L"Password", WS_CHILD|WS_VISIBLE, CRect(8,88,88,101), this, IDC_LBLPASSWORD);
	lblMediaChannels.Create(L"Media channels", WS_CHILD|WS_VISIBLE, CRect(8,184,209,197), this, IDC_LBLMEDIACHANNELS);


	edtServername.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, CRect(92, 16, 209, 37), this, IDC_EDTSERVERNAME);
	edtUsername.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, CRect(92, 48, 209, 69), this, IDC_EDTUSERNAME);
	edtPassword.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, CRect(92, 80, 209, 101), this, IDC_EDTPASSWORD);

	btnConnect.Create(L"Connect", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(8,120,92,145), this, IDC_BTNCONNECT);
	btnDisconnect.Create(L"Disconnect", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(117,120,209,145), this, IDC_BTNDISCONNECT);

	pbConnectProgress.Create(WS_CHILD|WS_VISIBLE, CRect(8,152,209,169), this, IDC_CONNECTPROGRESS);   

	lbMediaChannels.Create(WS_CHILD|WS_VISIBLE|WS_BORDER, CRect(8,200,209,675), this, IDC_LBMEDIACHANNELS);

	btnLive.Create(L"Live", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(463,538,512,563), this, IDC_BTNLIVE);
	btnStop.Create(L"Stop", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(328,538,377,563), this, IDC_BTNSTOP);
	btnForward.Create(L">", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(383,538,416,563), this, IDC_BTNFORWARD);
	btnBackward.Create(L"<", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(288,538,321,563), this, IDC_BTNBACKWARD);
	btnBOD.Create(L"|<", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(248,538,281,563), this, IDC_BTNBOD);
	btnEOD.Create(L">|", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(423,538,456,563), this, IDC_BTNEOD);

	btnExportSinglePicture.Create(L"Export", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(519,538,594,563), this, IDC_BTNEXPORTSINGLEPICTURE);


	cbScaleToSize.Create(L"Scale to viewer size", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, CRect(248,570,399,587), this, IDC_CBSCALETOSIZE);
	cbDoCustomDraw.Create(L"Do custom draw", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, CRect(248,594,374,611), this, IDC_CBDOCUSTOMDRAW);
	cbDisplayText.Create(L"Display text", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, CRect(248,618,374,635), this, IDC_CBDISPLAYTEXT);

	btnOverrideLiveStreamFrameDistance.Create(L"Override live stream frame distance", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, CRect(248,646,550,671), this, IDC_BTNFRAMEDISTANCE);
	edtFrameDistance.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, CRect(558, 650, 658, 671), this, IDC_EDTFRAMEDISTANCE);
	lblMs.Create(L"ms (0=disable)", WS_CHILD|WS_VISIBLE, CRect(662,650,762,667), this, IDC_LBLMS);


	bEnablePhawk.Create(L"Enable Phawk", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(600, 538, 703, 563), this, IDC_ENABLEPHAWK);
	btnRadioFog.Create(L"Fog " , WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON, CRect(710, 538, 760, 563), this, IDC_BTNRADIOFOG);btnRadioFog.EnableWindow(0);
	btnRadioLOWLIGHT.Create(L"Low-Light ", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON, CRect(762, 538, 848, 563), this, IDC_BTNRADIOLOWLIGHT);btnRadioLOWLIGHT.EnableWindow(0);
	btnRadioRAIN.Create(L"Rain", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON, CRect(850, 538, 902, 563), this, IDC_BTNRADIORAIN);btnRadioRAIN.EnableWindow(0);
	btnRadioSNOW.Create(L"Snow", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON, CRect(904, 538, 980, 563), this, IDC_BTNRADIOSNOW);btnRadioSNOW.EnableWindow(0);

	

		


	CheckDlgButton(IDC_CBSCALETOSIZE, BST_CHECKED);

	edtServername.SetWindowText(L"localhost");
	edtUsername.SetWindowText(L"sysadmin");
	edtPassword.SetWindowText(L"masterkey");
	edtFrameDistance.SetWindowText(L"500");

	m_NewPicDecompressed = false;

	m_DoCustomDraw = false;
    m_CustomDrawFlagChanged = false;

	CoInitialize(NULL);

	// initialize handles
    m_GngServer = NULL;
	m_GngOffscreenViewer = NULL;
	m_GngDecompBuffer1 = NULL;
	m_GngDecompBuffer2 = NULL;
    m_NewPicDecompBuffer = NULL;
	m_GngSetup  = NULL;

	return TRUE;
}

afx_msg void CMainWin::OnClose()
{
	DisconnectFromServer();

	CFrameWnd::OnClose();
}


afx_msg BOOL CMainWin::OnEraseBkgnd(CDC* pDC)
{
	CBrush backBrush(RGB(236, 233, 216));
	CBrush* pOldBrush = pDC->SelectObject(&backBrush);
	CRect rect;
	pDC->GetClipBox(&rect);
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
	pDC->SelectObject(pOldBrush);

	return TRUE;
}

afx_msg void CMainWin::OnPaint()
{
	PAINTSTRUCT ps;
	CDC* pDC = BeginPaint(&ps);

	// check if new decompressed picture available
    m_CSBuffer.Lock();
    if(m_NewPicDecompressed)
    {
	    if(m_NewPicDecompBuffer == m_GngDecompBuffer2)
            m_CSBuffer2.Lock();
        else
            m_CSBuffer1.Lock();

        // new decompressed picture available
	    void* BufPointer;
        DWORD BufWidth, BufHeight, BufPitch;
	    BITMAPINFO BmpInfo;

	    // get a pointer to the bits of the decompressed image
	    m_NewPicDecompBuffer->GetBufPointer(BufPointer, BufWidth, BufHeight, BufPitch);
	    // get a bitmap info header, that fits to the decompressed image
	    m_NewPicDecompBuffer->GetBitmapInfoHdr(BmpInfo.bmiHeader);

		

        // render the decompressed image into a GDI+ memory bitmap
        Bitmap MemBmp(BufWidth, BufHeight, BufPitch, PixelFormat32bppARGB, (BYTE*)BufPointer);
		
		
        // vertical flip the memory bitmap
        MemBmp.RotateFlip(RotateNoneFlipY);
		//GPU::Stream imgStream = new MemoryStream();

		// Call Prohawk (PTGDE) for image enhancement
		Bitmap *pMemBmp;
		
		pMemBmp = &MemBmp;
		pair<cv::Mat, cv::Mat> PhwkEnhncdImage;
		cv::Mat  phawk_output_image, phawk_retval;

		if (phawkflag !=0 && phawkFilterFlag!="") { PhwkEnhncdImage = PhawkImageEnhancer(MemBmp, phawkFilterFlag); }
		phawk_retval = PhwkEnhncdImage.first;
		phawk_output_image = PhwkEnhncdImage.second;

		

		
		cv::Size size = phawk_retval.size();
		Bitmap pHwkMemBmp(size.width, size.height, phawk_retval.step1(), PixelFormat24bppRGB, phawk_output_image.data);
		if (phawkflag !=0) { pMemBmp = &pHwkMemBmp; }
		

		
		Graphics graphics(pDC->m_hDC);
	
	    bool Scale = (IsDlgButtonChecked(IDC_CBSCALETOSIZE) == BST_CHECKED);
        if(Scale)
	
		 
			   graphics.DrawImage((Image*)pMemBmp,
                m_ViewerRect.left, m_ViewerRect.top, 
                (m_ViewerRect.right - m_ViewerRect.left), (m_ViewerRect.bottom - m_ViewerRect.top));
        else
		graphics.DrawImage((Image*)pMemBmp, m_ViewerRect.left, m_ViewerRect.top);




	    if(m_NewPicDecompBuffer == m_GngDecompBuffer2)
            m_CSBuffer2.Unlock();
        else
            m_CSBuffer1.Unlock();

        m_NewPicDecompressed = false;
    }
    m_CSBuffer.Unlock();

	EndPaint(&ps);
}

afx_msg void CMainWin::btnConnectClicked()
{
	DisconnectFromServer();

	CString Address;
	CString Username;
	CString Password;

	edtServername.GetWindowText(Address);
	edtUsername.GetWindowText(Username);
	edtPassword.GetWindowText(Password);

	ConnectToServer(Address, Username, Password);
}

afx_msg void CMainWin::btnDisconnectClicked()
{
	DisconnectFromServer();
}

afx_msg void CMainWin::btnLiveClicked()
{
	OffscreenViewerConnectDB(pmPlayStream);
}

afx_msg void CMainWin::btnStopClicked()
{
	OffscreenViewerConnectDB(pmPlayStop);
}

afx_msg void CMainWin::btnForwardClicked()
{
	OffscreenViewerConnectDB(pmPlayForward);
}

afx_msg void CMainWin::btnBackwardClicked()
{
	OffscreenViewerConnectDB(pmPlayBackward);
}

afx_msg void CMainWin::btnBODClicked()
{
	OffscreenViewerConnectDB(pmPlayBOD);
}

afx_msg void CMainWin::btnEODClicked()
{
	OffscreenViewerConnectDB(pmPlayEOD);
}

afx_msg void CMainWin::btnExportSinglePictureClicked()
{
	if(m_GngOffscreenViewer == NULL)
		return;

	TMPPictureExportParams MPPictureExportParams;

	// export the actual image of the offscreen viewer into a bitmap file
	memset(&MPPictureExportParams, 0, sizeof(MPPictureExportParams));
	MPPictureExportParams.StructSize = sizeof(MPPictureExportParams);
	MPPictureExportParams.DestType = edtBMP;

	TCHAR szPath[MAX_PATH];
	SHGetFolderPath(NULL,  
					CSIDL_MYDOCUMENTS|CSIDL_FLAG_CREATE, 
					NULL, 
					0, 
					szPath);
	PathAppend(szPath, TEXT("Pic.bmp"));

	MPPictureExportParams.FileName = szPath;
	MPPictureExportParams.DontShowDialog = true;

	m_GngOffscreenViewer->ExportSinglePicture(MPPictureExportParams);
}

afx_msg void CMainWin::cbScaleToSizeClicked()
{
	if(m_GngOffscreenViewer == NULL)
		return;

	// scale or don't scale the decompressed image to the offscreen viewer object size
	bool IsChecked = (IsDlgButtonChecked(IDC_CBSCALETOSIZE) == BST_CHECKED);
	m_GngOffscreenViewer->SetOffscreenViewerSize((m_ViewerRect.right - m_ViewerRect.left), (m_ViewerRect.bottom - m_ViewerRect.top), IsChecked);
	m_GngOffscreenViewer->Refresh(true);
}

afx_msg void CMainWin::cbDoCustomDrawClicked()
{
  m_DoCustomDraw = (IsDlgButtonChecked(IDC_CBDOCUSTOMDRAW) == BST_CHECKED);
  m_CustomDrawFlagChanged = true;

  if(m_GngOffscreenViewer == NULL)
		return;

  m_GngOffscreenViewer->Refresh(true);
}

afx_msg void CMainWin::cbDisplayTextClicked()
{
	if(m_GngOffscreenViewer == NULL)
		return;

	TViewTextParams Params;
	m_GngOffscreenViewer->GetTextParams(Params);
	bool IsChecked = (IsDlgButtonChecked(IDC_CBDISPLAYTEXT) == BST_CHECKED);
	if(IsChecked)
	{
		// display text over the image in the decompression buffer
		Params.InsertPicInfo = true;
		Params.FontSize      = 15;
		Params.Position      = tpTopLeft;
	}
	else
		Params.InsertPicInfo = false;
	m_GngOffscreenViewer->SetTextParams(Params);
	m_GngOffscreenViewer->Refresh(true);
}

afx_msg void CMainWin::cbEnablePhawkClicked(){
	if (phawkflag != 1) {
		phawkflag = 1;
		bEnablePhawk.SetWindowTextW(_T("Disable Phawk"));
		btnRadioFog.EnableWindow(1);
		btnRadioLOWLIGHT.EnableWindow(1);
		btnRadioRAIN.EnableWindow(1);
		btnRadioSNOW.EnableWindow(1);

		phawkFilterFlag = "LOWLIGHT"; //Default Filter
		btnRadioLOWLIGHT.SetCheck(1);
		btnRadioFog.SetCheck(0);
		btnRadioRAIN.SetCheck(0);
		btnRadioSNOW.SetCheck(0);

		

	}
	else { phawkflag = 0;
	bEnablePhawk.SetWindowTextW(_T("Enable Phawk"));
	btnRadioFog.EnableWindow(0);
	btnRadioLOWLIGHT.EnableWindow(0);
	btnRadioRAIN.EnableWindow(0);
	btnRadioSNOW.EnableWindow(0);

	btnRadioRAIN.SetCheck(0);
	btnRadioSNOW.SetCheck(0);
	}
}

afx_msg void CMainWin::cbPhawkFOGClicked() {
	phawkFilterFlag = "FOG";
	INT m_nIndex=1;
	btnRadioFog.SetCheck(1);//btnRadioFog.EnableWindow(0);
	btnRadioLOWLIGHT.SetCheck(0);
	btnRadioRAIN.SetCheck(0);
	btnRadioSNOW.SetCheck(0);

}




afx_msg void CMainWin::cbPhawkLOWLIGHTClicked() {
	phawkFilterFlag = "LOWLIGHT";
	INT m_nIndex = 1;
	//IsDlgButtonChecked(IDC_BTNRADIOFOG);
	btnRadioLOWLIGHT.SetCheck(1);
	btnRadioFog.SetCheck(0);
	btnRadioRAIN.SetCheck(0);
	btnRadioSNOW.SetCheck(0);

}

afx_msg void CMainWin::cbPhawkRAINClicked() {
	phawkFilterFlag = "RAIN";
	INT m_nIndex = 1;
	btnRadioFog.SetCheck(0);//btnRadioFog.EnableWindow(0);
	btnRadioLOWLIGHT.SetCheck(0);
	btnRadioRAIN.SetCheck(1);
	btnRadioFog.SetCheck(0);
	btnRadioSNOW.SetCheck(0);
}

afx_msg void CMainWin::cbPhawkSNOWClicked() {
	phawkFilterFlag = "SNOW";
	INT m_nIndex = 1;
	btnRadioFog.SetCheck(0);//btnRadioFog.EnableWindow(0);
	btnRadioLOWLIGHT.SetCheck(0);
	btnRadioRAIN.SetCheck(0);
	btnRadioFog.SetCheck(0);
	btnRadioSNOW.SetCheck(1);
}


afx_msg void CMainWin::btnOverrideLiveStreamFrameDistanceClicked()
{
	if(m_GngOffscreenViewer == NULL)
		return;

	unsigned __int32 AvrgFrameDistanceInMs = 500;

	CString FrameDistanceStr;

	edtFrameDistance.GetWindowText(FrameDistanceStr);

	AvrgFrameDistanceInMs = _wtoi(FrameDistanceStr.GetBuffer());

	FrameDistanceStr.Format(_T("%d"), AvrgFrameDistanceInMs);

	edtFrameDistance.SetWindowText(FrameDistanceStr);

	// override the frame distance of the live stream with a user defined value
	m_GngOffscreenViewer->OverrideLiveStreamFrameDistance(AvrgFrameDistanceInMs);
}

void CMainWin::ConnectToServer(CString &Address, CString &Username, CString &Password)
{
	if(m_GngServer == NULL)
		// create a server object instance
		m_GngServer = DBICreateRemoteServer();
	// encode the password
    std::string EncodedPassword = DBIEncodeString(Password);
	// initialize the connection parameters  
	TGngServerConnectParams ConnectParams(Address, Username, EncodedPassword.c_str());
	m_GngServer->SetConnectParams(ConnectParams);
	// connect to the server
	TConnectResult ConnectResult = m_GngServer->Connect(&ConnectProgressCB, this);
	if(ConnectResult == connectOk)
		FillMediaChannelsList();
	else
		MessageBox(L"connection failed!");
}

bool CMainWin::ConnectProgress(unsigned __int32 Progress, unsigned __int32 MaxProgress)
{
	pbConnectProgress.SetRange32(0, MaxProgress);
	pbConnectProgress.SetPos(Progress);

	return TRUE;
}

void CMainWin::DisconnectFromServer()
{
	pbConnectProgress.SetPos(0);
	ClearMediaChannelsList();

	DestroyOffscreenViewer();

	if(m_GngServer != NULL)
	{
		m_GngServer->Disconnect(INFINITE);
		m_GngServer->Destroy();
		m_GngServer = NULL;
	}
}

void CMainWin::CreateOffscreenViewer()
{
	DestroyOffscreenViewer();

	RECT re;
	this->GetClientRect(&re);

	m_ViewerRect.left = 248;
	m_ViewerRect.top = 16;
	m_ViewerRect.right = (re.right - re.left) - 13;
	m_ViewerRect.bottom = (re.bottom - re.top) -185;

	LONG pnlViewerWidth = m_ViewerRect.right - m_ViewerRect.left;
	LONG pnlViewerHeight = m_ViewerRect.bottom - m_ViewerRect.top;

	// create two decompression buffer object instances
	m_GngDecompBuffer1 = GMPCreateDecompBuffer();
	m_GngDecompBuffer1->SetBufferSize(pnlViewerWidth, pnlViewerHeight, dbfRGB32);
	m_GngDecompBuffer2 = GMPCreateDecompBuffer();
	m_GngDecompBuffer2->SetBufferSize(pnlViewerWidth, pnlViewerHeight, dbfRGB32);

	m_NewPicDecompressed = FALSE;

	m_PicCounter = 0;

	// create the offscreen viewer object instance
	m_GngOffscreenViewer = GMPCreateOffscreenViewer(m_GngDecompBuffer1);
	
    // do custom draw over the image in the decompression buffer
	m_GngOffscreenViewer->SetCustomDrawCallBack(CustomDrawCallBackExCB, this);

	TViewTextParams Params;
	m_GngOffscreenViewer->GetTextParams(Params);
	bool IsChecked = (IsDlgButtonChecked(IDC_CBDISPLAYTEXT) == BST_CHECKED);
	if(IsChecked)
	{
		// display text over the image in the decompression buffer
		Params.InsertPicInfo = true;
		Params.FontSize      = 15;
		Params.Position      = tpTopLeft;
	}
	else
		Params.InsertPicInfo = false;
	m_GngOffscreenViewer->SetTextParams(Params);

	IsChecked = (IsDlgButtonChecked(IDC_CBSCALETOSIZE) == BST_CHECKED);
	m_GngOffscreenViewer->SetOffscreenViewerSize(pnlViewerWidth, pnlViewerHeight, IsChecked);
	m_GngOffscreenViewer->Refresh();

    // set callbacks of the offscreen viewer objects
	m_GngOffscreenViewer->SetOffscreenViewerCallBack(NewOffscreenImageCallbackCB, this);
	m_GngOffscreenViewer->SetOffscreenViewerAcceptCallBack(NewOffscreenImageAcceptCallbackCB, this);
}

void CMainWin::DestroyOffscreenViewer()
{
	if(m_GngOffscreenViewer != NULL)
	{
		m_GngOffscreenViewer->Disconnect(TRUE);
		m_GngOffscreenViewer->SetCustomDrawCallBack(NULL, NULL);
		m_GngOffscreenViewer->SetOffscreenViewerCallBack(NULL, NULL);
		m_GngOffscreenViewer->SetOffscreenViewerAcceptCallBack(NULL, NULL);
		m_GngOffscreenViewer->Destroy();
		m_GngOffscreenViewer = NULL;
	}
	if(m_GngDecompBuffer1 != NULL)
	{
		m_GngDecompBuffer1->Destroy();
		m_GngDecompBuffer1 = NULL;
	}
	if(m_GngDecompBuffer2 != NULL)
	{
		m_GngDecompBuffer2->Destroy();
		m_GngDecompBuffer2 = NULL;
	}
    m_NewPicDecompBuffer = NULL;
    m_NewPicDecompressed = false;
}

void CMainWin::FillMediaChannelsList()
{
	if(m_GngServer == NULL)
		return;

	ClearMediaChannelsList();

    std::vector<TMediaChannelRecordEx> MediaChannelList;

    GMPQueryMediaChannelList(m_GngServer, ctGngServer, mtServer, MediaChannelList);

	
	
	

    for(std::vector<TMediaChannelRecordEx>::iterator it = MediaChannelList.begin(); it != MediaChannelList.end(); ++it)
    {
        if((*it).IsActive)
        {
		    // create a media channel descriptor and store it in the listbox
            TMediaChannelDescriptor* NewMediaChannel = new TMediaChannelDescriptor((*it).ChannelID, (*it).GlobalNumber, (*it).Desc, (*it).Name);
            int NewIndex = lbMediaChannels.AddString(NewMediaChannel->m_Name.c_str());
		    lbMediaChannels.SetItemDataPtr(NewIndex, NewMediaChannel);
			
        }
		
		
    }

// alternative: if the GngMediaPlayer.DLL can not be used, fetch all available media channels with the help of DBI functions:

/*

	// create a setup object instance
	m_GngSetup = m_GngServer->CreateRegistry();
	if(m_GngSetup != NULL)
	{
		// define an array for the setup read request
		TGngSetupReadRequest SetupReadRequest[1];
		SetupReadRequest[0].NodeName = "/";
		// read the setup data from the server
		m_GngSetup->ReadNodes(SetupReadRequest, 1);
		// define an array for the GUIDs of the available media channels
		std::vector<GUID> MediaChannelGuids;
		// get the media channel GUIDs out of the setup data
	    m_GngSetup->GetMediaChannels(MediaChannelGuids);
		// get the data of each single media channel
        for(std::vector<GUID>::iterator it = MediaChannelGuids.begin(); it != MediaChannelGuids.end(); ++it)
		{
			__int64			MappedID;
			unsigned int	GlobalNumber;
			bool			Active;
			std::wstring	Name;
			std::wstring	Description;

			m_GngSetup->GetMediaChannelSettings(*it, MappedID, GlobalNumber, Active, Name, Description);
			if(Active)
			{
				// create a media channel descriptor and store it in the listbox
				TMediaChannelDescriptor* NewMediaChannel = new TMediaChannelDescriptor(MappedID, GlobalNumber, Description, Name);
				int NewIndex = lbMediaChannels.AddString(Name.c_str());
				lbMediaChannels.SetItemDataPtr(NewIndex, NewMediaChannel);
			}
		}

		m_GngSetup->Destroy();
		m_GngSetup = NULL;
	}

*/

	if(lbMediaChannels.GetCount() > 0)
		lbMediaChannels.SetCurSel(0);
}

void CMainWin::ClearMediaChannelsList()
{
	while(lbMediaChannels.GetCount() > 0)
	{
		if(lbMediaChannels.GetItemDataPtr(0) != NULL)
		{
			// remove the media channel descriptor stored in the listbox
			TMediaChannelDescriptor* MediaChannel;
			MediaChannel = reinterpret_cast<TMediaChannelDescriptor*>(lbMediaChannels.GetItemDataPtr(0));
			delete MediaChannel;
			lbMediaChannels.SetItemDataPtr(0, NULL);
			lbMediaChannels.DeleteString(0);
		}
	}
}

BOOL CMainWin::GetSelectedMediaChannel(TMediaChannelDescriptor*& SelectedMediaChannel)
{
	int SelectedItemIndex = lbMediaChannels.GetCurSel();

	if(SelectedItemIndex >= 0)
	{
		SelectedMediaChannel  = reinterpret_cast<TMediaChannelDescriptor*>(lbMediaChannels.GetItemDataPtr(SelectedItemIndex));
		return TRUE;
	}
	else
		return FALSE;
}

void CMainWin::OffscreenViewerConnectDB(TPlayMode APlayMode)
{
	if(m_GngServer == NULL)
		return;

	if(m_GngOffscreenViewer == NULL)
		CreateOffscreenViewer();

	TViewerStatus ViewerStatus;
	m_GngOffscreenViewer->GetStatus(ViewerStatus);

	TMediaChannelDescriptor* SelectedMediaChannel;

	if(GetSelectedMediaChannel(SelectedMediaChannel))
	{
		if(ViewerStatus.MediaChID != SelectedMediaChannel->m_MediaChannelID)
		{
			// initialize the offscreen viewer parameters

			TMPConnectData ViewerConnectData;
			memset(&ViewerConnectData, 0, sizeof(ViewerConnectData));
			ViewerConnectData.StructSize = sizeof(ViewerConnectData);
			ViewerConnectData.Connection = reinterpret_cast<void*>(m_GngServer);
			ViewerConnectData.ServerType = ctGngServer;
			ViewerConnectData.MediaType = mtServer;
			ViewerConnectData.MediaChDesc = NULL;
			ViewerConnectData.DisableAudio = true;
			ViewerConnectData.PlayLoop = false;
			ViewerConnectData.Wait = false; 
			ViewerConnectData.MediaChID = SelectedMediaChannel->m_MediaChannelID;
			
			
			
			
			// connect the offscreen viewer with a media channel and set its playmode (stream, ...)
			m_GngOffscreenViewer->ConnectDB(ViewerConnectData, APlayMode, NULL, NULL, NULL);
			

		
			
		}
		else
		{
			// only set the playmode of the allready connected viewer
			m_GngOffscreenViewer->SetPlayMode(APlayMode);
		}
	}
}





bool CMainWin::CustomDrawCallBackEx(HDC ViewerDC, const TRect& ClientRect, const TRect& SrcRect,
									const TPicData& PicData, const TViewerStatus& ViewerStatus, 
									const TEventData* MscEventData)
{
	if (PicData.GngMediaDesc !=0 ){
		if(m_DoCustomDraw)
		{
			CSingleLock singleLock(&m_CSPicCounter);
			singleLock.Lock();
			__int64 PicCounter = m_PicCounter;
			singleLock.Unlock();

			if((PicCounter % 2) == 0)
			{
				CString s = CString("");
				s.Format(L"Counter: %d ", PicCounter);

				SIZE TextSize;
				if(GetTextExtentPoint32(ViewerDC, s.GetBuffer(), s.GetLength(), &TextSize))
				{
					int x = ((ClientRect.right - ClientRect.left) / 2) - (TextSize.cx / 2);
					int y = ((ClientRect.bottom - ClientRect.top) / 2) - (TextSize.cy / 2);
					int OldBkMode = SetBkMode(ViewerDC, TRANSPARENT);
					TextOut(ViewerDC, x, y, s.GetBuffer(), s.GetLength());
					SetBkMode(ViewerDC, OldBkMode);
				}
				return true;
			}
			else
				return false;
		}
		else
		{
			// force the erasing of the internal custom draw buffer, after custom draw has been deactivated
			if(m_CustomDrawFlagChanged)
				return true;
			else
				return false;
		}
		m_CustomDrawFlagChanged = false;
	}
	return false;
}

bool CMainWin::NewOffscreenImageCallback(const HGngDecompBuffer OffscreenBufferHandle, const TRect& SrcRect,
										 const TPicData& PicData, const TViewerStatus& ViewerStatus,
										 const TEventData* MscEventData)
{
	
	if (PicData.GngMediaDesc!=0 ){
		CSingleLock singleLockPicCounter(&m_CSPicCounter);
		singleLockPicCounter.Lock();

		// increase the picture counter
		m_PicCounter++;
		singleLockPicCounter.Unlock();

		// new decompressed picture available
		m_CSBuffer.Lock();

		m_NewPicDecompressed = true;
		if(OffscreenBufferHandle == m_GngDecompBuffer2)
		{
			m_CSBuffer2.Unlock();
			m_NewPicDecompBuffer = m_GngDecompBuffer2;
		}
		else
		{
			m_CSBuffer1.Unlock();
			m_NewPicDecompBuffer = m_GngDecompBuffer1;
		}
		m_CSBuffer.Unlock();

		// notify main thread 
		this->InvalidateRect(&m_ViewerRect, FALSE);
	}
	return true;
}

bool CMainWin::NewOffscreenImageAcceptCallback(HGngDecompBuffer& OffscreenBufferHandle, const TPicData& PicData,
											   const TViewerStatus& ViewerStatus, const TEventData* MscEventData)
{
	if (PicData.GngMediaDesc!=0 ){
		// switch between the both decompression buffers
		if(OffscreenBufferHandle == m_GngDecompBuffer2)
		{
			m_CSBuffer1.Lock();
			OffscreenBufferHandle = m_GngDecompBuffer1;
		}
		else
		{
			m_CSBuffer2.Lock();
			OffscreenBufferHandle = m_GngDecompBuffer2;
		}
	}
	return true;
	
}



BOOL CApp::InitInstance()

{
	try
	{
		m_pMainWnd = new CMainWin;

		m_pMainWnd->ShowWindow(m_nCmdShow);

		m_pMainWnd->UpdateWindow();

		return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}


BEGIN_MESSAGE_MAP(CMainWin, CFrameWnd)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTNCONNECT, btnConnectClicked)
	ON_BN_CLICKED(IDC_BTNDISCONNECT, btnDisconnectClicked)
	ON_BN_CLICKED(IDC_BTNLIVE, btnLiveClicked)
	ON_BN_CLICKED(IDC_BTNSTOP, btnStopClicked)
	ON_BN_CLICKED(IDC_BTNFORWARD, btnForwardClicked)
	ON_BN_CLICKED(IDC_BTNBACKWARD, btnBackwardClicked)
	ON_BN_CLICKED(IDC_BTNEXPORTSINGLEPICTURE, btnExportSinglePictureClicked)
	ON_BN_CLICKED(IDC_CBSCALETOSIZE, cbScaleToSizeClicked)
	ON_BN_CLICKED(IDC_CBDOCUSTOMDRAW, cbDoCustomDrawClicked)
	ON_BN_CLICKED(IDC_CBDISPLAYTEXT, cbDisplayTextClicked)
	ON_BN_CLICKED(IDC_BTNBOD, btnBODClicked)
	ON_BN_CLICKED(IDC_BTNEOD, btnEODClicked)
	ON_BN_CLICKED(IDC_BTNFRAMEDISTANCE, btnOverrideLiveStreamFrameDistanceClicked)

	ON_BN_CLICKED(IDC_ENABLEPHAWK, cbEnablePhawkClicked)
	ON_BN_CLICKED(IDC_BTNRADIOFOG, cbPhawkFOGClicked)
	ON_BN_CLICKED(IDC_BTNRADIOLOWLIGHT, cbPhawkLOWLIGHTClicked)
	ON_BN_CLICKED(IDC_BTNRADIORAIN, cbPhawkRAINClicked)
	ON_BN_CLICKED(IDC_BTNRADIOSNOW, cbPhawkSNOWClicked)


END_MESSAGE_MAP()

CApp App;
