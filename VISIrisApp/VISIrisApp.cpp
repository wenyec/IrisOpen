//////////////////////////////////////////////////////////////////////////
//
// winmain.cpp : Application entry-point
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <iomanip>
#include <Dshow.h>
#include <strmif.h>
//#include <ks.h>
//#include <ksproxy.h>
#include <vidcap.h>
//#include <ksmedia.h>

#include <comdef.h>

#include <objbase.h>

#include "VISIrisApp.h"
#include "IrisProc.h"
#include "MFCaptureD3D.h"
#include "resource.h"
#include "PictureCtrl.h"
#include "ImageConvert.h"
#include "GetDevice.h"
#include "VIS5mpBWExp.h"
#include "VIS5mpBWEdgeEn.h"
#include "VIS5mpBWGam2DN.h"
#include "VISImageProc.h"
#include "VIS2mpColFocusZoom.h"
#include "VIS5MPColSupWBL.h"
#include "VISImpageProc5mpCol.h"

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------
#define ABS(x) (((x) > 0) ? (x) : -(x))

// window messages
#define WM_FGNOTIFY WM_USER+1

/* window buttons */
const int IDC_BTN_ENROLL = 8201;
const int IDC_BTN_SAVEG = 8202;
const int IDC_BTN_MATCH = 8203;
const int WS_DISPLY_IRIS = 8204;
static BYTE  *pBmpR = NULL;
static BYTE  *pBmpL = NULL;
BOOLEAN saveImageFlg = TRUE;
BYTE*  cvtImage2gray(BYTE* pData);
//const int IDC_BTN_SAVEG = 8202;
//const int IDC_BTN_MATCH = 8203;


#define BORDER 10

//#undef DEBUG
//#define DEBUG

#ifdef  DEBUG
//#define REGISTER_FILTERGRAPH
#endif


// Include the v6 common controls in the manifest
#pragma comment(linker, \
	"\"/manifestdependency:type='Win32' "\
	"name='Microsoft.Windows.Common-Controls' "\
	"version='6.0.0.0' "\
	"processorArchitecture='*' "\
	"publicKeyToken='6595b64144ccf1df' "\
	"language='*'\"")

//Videoloy Extension control
//void	InitProcAmp(IMFActivate *pActivate);
//void	InitCameraControl(IMFActivate *pActivate);
void	createMFInterfaceVideoSettings();

BOOL InitCapFilters();
void FreeCapFilters();
BOOL StopPreview();
BOOL StartPreview();
void TearDownGraph();
void RemoveDownstream(IBaseFilter *pf);
BOOL MakeBuilder();
BOOL MakeGraph();
void ResizeWindow(int w, int h);
BOOL BuildPreviewGraph();
BOOL StopCapture();

HRESULT getExtionControlPropertySize(ULONG PropertyId, ULONG *pulSize);
HRESULT getExtionControlProperty(ULONG PropertyId, ULONG ulSize, BYTE pValue[]);
HRESULT setExtionControlProperty(ULONG PropertyId, ULONG ulSize, BYTE pValue[]);
HRESULT getExtionControlProperty(int ID, int *Value);

// Standard Controls
HRESULT	getStandardControlPropertyRange(long PropertyID, long *lMin, long *lMax, long *lStep, long *lDefault, long *lCaps);
HRESULT	getStandardControlPropertyCurrentValue(long PropertyID, long *currValue, long *lCaps);

//Camera Terminal Controls
HRESULT	getCameraTerminalControlPropertyRange(long PropertyID, long *lMin, long *lMax, long *lStep, long *lDefault, long *lCaps);
HRESULT	getCameraTerminalControlPropertyCurrentValue(long PropertyID, long *currValue, long *lCaps);
void GetCurrentStillFormat(eMediaType *MediaType, TCHAR* Format); //add for mjpg image capture
void	AddDevicesToMenu();
void	IMonRelease(IMoniker *&pm);

void	readSnapPath();
void	writeSnapPath();

BOOL    InitializeApplication();
BOOL    InitializeWindow(HWND *pHwnd, HINSTANCE hInstance);
void    CleanUp();
INT     MessageLoop(HWND hwnd);
void ErrMsg(LPTSTR sz, ...);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK I2CControlDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ImageCaptureDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CameraRecoveryParamDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK LoginDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ROIControlDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ImageResSetDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
/* 2MP color */
INT_PTR CALLBACK CameraControlDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ZoomControlDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK IrisControlDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DayNightSettingsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NoiseReductionDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void    ShowErrorMessage(PCWSTR format, HRESULT hr);

// Window message handlers
BOOL    OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void    OnClose(HWND hwnd);
void    OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void    OnSize(/*HWND hwnd,*/ WPARAM wParam, LPARAM lParam/*, UINT state*/);
//void    OnDeviceChange(HWND hwnd, DEV_BROADCAST_HDR *pHdr);

// Command handlers
void    OnChooseDevice(HWND hwnd, BOOL bPrompt);
void	OnCameraControlMenu(HWND hwnd);   // Shutter and BLC
void    OnCameraControlMenu2Mcl(HWND hwnd);
void	OnDayNightMenu(HWND hwnd);
void	OnVideoQualityControlMenu(HWND hwnd);
void	OnVideoQualityControlMenu_5mpcol(HWND hwnd);
void	On2DNoiseReduction(HWND hwnd);    // Gamma and 2DNR 
void	On3DNoiseReduction2mpCol(HWND hwnd); // 3DNR
void	OnEdgeEnhanment(HWND hwnd);    // Edge Enhancement ... 
void    On5MPColSuppWBL(HWND hwnd);    // color suppression and WBL settings (5MP color)
void	OnAboutMenu(HWND hwnd);
void	OnI2CControlMenu(HWND hwnd);
void	OnZoomControlMenu(HWND hwnd);
void	OnIrisControlMenu(HWND hwnd);
void	OnImageCaptureMenu(HWND hwnd);
void	OnCameraRecoveryParamMenu(HWND hwnd);
void	OnVideoStreamingSetting(HWND hwnd);
void    OnROISetting(HWND hwnd);
void	OnImageResSetMenu(HWND hwnd);
HRESULT saveParamClicked(HWND hwnd);
HRESULT	resetParamClicked(HWND hwnd);

void    OnInitImageCaptureDialog(HWND hwnd);
BOOL	GetFolderSelection(HWND hWnd);
HRESULT	takeSnapShotClicked(HWND hwnd);
void	readSnapCount();
void	writeSnapCount();
HRESULT stillTrigger();
HRESULT setStilFmat(HWND hwnd, DWORD Width, DWORD Height);
/* for misumi=e-con capture filter features setting */
void ShowCapFilterPropPage(HWND hwnd);

// Constants 
const WCHAR CLASS_NAME[] = L"MFCapture Window Class";
const WCHAR WINDOW_NAME[] = L"Videology Iris ID";
const WCHAR WINDOW_NAME1[3][16] = { L"Test1", L"Test0", L"Test2" };

const GUID EXTGUID = { 0x3757CA7A, 0x1AA3, 0x495B, { 0x96, 0x8D, 0x8E, 0x36, 0x1F, 0x96, 0x76, 0x50 } };
const char BrandNameFile[] = "BrandName.txt";
const char BrandLogoFile[] = "BrandLogo.bmp";
const char snapCountFile[] = "snapCount.txt";
const char irisModeFile[] = "irisMode.txt";
const char snapPathFile[] = "snapPath.txt";
const char imageResFile[] = "ImageRes.txt"; //keep the current Res. setting for next openning.

// Global variables
HDEVNOTIFY  g_hdevnotify = NULL;
static int snapshotIndex = 1;

HWND ghwndAppMain;
HWND ghwndAppSub;
HWND ghwndButW;
HWND ghwndSub[3];
HWND ghwndBut[3];
HINSTANCE   ghInst;

int indexOf_shift(char* base, char* str, int startIndex);
int lastIndexOf(char* base, char* str);

//-------------------------------------------------------------------
// WinMain
//
// Application entry-point. 
//-------------------------------------------------------------------

char pathStr[MAX_PATH];
char pathExe[MAX_PATH];
char pathSnapImg[MAX_PATH];
char logMessage[2000];

INT WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, INT nCmdShow)
{
	HWND hwnd = 0;
	// get Exe Directory path
	HMODULE hModule = GetModuleHandleW(NULL); //get the module of the application.
	WCHAR path[MAX_PATH];
	WCHAR snapPath[32];
	DWORD size = 32;
	char bstr[32];
	char DefChar = ' ';

	GetModuleFileNameW(hModule, path, MAX_PATH);
	//convert from wide char to narrow char array
	//char pathStr[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, path, -1, pathStr, MAX_PATH, &DefChar, NULL);

	// change .exe to .log
	char *pch;
	pch = strstr(pathStr, ".exe");
	strncpy(pch, ".log", 4);

	int lastIndex = lastIndexOf(pathStr, "\\");
	if (lastIndex != -1)
	{
		strncpy(pathExe, pathStr, lastIndex + 1);
		pathExe[lastIndex + 1] = '\0';
	}

	SHGetSpecialFolderPath(HWND_DESKTOP, snapPath, CSIDL_DESKTOP, FALSE);
	WideCharToMultiByte(CP_ACP, 0, snapPath, -1, bstr, size, &DefChar, NULL);

	strcpy(pathSnapImg, bstr);
	strncpy(&pathSnapImg[lastIndexOf(pathSnapImg, "\0")], "\\\0", sizeof("\\\0"));
	//readSnapPath();
	writeSnapPath();

	(void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
	//InitializeApplication();
	/* test */

	//wc.style = 0;
	//wc.lpfnWndProc = (WNDPROC)WindowProc;
	//wc.cbClsExtra = 0;
	//wc.cbWndExtra = 0;
	//wc.hInstance = hInstance;
	//wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FX3_CAM));// LoadIcon(hInstance, TEXT("spintesticon"));
	//wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//wc.hbrBackground = GetStockObject(WHITE_BRUSH);
	//wc.lpszMenuName = (LPTSTR)TEXT("Menu");
	//wc.lpszClassName = (LPTSTR)TEXT("Main");

	//wc.lpfnWndProc = WindowProc;
	//wc.hInstance = hInstance;// GetModuleHandle(NULL);
	//wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//wc.lpszClassName = CLASS_NAME;
	//wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU_5MPBW);
	/*MAKEINTRESOURCE: converts an integer value to a resource type compatible with the resource-management functions.
	(the string containing teh name of the resource) */
	//wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_FX3_CAM));


	//if (!RegisterClass(&wc))
	//{
	//	MessageBox(NULL,
	//		L"register class wc",
	//		TEXT("TEST"), MB_OK | MB_ICONEXCLAMATION);
	//	return(FALSE);
	//}

	//ghInst = hInstance;
	//if (!(hwnd = CreateWindow(
	//	TEXT("Main"),//CLASS_NAME,
	//	WINDOW_NAME,
	//	WS_OVERLAPPEDWINDOW,// | WS_HSCROLL | WS_VSCROLL,
	//	CW_USEDEFAULT,
	//	CW_USEDEFAULT,
	//	CW_USEDEFAULT,
	//	CW_USEDEFAULT,
	//	NULL,
	//	NULL,
	//	hInstance,//GetModuleHandle(NULL),
	//	NULL
	//	))

	/*hwnd = CreateWindow(TEXT("Main"),
	TEXT("Maintest"),
	WS_OVERLAPPEDWINDOW,
	CW_USEDEFAULT, CW_USEDEFAULT,
	CW_USEDEFAULT, CW_USEDEFAULT,
	NULL, NULL, ghInst, NULL)))*/
	//	return 0;


	//
	// Create a couple of SpinCube custom controls, we'll size them later in
	//   the WM_SIZE message handler
	//

	//for (i = 0; i < 3; i++)

	//	ghwndSub[i] = CreateWindow(TEXT("Spincube"), TEXT("test"),
	//	WS_VISIBLE | WS_CHILD,
	//	0, 0, 0, 0, hwnd, NULL, NULL, NULL);


	//
	// Delete the SS_ERASE to the 1st & 4th controls so we get the
	//   trailing cubes effect.
	//


	/* end of the test */
	if (InitializeApplication() && InitializeWindow(&hwnd, hInstance))
	{
		//		if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
		//		{
		//			;
		//		}
		//		AfxGetInstanceHandle();

		MessageLoop(hwnd);
	}

	CleanUp();

	return 0;
}

void printLogMessage(const char* strMessage)
{
	FILE *fpDbgLog;
	fpDbgLog = fopen(pathStr, "a");
	if (fpDbgLog)
	{
		fprintf(fpDbgLog, "%s", strMessage);
		fclose(fpDbgLog);
	}
}

int indexOf_shift(char* base, char* str, int startIndex) {
	int result;
	int baselen = strlen(base);
	// str should not longer than base
	if (strlen(str) > baselen || startIndex > baselen) {
		result = -1;
	}
	else {
		if (startIndex < 0) {
			startIndex = 0;
		}
		char* pos = strstr(base + startIndex, str);
		if (pos == NULL) {
			result = -1;
		}
		else {
			result = pos - base;
		}
	}
	return result;
}

int lastIndexOf(char* base, char* str) {
	int result;
	// str should not longer than base
	if (strlen(str) > strlen(base)) {
		result = -1;
	}
	else {
		int start = 0;
		int endinit = strlen(base) - strlen(str);
		int end = endinit;
		int endtmp = endinit;
		while (start != end) {
			start = indexOf_shift(base, str, start);
			end = indexOf_shift(base, str, end);

			// not found from start
			if (start == -1) {
				end = -1; // then break;
			}
			else if (end == -1) {
				// found from start
				// but not found from end
				// move end to middle
				if (endtmp == (start + 1)) {
					end = start; // then break;
				}
				else {
					end = endtmp - (endtmp - start) / 2;
					if (end <= start) {
						end = start + 1;
					}
					endtmp = end;
				}
			}
			else {
				// found from both start and end
				// move start to end and
				// move end to base - strlen(str)
				start = end;
				end = endinit;
			}
		}
		result = start;
	}
	return result;
}


// Class to hold the callback function for the Sample Grabber filter.
class SampleGrabberCallback : public ISampleGrabberCB
{
public:
	// Fake referance counting.
	STDMETHODIMP_(ULONG) AddRef() { return 1; }
	STDMETHODIMP_(ULONG) Release() { return 2; }

	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject)
	{
		if (NULL == ppvObject) return E_POINTER;
		if (riid == __uuidof(IUnknown))
		{
			*ppvObject = static_cast<IUnknown*>(this);
			return S_OK;
		}
		if (riid == __uuidof(ISampleGrabberCB))
		{
			*ppvObject = static_cast<ISampleGrabberCB*>(this);
			return S_OK;
		}
		return E_NOTIMPL;
	}

	STDMETHODIMP SampleCB(double Time, IMediaSample *pSample)
	{
		return E_NOTIMPL;
	}

	struct IRISBMPINFO
	{
		BITMAPINFOHEADER	bmiHeader;
		RGBQUAD				bmiColors[256];
	};

	STDMETHODIMP BufferCB(double Time, BYTE *pBuffer, long BufferLen)
	{
		if (saveImageFlg)
		{
			_AMMediaType mt;
			DWORD dwWritten = 0;
			HRESULT hr = gcap.pSampleGrabber->GetConnectedMediaType((_AMMediaType *)&mt);

			if (!SUCCEEDED(hr))
				return VFW_E_INVALIDMEDIATYPE;

			if ((mt.majortype != MEDIATYPE_Video) ||
				(mt.formattype != FORMAT_VideoInfo) ||
				(mt.cbFormat < sizeof(VIDEOINFOHEADER)) ||
				(mt.pbFormat == NULL))
			{
				return VFW_E_INVALIDMEDIATYPE;
			}

			wchar_t snapHwTrigger[1000];
			wchar_t pathExeWch[2 * MAX_PATH];
			wchar_t tempFileName[80];

			if (pathExeWch)
			{
				memset(pathExeWch, 0, sizeof(pathExeWch));
				MultiByteToWideChar(CP_UTF8, 0, &pathSnapImg[0], (int)strlen(pathSnapImg), &pathExeWch[0], sizeof(pathExeWch));
			}

			wcscpy(snapHwTrigger, pathExeWch);

			SYSTEMTIME st;
			//GetSystemTime(&st);
			GetLocalTime(&st);
			if (gcap.stillsubType == MEDIA_MJPEG)
				hr = StringCbPrintf(tempFileName, sizeof(tempFileName), L"SnapShot%d%d%d_%d_%d_%d_%d.jpg", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
			else
				hr = StringCbPrintf(tempFileName, sizeof(tempFileName), L"SnapShot%d%d%d_%d_%d_%d_%d.bmp", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

			wcscat(snapHwTrigger, tempFileName);

			HANDLE hf = CreateFile(snapHwTrigger, GENERIC_WRITE,
				FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
			if (hf == INVALID_HANDLE_VALUE)
			{
				return E_FAIL;
			}
			if (gcap.stillsubType == MEDIA_MJPEG){
				// Create jpeg file
				BOOL bWriteFileResult = FALSE;
				bWriteFileResult = WriteFile(hf, pBuffer, BufferLen, &dwWritten, NULL);

			}
			else{
				// Create bitmap structure
				BYTE  *pBmp = NULL;
				IRISBMPINFO			bmpInfo;

				long cbBitmapInfoSize = mt.cbFormat - SIZE_PREHEADER;
				VIDEOINFOHEADER *pVideoHeader = (VIDEOINFOHEADER*)mt.pbFormat;

				BITMAPFILEHEADER bfh;
				ZeroMemory(&bfh, sizeof(bfh));
				bfh.bfType = 'MB';  // Little-endian for "BM".
				bfh.bfSize = sizeof(bfh) + BufferLen + cbBitmapInfoSize;
				bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + cbBitmapInfoSize;

				// Write the file header.
				DWORD dwWritten = 0;
				WriteFile(hf, &bfh, sizeof(bfh), &dwWritten, NULL);
				WriteFile(hf, HEADER(pVideoHeader), cbBitmapInfoSize, &dwWritten, NULL);
				WriteFile(hf, pBuffer, BufferLen, &dwWritten, NULL);

			}
			CloseHandle(hf);
		}
		else{
			/* display the image to Iris image windows: right and left */ //TODO...

			// 1. copy the video data into a memory pBmpR
			pBmpR = new BYTE[BufferLen];
			memcpy(pBmpR, pBuffer, BufferLen);

			//2. sends messge to the WindowProc 
			SendMessage(ghwndAppMain, WS_DISPLY_IRIS, (WPARAM)pBmpR, (LPARAM)BufferLen);
			/* end of the image display */
		}
		return S_OK;

	}
};

// Global instance of the class.
SampleGrabberCallback g_StillCapCB;

void createMFInterfaceVideoSettings()
{

	HRESULT hr = S_OK;
	ksNodeTree.isOK = false;
	ksNodeTree.isOKpProcAmpKsControl = false;
	ksNodeTree.isOKpProcAmp = false;

	if (gcap.pVCap != 0)
	{

		hr = gcap.pVCap->QueryInterface(__uuidof(IAMVideoProcAmp), (VOID**)&ksNodeTree.pProcAmp);
		if (SUCCEEDED(hr))
			ksNodeTree.isOKpProcAmp = TRUE;
		else
			ksNodeTree.isOKpProcAmp = FALSE;

		hr = gcap.pVCap->QueryInterface(__uuidof(IAMCameraControl), (VOID**)&ksNodeTree.pCamControl);
		if (SUCCEEDED(hr))
			ksNodeTree.isOKpCamControl = TRUE;
		else
			ksNodeTree.isOKpCamControl = FALSE;

		hr = gcap.pVCap->QueryInterface(__uuidof(IAMVideoControl), (VOID**)&ksNodeTree.pVideoControl);
		if (SUCCEEDED(hr))
			ksNodeTree.isOKpVideoControl = TRUE;
		else
			ksNodeTree.isOKpVideoControl = FALSE;


		if (SUCCEEDED(hr))
		{

			DWORD uiNumNodes;
			GUID guidNodeType;

			IKsTopologyInfo *pKsTopologyInfo = nullptr;
			IUnknown *pUnk = nullptr;
			IKsControl *pKsControl = nullptr;

			hr = gcap.pVCap->QueryInterface(__uuidof(IKsTopologyInfo),
				(VOID**)&pKsTopologyInfo);

			if (SUCCEEDED(hr))
			{
				// get nodes number in usb video device capture filter
				if (pKsTopologyInfo->get_NumNodes(&uiNumNodes) == S_OK)
				{
					// go thru all nodes searching for the node of the KSNODETYPE_DEV_SPECIFIC type,
					// node of this type - represents extension unit of the USB device

#ifdef DEBUG
					sprintf(logMessage, " \nFunction : createMFInterfaceVideoSettings \t Msg : uiNumNodes %d", uiNumNodes);
					printLogMessage(logMessage);
#endif
					for (UINT i = 0; i < uiNumNodes + 1; i++)
					{
						if (pKsTopologyInfo->get_NodeType(i, &guidNodeType) == S_OK)
						{

							if (guidNodeType == KSNODETYPE_DEV_SPECIFIC)
							{
#ifdef DEBUG
								sprintf(logMessage, " \nFunction : createMFInterfaceVideoSettings \t Msg : create node instance for KSNODETYPE_DEV_SPECIFIC");
								printLogMessage(logMessage);
#endif

								// create node instance
								hr = pKsTopologyInfo->CreateNodeInstance(i, __uuidof(IUnknown), (VOID**)&pUnk);

								// get IKsControl interface from node
								if (hr == S_OK)
								{
									hr = pUnk->QueryInterface(__uuidof(IKsControl), (VOID**)&pKsControl);
								}
								else
								{
#ifdef DEBUG
									sprintf(logMessage, " \nERROR \t Function : createMFInterfaceVideoSettings \t Msg : create node instance failed for KSNODETYPE_DEV_SPECIFIC");
									printLogMessage(logMessage);
#endif

								}

								// trying to read first control of the extension unit
								if (hr == S_OK)
								{
									ksNodeTree.nodeID = i;
									ksNodeTree.pKsControl = pKsControl;
									ksNodeTree.isOK = true;

								}
								else
								{
#ifdef DEBUG
									sprintf(logMessage, " \nERROR \t Function : createMFInterfaceVideoSettings \t Msg : pUnk->QueryInterface failed for KSNODETYPE_DEV_SPECIFIC");
									printLogMessage(logMessage);
#endif
								}

							}
							else if (guidNodeType == KSNODETYPE_VIDEO_PROCESSING)
							{
#ifdef DEBUG
								sprintf(logMessage, " \nFunction : createMFInterfaceVideoSettings \t Msg : create node instance for KSNODETYPE_VIDEO_PROCESSING");
								printLogMessage(logMessage);
#endif
								// create node instance
								hr = pKsTopologyInfo->CreateNodeInstance(i, __uuidof(IUnknown), (VOID**)&pUnk);

								// get IKsControl interface from node
								if (hr == S_OK)
								{
									hr = pUnk->QueryInterface(__uuidof(IKsControl), (VOID**)&pKsControl);
								}
								else
								{
#ifdef DEBUG
									sprintf(logMessage, " \nERROR \t Function : createMFInterfaceVideoSettings \t Msg : create node instance for KSNODETYPE_VIDEO_PROCESSING failed");
									printLogMessage(logMessage);
#endif
								}

								// trying to read first control of the extension unit
								if (hr == S_OK)
								{
									ksNodeTree.nodeIDProcAmpKsControl = i;
									ksNodeTree.pProcAmpKsControl = pKsControl;
									ksNodeTree.isOKpProcAmpKsControl = true;
								}
								else
								{
#ifdef DEBUG
									sprintf(logMessage, " \nERROR \t Function : createMFInterfaceVideoSettings \t Msg : pUnk->QueryInterface failed for KSNODETYPE_VIDEO_PROCESSING");
									printLogMessage(logMessage);
#endif
								}

							}
							else if (guidNodeType == KSNODETYPE_VIDEO_CAMERA_TERMINAL)
							{
#ifdef DEBUG
								sprintf(logMessage, " \nFunction : createMFInterfaceVideoSettings \t Msg : create node instance for KSNODETYPE_VIDEO_CAMERA_TERMINAL");
								printLogMessage(logMessage);
#endif
								// create node instance
								hr = pKsTopologyInfo->CreateNodeInstance(i, __uuidof(IUnknown), (VOID**)&pUnk);

								// get IKsControl interface from node
								if (hr == S_OK)
								{
									hr = pUnk->QueryInterface(__uuidof(IKsControl), (VOID**)&pKsControl);
								}
								else
								{
#ifdef DEBUG
									sprintf(logMessage, " \nERROR \t Function : createMFInterfaceVideoSettings \t Msg : create node instance for KSNODETYPE_VIDEO_CAMERA_TERMINAL failed");
									printLogMessage(logMessage);
#endif
								}

								// trying to read first control of the extension unit
								if (hr == S_OK)
								{
									ksNodeTree.nodeIDCamControlKsControl = i;
									ksNodeTree.pCamControlKsControl = pKsControl;
									ksNodeTree.isOKpCamControlKsControl = true;
								}
								else
								{
#ifdef DEBUG
									sprintf(logMessage, " \nERROR \t Function : createMFInterfaceVideoSettings \t Msg : pUnk->QueryInterface failed for KSNODETYPE_VIDEO_CAMERA_TERMINAL");
									printLogMessage(logMessage);
#endif
								}

							}
							else if (guidNodeType == KSNODETYPE_VIDEO_STREAMING)
							{
#ifdef DEBUG
								sprintf(logMessage, " \nFunction : createMFInterfaceVideoSettings \t Msg : create node instance for KSNODETYPE_VIDEO_STREAMING");
								printLogMessage(logMessage);
#endif

								// create node instance
								hr = pKsTopologyInfo->CreateNodeInstance(i, __uuidof(IUnknown), (VOID**)&pUnk);

								// get IKsControl interface from node
								if (hr == S_OK)
								{
									hr = pUnk->QueryInterface(__uuidof(IKsControl), (VOID**)&pKsControl);
								}
								else
								{
#ifdef DEBUG
									sprintf(logMessage, " \nERROR \t Function : createMFInterfaceVideoSettings \t Msg : create node instance for KSNODETYPE_VIDEO_STREAMING failed");
									printLogMessage(logMessage);
#endif
								}

								// trying to read first control of the extension unit
								if (hr == S_OK)
								{
									ksNodeTree.nodeIDVideoStreaming = i;
									ksNodeTree.pKsControlVideoStreaming = pKsControl;
									ksNodeTree.isOKVideoStreaming = true;
								}
								else
								{
#ifdef DEBUG
									sprintf(logMessage, " \nERROR \t Function : createMFInterfaceVideoSettings \t Msg : pUnk->QueryInterface failed for KSNODETYPE_VIDEO_STREAMING");
									printLogMessage(logMessage);
#endif
								}

							}

						}
					}
				}
				else
				{
#ifdef DEBUG
					sprintf(logMessage, " \nERROR \t Function : createMFInterfaceVideoSettings \t Msg : pKsTopologyInfo->get_NumNodes(&uiNumNodes) == S_OK failed");
					printLogMessage(logMessage);
#endif
				}
			}
			else
			{
#ifdef DEBUG
				sprintf(logMessage, " \nERROR \t Function : createMFInterfaceVideoSettings \t Msg : gcap.pVCap->QueryInterface failed");
				printLogMessage(logMessage);
#endif
			}
		}
		else
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : createMFInterfaceVideoSettings \t Msg : gcap.pmVideo->BindToObject failed");
			printLogMessage(logMessage);
#endif
		}

	}
	else
	{
#ifdef DEBUG
		sprintf(logMessage, "\nERROR \t Function : createMFInterfaceVideoSettings \t Msg : pmSelDevice is NULL");
		printLogMessage(logMessage);
#endif
	}

}

void ChooseDevices(IMoniker *pmVideo, IMoniker *pmAudio)
{
#define VERSIZE 40
#define DESCSIZE 80

	int versize = VERSIZE;
	int descsize = DESCSIZE;
	WCHAR wachVer[VERSIZE] = { 0 }, wachDesc[DESCSIZE] = { 0 };
	TCHAR tachStatus[VERSIZE + DESCSIZE + 5] = { 0 };


	// they chose a new device. rebuild the graphs
	if (gcap.pmVideo != pmVideo || gcap.pmAudio != pmAudio)
	{
		if (pmVideo)
		{
			pmVideo->AddRef();
		}
		if (pmAudio)
		{
			pmAudio->AddRef();
		}

		IMonRelease(gcap.pmVideo);
		IMonRelease(gcap.pmAudio);
		gcap.pmVideo = pmVideo;
		gcap.pmAudio = pmAudio;

		if (gcap.fPreviewing)
			StopPreview();
		if (gcap.fCaptureGraphBuilt || gcap.fPreviewGraphBuilt)
			TearDownGraph();

		FreeCapFilters();
		InitCapFilters();
#if 0 //for implement dynamic menu
		/* change menu format based on the CamIndex */
		HMENU hMainMenu, hSecMenu, hThdMenu, hForMenu;
		HINSTANCE hInstance = GetModuleHandle(NULL);
		//DestroyMenu(GetMenu(ghwndApp));
		switch (gcap.CamIndex)
		{
		case CAM5MP_BW:
			/* the base menu */
			hMainMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_5MPBW));
			SetMenu(ghwndApp, hMainMenu);
			break;
		case CAM5MP_COLOR:
			/* TODO menu modify */
			hMainMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_5MPCL));
			SetMenu(ghwndApp, hMainMenu);
			break;
		case CAM2MP_COLOR:
			/* TODO menu modify */
			hMainMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_2MPCL));
			SetMenu(ghwndApp, hMainMenu);
			break;
		case CAM1D2MP_COLOR:
			/* TODO menu modify */
			hMainMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_2MPCL));
			SetMenu(ghwndApp, hMainMenu);
			break;
		case CAMINVENDO:
			/* TODO menu modify */
			break;
		case CAM5MPMISUMI:
		case CAM5MPECON:
		default:
			/* TODO menu modify */
			hMainMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENUMISU));
			SetMenu(ghwndApp, hMainMenu);
#if 1
			/* delecte the fourth node of main menu */
			hMainMenu = GetMenu(ghwndAppMain);
			DeleteMenu(hMainMenu, 3, MF_BYPOSITION);
			/* delecte the second/third node of second menu in second node of main menu */
			hSecMenu = GetSubMenu(hMainMenu, 1);
			DeleteMenu(hSecMenu, 2, MF_BYPOSITION);
			DeleteMenu(hSecMenu, 1, MF_BYPOSITION);
			//RemoveMenu(hSecMenu, 2, MF_BYCOMMAND);
			//InsertMenu(hSecMenu, 1, 0x410, ID_FORMAT_VIDEORESOLUTION, L"Video Resolution");
#endif
			break;
		}
		//CWnd *pMain = AfxGetMainWnd();
		//		if (Menu == NULL)
		//Menu GetMenu();
		//int menuNum = Menu;
		//		if (Menu != NULL && menuNum > 0)
		//		{
		//			if (gcap.CamIndex != 2)//disable VIS option menu
		//			{
		//				Menu.DeleteMenu(menuNum - 1, MF_BYPOSITION);
		//			}
		//			else{
		//				Menu->EnableMenuItem(menuNum - 1, MF_BYPOSITION);
		//			}
		//		}
#endif
		gcap.fWantPreview = TRUE;
		if (gcap.fWantPreview)   // were we previewing?
		{
			BuildPreviewGraph();
			StartPreview();
		}

		//MakeMenuOptions();      // the UI choices change per device
	}

}

HRESULT getExtionControlPropertySize(ULONG PropertyId, ULONG *pulSize)
{
	HRESULT hr = S_OK;
	try
	{

		if (ksNodeTree.isOK)
		{

			ULONG ulBytesReturned;
			KSP_NODE ExtensionProp;

			if (!pulSize) return E_POINTER;

			ExtensionProp.Property.Set = EXTGUID;
			ExtensionProp.Property.Id = PropertyId;
			ExtensionProp.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
			ExtensionProp.NodeId = ksNodeTree.nodeID;

			hr = ksNodeTree.pKsControl->KsProperty((PKSPROPERTY)&ExtensionProp, sizeof(ExtensionProp), NULL, 0, &ulBytesReturned);
			Sleep(20);
			if (hr == HRESULT_FROM_WIN32(ERROR_MORE_DATA))
			{
				*pulSize = ulBytesReturned;
				hr = S_OK;
			}
		}
		else
		{
			hr = E_FAIL;
		}
	}
	catch (...)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nERROR \t Function:getExtionControlPropertySize \t Message:Exception occur while getting size of Node[%d].", PropertyId);
		printLogMessage(logMessage);
#endif
		hr = S_FALSE;
	}

	return hr;
}

HRESULT getExtionControlProperty(ULONG PropertyId, ULONG ulSize, BYTE pValue[])
{
	HRESULT hr = S_OK;
	try
	{

		if (ksNodeTree.isOK)
		{

			KSP_NODE ExtensionProp;
			ULONG ulBytesReturned;

			ExtensionProp.Property.Set = EXTGUID;
			ExtensionProp.Property.Id = PropertyId;
			ExtensionProp.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
			ExtensionProp.NodeId = ksNodeTree.nodeID;

			hr = ksNodeTree.pKsControl->KsProperty((PKSPROPERTY)&ExtensionProp, sizeof(ExtensionProp), (PVOID)pValue, ulSize, &ulBytesReturned);
			Sleep(50);
#ifdef DEBUG
			sprintf(logMessage, "\nbmRequestType:GET \t bRequest:GET_CUR \t wValue:%ld \t wIndex:0x03\t retValue:", PropertyId);
			printLogMessage(logMessage);

			for (int i = 0; i < (int)ulBytesReturned; i++)
			{
				sprintf(logMessage, " %.2x", (PCHAR)pValue[i]);
				printLogMessage(logMessage);
			}
#endif

		}
		else
		{
			hr = E_FAIL;
		}
	}
	catch (...)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nERROR \t Function:getExtionControlProperty \t Message:Exception occur while getting value of Node[%d].", PropertyId);
		printLogMessage(logMessage);
#endif
		hr = S_FALSE;
	}

	return hr;
}

HRESULT setExtionControlProperty(ULONG PropertyId, ULONG ulSize, BYTE pValue[])
{
	HRESULT hr = S_OK;
	try
	{
		if (ksNodeTree.isOK)
		{

			KSP_NODE ExtensionProp;
			ULONG ulBytesReturned;

			ExtensionProp.Property.Set = EXTGUID;
			ExtensionProp.Property.Id = PropertyId;
			ExtensionProp.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
			ExtensionProp.NodeId = ksNodeTree.nodeID;

#ifdef DEBUG
			sprintf(logMessage, "\nbmRequestType:SET \t bRequest:SET_CUR \t wValue:%ld \t wIndex:0x03\t PutValue:", PropertyId);
			printLogMessage(logMessage);
			for (int i = 0; i < (int)ulSize; i++)
			{
				sprintf(logMessage, " %.2x", (PCHAR)pValue[i]);
				printLogMessage(logMessage);
			}
#endif
			hr = ksNodeTree.pKsControl->KsProperty((PKSPROPERTY)&ExtensionProp, sizeof(ExtensionProp), (PVOID)pValue, ulSize, &ulBytesReturned);
		}
		else
		{
			hr = E_FAIL;
		}
	}
	catch (...)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nERROR \t Function:setExtionControlProperty \t Message:Exception occur while setting Node[%d].", PropertyId);
		printLogMessage(logMessage);
#endif
		hr = S_FALSE;
	}

	return hr;
}

HRESULT	getStandardControlPropertyRange(long PropertyID, long *lMin, long *lMax, long *lStep, long *lDefault, long *lCaps)
{
	HRESULT hr = S_OK;
	if (ksNodeTree.isOKpProcAmp)
	{
		// Get the range
		hr = ksNodeTree.pProcAmp->GetRange(PropertyID, lMin, lMax, lStep, lDefault, lCaps);
#ifdef DEBUG
		sprintf(logMessage, " \nFunction : getStandardControlPropertyRange \t Msg : PropertyID %d Min %ld Max %ld Default %ld", PropertyID, *lMin, *lMax, *lDefault);
		printLogMessage(logMessage);
#endif
	}

	if (!SUCCEEDED(hr))
	{
		*lMin = 0;
		*lMax = 0;
		*lStep = 0;
		*lDefault = 0;
		*lCaps = 0;
#ifdef DEBUG
		sprintf(logMessage, "\nERROR \t Function : getStandardControlPropertyRange \t Msg : pProcAmp->GetRange failed for PropertyID %d ", PropertyID);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT	getStandardControlPropertyCurrentValue(long PropertyID, long *currValue, long *lCaps)
{
	HRESULT hr = S_OK;
	if (ksNodeTree.isOKpProcAmp)
	{
		// Get the current value.
		hr = ksNodeTree.pProcAmp->Get(PropertyID, currValue, lCaps);
		Sleep(100);
#ifdef DEBUG
		sprintf(logMessage, " \nFunction : getStandardControlPropertyCurrentValue \t Msg : PropertyID %d currValue %ld lCaps %ld", PropertyID, *currValue, *lCaps);
		printLogMessage(logMessage);
#endif
	}

	if (!SUCCEEDED(hr))
	{
		*currValue = 0;
		*lCaps = 0;
#ifdef DEBUG
		sprintf(logMessage, "\nERROR \t Function : getStandardControlPropertyCurrentValue \t Msg : pProcAmp->Get failed for PropertyID %d ", PropertyID);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT	getCameraTerminalControlPropertyRange(long PropertyID, long *lMin, long *lMax, long *lStep, long *lDefault, long *lCaps)
{
	HRESULT hr = S_OK;
	if (ksNodeTree.isOKpCamControl)
	{
		// Get the range
		hr = ksNodeTree.pCamControl->GetRange(PropertyID, lMin, lMax, lStep, lDefault, lCaps);
#ifdef DEBUG
		sprintf(logMessage, " \nFunction : getCameraTerminalControlPropertyRange \t Msg : PropertyID %d Min %ld Max %ld Default %ld", PropertyID, *lMin, *lMax, *lDefault);
		printLogMessage(logMessage);
#endif
	}

	if (!SUCCEEDED(hr))
	{
		*lMin = 0;
		*lMax = 0;
		*lStep = 0;
		*lDefault = 0;
		*lCaps = 0;
#ifdef DEBUG
		sprintf(logMessage, "\nERROR \t Function : getCameraTerminalControlPropertyRange \t Msg : pProcAmp->GetRange failed for PropertyID %d ", PropertyID);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT	getCameraTerminalControlPropertyCurrentValue(long PropertyID, long *currValue, long *lCaps)
{
	HRESULT hr = S_OK;
	if (ksNodeTree.isOKpCamControl)
	{
		// Get the current value.
		hr = ksNodeTree.pCamControl->Get(PropertyID, currValue, lCaps);
#ifdef DEBUG
		sprintf(logMessage, " \nFunction : getCameraTerminalControlPropertyCurrentValue \t Msg : PropertyID %d currValue %ld lCaps %ld", PropertyID, *currValue, *lCaps);
		printLogMessage(logMessage);
#endif
	}

	if (!SUCCEEDED(hr))
	{
		*currValue = 0;
		*lCaps = 0;
#ifdef DEBUG
		sprintf(logMessage, "\nERROR \t Function : getCameraTerminalControlPropertyCurrentValue \t Msg : pProcAmp->Get failed for PropertyID %d ", PropertyID);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

# if 0
void resize(int edge, RECT & rect)
{
	int newWidth, newHeight, tmpW, tmpH, xExtra, yExtra;
	float new_aspR;
	RECT rcC;
	int cyBorder = GetSystemMetrics(SM_CYBORDER);

	GetClientRect(ghwndAppMain, &rcC);
	xExtra = rect.right - rect.left - rcC.right;
	yExtra = rect.bottom - rect.top - rcC.bottom + cyBorder;

	switch (edge)
	{
	case WMSZ_BOTTOM:
	case WMSZ_TOP:
		newWidth = (rcC.bottom - rcC.top)*aspRetio;
		rect.right = newWidth + rcC.left + xExtra;
		break;
	case WMSZ_TOPRIGHT:
	case WMSZ_TOPLEFT:
	case WMSZ_BOTTOMRIGHT:
	case WMSZ_BOTTOMLEFT:
		tmpW = rcC.right - rcC.left;
		tmpH = rcC.bottom - rcC.top;
		newWidth = (rcC.bottom - rcC.top)*aspRetio;
		newHeight = (rcC.right - rcC.left) / aspRetio;

		new_aspR = (float)tmpW / (float)tmpH;
		if (new_aspR > aspRetio)
			rect.bottom = newHeight + rcC.top + yExtra;
		else
			rect.right = newWidth + rcC.left + xExtra;

		break;
	case WMSZ_LEFT:
	case WMSZ_RIGHT:
		newHeight = (rcC.right - rcC.left) / aspRetio;
		rect.bottom = newHeight + rcC.top + yExtra;
		break;

	}
	rect.bottom += rect.top; rect.right += rect.left;
	//rect.top = 0; rect.left = 0;
	if (gcap.pVW)
	{
		gcap.pVW->SetWindowPosition(0, 0, rcC.right, rcC.bottom);
	}

}
#endif


//-------------------------------------------------------------------
//  WindowProc
//
//  Window procedure.
//-------------------------------------------------------------------

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPMINMAXINFO pInfo;
	BYTE* pData;
	long lLen;
	switch (uMsg)
	{
		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hwnd, WM_CLOSE, OnClose);
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
		//HANDLE_MSG(hwnd, WM_SIZE, OnSize);
		//HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGING, OnWindowPosChanging);
		//case WM_SYSCOMMAND:
	case WM_SIZE:
		OnSize(wParam, lParam);
		//SendMessage(ghwndApp, WM_SIZE, SIZE_RESTORED, 0);// force a WM_SIZE message
		return TRUE;
		//case WM_SIZING:
		//resize(int(wParam), *reinterpret_cast<LPRECT>(lParam));
		//return TRUE;

		/*
		case WM_APP_PREVIEW_ERROR:
		ShowErrorMessage(L"Error", (HRESULT)wParam);
		break;
		*/
		//case WM_DEVICECHANGE:
		//OnDeviceChange(hwnd, (PDEV_BROADCAST_HDR)lParam);
		//break;

	case WM_ERASEBKGND:
		return 1;

	case WM_FGNOTIFY:
		// uh-oh, something went wrong while capturing - the filtergraph
		// will send us events like EC_COMPLETE, EC_USERABORT and the one
		// we care about, EC_ERRORABORT.
		if (gcap.pME)
		{
			LONG event;
			LONG_PTR l1, l2;
			HRESULT hrAbort = S_OK;
			BOOL bAbort = FALSE;
			while (gcap.pME->GetEvent(&event, &l1, &l2, 0) == S_OK)
			{
				gcap.pME->FreeEventParams(event, l1, l2);
				if (event == EC_ERRORABORT)
				{
					StopCapture();
					bAbort = TRUE;
					hrAbort = static_cast<HRESULT>(l1);
					continue;
				}
				else if (event == EC_DEVICE_LOST)
				{
					// Check if we have lost a capture filter being used.
					// lParam2 of EC_DEVICE_LOST event == 1 indicates device added
					//                                 == 0 indicates device removed
					if (l2 == 0)
					{
						IBaseFilter *pf;
						IUnknown *punk = (IUnknown *)l1;
						if (S_OK == punk->QueryInterface(IID_IBaseFilter, (void **)&pf))
						{
							if (AreComObjectsEqual(gcap.pVCap, pf))
							{
								pf->Release();
								bAbort = FALSE;
								StopCapture();
								TCHAR szError[100];
								HRESULT hr = StringCchCopy(szError, 100,
									TEXT("Stopping Capture (Device Lost). Select New Capture Device\0"));
								//ErrMsg(szError);
								break;
							}
							pf->Release();
						}
					}
				}
			} // end while
			if (bAbort)
			{
				if (gcap.fWantPreview)
				{
					BuildPreviewGraph();
					StartPreview();
				}
				TCHAR szError[100];
				HRESULT hr = StringCchPrintf(szError, 100, TEXT("ERROR during capture, error code=%08x\0"), hrAbort);
				//ErrMsg(szError);
			}
		}
		break;

	case WM_PAINT:
		/*PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_BACKGROUND + 1));
		EndPaint(hwnd, &ps);*/
		break;
	case WS_DISPLY_IRIS:
		pData = (BYTE*)wParam;
		lLen = (long)lParam;
		BYTE* pRawData = new BYTE[lLen];
		memcpy(pRawData, pData, lLen);
		delete[] pData;
		pData = NULL;
# if 1 //remove for create bmp with the image covert class.
		_AMMediaType mt;
		DWORD dwWritten = 0;
		HRESULT hr = gcap.pSampleGrabber->GetConnectedMediaType((_AMMediaType *)&mt);

		if (!SUCCEEDED(hr))
			return VFW_E_INVALIDMEDIATYPE;

		VIDEOINFOHEADER *pVideoHeader = (VIDEOINFOHEADER*)mt.pbFormat;
		BITMAPINFO* pBitmapInfo = (BITMAPINFO*)HEADER(pVideoHeader);
#endif
		/* display the bitmap data on the iris window */
		HDC hdc_Rx = GetDC(ghwndSub[1]), hdc_Lx = GetDC(ghwndSub[2]);
		RECT rect;
		LONG left = 0, top = 0;
		float wWid, wHig;
		/* for Right Iris image */
		GetClientRect(ghwndSub[1], &rect);
		wWid = rect.right - rect.left;
		wHig = rect.bottom - rect.top;
			
		if (aspRetio <= (float)(wWid / wHig/*rc.right / rc.bottom*/)){//the heigh is used
			left = (rect.right - rect.bottom*aspRetio) / 2;
			rect.right = rect.bottom*aspRetio;
		}
		else{ //the bottom is used
			top = (rect.bottom - rect.right / aspRetio) / 2;
			rect.bottom = rect.right / aspRetio;
		}

		SetStretchBltMode(hdc_Rx, HALFTONE);
		StretchDIBits(hdc_Rx, left, top, rect.right - rect.left, rect.bottom - rect.top,
			0, 0, gcap.stillWidth, gcap.stillHeight, (void*)/*pRawData*/pRawData, pBitmapInfo, DIB_RGB_COLORS, SRCCOPY/*WHITENESS/*LR_LOADFROMFILE*/);

		ReleaseDC(ghwndSub[1], hdc_Rx);

		/* for Left Iris image*/ //TODO real second still image shot.
		GetClientRect(ghwndSub[2], &rect);
		wWid = rect.right - rect.left;
		wHig = rect.bottom - rect.top;

		if (aspRetio <= (float)(wWid / wHig/*rc.right / rc.bottom*/)){//the heigh is used
			left = (rect.right - rect.bottom*aspRetio) / 2;
			rect.right = rect.bottom*aspRetio;
		}
		else{ //the bottom is used
			top = (rect.bottom - rect.right / aspRetio) / 2;
			rect.bottom = rect.right / aspRetio;
		}

		SetStretchBltMode(hdc_Lx, HALFTONE);

		StretchDIBits(hdc_Lx, left, top, rect.right - rect.left, rect.bottom - rect.top,
			0, 0, gcap.stillWidth, gcap.stillHeight, (void*)/*pRawData*/pRawData, pBitmapInfo, DIB_RGB_COLORS, SRCCOPY/*WHITENESS/*LR_LOADFROMFILE*/);

		ReleaseDC(ghwndSub[2], hdc_Lx);
		/* create iris template */
		/*
		1. get 8-bit raw data array;
		2. open a dialog for the template ID
		3. create the template for this ID
		4. save the template and ID file
		*/
		cvtImage2gray(pRawData); //TODO convert the 16-bit YUV to 8-bit grayscale image with 640x480 ROI
		/* load a iris image with opencv method */
		//const char* pfilename = "C:\\Users\\wcheng\\VideologyProject\\CVprojs\\ThirdCVproj_IW\\Debug\\SnapShot449.bmp";
		//cv::Mat imgR = cv::imread(pfilename, CV_LOAD_IMAGE_GRAYSCALE);
		//cv::namedWindow("Iris Image", 0);
		//cv::imshow("Iris Image", imgR); //TODO unavailable
		//long lm_size = imgR.total();
		//cv::Mat imgRC(imgR);
        //pRawData = imgR.data;

		//CreateIrisTemplate(pRawData); //call irisId API

		/* 
		  save 8-bit bmp image fiel. convert the raw image to bmp image 
		*/
#if 1 // it should be removed into the save button message handle
		BYTE* bmpImage = CImageConvert::Raw8BitByteArrayToBmp
			(
			pRawData,
			640,//gcap.stillWidth,
			480//gcap.stillHeight
			);
		int size1, size2;
		size1 = sizeof(BITMAPFILEHEADER);
	
		wchar_t snapHwTrigger[1000];
		wchar_t pathExeWch[2 * MAX_PATH];
		wchar_t tempFileName[80];

		if (pathExeWch)
		{
			memset(pathExeWch, 0, sizeof(pathExeWch));
			MultiByteToWideChar(CP_UTF8, 0, &pathSnapImg[0], (int)strlen(pathSnapImg), &pathExeWch[0], sizeof(pathExeWch));
		}

		wcscpy(snapHwTrigger, pathExeWch);

		SYSTEMTIME st;
		//GetSystemTime(&st);
		GetLocalTime(&st);
		hr = StringCbPrintf(tempFileName, sizeof(tempFileName), L"SnapShot%d%d%d_%d_%d_%d_%d.bmp", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		
		wcscat(snapHwTrigger, tempFileName);

		HANDLE hf = CreateFile(snapHwTrigger, GENERIC_WRITE,
			FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
		if (hf == INVALID_HANDLE_VALUE)
		{
			return E_FAIL;
		}

		//WriteFile(hf, bmpImage, gcap.stillWidth * gcap.stillHeight + 1024 + 54, &dwWritten, NULL);
		WriteFile(hf, bmpImage, 640 * 480 + 1024 + 54, &dwWritten, NULL);

		CloseHandle(hf);
		delete[] bmpImage;
#endif
		delete[] pRawData;
		break;

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


//-------------------------------------------------------------------
// InitializeApplication
//
// Initializes the application.
//-------------------------------------------------------------------

BOOL InitializeApplication()
{
	HRESULT hr = S_OK;

	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if (SUCCEEDED(hr))
	{
		//hr = MFStartup(MF_VERSION);
	}

	return (SUCCEEDED(hr));
}

//-------------------------------------------------------------------
// CleanUp
//
// Releases resources.
//-------------------------------------------------------------------

void CleanUp()
{

	if (g_hdevnotify)
	{
		UnregisterDeviceNotification(g_hdevnotify);
	}
	//MFShutdown();
	CoUninitialize();
}


//-------------------------------------------------------------------
// InitializeWindow
//
// Creates the application window.
//-------------------------------------------------------------------

BOOL InitializeWindow(HWND *pHwnd, HINSTANCE hInstance)
{
	RECT   rect, rectW;

	WNDCLASS wc = { 0 };  //the window class that includes the manu pointer lpszMenuName.

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;// GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = CLASS_NAME;
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU_5MPBW);
	/*MAKEINTRESOURCE: converts an integer value to a resource type compatible with the resource-management functions.
	(the string containing teh name of the resource) */
	wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_FX3_CAM));


	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	HWND hwnd = CreateWindow(
		CLASS_NAME,
		WINDOW_NAME,
		WS_OVERLAPPED | WS_SYSMENU,// | WS_HSCROLL | WS_VSCROLL,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,//GetModuleHandle(NULL),
		NULL
		);

	if (!hwnd)
	{
		return FALSE;
	}

	/* setting the main window's dimension */
	GetClientRect(hwnd, &rect);
	GetWindowRect(hwnd, &rectW);
	rect.bottom = 750;
	rect.right = 1000;
	rectW.bottom = rectW.top + rect.bottom;
	rectW.right = rectW.left + rect.right;
	SetWindowPos(hwnd, NULL, 0, 0, rectW.right,
		rectW.bottom, SWP_NOZORDER | SWP_NOMOVE);
#if 1
	//ghwndButW = CreateWindow(CLASS_NAME, WINDOW_NAME1[0],
	//	WS_OVERLAPPEDWINDOW/*WS_VISIBLE*/ | WS_CHILD,
	//	0, rect.bottom - 10 * BORDER, 
	//	(rect.right - rect.left), (rect.bottom - rect.top) / 10, hwnd, NULL, NULL, NULL);


	/* create three buttons: enroll, save, and match */
	ghwndBut[0] = CreateWindow(L"BUTTON", L"ENROLL", WS_CHILD | WS_VISIBLE,
		rect.right - 48 * BORDER, rect.bottom - 10 * BORDER, //BORDER,
		(rect.right - rect.left) / 8, (rect.bottom - rect.top) / 12, //BORDER,
		hwnd/*ghwndButW*/, (HMENU)IDC_BTN_ENROLL, NULL, 0);
	ghwndBut[1] = CreateWindow(L"BUTTON", L"SAVE", WS_CHILD | WS_VISIBLE,
		rect.right - 44 * BORDER + (rect.right - rect.left) / 8, rect.bottom - 10 * BORDER, //BORDER,
		(rect.right - rect.left) / 8, (rect.bottom - rect.top) / 12, //BORDER,
		hwnd/*ghwndButW*/, (HMENU)IDC_BTN_SAVEG, NULL, 0);
	ghwndBut[2] = CreateWindow(L"BUTTON", L"MATCH", WS_CHILD | WS_VISIBLE,
		rect.right - 40 * BORDER + (rect.right - rect.left) / 4, rect.bottom - 10 * BORDER, //BORDER,
		(rect.right - rect.left) / 8, (rect.bottom - rect.top) / 12, //BORDER,
		hwnd/*ghwndButW*/, (HMENU)IDC_BTN_MATCH, NULL, 0);
#endif
	for (int i = 0; i < 3; i++)

		ghwndSub[i] = CreateWindow(CLASS_NAME, WINDOW_NAME1[i],
		WS_VISIBLE | WS_CHILD,
		0, 0, 0, 0, hwnd, NULL, NULL, NULL);

	ghwndAppSub = ghwndSub[0];
	ghwndAppMain = hwnd;
	//ghwndBut[0] = hWndButtonMain;

	if (gcap.fWantPreview)   // were we previewing?
	{
		BuildPreviewGraph();
		StartPreview();
	}

	//ShowWindow(hwnd, SW_SHOWDEFAULT SW_SHOWMAXIMIZED);

	SendMessage(hwnd, WM_SIZE, 0,
		MAKELONG((WORD)rect.right, (WORD)rect.bottom));
#if 0 
	ghwndBut[0] = CreateWindow(L"BUTTON", L"Start Camera",  //TODO more implementation
		WS_TABSTOP | WS_CHILD | WS_VISIBLE | /*BS_SPLITBUTTON*/BS_PUSHBUTTON,
		rect.right - 12 * BORDER, rect.bottom - 10 * BORDER,
		120, 40,
		hwnd, (HMENU)IDC_BTN_BUTTONA/*NULL*/, NULL /*NULL*/, NULL);

	//rect.right - 12 * BORDER, rect.bottom - 8 * BORDER,
	//	80, 40,

	GetWindowRect(ghwndBut[0], &rect);

#endif
	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

	gcap.isSetWin = TRUE; //set the flag of the subwindows be ture.
	*pHwnd = hwnd;// ghwndSub[0]; // hwnd;

	return TRUE;
}


//-------------------------------------------------------------------
// MessageLoop 
//
// Implements the window message loop.
//-------------------------------------------------------------------

INT MessageLoop(HWND hwnd)
{
	MSG msg = { 0 };

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DestroyWindow(hwnd);

	return INT(msg.wParam);
}


//-------------------------------------------------------------------
// OnCreate
//
// Handles the WM_CREATE message.
//-------------------------------------------------------------------

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT)
{
	HRESULT hr = S_OK;
	HINSTANCE hInstance = GetModuleHandle(NULL);

	// Register this window to get device notification messages.

	DEV_BROADCAST_DEVICEINTERFACE di = { 0 };
	di.dbcc_size = sizeof(di);
	di.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	di.dbcc_classguid = KSCATEGORY_CAPTURE;

	g_hdevnotify = RegisterDeviceNotification(
		hwnd,
		&di,
		DEVICE_NOTIFY_WINDOW_HANDLE
		);

	if (g_hdevnotify == NULL)
	{
		ShowErrorMessage(L"RegisterDeviceNotification failed.", HRESULT_FROM_WIN32(GetLastError()));
		return FALSE;
	}

	// Create the object that manages video preview. 
	//hr = CPreview::CreateInstance(hwnd, hwnd, &g_pPreview);

	//ghwndAppMain = hwnd;

	if (FAILED(hr))
	{
		ShowErrorMessage(L"CPreview::CreateInstance failed.", hr);
		return FALSE;
	}

	// Select the first available device (if any).
	OnChooseDevice(hwnd, FALSE);

	return TRUE;
}



//-------------------------------------------------------------------
// OnClose
//
// Handles WM_CLOSE messages.
//-------------------------------------------------------------------

void OnClose(HWND /*hwnd*/)
{
	PostQuitMessage(0);
}


//-------------------------------------------------------------------
// OnSize
//
// Handles WM_SIZE messages.
//-------------------------------------------------------------------

void OnSize(/*HWND hwnd,*/ WPARAM wParam, LPARAM lParam/*, UINT state*/)
{
	RECT rc, rcW, rcWM;
	PAINTSTRUCT ps;
	HRESULT hr = S_OK;
	int xExtra, yExtra;
	int cyBorder = GetSystemMetrics(SM_CYBORDER);
	long left = 0, top = 0;
	float wWid, wHig;
	HRESULT hr1, hr2;
	long lLeft, lWidth, lTop, lHeight;
	long lSrWidth, lSrHeight;
	AM_MEDIA_TYPE *pmt;
	hr = gcap.pVSC && gcap.pVSC->GetFormat(&pmt);
	lWidth = HEADER(pmt->pbFormat)->biWidth;
	lHeight = ABS(HEADER(pmt->pbFormat)->biHeight);
	DeleteMediaType(pmt);

	SmartPtr<IBasicVideo> pBV;

	hr = gcap.VideoRenderer->QueryInterface(IID_IBasicVideo, (void**)&pBV);

	//hr1 = pBV->get_VideoHeight(&lSrHeight);
	//hr2 = pBV->get_VideoWidth(&lSrWidth);
	//pBV->GetSourcePosition(&lLeft, &lTop, &lSrWidth, &lSrHeight);// for checking
	//pBV->SetSourcePosition(0, 0, lWidth, lHeight);
	if (!gcap.isSetWin){
		GetClientRect(ghwndAppMain, &rc);
		GetWindowRect(ghwndAppMain, &rcW);
		rc.bottom = 750;
		rc.right = 1000;
		rcW.bottom = rcW.top + rc.bottom;
		rcW.right = rcW.left + rc.right;
		SetWindowPos(ghwndAppMain, NULL, 0, 0, rcW.right,
			rcW.bottom, SWP_NOZORDER | SWP_NOMOVE);

		GetWindowRect(ghwndAppMain, &rcW);
		GetClientRect(ghwndAppMain, &rc);
		if ((ghwndSub[2] == NULL) && (ghwndBut[2] == NULL))
			return;
		SetWindowPos(ghwndSub[0], NULL,
			BORDER, BORDER,
			(rc.right - rc.left) - 2 * BORDER, (rc.bottom - rc.top) * 2 / 5 - BORDER, // BORDER,
			SWP_SHOWWINDOW);
		SetWindowPos(ghwndSub[1], NULL,
			4 * BORDER, (rc.bottom - rc.top) * 2 / 5 + 5 * BORDER, //BORDER,
			(rc.right - rc.left) / 2 - 8 * BORDER, (rc.bottom - rc.top) * 2 / 5 - BORDER * 3, //BORDER,
			SWP_SHOWWINDOW);
		SetWindowPos(ghwndSub[2], NULL,
			(rc.right - rc.left) / 2 + 3 * BORDER, (rc.bottom - rc.top) * 2 / 5 + 5 * BORDER, //BORDER,
			(rc.right - rc.left) / 2 - 8 * BORDER, (rc.bottom - rc.top) * 2 / 5 - BORDER * 3, //BORDER,
			SWP_SHOWWINDOW);
	#if 1
		SetWindowPos(ghwndBut[0], NULL,
			rc.right - 52 * BORDER, rc.bottom - 10 * BORDER, //BORDER,
			(rc.right - rc.left) / 8, (rc.bottom - rc.top) / 12, //BORDER,
			SWP_SHOWWINDOW);
		SetWindowPos(ghwndBut[1], NULL,
			rc.right - 48 * BORDER + (rc.right - rc.left) / 8, rc.bottom - 10 * BORDER, //BORDER,
			(rc.right - rc.left) / 8, (rc.bottom - rc.top) / 12, //BORDER,
			SWP_SHOWWINDOW);
		SetWindowPos(ghwndBut[2], NULL,
			rc.right - 44 * BORDER + (rc.right - rc.left) / 4, rc.bottom - 10 * BORDER, //BORDER,
			(rc.right - rc.left) / 8, (rc.bottom - rc.top) / 12, //BORDER,
			SWP_SHOWWINDOW);
	#endif
		ghwndAppSub = ghwndSub[0];
		GetWindowRect(ghwndAppMain, &rcWM);
	}

	GetClientRect(ghwndAppSub, &rc);
	GetWindowRect(ghwndAppSub, &rcW);


	switch (wParam){
#if 1
	case SIZE_MAXIMIZED:
		return;
		//wp = wParam; lp = lParam;
		pBV->GetSourcePosition(&lLeft, &lTop, &lSrWidth, &lSrHeight);// for checking
		pBV->SetSourcePosition(0, 0, lWidth, lHeight);

		wWid = ((int)lParam & 0xffff);
		wHig = ((int)lParam >> 16);
#if 1
		if (initCtrlSetting.isFull){
			if (aspRetio <= (float)(wWid / wHig/*rc.right / rc.bottom*/)){//the heigh is used
				left = (rc.right - rc.bottom*aspRetio) / 2;
				rc.right = rc.bottom*aspRetio;
			}
			else{ //the bottom is used
				top = (rc.bottom - rc.right / aspRetio) / 2;
				rc.bottom = rc.right / aspRetio;
			}

		}
		else{
			GetClientRect(ghwndAppSub, &rc);
			lWidth = rc.right - rc.left;
			lHeight = rc.bottom - rc.top;

			//			pBV->GetSourcePosition(&lLeft, &lTop, &lSrWidth, &lSrHeight);// for checking
			pBV->SetSourcePosition(0, 0, lWidth, lHeight);
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t onsize: SIZE_RESTORED Event(org pixel) : set window pos Lef %d Top %d srWid %d srHei %d lWid %d lHei %d",
				lLeft, lTop, lSrWidth, lSrHeight, lWidth, lHeight);
			printLogMessage(logMessage);
#endif
		}
#endif
#ifdef DEBUG
		sprintf(logMessage, " \nERROR \t onsize: SIZE_MAXIMSIZE Event : set window pos set window pos Asp %f lWid %d lHei %d lef %d top %d",
			aspRetio, lWidth, lHeight, left, top);
		printLogMessage(logMessage);
#endif

		break;
#endif
	case SIZE_MINIMIZED:
		//wp = wParam; lp = lParam;
		wWid = ((int)lParam & 0xffff);
		wHig = ((int)lParam >> 16);
		break;
	case SIZE_RESTORED:
		//wp = wParam; lp = lParam;
		xExtra = rcW.right - rcW.left - rc.right;
		yExtra = rcW.bottom - rcW.top - rc.bottom + cyBorder;  //TODO
		if (initCtrlSetting.isFull){
			pBV->GetSourcePosition(&lLeft, &lTop, &lSrWidth, &lSrHeight);// for checking
			pBV->SetSourcePosition(0, 0, lWidth, lHeight);

			wWid = rc.right; //((int)lParam & 0xffff);//
			wHig = rc.bottom; //((int)lParam >> 16);//
			//xExtra = rcW.right - rcW.left - rc.right;
			//yExtra = rcW.bottom - rcW.top - rc.bottom + cyBorder;
#if 1
			if (aspRetio <= (float)(wWid / wHig/*rc.right / rc.bottom*/)){//the heigh is used
				left = (rc.right - rc.bottom*aspRetio) / 2;
				rc.right = rc.bottom*aspRetio;
			}
			else{ //the bottom is used
				top = (rc.bottom - rc.right / aspRetio) / 2;
				rc.bottom = rc.right / aspRetio;
			}
			/* the window size treating */
#endif
			//			SetWindowPos(ghwndAppSub, NULL, 0, 0, /*wWid + xExtra*/rcW.right - rcW.left,
			//				/*wHig + yExtra*/rcW.bottom - rcW.top, SWP_NOZORDER | SWP_NOMOVE);
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t onsize: SIZE_RESTORED Event : set window pos Asp %f extY %d cx %d cy %d Lcx %d Lcy %d",
				aspRetio, yExtra, rcW.right - rcW.left, rcW.bottom - rcW.top, lWidth, lHeight);
			printLogMessage(logMessage);
#endif

		}
		else{
			//get the video size
			//AM_MEDIA_TYPE *pmt;
			//if (gcap.pVSC && gcap.pVSC->GetFormat(&pmt) == S_OK)
			//{
			//wWid = HEADER(pmt->pbFormat)->biWidth;((int)lParam & 0xffff);
			//wHig = ABS(HEADER(pmt->pbFormat)->biHeight);
			//SmartPtr<IBasicVideo> pBV;

			//hr = gcap.VideoRenderer->QueryInterface(IID_IBasicVideo, (void**)&pBV);

			//if (1)
			{
				//HRESULT hr1, hr2;
				//long lLeft, lWidth, lTop, lHeight;
				//long lSrWidth, lSrHeight;

				//hr1 = pBV->get_VideoHeight(&lSrHeight);
				//hr2 = pBV->get_VideoWidth(&lSrWidth);

				// default capture format
				//	SetWindowPos(ghwndAppSub, NULL, 0, 0, lWidth + xExtra,
				//		lHeight + yExtra, SWP_NOZORDER | SWP_NOMOVE); //set window size

				GetClientRect(ghwndAppSub, &rc);
				lWidth = rc.right - rc.left;
				lHeight = rc.bottom - rc.top;

				pBV->GetSourcePosition(&lLeft, &lTop, &lSrWidth, &lSrHeight);// for checking
				pBV->SetSourcePosition(0, 0, lWidth, lHeight);
#ifdef DEBUG
				sprintf(logMessage, " \nERROR \t onsize: SIZE_RESTORED Event(org pixel) : set window pos Lef %d Top %d srWid %d srHei %d lWid %d lHei %d",
					lLeft, lTop, lSrWidth, lSrHeight, lWidth, lHeight);
				printLogMessage(logMessage);
#endif

			}
		}
		break;
	default:
		wWid = ((int)lParam & 0xffff);
		wHig = ((int)lParam >> 16);
		break;
	}

	// If we got here, gcap.pVW is not NULL 
	//ASSERT(gcap.pVW != NULL);

	if (gcap.pVW)
	{

		/*
		hr = gcap.pVW->QueryInterface(IID_IBasicVideo, (void**)&pBV);

		if (SUCCEEDED(hr))
		{
		HRESULT hr1, hr2;
		//long lWidth, lHeight;

		hr1 = pBV->get_VideoHeight(&lHeight);
		hr2 = pBV->get_VideoWidth(&lWidth);
		if (SUCCEEDED(hr1) && SUCCEEDED(hr2))
		{
		//ResizeWindow(lWidth, abs(lHeight));
		}
		}
		if ((rc.bottom != 0) && (aspRetio <= (rc.right / rc.bottom))){//the heigh is used
		left = (rc.right - rc.bottom*aspRetio)/2;
		rc.right = rc.bottom*aspRetio;
		}
		else{ //the bottom is used
		top = (rc.bottom - rc.right / aspRetio)/2;
		rc.bottom = rc.right / aspRetio;
		}
		*/
		//gcap.pVW->put_Owner((OAHWND)GetSafeHwnd());
		//gcap.pVW->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);
		//gcap.pVW->put_MessageDrain((OAHWND)GetSafeHwnd());
		//HDC hdc = BeginPaint(ghwndAppSub, &ps);
		// Resize our window to the new capture size.
		//FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_BACKGROUND + 3));
		//gcap.pVW->SetWindowPosition(/*0*/left, /*0*/top, /*wWid*/rc.right, /*wHig*/rc.bottom);
		gcap.pVW->SetWindowPosition(/*0*/left + xExtra, /*0*/top + yExtra, rc.right - rc.left, rc.bottom - rc.top);
		//EndPaint(ghwndAppSub, &ps);
#if 1
		/* pain sub-windows */
		HDC hdc;
		hdc = BeginPaint(ghwndAppMain, &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_BACKGROUND + 1));
		EndPaint(ghwndAppMain, &ps);

		hdc = BeginPaint(ghwndSub[0], &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_BACKGROUND + 2));
		EndPaint(ghwndSub[0], &ps);

		hdc = BeginPaint(ghwndSub[1], &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_BACKGROUND + 2));
		EndPaint(ghwndSub[1], &ps);

		hdc = BeginPaint(ghwndSub[2], &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_BACKGROUND + 2));
		EndPaint(ghwndSub[2], &ps);

		//		hdc = BeginPaint(ghwndBut[0], &ps);
		//		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_BACKGROUND + 2));
		//		EndPaint(ghwndBut[0], &ps);

#endif

		//gcap.pVW->put_Visible(OATRUE);
	}
}

BOOL MenuTestFg = FALSE;
//-------------------------------------------------------------------
// OnCommand 
//
// Handles WM_COMMAND messages
//-------------------------------------------------------------------

void OnCommand(HWND hwnd, int id, HWND /*hwndCtl*/, UINT /*codeNotify*/)
{
	HMENU hMenu1, hMenu2, hMenu3, hMenu4, hMenu5, hMainMenu;
	LPMENUINFO MenuInfo, MainMenuInfo;
	switch (id)
	{
	case ID_FILE_CHOOSEDEVICE:
		OnChooseDevice(hwnd, TRUE);
		break;

	case MENU_CAM_CONTROL:
	case ID_CAMERASETTINGS_SHUT_20_12MC:
		OnCameraControlMenu2Mcl(hwnd);
		break;

	case MENU_DAY_NIGHT_SETTING:
	case ID_DAYNIGHTSETTINGS_20_12MC:
		OnDayNightMenu(hwnd);
		break;

	case ID_SETTING_VIDEOQUALITYSETTINGS:
	case ID_SETTING_VIDEOQUALITYSETTINGS_20_12MC:
		OnVideoQualityControlMenu(hwnd);
		break;
	case ID_SETTING_VIDEOQUALITYSETTINGS_5MPCOL:
		OnVideoQualityControlMenu_5mpcol(hwnd);
		break;
	case ID_SETTING_VIDEOQUALITYSETTINGS_MISU:
		ShowCapFilterPropPage(hwnd);
		break;
	case ID_SETTING_3DNOISEREDUCTION:
	case ID_3DNR_SETTINGS_20_12MC:
		On3DNoiseReduction2mpCol(hwnd);
		//On3DNoiseReduction(hwnd);	// keep the 3D NR in the future
		break;
	case MENU_I2C_CONTROL:
		OnI2CControlMenu(hwnd);
		break;

	case MENU_ZOOM_CONTROL:
		OnZoomControlMenu(hwnd);
		break;

	case MENU_IRIS_CONTROL:
	case ID_CAMERA_TERMINAL_SETTINGS_20_12MC:
		OnIrisControlMenu(hwnd);
		break;

	case ID_SNAPSHOT_SETSAVINGDIRECTORY: //MENU_IMAGE_CAPTURE:
		OnImageCaptureMenu(hwnd);
		break;

	case ID_SNAPSHOT_SETIMAGERESOLUTION:
		OnImageResSetMenu(hwnd);
		break;

	case ID_SNAPSHOT_GETIMAGE:
		//OnSnapshotMenu(hwnd);
		saveImageFlg = TRUE;
		readSnapCount();
		stillTrigger();		// to start a still image capture.
		writeSnapCount();
		break;

	case MENU_CAM_PARAM_RECOVERY:
		OnCameraRecoveryParamMenu(hwnd);
		break;

	case ID_FORMAT_VIDEORESOLUTION:
		OnVideoStreamingSetting(hwnd);
		break;

	case ID_FORMAT_REGIONOFINTREST:
		OnROISetting(hwnd);
		break;

	case ID_PLAYBACKSTYLE_FULLIMAGE:
		initCtrlSetting.isFull = 1;
		hMenu1 = GetMenu(hwnd);
		CheckMenuItem(hMenu1, ID_PLAYBACKSTYLE_FULLIMAGE, MF_CHECKED | MF_BYCOMMAND);
		CheckMenuItem(hMenu1, ID_PLAYBACKSTYLE_ORIGINALPIXELS, MF_UNCHECKED | MF_BYCOMMAND);
		SendMessage(ghwndAppMain, WM_SIZE, SIZE_RESTORED, 0);// force a WM_SIZE message
		break;

	case ID_PLAYBACKSTYLE_ORIGINALPIXELS:
		initCtrlSetting.isFull = 0;
		hMenu1 = GetMenu(hwnd);
		CheckMenuItem(hMenu1, ID_PLAYBACKSTYLE_FULLIMAGE, MF_UNCHECKED | MF_BYCOMMAND);
		CheckMenuItem(hMenu1, ID_PLAYBACKSTYLE_ORIGINALPIXELS, MF_CHECKED | MF_BYCOMMAND);
		SendMessage(ghwndAppMain, WM_SIZE, SIZE_RESTORED, 0);// force a WM_SIZE message
		break;

		/*new dialogs for 5MP b/w */
	case ID_5MPCAMERASETTINGS_SHUT:
		OnCameraControlMenu(hwnd);
		break;

	case ID_5MPCAMERASETTINGS_EDGEENHANCEMENT:
		OnEdgeEnhanment(hwnd);
		break;

	case ID_5MPCAMERASETTINGS_GAMMAAND2DNR:
		On2DNoiseReduction(hwnd);
		break;
	case ID_CAMERA_SAVEPARAMETERS:
		saveParamClicked(hwnd);
		//OnEdgeEnhanment(hwnd);
		break;

		/*new dialogs for 5MP color */
	case ID_5MPCOLCAMERASETTINGS_WBLCOLSUPPRESSION:
		On5MPColSuppWBL(hwnd);
		break;
	case ID_5MPCOLCAMERAMANAGEMENT_PIXELCORRECTION:
		hMenu1 = GetMenu(hwnd);
		getRegVal(REG_PIXELCORRECT, &initCtrlSetting.pixelCorrect);
		if (initCtrlSetting.pixelCorrect == PIXCORRECT_OFF) //TODO: should check the device if it is on correction mode.
		{
			CheckMenuItem(hMenu1, ID_5MPCOLCAMERAMANAGEMENT_PIXELCORRECTION, MF_CHECKED | MF_BYCOMMAND);
			initCtrlSetting.pixelCorrect = PIXCORRECT_ON;
			setRegVal(REG_PIXELCORRECT, &initCtrlSetting.pixelCorrect);
		}
		else
		{
			CheckMenuItem(hMenu1, ID_5MPCOLCAMERAMANAGEMENT_PIXELCORRECTION, MF_UNCHECKED | MF_BYCOMMAND);
			initCtrlSetting.pixelCorrect = PIXCORRECT_OFF;
			setRegVal(REG_PIXELCORRECT, &initCtrlSetting.pixelCorrect);
		}
		break;


		/* for 2.0/1.2MP color */
	case ID_FOCUS_ZOOM_SETTINGS_20_12MC:
		OnZoomControlMenu(hwnd);
		break;

	case ID_CAMERA_RESETSETTINGS:
		resetParamClicked(hwnd);
		//On2DNoiseReduction(hwnd);
		break;

		/*
		MENUITEM "Shutter and BLC",             ID_5MPCAMERASETTINGS_SHUT
		MENUITEM "Edge Enhancement...",         ID_5MPCAMERASETTINGS_EDGEENHANCEMENT
		MENUITEM "Gamma and 2DNR",              ID_5MPCAMERASETTINGS_GAMMAAND2DNR

		*/
	case MENU_ABOUT:
		OnAboutMenu(hwnd);
		break;
	case IDC_BTN_ENROLL:
		/* copy the snapshot code here */
		saveImageFlg = FALSE;
		readSnapCount();
		stillTrigger();		// to start a still image capture.
		writeSnapCount();
		//}
		break;
	case IDC_BTN_SAVEG:
		MessageBox(ghwndAppMain, L"Command", L"SAVE", MB_OK);

		break;
	case IDC_BTN_MATCH:
		MessageBox(ghwndAppMain, L"Command", L"MATCH", MB_OK);

		break;
	}
}

void IMonRelease(IMoniker *&pm)
{
	if (pm)
	{
		pm->Release();
		pm = 0;
	}
}

void AddDevicesToMenu()
{

	if (gcap.fDeviceMenuPopulated)
	{
		return;
	}
	gcap.fDeviceMenuPopulated = true;
	gcap.iNumVCapDevices = 0;
	gcap.iSelectedDeviceIndex = -1;

	UINT    uIndex = 0;
	HRESULT hr = S_OK;

	for (int i = 0; i < NUMELMS(gcap.rgpmVideoMenu); i++)
	{
		IMonRelease(gcap.rgpmVideoMenu[i]);
	}
	for (int i = 0; i < NUMELMS(gcap.rgpmAudioMenu); i++)
	{
		IMonRelease(gcap.rgpmAudioMenu[i]);
	}


	// enumerate all video capture devices
	ICreateDevEnum *pCreateDevEnum = 0;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	if (hr != NOERROR)
	{
#ifdef DEBUG
		sprintf(logMessage, " \nERROR \t Function : AddDevicesToMenu \t Msg :  Error Creating Device Enumerator");
		printLogMessage(logMessage);
#endif
		return;
	}

	IEnumMoniker *pEm = 0;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEm, 0);
	if (hr != NOERROR)
	{
#ifdef DEBUG
		sprintf(logMessage, " \nERROR \t Function : AddDevicesToMenu \t Msg :  Sorry, you have no video capture hardware.\r\nVideo capture will not function properly.");
		printLogMessage(logMessage);
#endif

		goto EnumAudio;
	}

	pEm->Reset();
	ULONG cFetched;
	IMoniker *pM;

	gcap.defaultVideoLogyCamID = -2;

	while (hr = pEm->Next(1, &pM, &cFetched), hr == S_OK)
	{
		IPropertyBag *pBag = 0;

		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if (SUCCEEDED(hr))
		{
			VARIANT var;	//what the different VAEIANR & CComVariant?
			//CComVariant var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"DevicePath", &var, NULL);

			CStringW visName = var.bstrVal;
			visName = visName.MakeLower();
			if (visName.Find(L"usb#") < 0)
				continue;
			int i = visName.Find(L"vid_");
			ULONG vid = wcstol(visName.Mid(i + 4, 4), NULL, 16);
			i = visName.Find(L"pid_");

			ULONG pid = wcstol(visName.Mid(i + 4, 4), NULL, 16);
			ULONG vidpid = (vid << 16) | pid;

			wchar_t *ptr = wcsstr(var.bstrVal, L"vid_1bbd");

			if (ptr != nullptr && gcap.defaultVideoLogyCamID == -2)
			{
				gcap.defaultVideoLogyCamID = -1;
			}
			//hr = pBag->Read(L"Description", &var, NULL);
			hr = pBag->Read(L"FriendlyName", &var, NULL);

			// we have a usb camera, then extract teh VID and PID.
			// \\?\usb#vid_xxxx&pid_xxxx
			// technically, the format of the device path is undocumented.
			// however, this format has remained stable since Win95 OSR2 and is unlikely to change in our lifetime.

#ifdef DEBUG
			sprintf(logMessage, " \nFunction : AddDevicesToMenu \t Msg : FriendlyName  %ls", var.bstrVal);
			printLogMessage(logMessage);
#endif

			if (hr == NOERROR)
			{
				if (gcap.defaultVideoLogyCamID == -1)
					gcap.defaultVideoLogyCamID = uIndex;
				ASSERT(gcap.rgpmVideoMenu[uIndex] == 0);
				gcap.rgpmVideoMenu[uIndex] = pM;
				wcscpy(gcap.vis_camID[uIndex].rgpmVideoFriendlyName, var.bstrVal);
				gcap.vis_camID[uIndex].VidPid = vidpid;

				SysFreeString(var.bstrVal);

				pM->AddRef();
				uIndex++;
			}
			pBag->Release();
		}

		pM->Release();

	}
	pEm->Release();

	gcap.iNumVCapDevices = uIndex;
	if (gcap.iNumVCapDevices > 0)
		gcap.iSelectedDeviceIndex = 0;

	/*
	#ifdef DEBUG
	sprintf(logMessage, " \nFunction : AddDevicesToMenu \t Msg : IMoniker obj for vid Device found  %d", gcap.iNumVCapDevices);
	printLogMessage(logMessage);
	#endif
	*/

EnumAudio:

	// enumerate all audio capture devices
	uIndex = 0;

	ASSERT(pCreateDevEnum != NULL);

	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEm, 0);
	pCreateDevEnum->Release();
	if (hr != NOERROR)
		return;
	pEm->Reset();

	while (hr = pEm->Next(1, &pM, &cFetched), hr == S_OK)
	{
		IPropertyBag *pBag;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if (SUCCEEDED(hr))
		{
			VARIANT var;
			var.vt = VT_BSTR;
			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR)
			{
				ASSERT(gcap.rgpmAudioMenu[uIndex] == 0);
				gcap.rgpmAudioMenu[uIndex] = pM;
				wcsncpy(gcap.rgpmAudioFriendlyName[uIndex], var.bstrVal, sizeof(var.bstrVal));
				SysFreeString(var.bstrVal);

				pM->AddRef();
				uIndex++;
			}
			pBag->Release();
		}
		pM->Release();
	}

	pEm->Release();
}


void OnCameraControlMenu(HWND hwnd)
{

	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		;
	}
	AfxGetInstanceHandle();
	VIS5mpBWExp c_5mpBWExp; // add the get device class --wcheng
	c_5mpBWExp.devCap = &gcap;
	c_5mpBWExp.camNodeTree = &ksNodeTree;
	c_5mpBWExp.saveCameraControlInitSetting();
	c_5mpBWExp.DoModal();

}

void OnCameraControlMenu2Mcl(HWND hwnd)
{

	INT_PTR result = DialogBoxParam(
		GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDD_CAM_CONTROL2MPCL),
		hwnd,
		CameraControlDlgProc,
		NULL
		);

	if (result == IDOK)
	{
		//TODO : send extension cmd to camera
		// int selIndex = param.selMirrorIndex;
	}

}

void OnDayNightMenu(HWND hwnd)
{

	INT_PTR result = DialogBoxParam(
		GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDD_DAY_NIGHT_SETTINGS),
		hwnd,
		DayNightSettingsDlgProc,
		NULL
		);

	if (result == IDOK)
	{
		//TODO : send extension cmd to camera
		// int selIndex = param.selMirrorIndex;
	}
	/*
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
	;
	}
	AfxGetInstanceHandle();
	VIS2mpColFocusZoom c_2mpColFocusZoom; // add the get device class --wcheng
	c_2mpColFocusZoom.devCap = &gcap;
	c_2mpColFocusZoom.camNodeTree = &ksNodeTree;
	c_2mpColFocusZoom.saveFocusZoomInitSetting();
	c_2mpColFocusZoom.DoModal();
	*/
}

void OnVideoQualityControlMenu(HWND hwnd)
{
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		;
	}
	AfxGetInstanceHandle();
	VISImageProc c_5mpBWImageCtrl; // add the get device class. it's for all cameras --wcheng
	c_5mpBWImageCtrl.devCap = &gcap;
	c_5mpBWImageCtrl.camNodeTree = &ksNodeTree;
	c_5mpBWImageCtrl.saveImageInitSetting();
	c_5mpBWImageCtrl.DoModal();
}

void OnVideoQualityControlMenu_5mpcol(HWND hwnd)
{
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		;
	}
	AfxGetInstanceHandle();
	VISImpageProc5mpCol c_5mpColImageCtrl; // add the get device class. it's for all cameras --wcheng
	c_5mpColImageCtrl.devCap = &gcap;
	c_5mpColImageCtrl.camNodeTree = &ksNodeTree;
	c_5mpColImageCtrl.saveImageInitSetting();
	c_5mpColImageCtrl.DoModal();
}

#if 1 //keep for the 3D ND in futrue
void On3DNoiseReduction2mpCol(HWND hwnd)
{
	INT_PTR result = DialogBoxParam(
		GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDD_3D_NOISE_REDUCTION),
		hwnd,
		NoiseReductionDlgProc,
		NULL
		);

	if (result == IDOK)
	{
		//TODO : send extension cmd to camera
		// int selIndex = param.selMirrorIndex;
	}

	/*
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
	;
	}
	AfxGetInstanceHandle();
	VIS2mpColFocusZoom c_2mpColFocusZoom; // add the get device class --wcheng
	c_2mpColFocusZoom.devCap = &gcap;
	c_2mpColFocusZoom.camNodeTree = &ksNodeTree;
	c_2mpColFocusZoom.saveFocusZoomInitSetting();
	c_2mpColFocusZoom.DoModal();
	*/
}
#endif

void On2DNoiseReduction(HWND hwnd)
{
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		;
	}
	AfxGetInstanceHandle();
	VIS5mpBWGam2DN c_5mpBWGam2DNR; // add the get device class --wcheng
	c_5mpBWGam2DNR.devCap = &gcap;
	c_5mpBWGam2DNR.camNodeTree = &ksNodeTree;
	c_5mpBWGam2DNR.saveGammaInitSetting();
	c_5mpBWGam2DNR.DoModal();
}

void OnEdgeEnhanment(HWND hwnd)
{
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		;
	}
	AfxGetInstanceHandle();
	VIS5mpBWEdgeEn c_5mpBWEdge; // add the get device class --wcheng
	c_5mpBWEdge.devCap = &gcap;
	c_5mpBWEdge.camNodeTree = &ksNodeTree;
	c_5mpBWEdge.saveEnhanceInitSetting();
	c_5mpBWEdge.DoModal();
}

void    On5MPColSuppWBL(HWND hwnd)
{
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		;
	}
	AfxGetInstanceHandle();
	VIS5MPColSupWBL c_5mpColSup; // add the get device class --wcheng
	c_5mpColSup.devCap = &gcap;
	c_5mpColSup.camNodeTree = &ksNodeTree;
	c_5mpColSup.saveColSupWBLInitSetting();
	c_5mpColSup.DoModal();
}

void OnAboutMenu(HWND hwnd)
{

	INT_PTR result = DialogBoxParam(
		GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDD_ABOUT),
		hwnd,
		AboutDlgProc,
		NULL
		);

	if (result == IDOK)
	{
		//TODO : send extension cmd to camera
		// int selIndex = param.selMirrorIndex;
	}

}

HWND hwndForI2C;

void OnI2CControlMenu(HWND hwnd)
{
	hwndForI2C = hwnd;

	INT_PTR result = DialogBoxParam(
		GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDD_LOGIN),
		hwnd,
		LoginDlgProc,
		NULL
		);

	if (result == IDOK)
	{
		//TODO : send extension cmd to camera
		// int selIndex = param.selMirrorIndex;
	}

}

void OnZoomControlMenu(HWND hwnd)
{
#if 1
	INT_PTR result = DialogBoxParam(
		GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDD_ZOOM_CONTROL),
		hwnd,
		ZoomControlDlgProc,
		NULL
		);

	if (result == IDOK)
	{
		//TODO : send extension cmd to camera
		// int selIndex = param.selMirrorIndex;
	}
#else
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		;
	}
	AfxGetInstanceHandle();
	VIS2mpColFocusZoom c_2mpColFocusZoom; // add the get device class --wcheng
	c_2mpColFocusZoom.devCap = &gcap;
	c_2mpColFocusZoom.camNodeTree = &ksNodeTree;
	c_2mpColFocusZoom.saveFocusZoomInitSetting();
	SetWindowLong(GetDlgItem(hwnd, IDC_BTN_ZOOM_IN), GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_NOPARENTNOTIFY);
	SetWindowLong(GetDlgItem(hwnd, IDC_BTN_ZOOM_OUT), GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_NOPARENTNOTIFY);
	SetWindowLong(GetDlgItem(hwnd, IDC_BTN_FOCUS_START), GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_NOPARENTNOTIFY);
	SetWindowLong(GetDlgItem(hwnd, IDC_BTN_FOCUS_NEAR), GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_NOPARENTNOTIFY);
	SetWindowLong(GetDlgItem(hwnd, IDC_BTN_FOCUS_FAR), GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_NOPARENTNOTIFY);

	c_2mpColFocusZoom.DoModal();
#endif
}


void OnIrisControlMenu(HWND hwnd)
{

	INT_PTR result = DialogBoxParam(
		GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDD_IRIS_CONTROL),
		hwnd,
		IrisControlDlgProc,
		NULL
		);

	if (result == IDOK)
	{
		//TODO : send extension cmd to camera
		// int selIndex = param.selMirrorIndex;
	}
	/*
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
	;
	}
	AfxGetInstanceHandle();
	VIS2mpColFocusZoom c_2mpColFocusZoom; // add the get device class --wcheng
	c_2mpColFocusZoom.devCap = &gcap;
	c_2mpColFocusZoom.camNodeTree = &ksNodeTree;
	c_2mpColFocusZoom.saveFocusZoomInitSetting();
	c_2mpColFocusZoom.DoModal();
	*/
}

void OnImageCaptureMenu(HWND hwnd)
{

	INT_PTR result = DialogBoxParam(
		GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDD_IMG_CAPTURE),
		hwnd,
		ImageCaptureDlgProc,
		NULL
		);

	if (result == IDOK)
	{
		//TODO : send extension cmd to camera
		// int selIndex = param.selMirrorIndex;
	}

}

void OnImageResSetMenu(HWND hwnd)
{

	INT_PTR result = DialogBoxParam(
		GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDD_STILLRES),
		hwnd,
		ImageResSetDlgProc,
		NULL
		);

	if (result == IDOK)
	{
		//TODO : send extension cmd to camera
		// int selIndex = param.selMirrorIndex;
	}

}

void OnCameraRecoveryParamMenu(HWND hwnd)
{

	INT_PTR result = DialogBoxParam(
		GetModuleHandle(NULL),
		MAKEINTRESOURCE(IDD_CAM_PARAM_RECOVERY),
		hwnd,
		CameraRecoveryParamDlgProc,
		NULL
		);

	if (result == IDOK)
	{
		//TODO : send extension cmd to camera
		// int selIndex = param.selMirrorIndex;
	}

}

void    reROIMode(HWND hwnd, int sel);

void OnVideoStreamingSetting(HWND hwnd)
{

	HRESULT hr = S_OK;
	ISpecifyPropertyPages *pSpec;
	CAUUID  cauuid;
	int height;

	StopPreview();
	TearDownGraph();

	hr = gcap.pVSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
	hr = pSpec->GetPages(&cauuid);

	//HWND hLocatorWnd = NULL;
	LPCOLESTR lpszCaption = TEXT("Video Pin");

	//hLocatorWnd = CreateWindow(TEXT("Button"), TEXT("Locator"),
	//WS_POPUPWINDOW, mainRect.right, mainRect.top, 20, 20, hwndApp,
	//NULL, NULL, NULL);

	hr = OleCreatePropertyFrame(ghwndAppSub, 0, 0, lpszCaption,
		1, (IUnknown **)&gcap.pVSC, cauuid.cElems,
		(GUID *)cauuid.pElems, 0, 0, NULL);

	AM_MEDIA_TYPE *pmt;

	// default capture format
	if (gcap.pVSC && gcap.pVSC->GetFormat(&pmt) == S_OK)
	{
		// DV capture does not use a VIDEOINFOHEADER
		if (pmt->formattype == FORMAT_VideoInfo)
		{
			// resize our window to the default capture size
			aspRetio = (float)(HEADER(pmt->pbFormat)->biWidth) / (float)(ABS(HEADER(pmt->pbFormat)->biHeight));
			image_w = HEADER(pmt->pbFormat)->biWidth;
			image_h = ABS(HEADER(pmt->pbFormat)->biHeight);
			ResizeWindow(HEADER(pmt->pbFormat)->biWidth, ABS(HEADER(pmt->pbFormat)->biHeight));
			//height = HEADER(pmt->pbFormat)->biHeight;
			//aspRetio = (float)(HEADER(pmt->pbFormat)->biWidth) / (float)(ABS(HEADER(pmt->pbFormat)->biHeight));
		}
		if (pmt->majortype != MEDIATYPE_Video)
		{
			// This capture filter captures something other that pure video.
			// Maybe it's DV or something?  Anyway, chances are we shouldn't
			// allow capturing audio separately, since our video capture
			// filter may have audio combined in it already!
			gcap.fCapAudioIsRelevant = FALSE;
			gcap.fCapAudio = FALSE;
		}
		DeleteMediaType(pmt);
	}

	CoTaskMemFree(cauuid.pElems);
	pSpec->Release();

	hr = gcap.pFg->AddFilter(gcap.pSampleGrabberFilter, L"SampleGrab");
	hr = gcap.pFg->AddFilter(gcap.pNullRenderer, L"NullRender");
	//hr = gcap.pFg->AddFilter(gcap.VideoRenderer, L"VideoRenderer");

	BuildPreviewGraph();
	StartPreview();

}

void    OnROISetting(HWND hwnd)
{

	HRESULT hr = S_OK;
	ISpecifyPropertyPages *pSpec;
	CAUUID  cauuid;
	int width;

	//StopPreview();
	//TearDownGraph();

	hr = gcap.pVSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
	hr = pSpec->GetPages(&cauuid);

	AM_MEDIA_TYPE *pmt;

	// default capture format
	if (gcap.pVSC && gcap.pVSC->GetFormat(&pmt) == S_OK)
	{
		// DV capture does not use a VIDEOINFOHEADER
		if (pmt->formattype == FORMAT_VideoInfo)
		{
			// resize our window to the default capture size
			ResizeWindow(HEADER(pmt->pbFormat)->biWidth, ABS(HEADER(pmt->pbFormat)->biHeight));
			if ((HEADER(pmt->pbFormat)->biHeight) == 720 || (HEADER(pmt->pbFormat)->biHeight) == 480)
			{
				INT_PTR result = DialogBoxParam(
					GetModuleHandle(NULL),
					MAKEINTRESOURCE(IDD_ROI_CONTROL),
					hwnd,
					ROIControlDlgProc,
					NULL
					);

				if (result == IDOK)
				{
					//TODO : send extension cmd to camera
					// int selIndex = param.selMirrorIndex;
				}

			}
			//	width = HEADER(pmt->pbFormat)->biHeight;//call I2C command to send the ROI relate
			//aspRetio = (float)(HEADER(pmt->pbFormat)->biWidth) / (float)(ABS(HEADER(pmt->pbFormat)->biHeight));
		}
		DeleteMediaType(pmt);
	}

	CoTaskMemFree(cauuid.pElems);
	pSpec->Release();
}

//-------------------------------------------------------------------
//  OnChooseDevice
//
//  Select a video capture device.
//
//  hwnd:    A handle to the application window.
/// bPrompt: If TRUE, prompt to user to select the device. Otherwise,
//           select the first device in the list.
//-------------------------------------------------------------------
void OnChooseDevice(HWND hwnd, BOOL bPrompt)
{
	HRESULT hr = S_OK;

	UINT iDevice = 0;   // Index into the array of devices
	BOOL bCancel = FALSE;
	gcap.iSelectedDeviceIndex = 0xFF; //set a unvalid number

	// Ask for source type = video capture devices.
	gcap.fDeviceMenuPopulated = false;
	AddDevicesToMenu(); // enumerate devide list for extended control.

	if (FAILED(hr)) { goto done; }

	// NOTE: param.count might be zero.

	if (bPrompt)
	{
		// Ask the user to select a device.
		if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
		{
			;
		}
		AfxGetInstanceHandle();
		GetDevice c_getDevice; // add the get device class --wcheng
		c_getDevice.devCap = &gcap;
		c_getDevice.visCamID = visID;
		hr = c_getDevice.DoModal();
	}
	else
	{
		if (gcap.iNumVCapDevices > 0){
			int i, j;
			for (i = 0; i < gcap.iNumVCapDevices; i++){
				for (j = 0; j < 16; j++){
					if (gcap.vis_camID[i].VidPid == visID[0].VidPid){
						gcap.iSelectedDeviceIndex = i;
						gcap.CamIndex = j;
						i = 0xf; //jump out of the i-for loop
						break;
					}
				}
			}
			//gcap.iSelectedDeviceIndex = 0;
		}
		else{
			if (gcap.defaultVideoLogyCamID >= 0){
				gcap.iSelectedDeviceIndex = gcap.defaultVideoLogyCamID;
			}
			else;// show no device message and close the application.
		}

	}
	if (gcap.iSelectedDeviceIndex == 0xFF){//only the 5M b/w camera works as Iris camera.
		MessageBox(ghwndAppMain, L"No Iris Camera Selected!", L"Fail Message", MB_OK);
		return;
	}
	if (gcap.iNumVCapDevices > 0)
	{
		// Give this source to the CPlayer object for preview.
		ChooseDevices(gcap.rgpmVideoMenu[gcap.iSelectedDeviceIndex], NULL);
	}

	RECT r;
	//GetClientRect(ghwndApp, &r);
	GetWindowRect(ghwndAppSub, &r);
	//aspRetio = (float)(r.right - r.left) / (float)(r.bottom - r.top);

done:

	if (FAILED(hr))
	{
		ShowErrorMessage(L"Cannot create a video capture device", hr);
	}
}


/////////////////////////////////////////////////////////////////////

//HRESULT getBLCRangeValue(int PropertyId, int *hpos, int *hsize, int *vpos, int *vsize);
//HRESULT getExposureAGCLvlValue(int PropertyId, int *ExpValue, int *AgcLvlValue);
//HRESULT setExposureAGCLvlValue(HWND hwnd, int PropertyId, int ExpValue, int AgcLvlValue);
HRESULT getExt2ControlValues(int PropertyId, int *ExpValue, int *AgcLvlValue);
HRESULT setExt2ControlValues(int PropertyId, int ExpValue, int AgcLvlValue);
HRESULT setExtControls(int PropertyId, int PropertyValue);

HRESULT	getExtControlValue(int PropertyId, int *curValue)
{
	HRESULT hr = S_OK;
	ULONG ulSize;
	BYTE *pbPropertyValue;
	int retValue = 0;

	hr = getExtionControlPropertySize(PropertyId, &ulSize);
	if (FAILED(hr))
	{
#ifdef DEBUG
		sprintf(logMessage, " \nERROR \t Function : getExtControlValue \t Msg : Unable to find property[%d] size : %x", PropertyId, hr);
		printLogMessage(logMessage);
#endif
	}
	else
	{
		pbPropertyValue = new BYTE[ulSize];
		if (!pbPropertyValue)
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : getExtControlValue \t Msg : Unable to allocate memory for property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}

		hr = getExtionControlProperty(PropertyId, ulSize, pbPropertyValue);

		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : getExtControlValue \t Msg : Unable to get property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}

		memcpy(&retValue, (char *)pbPropertyValue, ulSize);

#ifdef DEBUG
		sprintf(logMessage, " \nFunction : getExtControlValue \t Msg : property[%d] value[%d]", PropertyId, retValue);
		printLogMessage(logMessage);
#endif
		delete[] pbPropertyValue;
	}
	*curValue = retValue;
	return hr;

}


HRESULT getBLCRangeValue(int PropertyId, int *hpos, int *hsize, int *vpos, int *vsize)
{
	HRESULT hr = S_OK;
	ULONG ulSize;
	BYTE *pbPropertyValue;
	int retValue = 0;

	*hpos = 0;
	*hsize = 0;
	*vpos = 0;
	*vsize = 0;

	hr = getExtionControlPropertySize(PropertyId, &ulSize);
	//ulSize = 2;
	if (FAILED(hr) || (ulSize != 2))
	{
#ifdef DEBUG
		sprintf(logMessage, " \nERROR \t Function : getBLCRangeValue \t Msg : Unable to find property[%d] size : %x", PropertyId, hr);
		printLogMessage(logMessage);
#endif
	}
	else
	{
		pbPropertyValue = new BYTE[ulSize];
		if (!pbPropertyValue)
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : getBLCRangeValue \t Msg : Unable to allocate memory for property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}

		hr = getExtionControlProperty(PropertyId, ulSize, pbPropertyValue);
		//pbPropertyValue[0] = 0x05;
		//pbPropertyValue[1] = 0x06;

		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : getBLCRangeValue \t Msg : Unable to get property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}

		// *pbPropertyValue = (*pbPropertyValue & 0x0000) | ((vpos & 0x000F) << 12) | ((hpos & 0x000F) << 8) | ((vsize & 0x000F) << 4) | ((hsize & 0x000F));

		/*
		*hsize = ((*pbPropertyValue) & 0x0F);
		*vsize = ((*pbPropertyValue) >> 4 & 0x0F);
		*hpos = ((*(pbPropertyValue+1)) & 0x0F);
		*vpos = ((*(pbPropertyValue+1) >> 4) & 0x0F);
		*/

		*hpos = ((*pbPropertyValue) & 0x0F);
		*vpos = ((*pbPropertyValue) >> 4 & 0x0F);
		*hsize = ((*(pbPropertyValue + 1)) & 0x0F);
		*vsize = ((*(pbPropertyValue + 1) >> 4) & 0x0F);


#ifdef DEBUG
		sprintf(logMessage, " \nFunction : getBLCRangeValue \t Msg : property[%d] value[%d]", PropertyId, retValue);
		printLogMessage(logMessage);
#endif
		delete[] pbPropertyValue;
	}

	return hr;
}

HRESULT getExt2ControlValues(int PropertyId, int *ExpValue, int *AgcLvlValue)
{
	HRESULT hr = S_OK;
	ULONG ulSize;
	BYTE *pbPropertyValue;
	int retValue = 0;

	*ExpValue = 0;
	*AgcLvlValue = 0;

	hr = getExtionControlPropertySize(PropertyId, &ulSize);
	if (FAILED(hr) || (ulSize != 4))
	{
#ifdef DEBUG
		sprintf(logMessage, " \nERROR \t Function : getExt2ControlValues \t Msg : Unable to find property[%d] size : %x", PropertyId, hr);
		printLogMessage(logMessage);
#endif
	}
	else
	{
		pbPropertyValue = new BYTE[ulSize];
		if (!pbPropertyValue)
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : getExt2ControlValues \t Msg : Unable to allocate memory for property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}

		hr = getExtionControlProperty(PropertyId, ulSize, pbPropertyValue);

		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : getExt2ControlValues \t Msg : Unable to get property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}

		memcpy(ExpValue, (char *)&pbPropertyValue[0], 2);
		memcpy(AgcLvlValue, (char *)&pbPropertyValue[2], 2);

#ifdef DEBUG
		sprintf(logMessage, " \nFunction : getExt2ControlValues \t Msg : property[%d] value[%d]", PropertyId, retValue);
		printLogMessage(logMessage);
#endif
		delete[] pbPropertyValue;
	}

	return hr;
}

HRESULT setExt2ControlValues(int PropertyId, int ExpValue, int AgcLvlValue)
{
	HRESULT hr = S_OK;

	try
	{
#if 0
		if (ExpValue == 0)
		{
			HWND hListSldAGCLVL = GetDlgItem(hwnd, IDC_SLD_AGC_LVL);
			EnableWindow(hListSldAGCLVL, FALSE);
		}
		else
		{
			HWND hListSldAGCLVL = GetDlgItem(hwnd, IDC_SLD_AGC_LVL);
			EnableWindow(hListSldAGCLVL, TRUE);
		}
#endif
		if (ExpValue != LB_ERR && ksNodeTree.isOK)
		{

			ULONG ulSize;
			BYTE *pbPropertyValue;

			hr = getExtionControlPropertySize(PropertyId, &ulSize);
			if (FAILED(hr) || (ulSize != 4)) // first two byte Exposure Mode & last two byte AGC level mast be 4 byte
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : setExt2ControlValues \t Msg : Unable to find property size : %x", hr);
				printLogMessage(logMessage);
#endif
			}
			else
			{
				pbPropertyValue = new BYTE[ulSize];
				if (!pbPropertyValue)
				{
#ifdef DEBUG
					sprintf(logMessage, "\nERROR \t Function : setExt2ControlValues \t Msg : Unable to allocate memory for property value");
					printLogMessage(logMessage);
#endif
				}
				else
				{
					int ExposureByte = 2;
					int AgcLvlByte = 2;
					memcpy(&pbPropertyValue[0], (char*)&ExpValue, ExposureByte); // first two byte Exposure Mode & last two byte AGC level
					memcpy(&pbPropertyValue[ExposureByte], (char*)&AgcLvlValue, AgcLvlByte); // first two byte Exposure Mode & last two byte AGC level

					hr = setExtionControlProperty(PropertyId, ulSize, pbPropertyValue);
				}
				delete[] pbPropertyValue;
			}

#ifdef DEBUG
			sprintf(logMessage, "\nFunction : setExt2ControlValues \t Msg : Return Value:%ld", hr);
			printLogMessage(logMessage);
#endif
		}

	}
	catch (...)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nERROR : In Function : setExt2ControlValues");
		printLogMessage(logMessage);
#endif
	}

	return hr;

}

HRESULT setExtControls(int PropertyId, int PropertyValue)
{
	HRESULT hr = S_OK;
	ULONG ulSize;
	BYTE *pbPropertyValue;

	hr = getExtionControlPropertySize(PropertyId, &ulSize);
	if (SUCCEEDED(hr))
	{

		pbPropertyValue = new BYTE[ulSize];
		if (!pbPropertyValue)
		{
#ifdef DEBUG
			sprintf(logMessage, "\nError \t Function : resetCameraControlInitSetting \t Msg : Unable to allocate memory for property value");
			printLogMessage(logMessage);
#endif
		}
		else
		{
			memcpy(pbPropertyValue, (char*)&PropertyValue, ulSize);
			hr = setExtionControlProperty(PropertyId, ulSize, pbPropertyValue);
		}
		delete[] pbPropertyValue;
	}
	return hr;
}
/*will delet 1*/
//HRESULT OnShutCtrlChange(HWND hwnd)

/*will delet 2*/
//HRESULT OnSenUpModeChange(HWND hwnd)

/*will delet 3*/
//HRESULT onAEReferenceLevelChange(HWND hwnd)

/*will delet 4*/
//HRESULT OnExposureModeChange(HWND hwnd)

/*will delet 6*/
//HRESULT onAEShutLevelChange(HWND hwnd)

/*will delet 5*/
//HRESULT onAgcLvlChange(HWND hwnd)
#if 0
HRESULT OnhExHystLvlChangeSLD(HWND hwnd)
{

	HRESULT hr = S_OK;
	long ExHystSldPos = 0;
	CString strPos;
	HWND hSLDHystLvl = GetDlgItem(hwnd, IDC_SLD_AE_HYSTER);
	ExHystSldPos = (long)SendMessageA(hSLDHystLvl, TBM_GETPOS, TRUE, ExHystSldPos);

	//HWND hListExpoMode = GetDlgItem(hwnd, IDC_COMBO_EXPOSURE_MODE);
	//int sel = ComboBox_GetCurSel(hListExpoMode);

	HWND hEditHystLVL = GetDlgItem(hwnd, IDC_EDIT_AE_HYSTER);
	strPos.Format(L"%ld", ExHystSldPos);
	Edit_SetText(hEditHystLVL, strPos);

	if (ksNodeTree.isOK)
	{
		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 20;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		if (FAILED(hr) || (ulSize != 2)) // 2 bytes value
		{
#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : onAgcLvlChange \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : onAgcLvlChange \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				//int ExposureByte = 2;
				int HystLvlByte = 2;
				//memcpy(&pbPropertyValue[0], (char*)&sel, ExposureByte); // first two byte Exposure Mode & last two byte AGC level
				memcpy(&pbPropertyValue[0], (char*)&ExHystSldPos, HystLvlByte); //

				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
			}
			delete[] pbPropertyValue;
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : onAgcLvlChange \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT OnhExCtrlSpeedLvlChangeSLD(HWND hwnd)
{

	HRESULT hr = S_OK;
	long ExCtrlSpdSldPos = 0;
	CString strPos;
	HWND hSLDAeCtrlspdLVL = GetDlgItem(hwnd, IDC_SLD_AE_CTRLSPEED);
	ExCtrlSpdSldPos = (long)SendMessageA(hSLDAeCtrlspdLVL, TBM_GETPOS, TRUE, ExCtrlSpdSldPos);

	//HWND hListExpoMode = GetDlgItem(hwnd, IDC_COMBO_EXPOSURE_MODE);
	//int sel = ComboBox_GetCurSel(hListExpoMode);

	HWND hEditCtrlSpdLVL = GetDlgItem(hwnd, IDC_EDIT_AE_CTRLSPEED);
	strPos.Format(L"%ld", ExCtrlSpdSldPos);
	Edit_SetText(hEditCtrlSpdLVL, strPos);

	if (ksNodeTree.isOK)
	{
		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 21;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		if (FAILED(hr) || (ulSize != 2)) // The length of EX control speed level must be 2 bytes
		{
#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : onAgcLvlChange \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : onAgcLvlChange \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				//int ExposureByte = 2;
				int ExCtrlSpdLvlByte = 2;
				//memcpy(&pbPropertyValue[0], (char*)&sel, ExposureByte); // first two byte Exposure Mode & last two byte AGC level
				memcpy(&pbPropertyValue[0], (char*)&ExCtrlSpdSldPos, ExCtrlSpdLvlByte); // first two byte Exposure Mode & last two byte AGC level

				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
			}
			delete[] pbPropertyValue;
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : onAgcLvlChange \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}
#endif
/*will delet 7*/
//HRESULT onBLCRangeChange(HWND hwnd)

/*will delet 11*/
//HRESULT OnCameraModeChange(HWND hwnd)

/*will delet 14*/
//HRESULT OnBLCGridChange(HWND hwnd)

/*will delet 12*/
//HRESULT OnBLCWeightFactorChange(HWND hwnd)

/*will delet 13*/
//HRESULT OnBLCWeightFactorChangeSLD(HWND hwnd)

/*will delet 8*/
//HRESULT OnSelectedIndexChangeMirror(HWND hwnd)

int getMainsFrequency()
{
	long retValue;
	long lCap;
	HRESULT hr = S_OK;

	if (ksNodeTree.isOKpProcAmp)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nbmRequestType:GET \t bRequest:GET_CUR \t wValue:Mains frequency \t wIndex:0x02");
		printLogMessage(logMessage);
#endif
		hr = ksNodeTree.pProcAmp->Get(KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY, &retValue, &lCap);
#ifdef DEBUG
		sprintf(logMessage, "\nFunction : OnMainsFrequencyChange Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	if (!SUCCEEDED(hr))
		retValue = 0;

	return (int)retValue;

}
/*will delet 9*/
//HRESULT OnMainsFrequencyChange(HWND hwnd)

/*will delet 10*/
//HRESULT	onBacklightCompensationChange(HWND hwnd)

/////////////////////////////////////////////////////////////////////

// Dialog functions
void    OnInitDayNightSettingsDialog(HWND hwnd);
void	saveDayNightInitSetting(HWND hwnd);
void	resetDayNightInitSetting(HWND hwnd);
HRESULT OnDayNightModeChange(HWND hwnd);
HRESULT	onDayNightSwDelay(HWND hwnd);
HRESULT	onDayNightStLvl(HWND hwnd);
HRESULT	onNightDayStLvl(HWND hwnd);

//-------------------------------------------------------------------
//  DayNightSettingsDlgProc
//
//  DayNightSettingsDlgProc procedure for the "Extension settings" dialog.
//-------------------------------------------------------------------

INT_PTR CALLBACK DayNightSettingsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
	case WM_INITDIALOG:
#ifdef DEBUG
		sprintf(logMessage, "\nCommand Fired : WM_INITDIALOG ID : IDD_DAY_NIGHT_SETTINGS");
		printLogMessage(logMessage);
#endif
		saveDayNightInitSetting(hwnd);
		OnInitDayNightSettingsDialog(hwnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		case IDC_COMBO_DAY_NIGHT_MODE:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case CBN_SELCHANGE:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_COMBO_DAY_NIGHT_MODE Event : CBN_SELCHANGE");
				printLogMessage(logMessage);
#endif
				OnDayNightModeChange(hwnd);
				break;
			}
			return TRUE;

		case IDCANCEL:
			resetDayNightInitSetting(hwnd);
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		}
		break;

	case WM_HSCROLL:
		HWND hListSldDNDelay = GetDlgItem(hwnd, IDC_SLD_DAY_NIGHT_SW_DELAY);
		if ((HWND)lParam == hListSldDNDelay)
		{
			switch (LOWORD(wParam)) // Find out what message it was
			{
			case TB_ENDTRACK: // This means that the list is about to display
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_SLD_DAY_NIGHT_SW_DELAY Event : TB_ENDTRACK");
				printLogMessage(logMessage);
#endif
				onDayNightSwDelay(hwnd);
				break;
			}
			return TRUE;
		}

		HWND hListSldDNStlvl = GetDlgItem(hwnd, IDC_SLIDER_DAY_TO_NIGHT_ST_LVL);
		if ((HWND)lParam == hListSldDNStlvl)
		{
			switch (LOWORD(wParam)) // Find out what message it was
			{
			case TB_ENDTRACK: // This means that the list is about to display
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_SLIDER_DAY_TO_NIGHT_ST_LVL Event : TB_ENDTRACK");
				printLogMessage(logMessage);
#endif
				onDayNightStLvl(hwnd);
				break;
			}
			return TRUE;
		}

		HWND hListSldNDStlvl = GetDlgItem(hwnd, IDC_SLIDER_NIGHT_TO_DAY_ST_LVL);
		if ((HWND)lParam == hListSldNDStlvl)
		{
			switch (LOWORD(wParam)) // Find out what message it was
			{
			case TB_ENDTRACK: // This means that the list is about to display
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_SLIDER_NIGHT_TO_DAY_ST_LVL Event : TB_ENDTRACK");
				printLogMessage(logMessage);
#endif
				onNightDayStLvl(hwnd);
				break;
			}
			return TRUE;
		}

		break;

	}

	return FALSE;
}


void OnInitDayNightSettingsDialog(HWND hwnd)
{
	//HRESULT hr = S_OK;
	CString retValueStr;
	int retValue = 0;

	HRESULT hr = S_OK;

	HWND hListDayNightMode = GetDlgItem(hwnd, IDC_COMBO_DAY_NIGHT_MODE);
	hr = getExtControlValue(6, &retValue);
	if (FAILED(hr))
	{
		EnableWindow(hListDayNightMode, FALSE);
	}
	ComboBox_AddString(hListDayNightMode, L"AUTO");
	ComboBox_AddString(hListDayNightMode, L"DAY_MODE");
	ComboBox_AddString(hListDayNightMode, L"NIGHT_MODE");
	ComboBox_SetCurSel(hListDayNightMode, retValue);

	HWND hListSldDNDelay = GetDlgItem(hwnd, IDC_SLD_DAY_NIGHT_SW_DELAY);
	hr = getExtControlValue(7, &retValue);
	if (FAILED(hr))
	{
		EnableWindow(hListSldDNDelay, FALSE);
	}
	SendMessageA(hListSldDNDelay, TBM_SETRANGEMAX, TRUE, 63);
	SendMessageA(hListSldDNDelay, TBM_SETRANGEMIN, TRUE, 0);
	SendMessageA(hListSldDNDelay, TBM_SETPAGESIZE, TRUE, 1);
	SendMessageA(hListSldDNDelay, TBM_SETPOS, TRUE, retValue);
	HWND hListDayNightSwDelay = GetDlgItem(hwnd, IDC_EDIT_DAY_NIGHT_SW_DELAY);
	retValueStr.Format(L"%d", retValue);
	Edit_SetText(hListDayNightSwDelay, retValueStr);
	EnableWindow(hListDayNightSwDelay, FALSE);

	HWND hListSldDNStlvl = GetDlgItem(hwnd, IDC_SLIDER_DAY_TO_NIGHT_ST_LVL);
	hr = getExtControlValue(8, &retValue);
	if (FAILED(hr))
	{
		EnableWindow(hListSldDNStlvl, FALSE);
	}
	SendMessageA(hListSldDNStlvl, TBM_SETRANGEMAX, TRUE, 100);
	SendMessageA(hListSldDNStlvl, TBM_SETRANGEMIN, TRUE, 0);
	SendMessageA(hListSldDNStlvl, TBM_SETPAGESIZE, TRUE, 1);
	SendMessageA(hListSldDNStlvl, TBM_SETPOS, TRUE, retValue);
	HWND hListDayNightStLvl = GetDlgItem(hwnd, IDC_EDIT_DAY_TO_NIGHT_ST_LVL);
	retValueStr.Format(L"%d", retValue);
	Edit_SetText(hListDayNightStLvl, retValueStr);
	EnableWindow(hListDayNightStLvl, FALSE);

	HWND hListSldNDStlvl = GetDlgItem(hwnd, IDC_SLIDER_NIGHT_TO_DAY_ST_LVL);
	hr = getExtControlValue(9, &retValue);
	if (FAILED(hr))
	{
		EnableWindow(hListSldNDStlvl, FALSE);
	}
	SendMessageA(hListSldNDStlvl, TBM_SETRANGEMAX, TRUE, 100);
	SendMessageA(hListSldNDStlvl, TBM_SETRANGEMIN, TRUE, 0);
	SendMessageA(hListSldNDStlvl, TBM_SETPAGESIZE, TRUE, 1);
	SendMessageA(hListSldNDStlvl, TBM_SETPOS, TRUE, retValue);
	HWND hListNightDayStLvl = GetDlgItem(hwnd, IDC_EDIT_NIGHT_TO_DAY_ST_LVL);
	retValueStr.Format(L"%d", retValue);
	Edit_SetText(hListNightDayStLvl, retValueStr);
	EnableWindow(hListNightDayStLvl, FALSE);

}

void saveDayNightInitSetting(HWND hwnd)
{
	// reset global struct
	initCtrlSetting.DayNightMode = 0;
	initCtrlSetting.DayNightSwitchDelay = 0;
	initCtrlSetting.DaytoNightStartLevel = 0;
	initCtrlSetting.NighttoDayStartLevel = 0;

	// get current Value
	getExtControlValue(6, &initCtrlSetting.DayNightMode);
	getExtControlValue(7, &initCtrlSetting.DayNightSwitchDelay);
	getExtControlValue(8, &initCtrlSetting.DaytoNightStartLevel);
	getExtControlValue(9, &initCtrlSetting.NighttoDayStartLevel);

}

void resetDayNightInitSetting(HWND hwnd)
{
	HWND hListDayNightMode = GetDlgItem(hwnd, IDC_COMBO_DAY_NIGHT_MODE);
	int sel = ComboBox_GetCurSel(hListDayNightMode);
	if (sel != initCtrlSetting.DayNightMode)
		setExtControls(6, initCtrlSetting.DayNightMode);

	HWND hListSldDNDelay = GetDlgItem(hwnd, IDC_SLD_DAY_NIGHT_SW_DELAY);
	long SldPos = (long)SendMessageA(hListSldDNDelay, TBM_GETPOS, TRUE, SldPos);
	if (SldPos != initCtrlSetting.DayNightSwitchDelay)
		setExtControls(7, initCtrlSetting.DayNightSwitchDelay);

	HWND hListSldDNStlvl = GetDlgItem(hwnd, IDC_SLIDER_DAY_TO_NIGHT_ST_LVL);
	SldPos = (long)SendMessageA(hListSldDNStlvl, TBM_GETPOS, TRUE, SldPos);
	if (SldPos != initCtrlSetting.DaytoNightStartLevel)
		setExtControls(8, initCtrlSetting.DaytoNightStartLevel);

	HWND hListSldNDStlvl = GetDlgItem(hwnd, IDC_SLIDER_NIGHT_TO_DAY_ST_LVL);
	SldPos = (long)SendMessageA(hListSldNDStlvl, TBM_GETPOS, TRUE, SldPos);
	if (SldPos != initCtrlSetting.NighttoDayStartLevel)
		setExtControls(9, initCtrlSetting.NighttoDayStartLevel);
}

HRESULT OnDayNightModeChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	HWND hListDayNightMode = GetDlgItem(hwnd, IDC_COMBO_DAY_NIGHT_MODE);

	int sel = ComboBox_GetCurSel(hListDayNightMode);

	if (sel != LB_ERR && ksNodeTree.isOK)
	{

		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 6;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : OnDayNightModeChange \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : OnDayNightModeChange \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				memcpy(pbPropertyValue, (char*)&sel, ulSize);
				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
			}
			delete[] pbPropertyValue;
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : OnDayNightModeChange \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT onDayNightSwDelay(HWND hwnd)
{
	HRESULT hr = S_OK;
	long SldPos = 0;
	CString strPos;
	HWND hListSldDNDelay = GetDlgItem(hwnd, IDC_SLD_DAY_NIGHT_SW_DELAY);
	SldPos = (long)SendMessageA(hListSldDNDelay, TBM_GETPOS, TRUE, SldPos);

	HWND hListDayNightSwDelay = GetDlgItem(hwnd, IDC_EDIT_DAY_NIGHT_SW_DELAY);
	strPos.Format(L"%ld", SldPos);
	Edit_SetText(hListDayNightSwDelay, strPos);

	if (ksNodeTree.isOK)
	{
		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 7;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : onDayNightSwDelay \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : onDayNightSwDelay \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				memcpy(pbPropertyValue, (char*)&SldPos, ulSize);
				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
			}
			delete[] pbPropertyValue;
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : onDayNightSwDelay \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT onDayNightStLvl(HWND hwnd)
{
	HRESULT hr = S_OK;
	long SldPos = 0;
	CString strPos;
	HWND hListSldDNStlvl = GetDlgItem(hwnd, IDC_SLIDER_DAY_TO_NIGHT_ST_LVL);
	SldPos = (long)SendMessageA(hListSldDNStlvl, TBM_GETPOS, TRUE, SldPos);

	HWND hListDayNightStLvl = GetDlgItem(hwnd, IDC_EDIT_DAY_TO_NIGHT_ST_LVL);
	strPos.Format(L"%ld", SldPos);
	Edit_SetText(hListDayNightStLvl, strPos);

	if (ksNodeTree.isOK)
	{
		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 8;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : onDayNightStLvl \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : onDayNightStLvl \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				memcpy(pbPropertyValue, (char*)&SldPos, ulSize);
				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
			}
			delete[] pbPropertyValue;
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : onDayNightStLvl \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT onNightDayStLvl(HWND hwnd)
{
	HRESULT hr = S_OK;
	long SldPos = 0;
	CString strPos;
	HWND hListSldNDStlvl = GetDlgItem(hwnd, IDC_SLIDER_NIGHT_TO_DAY_ST_LVL);
	SldPos = (long)SendMessageA(hListSldNDStlvl, TBM_GETPOS, TRUE, SldPos);

	HWND hListNightDayStLvl = GetDlgItem(hwnd, IDC_EDIT_NIGHT_TO_DAY_ST_LVL);
	strPos.Format(L"%ld", SldPos);
	Edit_SetText(hListNightDayStLvl, strPos);

	if (ksNodeTree.isOK)
	{
		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 9;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : onNightDayStLvl \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : onNightDayStLvl \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				memcpy(pbPropertyValue, (char*)&SldPos, ulSize);
				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
			}
			delete[] pbPropertyValue;
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : onNightDayStLvl \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}



//-------------------------------------------------------------------
//  VideoQualityControlDlgProc
//
//  VideoQualityControlDlgProc procedure for the "Extension settings" dialog.
//-------------------------------------------------------------------
#if 1
HRESULT getWhiteBalanceComponent(int *redValue, int *blueValue)
{
	HRESULT hr = S_OK;

	if (ksNodeTree.isOKpProcAmpKsControl)
	{

		KSP_NODE ExtensionProp;
		ULONG ulBytesReturned;
		KSPROPERTY_VIDEOPROCAMP_NODE_S2 setflag, retValue;

		ExtensionProp.Property.Set = PROPSETID_VIDCAP_VIDEOPROCAMP;
		ExtensionProp.Property.Id = KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE_COMPONENT;
		ExtensionProp.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
		ExtensionProp.NodeId = ksNodeTree.nodeIDProcAmpKsControl;

		setflag.NodeProperty = ExtensionProp;
		setflag.Flags = KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
		setflag.Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;

		//hr = ksNodeTree.pProcAmpKsControl->KsProperty((PKSPROPERTY)&retValue, sizeof(retValue), NULL, 0, &ulBytesReturned);
		hr = ksNodeTree.pProcAmpKsControl->KsProperty((PKSPROPERTY)&setflag, sizeof(setflag), &setflag, sizeof(setflag), &ulBytesReturned);

		if (SUCCEEDED(hr))
		{
			*redValue = setflag.Value2;
			*blueValue = setflag.Value1;
		}

	}

	return hr;
}

HRESULT setWhiteBalanceComponent(int redValue, int blueValue)
{
	HRESULT hr = S_OK;

	if (ksNodeTree.isOKpProcAmpKsControl)
	{

		KSP_NODE ExtensionProp;
		ULONG ulBytesReturned;
		KSPROPERTY_VIDEOPROCAMP_NODE_S2 setflag;

		ExtensionProp.Property.Set = PROPSETID_VIDCAP_VIDEOPROCAMP;
		ExtensionProp.Property.Id = KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE_COMPONENT;
		ExtensionProp.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
		ExtensionProp.NodeId = ksNodeTree.nodeIDProcAmpKsControl;

		setflag.NodeProperty = ExtensionProp;
		//setflag.Value1 = redValue;
		//setflag.Value2 = blueValue;
		memcpy((char*)&setflag.Value1, (char*)&blueValue, 2); // first 2 byte blue 
		memcpy((char*)&setflag.Value1 + 2, (char*)&redValue, 2);  // last 2 byte red

		setflag.Flags = KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;
		setflag.Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_AUTO;

#ifdef DEBUG
		sprintf(logMessage, "\nbmRequestType:SET \t bRequest:SET_CUR \t wValue:0xc \t wIndex:0x02\t PutValue: red=%ld,blue=%ld", redValue, blueValue);
		printLogMessage(logMessage);
#endif

		hr = ksNodeTree.pProcAmpKsControl->KsProperty((PKSPROPERTY)&setflag, sizeof(setflag), &setflag, sizeof(setflag), &ulBytesReturned);
	}

	return hr;
}
#endif
#if 0
HRESULT onHueChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	long SldPos = 0;
	CString strPos;
	HWND hListSldHue = GetDlgItem(hwnd, IDC_SLD_HUE);
	SldPos = (long)SendMessageA(hListSldHue, TBM_GETPOS, TRUE, SldPos);

	HWND hListHue = GetDlgItem(hwnd, IDC_EDIT_HUE);
	strPos.Format(L"%ld", SldPos);
	Edit_SetText(hListHue, strPos);

	if (ksNodeTree.isOKpProcAmp)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nbmRequestType:SET \t bRequest:SET_CUR \t wValue:Hue \t wIndex:0x02\t PutValue:%ld", SldPos);
		printLogMessage(logMessage);
#endif
		hr = ksNodeTree.pProcAmp->Set(KSPROPERTY_VIDEOPROCAMP_HUE, SldPos, VideoProcAmp_Flags_Manual);
#ifdef DEBUG
		sprintf(logMessage, "\nFunction : onHueChange Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT onSaturationChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	long SldPos = 0;
	CString strPos;
	HWND hListSldSaturation = GetDlgItem(hwnd, IDC_SLD_SATURATION);
	SldPos = (long)SendMessageA(hListSldSaturation, TBM_GETPOS, TRUE, SldPos);

	HWND hListSaturation = GetDlgItem(hwnd, IDC_EDIT_SATURATION);
	strPos.Format(L"%ld", SldPos);
	Edit_SetText(hListSaturation, strPos);

	if (ksNodeTree.isOKpProcAmp)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nbmRequestType:SET \t bRequest:SET_CUR \t wValue:Saturation \t wIndex:0x02\t PutValue:%ld", SldPos);
		printLogMessage(logMessage);
#endif
		hr = ksNodeTree.pProcAmp->Set(KSPROPERTY_VIDEOPROCAMP_SATURATION, SldPos, VideoProcAmp_Flags_Manual);
#ifdef DEBUG
		sprintf(logMessage, "\nFunction : onSaturationChange Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}
#endif
#if 0
HRESULT	onWhiteBalanceControlChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	HWND hListBackLight = GetDlgItem(hwnd, IDC_COMBO_WHTBLCMODE);

	int sel = ComboBox_GetCurSel(hListBackLight);

	if (sel != LB_ERR && ksNodeTree.isOKpProcAmp)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nbmRequestType:SET \t bRequest:SET_CUR \t wValue:WhiteBalance \t wIndex:0x02\t PutValue:%d", sel);
		printLogMessage(logMessage);
#endif
		hr = ksNodeTree.pProcAmp->Set(KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE, (long)sel, VideoProcAmp_Flags_Auto);

		HWND hListSldWhiteBalanceComp = GetDlgItem(hwnd, IDC_SLD_WHTCOM);
		HWND hListSldWhiteBalanceCompRed = GetDlgItem(hwnd, IDC_SLD_WHTCOM_RED);
		if (SUCCEEDED(hr) && sel == 2)	// if it's manual mode enable red & blue comp
		{
			EnableWindow(hListSldWhiteBalanceComp, TRUE);
			EnableWindow(hListSldWhiteBalanceCompRed, TRUE);
		}
		else
		{
			EnableWindow(hListSldWhiteBalanceComp, FALSE);
			EnableWindow(hListSldWhiteBalanceCompRed, FALSE);
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : onWhiteBalanceControlChange Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT onWhiteBalanceCompChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	long SldPos = 0;
	CString strPos;
	HWND hListSldWhiteBalanceComp = GetDlgItem(hwnd, IDC_SLD_WHTCOM);
	SldPos = (long)SendMessageA(hListSldWhiteBalanceComp, TBM_GETPOS, TRUE, SldPos);

	long SldPosRed = 0;
	HWND hListSldWhiteBalanceCompRed = GetDlgItem(hwnd, IDC_SLD_WHTCOM_RED);
	SldPosRed = (long)SendMessageA(hListSldWhiteBalanceCompRed, TBM_GETPOS, TRUE, SldPosRed);

	HWND hListWhiteBalanceComp = GetDlgItem(hwnd, IDC_EDIT_WHTCOM);
	strPos.Format(L"%ld", SldPos);
	Edit_SetText(hListWhiteBalanceComp, strPos);

	HWND hListWhiteBalanceCompRed = GetDlgItem(hwnd, IDC_EDIT_WHTCOM_RED);
	strPos.Format(L"%ld", SldPosRed);
	Edit_SetText(hListWhiteBalanceCompRed, strPos);

	hr = setWhiteBalanceComponent(SldPosRed, SldPos);

#ifdef DEBUG
	sprintf(logMessage, "\nFunction : onWhiteBalanceCompBlueChange Msg : Return Value:%ld", hr);
	printLogMessage(logMessage);
#endif


	return hr;
}

#endif

/////////////////////////////////////////////////////////////////////

// Dialog functions
void    OnInitNoiseReductionDialog(HWND hwnd);
void	saveNoiseReductionInitSetting(HWND hwnd);
void	resetNoiseReductionInitSetting(HWND hwnd);
HRESULT On3DNoiseReductionMode(HWND hwnd);
HRESULT on3DNoiseRedLvlChange(HWND hwnd);

#if 1//keep for 3D noise deduction
INT_PTR CALLBACK NoiseReductionDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
	case WM_INITDIALOG:
#ifdef DEBUG
		sprintf(logMessage, "\nCommand Fired : WM_INITDIALOG ID : IDD_3D_NOISE_REDUCTION");
		printLogMessage(logMessage);
#endif
		saveNoiseReductionInitSetting(hwnd);
		OnInitNoiseReductionDialog(hwnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			//OnSelectedIndexChangeMirror(hwnd, pParam);
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		case IDC_COMBO_3D_NOISE_REDU_MODE:  //IDC_COMBO_3D_NOISE_REDU_MODE
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case CBN_SELCHANGE:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_COMBO_3D_NOISE_REDU_MODE Event : CBN_SELCHANGE");
				printLogMessage(logMessage);
#endif
				On3DNoiseReductionMode(hwnd);
				break;
			}
			return TRUE;

		case IDCANCEL:
			resetNoiseReductionInitSetting(hwnd);
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		}
		break;

	case WM_HSCROLL:
		HWND hListSld3DNoise = GetDlgItem(hwnd, IDC_SLD_3D_NOISE_REDU_VAL);
		if ((HWND)lParam == hListSld3DNoise)
		{
			switch (LOWORD(wParam)) // Find out what message it was
			{
			case TB_ENDTRACK: // This means that the list is about to display
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_SLD_3D_NOISE_REDU_VAL Event : TB_ENDTRACK");
				printLogMessage(logMessage);
#endif
				on3DNoiseRedLvlChange(hwnd);
				break;
			}
			return TRUE;
		}

		break;

	}

	return FALSE;
}
#endif
void OnInitNoiseReductionDialog(HWND hwnd)
{
	HRESULT hr = S_OK;
	int retValue = 0;
	CString retValueStr;

	HWND hListWhiteBalanceMode = GetDlgItem(hwnd, IDC_COMBO_3D_NOISE_REDU_MODE);
	hr = getExtControlValue(4, &retValue);
	if (FAILED(hr))
	{
		EnableWindow(hListWhiteBalanceMode, FALSE);
	}
	ComboBox_AddString(hListWhiteBalanceMode, L"OFF");
	ComboBox_AddString(hListWhiteBalanceMode, L"ON");
	ComboBox_SetCurSel(hListWhiteBalanceMode, retValue);

	HWND hListSld3DNoise = GetDlgItem(hwnd, IDC_SLD_3D_NOISE_REDU_VAL);
	hr = getExtControlValue(5, &retValue);
	if (FAILED(hr))
	{
		EnableWindow(hListSld3DNoise, FALSE);
	}
	SendMessageA(hListSld3DNoise, TBM_SETRANGEMAX, TRUE, 64);
	SendMessageA(hListSld3DNoise, TBM_SETRANGEMIN, TRUE, 0);
	SendMessageA(hListSld3DNoise, TBM_SETPAGESIZE, TRUE, 1);
	SendMessageA(hListSld3DNoise, TBM_SETPOS, TRUE, retValue);

	HWND hList3DNoiseReduMode = GetDlgItem(hwnd, IDC_EDIT_3D_NOISE_REDU_VAL);
	retValueStr.Format(L"%d", retValue);
	Edit_SetText(hList3DNoiseReduMode, retValueStr);
	EnableWindow(hList3DNoiseReduMode, FALSE);

}

void saveNoiseReductionInitSetting(HWND hwnd)
{
	// reset global struct
	initCtrlSetting.NoiseReductionMode = 0;
	initCtrlSetting.NoiseReductionControl = 0;

	// get current Value
	getExtControlValue(4, &initCtrlSetting.NoiseReductionMode);
	getExtControlValue(5, &initCtrlSetting.NoiseReductionControl);

}

void resetNoiseReductionInitSetting(HWND hwnd)
{
	HWND hListWhiteBalanceMode = GetDlgItem(hwnd, IDC_COMBO_3D_NOISE_REDU_MODE);
	int sel = ComboBox_GetCurSel(hListWhiteBalanceMode);
	if (sel != initCtrlSetting.NoiseReductionMode)
		setExtControls(4, initCtrlSetting.NoiseReductionMode);

	HWND hListSld3DNoise = GetDlgItem(hwnd, IDC_SLD_3D_NOISE_REDU_VAL);
	long noiseRedSldPos = (long)SendMessageA(hListSld3DNoise, TBM_GETPOS, TRUE, noiseRedSldPos);
	if (noiseRedSldPos != initCtrlSetting.NoiseReductionControl)
		setExtControls(5, initCtrlSetting.NoiseReductionControl);
}


HRESULT On3DNoiseReductionMode(HWND hwnd)
{
	HRESULT hr = S_OK;
	HWND hListWhiteBalanceMode = GetDlgItem(hwnd, IDC_COMBO_3D_NOISE_REDU_MODE);

	int sel = ComboBox_GetCurSel(hListWhiteBalanceMode);

	if (sel != LB_ERR && ksNodeTree.isOK)
	{

		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 4;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : On3DNoiseReductionMode \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : On3DNoiseReductionMode \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				memcpy(pbPropertyValue, (char*)&sel, ulSize);
				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
			}
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : On3DNoiseReductionMode \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT on3DNoiseRedLvlChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	long noiseRedSldPos = 0;
	CString strPos;
	HWND hListSld3DNoise = GetDlgItem(hwnd, IDC_SLD_3D_NOISE_REDU_VAL);
	noiseRedSldPos = (long)SendMessageA(hListSld3DNoise, TBM_GETPOS, TRUE, noiseRedSldPos);

	HWND hList3DNoiseReduMode = GetDlgItem(hwnd, IDC_EDIT_3D_NOISE_REDU_VAL);
	strPos.Format(L"%ld", noiseRedSldPos);
	Edit_SetText(hList3DNoiseReduMode, strPos);

	if (ksNodeTree.isOK)
	{
		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 5;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : on3DNoiseRedLvlChange \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : on3DNoiseRedLvlChange \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				memcpy(pbPropertyValue, (char*)&noiseRedSldPos, ulSize);
				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
			}
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : on3DNoiseRedLvlChange \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

#if 0 // keep for the 3DNR in future
HRESULT On3DNoiseReductionMode(HWND hwnd)
{
	HRESULT hr = S_OK;
	HWND hListWhiteBalanceMode = GetDlgItem(hwnd, IDC_COMBO_3D_NOISE_REDU_MODE);

	int sel = ComboBox_GetCurSel(hListWhiteBalanceMode);

	if (sel != LB_ERR && ksNodeTree.isOK)
	{

		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 4;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : On3DNoiseReductionMode \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : On3DNoiseReductionMode \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				memcpy(pbPropertyValue, (char*)&sel, ulSize);
				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
			}
			delete[] pbPropertyValue;
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : On3DNoiseReductionMode \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}
#endif

/////////////////////////////////////////////////////////////////////

// Dialog functions
void    OnInitAboutDialog(HWND hwnd);

//-------------------------------------------------------------------
//  DayNightSettingsDlgProc
//
//  DayNightSettingsDlgProc procedure for the "Extension settings" dialog.
//-------------------------------------------------------------------

INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
	case WM_INITDIALOG:
#ifdef DEBUG
		sprintf(logMessage, "\nCommand Fired : WM_INITDIALOG ID : IDD_ABOUT");
		printLogMessage(logMessage);
#endif
		OnInitAboutDialog(hwnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			//EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		case IDCANCEL:
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		}
		break;


	}

	return FALSE;
}

void OnInitAboutDialog(HWND hwnd)
{

	//HWND hListIDCBrandLog = GetDlgItem(hwnd, IDC_BRAND_LOGO);

	//HWND hListIDCBrandLog = GetDlgItem(hwnd, IDC_STR_BRAND_NAME);
	//Edit_SetText(hListIDCBrandLog, L"VideoLogy Inc");
	//wchar_t wcs[500];
	wchar_t *wcsBrandNameContent;
	//wchar_t *wcsBrandLogoFilePath;
	wchar_t wcsBrandLogoFilePath[1000];
	char *tempBuff;
	size_t sz;
	char brandFiletxt[260];
	char brandLogobmp[260];
	strcpy(brandFiletxt, pathExe);
	strcat(brandFiletxt, BrandNameFile);

	strcpy(brandLogobmp, pathExe);
	strcat(brandLogobmp, BrandLogoFile);

	//wcsBrandLogoFilePath = (wchar_t*)malloc(sizeof(wchar_t)*(strlen(brandLogobmp)+1));
	size_t result = strlen(brandLogobmp);

	if (wcsBrandLogoFilePath)
	{
		memset(wcsBrandLogoFilePath, 0, sizeof(wcsBrandLogoFilePath));
		//int length = mbsrtowcs(wcsBrandLogoFilePath, (const char**)&brandLogobmp, result, NULL);
		MultiByteToWideChar(CP_UTF8, 0, &brandLogobmp[0], (int)strlen(brandLogobmp), &wcsBrandLogoFilePath[0], sizeof(wcsBrandLogoFilePath));

	}

	FILE *fp = fopen(brandFiletxt, "r");
	if (fp)
	{
		// obtain file size:
		fseek(fp, 0, SEEK_END);
		sz = ftell(fp);
		rewind(fp);

		if (sz)
		{
			// allocate memory to contain the whole file:
			tempBuff = (char*)malloc(sizeof(char)*sz);
			wcsBrandNameContent = (wchar_t*)malloc(sizeof(wchar_t)*sz);

			if (tempBuff)
			{
				memset(tempBuff, 0x00, sizeof(tempBuff));
				// copy the file into the buffer:
				size_t result = fread(tempBuff, 1, sz, fp);

				if (result != sz)
				{
					if (feof(fp))
					{
						//::MessageBox(NULL, __T("Premature end of file brandName.txt ."), __T("Error"), MB_OK);
					}
					else
					{
						ERROR("File read error.");
						::MessageBox(NULL, __T("BrandName File read failed."), __T("Error"), MB_OK);
					}
				}

				//int x = strlen(tempBuff);
				//memset(wcs, 0, sizeof(wcs));
				//int length = mbsrtowcs(wcs, (const char**)&tempBuff, result, NULL);

				memset(wcsBrandNameContent, 0, sizeof(wcsBrandNameContent));
				int length = mbsrtowcs(wcsBrandNameContent, (const char**)&tempBuff, result, NULL);
				wcsBrandNameContent[result] = '\0';
				//wcs[result] = L'\0';
				//Edit_SetText(hListIDCBrandLog, wcs);
				fclose(fp);
			}
		}
	}
	else
	{
		::MessageBox(NULL, __T("BrandName.txt File not found."), __T("Error"), MB_OK);
	}


	BITMAP qBitmap;
	qBitmap.bmHeight = 0;

	HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, wcsBrandLogoFilePath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	// Verify that the image was loaded
	if (hBitmap == NULL)
	{
		::MessageBox(NULL, __T("Brand Logo Image Loading Failed."), __T("Error"), MB_OK);
		//return ;
	}
	else
	{
		// Get the bitmap's parameters and verify the get
		int iReturn = GetObject(reinterpret_cast<HGDIOBJ>(hBitmap), sizeof(BITMAP), reinterpret_cast<LPVOID>(&qBitmap));
		if (!iReturn)
		{
			::MessageBox(NULL, __T("Brand Logo Image Loading Failed."), __T("Error"), MB_OK);
			//return;
		}
		else
		{
			//HWND hStatic = CreateWindow("STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, 100, 100, 200, 200, hwnd, (HMENU)10000, hInstance, NULL);
			//SendMessage(hListIDCBrandLog, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap);
			//SendMessage(hStatic, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap);
			if (qBitmap.bmWidth <= 320 && qBitmap.bmHeight <= 240)
			{
				HWND picBoxDisp = CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP | WS_TABSTOP | WS_BORDER, 20, 20, 320, 240, hwnd, (HMENU)10000, NULL, NULL);
				//hBitmap = (HBITMAP)LoadImage(NULL, L"bitmap2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
				SendMessage(picBoxDisp, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap);
			}
			else
			{
				::MessageBox(NULL, __T("Brand Image too large.\nIt should be less then 320x240"), __T("Error"), MB_OK);
				qBitmap.bmHeight = 0;
			}
		}
	}

	HWND txtBoxDisp = CreateWindow(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | WS_TABSTOP, 20, qBitmap.bmHeight + 50, 320, 240, hwnd, (HMENU)10000, NULL, NULL);
	Edit_SetText(txtBoxDisp, wcsBrandNameContent);

}

/////////////////////////////////////////////////////////////////////

//Login Dialog functions

INT_PTR CALLBACK LoginDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static wchar_t userName[24], pass[24];
	HWND hListUserName = GetDlgItem(hwnd, IDC_EDIT_USERNAME);
	HWND hListPass = GetDlgItem(hwnd, IDC_EDIT_PASS);
	HWND hListStatus = GetDlgItem(hwnd, IDC_STR_STATUS);

	switch (msg)
	{
	case WM_INITDIALOG:
		//OnInitLoginDialog(hwnd);
		if (!StrCmpW(userName, L"admin") && !StrCmpW(pass, L"admin"))
		{
			EndDialog(hwnd, LOWORD(wParam));
			DialogBox(NULL, MAKEINTRESOURCE(IDD_I2C_CONTROL), hwndForI2C, (DLGPROC)I2CControlDlgProc);
		}
		else
			Static_SetText(hListStatus, L"");
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			// ckeck pass is correct ?
			Edit_GetText(hListUserName, userName, 24);
			Edit_GetText(hListPass, pass, 24);
			if (!StrCmpW(userName, L"admin") && !StrCmpW(pass, L"admin"))
			{
				EndDialog(hwnd, LOWORD(wParam));
				DialogBox(NULL, MAKEINTRESOURCE(IDD_I2C_CONTROL), hwndForI2C, (DLGPROC)I2CControlDlgProc);
			}
			else
			{
				Static_SetText(hListStatus, L"Invalid UserName or Password.");
				Edit_SetText(hListUserName, L"");
				Edit_SetText(hListPass, L"");
			}

			return FALSE;

		case IDCANCEL:
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		}
		break;


	}

	return FALSE;
}



/////////////////////////////////////////////////////////////////////

// Dialog functions
void    OnInitI2CControlDialog(HWND hwnd);
HRESULT readI2cCommand(HWND hwnd);
HRESULT writeI2cCommand(HWND hwnd);

//-------------------------------------------------------------------
//  DayNightSettingsDlgProc
//
//  DayNightSettingsDlgProc procedure for the "Extension settings" dialog.
//-------------------------------------------------------------------

INT_PTR CALLBACK I2CControlDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
	case WM_INITDIALOG:
#ifdef DEBUG
		sprintf(logMessage, "\nCommand Fired : WM_INITDIALOG ID : IDD_I2C_CONTROL");
		printLogMessage(logMessage);
#endif
		OnInitI2CControlDialog(hwnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			//EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		case IDC_BTN_READ:
#ifdef DEBUG
			sprintf(logMessage, "\nCommand Fired : I2C Read ID : IDC_BTN_READ");
			printLogMessage(logMessage);
#endif
			readI2cCommand(hwnd);
			return TRUE;

		case IDC_BTN_WRITE:
#ifdef DEBUG
			sprintf(logMessage, "\nCommand Fired : I2C Write ID : IDC_BTN_WRITE");
			printLogMessage(logMessage);
#endif
			writeI2cCommand(hwnd);
			return TRUE;

		case IDCANCEL:
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		}
		break;


	}

	return FALSE;
}

static BYTE I2Cdata[11];

void OnInitI2CControlDialog(HWND hwnd)
{
	CString dataVal;
	//dataVal.Format(L"%.2x", pbPropertyValue[9]);
	//Edit_SetText(hListData1, dataVal);
	//dataVal.Format(L"%.2x", pbPropertyValue[10]);
	//Edit_SetText(hListData2, dataVal);


	HWND hListAddrByte = GetDlgItem(hwnd, IDC_EDIT_ADDR_BYTE);
	dataVal.Format(L"%.2x", I2Cdata[1]);
	Edit_SetText(hListAddrByte, dataVal);
	//Edit_SetText(hListAddrByte, L"0");
	HWND hListAddr1 = GetDlgItem(hwnd, IDC_EDIT_ADDR1);
	dataVal.Format(L"%.2x", I2Cdata[2]);
	Edit_SetText(hListAddr1, dataVal);
	//Edit_SetText(hListAddr1, L"00");
	HWND hListAddr2 = GetDlgItem(hwnd, IDC_EDIT_ADDR2);
	dataVal.Format(L"%.2x", I2Cdata[3]);
	Edit_SetText(hListAddr2, dataVal);
	//Edit_SetText(hListAddr2, L"00");
	HWND hListAddr3 = GetDlgItem(hwnd, IDC_EDIT_ADDR3);
	dataVal.Format(L"%.2x", I2Cdata[4]);
	Edit_SetText(hListAddr3, dataVal);
	//Edit_SetText(hListAddr3, L"00");
	HWND hListAddr4 = GetDlgItem(hwnd, IDC_EDIT_ADDR4);
	dataVal.Format(L"%.2x", I2Cdata[5]);
	Edit_SetText(hListAddr4, dataVal);
	//Edit_SetText(hListAddr4, L"00");
	HWND hListAddr5 = GetDlgItem(hwnd, IDC_EDIT_ADDR5);
	dataVal.Format(L"%.2x", I2Cdata[6]);
	Edit_SetText(hListAddr5, dataVal);
	//Edit_SetText(hListAddr5, L"00");
	HWND hListAddr6 = GetDlgItem(hwnd, IDC_EDIT_ADDR6);
	dataVal.Format(L"%.2x", I2Cdata[7]);
	Edit_SetText(hListAddr6, dataVal);
	//Edit_SetText(hListAddr6, L"00");
	HWND hListDataByte = GetDlgItem(hwnd, IDC_EDIT_DATA_BYTE);
	dataVal.Format(L"%.2x", I2Cdata[8]);
	Edit_SetText(hListDataByte, dataVal);
	//Edit_SetText(hListDataByte, L"0");
	HWND hListData1 = GetDlgItem(hwnd, IDC_EDIT_DATA1);
	dataVal.Format(L"%.2x", I2Cdata[9]);
	Edit_SetText(hListData1, dataVal);
	//Edit_SetText(hListData1, L"00");
	HWND hListData2 = GetDlgItem(hwnd, IDC_EDIT_DATA2);
	dataVal.Format(L"%.2x", I2Cdata[10]);
	Edit_SetText(hListData2, dataVal);
	//Edit_SetText(hListData2, L"00");


}

HRESULT readI2cCommand(HWND hwnd)
{
	HRESULT hr = S_OK;
	ULONG ulSize;
	BYTE *pbPropertyValue;

	int PropertyId = 16;

	hr = getExtionControlPropertySize(PropertyId, &ulSize);
	if (FAILED(hr))
	{
#ifdef DEBUG
		sprintf(logMessage, " \nERROR \t Function : readI2cCommand \t Msg : Unable to find property[%d] size : %x", PropertyId, hr);
		printLogMessage(logMessage);
#endif
	}
	else
	{
		pbPropertyValue = new BYTE[ulSize];
		if (!pbPropertyValue)
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : readI2cCommand \t Msg : Unable to allocate memory for property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}

		char str[4] = { '\0' };
		char DefChar = ' ';
		int xHexByte;
		wchar_t AddrByte[4];

		pbPropertyValue[0] = 0x00; // I2C read comman first byte 0

		// 2 byte : Number of addr byte
		HWND hListAddrByte = GetDlgItem(hwnd, IDC_EDIT_ADDR_BYTE);
		Edit_GetText(hListAddrByte, AddrByte, 2);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[1], (char *)&xHexByte, 1);

		// 3 byte : addr byte 1
		HWND hListAddr1 = GetDlgItem(hwnd, IDC_EDIT_ADDR1);
		Edit_GetText(hListAddr1, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[2], (char *)&xHexByte, 1);

		// 4 byte : addr byte 2
		HWND hListAddr2 = GetDlgItem(hwnd, IDC_EDIT_ADDR2);
		Edit_GetText(hListAddr2, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[3], (char *)&xHexByte, 1);

		// 5 byte : addr byte 3
		HWND hListAddr3 = GetDlgItem(hwnd, IDC_EDIT_ADDR3);
		Edit_GetText(hListAddr3, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[4], (char *)&xHexByte, 1);

		// 6 byte : addr byte 4
		HWND hListAddr4 = GetDlgItem(hwnd, IDC_EDIT_ADDR4);
		Edit_GetText(hListAddr4, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[5], (char *)&xHexByte, 1);

		// 7 byte : addr byte 5
		HWND hListAddr5 = GetDlgItem(hwnd, IDC_EDIT_ADDR5);
		Edit_GetText(hListAddr5, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[6], (char *)&xHexByte, 1);

		// 8 byte : addr byte 6
		HWND hListAddr6 = GetDlgItem(hwnd, IDC_EDIT_ADDR6);
		Edit_GetText(hListAddr6, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[7], (char *)&xHexByte, 1);

		// 9 byte : Number of addr byte
		HWND hListDataByte = GetDlgItem(hwnd, IDC_EDIT_DATA_BYTE);
		Edit_GetText(hListDataByte, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[8], (char *)&xHexByte, 1);

		// 10 byte : addr byte 1
		HWND hListData1 = GetDlgItem(hwnd, IDC_EDIT_DATA1);
		Edit_GetText(hListData1, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[9], (char *)&xHexByte, 1);

		// 11 byte : addr byte 2
		HWND hListData2 = GetDlgItem(hwnd, IDC_EDIT_DATA2);
		Edit_GetText(hListData2, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[10], (char *)&xHexByte, 1);

		hr = setExtionControlProperty(PropertyId, ulSize, pbPropertyValue);
		memset(pbPropertyValue, 0x00, ulSize);
		hr = getExtionControlProperty(PropertyId, ulSize, pbPropertyValue);

		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : readI2cCommand \t Msg : Unable to get property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			CString dataVal;
			if ((int)pbPropertyValue[8] == 2)
			{
				dataVal.Format(L"%.2x", pbPropertyValue[9]);
				Edit_SetText(hListData1, dataVal);
				dataVal.Format(L"%.2x", pbPropertyValue[10]);
				Edit_SetText(hListData2, dataVal);
			}
			else if ((int)pbPropertyValue[8] == 1)
			{
				dataVal.Format(L"%.2x", pbPropertyValue[9]);
				Edit_SetText(hListData1, dataVal);
				Edit_SetText(hListData2, L"00");
			}
			else
			{
				Edit_SetText(hListData1, L"00");
				Edit_SetText(hListData2, L"00");
			}
			memcpy(&I2Cdata, pbPropertyValue, 11);
		}
		delete[] pbPropertyValue;
	}

	return hr;

}

HRESULT writeI2cCommand(HWND hwnd)
{
	HRESULT hr = S_OK;
	ULONG ulSize;
	BYTE *pbPropertyValue;

	int PropertyId = 16;

	hr = getExtionControlPropertySize(PropertyId, &ulSize);
	if (FAILED(hr))
	{
#ifdef DEBUG
		sprintf(logMessage, " \nERROR \t Function : writeI2cCommand \t Msg : Unable to find property[%d] size : %x", PropertyId, hr);
		printLogMessage(logMessage);
#endif
	}
	else
	{
		pbPropertyValue = new BYTE[ulSize];
		if (!pbPropertyValue)
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : writeI2cCommand \t Msg : Unable to allocate memory for property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}

		char str[4] = { '\0' };
		char DefChar = ' ';
		int xHexByte;
		wchar_t AddrByte[4];

		pbPropertyValue[0] = 0x01; // I2C write comman first byte 1

		// 2 byte : Number of addr byte
		HWND hListAddrByte = GetDlgItem(hwnd, IDC_EDIT_ADDR_BYTE);
		Edit_GetText(hListAddrByte, AddrByte, 2);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[1], (char *)&xHexByte, 1);

		// 3 byte : addr byte 1
		HWND hListAddr1 = GetDlgItem(hwnd, IDC_EDIT_ADDR1);
		Edit_GetText(hListAddr1, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[2], (char *)&xHexByte, 1);

		// 4 byte : addr byte 2
		HWND hListAddr2 = GetDlgItem(hwnd, IDC_EDIT_ADDR2);
		Edit_GetText(hListAddr2, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[3], (char *)&xHexByte, 1);

		// 5 byte : addr byte 3
		HWND hListAddr3 = GetDlgItem(hwnd, IDC_EDIT_ADDR3);
		Edit_GetText(hListAddr3, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[4], (char *)&xHexByte, 1);

		// 6 byte : addr byte 4
		HWND hListAddr4 = GetDlgItem(hwnd, IDC_EDIT_ADDR4);
		Edit_GetText(hListAddr4, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[5], (char *)&xHexByte, 1);

		// 7 byte : addr byte 5
		HWND hListAddr5 = GetDlgItem(hwnd, IDC_EDIT_ADDR5);
		Edit_GetText(hListAddr5, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[6], (char *)&xHexByte, 1);

		// 8 byte : addr byte 6
		HWND hListAddr6 = GetDlgItem(hwnd, IDC_EDIT_ADDR6);
		Edit_GetText(hListAddr6, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[7], (char *)&xHexByte, 1);

		// 9 byte : Number of data byte
		HWND hListDataByte = GetDlgItem(hwnd, IDC_EDIT_DATA_BYTE);
		Edit_GetText(hListDataByte, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[8], (char *)&xHexByte, 1);

		// 10 byte : data byte 1
		HWND hListData1 = GetDlgItem(hwnd, IDC_EDIT_DATA1);
		Edit_GetText(hListData1, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[9], (char *)&xHexByte, 1);

		// 11 byte : data byte 2
		HWND hListData2 = GetDlgItem(hwnd, IDC_EDIT_DATA2);
		Edit_GetText(hListData2, AddrByte, 3);
		WideCharToMultiByte(CP_ACP, 0, AddrByte, -1, str, 2, &DefChar, NULL);
		str[2] = '\0';
		xHexByte = (long long)strtoll(str, NULL, 16);
		memcpy(&pbPropertyValue[10], (char *)&xHexByte, 1);

		hr = setExtionControlProperty(PropertyId, ulSize, pbPropertyValue);
		memcpy(&I2Cdata, pbPropertyValue, 11);
		delete[] pbPropertyValue;
	}

	return hr;

}

HRESULT I2cCommandInt(BYTE *pwritedata)
{
	HRESULT hr = S_OK;
	ULONG ulSize;
	BYTE *pbPropertyValue;

	int PropertyId = 16;

	hr = getExtionControlPropertySize(PropertyId, &ulSize);
	if (FAILED(hr))
	{
#ifdef DEBUG
		sprintf(logMessage, " \nERROR \t Function : writeI2cCommandInt \t Msg : Unable to find property[%d] size : %x", PropertyId, hr);
		printLogMessage(logMessage);
#endif
	}
	else
	{
		pbPropertyValue = new BYTE[ulSize];
		if (!pbPropertyValue)
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : writeI2cCommandInt \t Msg : Unable to allocate memory for property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}

		pbPropertyValue[0] = *(pwritedata + 0); // I2C write comman first byte 1
		pbPropertyValue[1] = *(pwritedata + 1); // 1 byte: for number of addr byte
		pbPropertyValue[2] = *(pwritedata + 2); // 6 bytes: for addrs
		pbPropertyValue[3] = *(pwritedata + 3);
		pbPropertyValue[4] = *(pwritedata + 4);
		pbPropertyValue[5] = *(pwritedata + 5);
		pbPropertyValue[6] = *(pwritedata + 6);
		pbPropertyValue[7] = *(pwritedata + 7);
		pbPropertyValue[8] = *(pwritedata + 8); // 1 byte: for number of data
		pbPropertyValue[9] = *(pwritedata + 9); // 2 bytes: for data
		pbPropertyValue[10] = *(pwritedata + 10);

		hr = setExtionControlProperty(PropertyId, ulSize, pbPropertyValue);
		delete[] pbPropertyValue;
	}

	return hr;

}

HRESULT I2cCommandOutt(BYTE *pwritedata, int *value)
{
	HRESULT hr = S_OK;
	ULONG ulSize;
	BYTE *pbPropertyValue;

	int PropertyId = 16;
	int data;
	hr = getExtionControlPropertySize(PropertyId, &ulSize);
	if (FAILED(hr))
	{
#ifdef DEBUG
		sprintf(logMessage, " \nERROR \t Function : writeI2cCommandInt \t Msg : Unable to find property[%d] size : %x", PropertyId, hr);
		printLogMessage(logMessage);
#endif
	}
	else
	{
		pbPropertyValue = new BYTE[ulSize];
		if (!pbPropertyValue)
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : writeI2cCommandInt \t Msg : Unable to allocate memory for property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}

		pbPropertyValue[0] = *(pwritedata + 0); // I2C write comman first byte 1
		pbPropertyValue[1] = *(pwritedata + 1); // 1 byte: for number of addr byte
		pbPropertyValue[2] = *(pwritedata + 2); // 6 bytes: for addrs
		pbPropertyValue[3] = *(pwritedata + 3);
		pbPropertyValue[4] = *(pwritedata + 4);
		pbPropertyValue[5] = *(pwritedata + 5);
		pbPropertyValue[6] = *(pwritedata + 6);
		pbPropertyValue[7] = *(pwritedata + 7);
		pbPropertyValue[8] = *(pwritedata + 8); // 1 byte: for number of data
		pbPropertyValue[9] = *(pwritedata + 9); // 2 bytes: for data
		pbPropertyValue[10] = *(pwritedata + 10);

		hr = setExtionControlProperty(PropertyId, ulSize, pbPropertyValue);
		memset(pbPropertyValue, 0x00, ulSize);
		hr = getExtionControlProperty(PropertyId, ulSize, pbPropertyValue);
		data = pbPropertyValue[9];
		delete[] pbPropertyValue;
		*value = data;
	}

	return hr;

}

void getRegVal(BYTE Regadd, int *data)
{
	BYTE *pbPropertyValue;
	HRESULT hr = S_OK;
	ULONG ulSize = 11;
	int value;
	int PropertyId = 16;
	pbPropertyValue = new BYTE[ulSize];
	pbPropertyValue[0] = 0x00; // I2C read comman first byte 1
	pbPropertyValue[1] = 0x04;
	pbPropertyValue[2] = 0x70;
	pbPropertyValue[3] = 0x52;
	pbPropertyValue[4] = 0x30;
	pbPropertyValue[5] = Regadd;
	pbPropertyValue[6] = 0x00;
	pbPropertyValue[7] = 0x00;
	pbPropertyValue[8] = 0x01;
	pbPropertyValue[9] = 0x01;
	pbPropertyValue[10] = 0x00;

	I2cCommandOutt(pbPropertyValue, &value);
	delete[] pbPropertyValue;
	*data = value;
}

void setRegVal(BYTE Regadd, int* data)
{
	BYTE *pbPropertyValue, value;
	HRESULT hr = S_OK;
	ULONG ulSize = 11;
	int PropertyId = 16;
	value = *data;

	pbPropertyValue = new BYTE[ulSize];
	pbPropertyValue[0] = 0x01; // I2C write comman first byte 1
	pbPropertyValue[1] = 0x04;
	pbPropertyValue[2] = 0x70;
	pbPropertyValue[3] = 0x52;
	pbPropertyValue[4] = 0x30;
	pbPropertyValue[5] = Regadd;
	pbPropertyValue[6] = 0x00;
	pbPropertyValue[7] = 0x00;
	pbPropertyValue[8] = 0x01;
	pbPropertyValue[9] = value;
	pbPropertyValue[10] = 0x00;

	I2cCommandInt(pbPropertyValue);
	delete[] pbPropertyValue;
}
/////////////////////////////////////////////////////////////////////

// Dialog functions
void    OnInitZoomControlDialog(HWND hwnd);
HRESULT	onDigZoomControlChange(HWND hwnd);
HRESULT	onOptZoomControlChange(HWND hwnd);
HRESULT	onDigZoomControlPosChange(HWND hwnd);

void	resetZoomControlInitSetting(HWND hwnd);
void	saveZoomControlInitSetting(HWND hwnd);
void	opticalZoomIn();
void	opticalZoomOut();
void	opticalZoomStop();
void    focusMode(HWND hwnd);
void    focusZoomTrk(HWND hwnd);
void    startFocusOne();
void    manuFocusNear();
void    manuFocusFar();
void	saveROIInitSetting(HWND hwnd);
void    OnInitROIDialog(HWND hwnd);
void	resetROIInitSetting(HWND hwnd);
void    ROIMode(HWND hwnd);

//-------------------------------------------------------------------
//  DayNightSettingsDlgProc
//
//  DayNightSettingsDlgProc procedure for the "Extension settings" dialog.
//-------------------------------------------------------------------

INT_PTR CALLBACK ZoomControlDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	POINT point;
	switch (msg)
	{
	case WM_INITDIALOG:
#ifdef DEBUG
		sprintf(logMessage, "\nCommand Fired : WM_INITDIALOG ID : IDD_ZOOM_CONTROL");
		printLogMessage(logMessage);
#endif
		SetWindowLong(GetDlgItem(hwnd, IDC_BTN_ZOOM_IN), GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_NOPARENTNOTIFY);
		SetWindowLong(GetDlgItem(hwnd, IDC_BTN_ZOOM_OUT), GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_NOPARENTNOTIFY);
		SetWindowLong(GetDlgItem(hwnd, IDC_BTN_FOCUS_START), GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_NOPARENTNOTIFY);
		SetWindowLong(GetDlgItem(hwnd, IDC_BTN_FOCUS_NEAR), GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_NOPARENTNOTIFY);
		//GetDlgItem(IDC_BTN_FOCUS_START);

		SetWindowLong(GetDlgItem(hwnd, IDC_BTN_FOCUS_FAR), GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_NOPARENTNOTIFY);
		saveZoomControlInitSetting(hwnd);
		OnInitZoomControlDialog(hwnd);
		return TRUE;
		break;


	case WM_PARENTNOTIFY:
		switch (LOWORD(wParam))
		{
		case WM_LBUTTONDOWN:
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);
			HWND pointHwnd = ChildWindowFromPoint(hwnd, point);
			if (pointHwnd == GetDlgItem(hwnd, IDC_BTN_ZOOM_IN))
			{
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_BTN_ZOOM_IN Event : button pressed");
				printLogMessage(logMessage);
#endif
				opticalZoomIn();
				//MessageBox(hwnd, L"Zoom in command", WINDOW_NAME, MB_OK);
			}
			else if (pointHwnd == GetDlgItem(hwnd, IDC_BTN_ZOOM_OUT))
			{
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_BTN_ZOOM_OUT Event : button pressed");
				printLogMessage(logMessage);
#endif
				opticalZoomOut();
				//MessageBox(hwnd, L"Zoom out command", WINDOW_NAME, MB_OK);
			}
			else if (pointHwnd == GetDlgItem(hwnd, IDC_BTN_FOCUS_START))
			{
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_BTN_FOCUS_START Event : button pressed");
				printLogMessage(logMessage);
#endif
				startFocusOne();
				//MessageBox(hwnd, L"Zoom in command", WINDOW_NAME, MB_OK);
			}
			else if (pointHwnd == GetDlgItem(hwnd, IDC_BTN_FOCUS_NEAR))
			{
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_BTN_FOCUS_NEAR Event : button pressed");
				printLogMessage(logMessage);
#endif
				manuFocusNear();
				//MessageBox(hwnd, L"Zoom out command", WINDOW_NAME, MB_OK);
			}
			else if (pointHwnd == GetDlgItem(hwnd, IDC_BTN_FOCUS_FAR))
			{
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_BTN_FOCUS_FAR Event : button pressed");
				printLogMessage(logMessage);
#endif
				manuFocusFar();
				//MessageBox(hwnd, L"Zoom out command", WINDOW_NAME, MB_OK);
			}


			return TRUE;
		}
		return TRUE;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		case IDC_BTN_ZOOM_IN:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case BN_CLICKED:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_BTN_ZOOM_IN Event : button released");
				printLogMessage(logMessage);
#endif
				opticalZoomStop();
				//MessageBox(hwnd, L"Zoom stop command", WINDOW_NAME, MB_OK);
				break;
			}
			return TRUE;

		case IDC_BTN_ZOOM_OUT:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case BN_CLICKED:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_BTN_ZOOM_OUT Event : button released");
				printLogMessage(logMessage);
#endif
				opticalZoomStop();
				//MessageBox(hwnd, L"Zoom stop command", WINDOW_NAME, MB_OK);
				break;
			}
			return TRUE;

		case IDC_BTN_FOCUS_NEAR:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case BN_CLICKED:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_BTN_FOCUS_NEAR Event : button released");
				printLogMessage(logMessage);
#endif
				opticalZoomStop();
				//MessageBox(hwnd, L"Zoom stop command", WINDOW_NAME, MB_OK);
				break;
			}
			return TRUE;

		case IDC_BTN_FOCUS_FAR:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case BN_CLICKED:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_BTN_FOCUS_FAR Event : button released");
				printLogMessage(logMessage);
#endif
				opticalZoomStop();
				//MessageBox(hwnd, L"Zoom stop command", WINDOW_NAME, MB_OK);
				break;
			}
			return TRUE;


		case IDC_COMBO_FOCUS_MODE:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case CBN_SELCHANGE:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_COMBO_FOCUS_MODE Event : CBN_SELCHANGE");
				printLogMessage(logMessage);
#endif
				//OnShutCtrlChange(hwnd);
				focusMode(hwnd);
				break;
			}
			return TRUE;

		case IDC_COMBO_FOCUS_ZOOMTRC:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case CBN_SELCHANGE:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_COMBO_FOCUS_ZOOMTRC Event : CBN_SELCHANGE");
				printLogMessage(logMessage);
#endif
				//OnShutCtrlChange(hwnd);
				focusZoomTrk(hwnd);
				break;
			}
			return TRUE;


		case IDCANCEL:
			resetZoomControlInitSetting(hwnd);
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		}
		break;

	case WM_HSCROLL:
		HWND hListSldDigZoomPos = GetDlgItem(hwnd, IDC_SLD_DIG_ZOOM_POS);
		if ((HWND)lParam == hListSldDigZoomPos)
		{
			switch (LOWORD(wParam)) // Find out what message it was
			{
			case TB_ENDTRACK: // This means that the list is about to display
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_SLD_DIG_ZOOM_POS Event : TB_ENDTRACK");
				printLogMessage(logMessage);
#endif
				onDigZoomControlPosChange(hwnd);
				break;
			}
			return TRUE;
		}
		break;

	}

	return FALSE;
}

void opticalZoomIn()
{
	HRESULT hr = S_OK;
	//KSPROPERTY_CAMERACONTROL_ZOOM // not implemented
	//getCameraTerminalControlPropertyCurrentValue(KSPROPERTY_CAMERACONTROL_ZOOM_RELATIVE, &currValue, &lCaps);
	if (ksNodeTree.isOKpCamControl)
	{

		//byte valueOpZoom[2];
		//valueOpZoom[0] = 1;
		//valueOpZoom[1] = 0;
		long valueOpZoom = 1;
		/*
		long valueOpZoom;
		int firstByte = 1;
		int sendByte = 0;
		memcpy((char*)&valueOpZoom, (char*)&firstByte, 2); // first 2 byte blue
		memcpy((char*)&valueOpZoom + 2, (char*)&sendByte, 2);  // last 2 byte red
		*/
		hr = ksNodeTree.pCamControl->Set(KSPROPERTY_CAMERACONTROL_ZOOM_RELATIVE, (long)valueOpZoom, CameraControl_Flags_Manual);

#ifdef DEBUG
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		sprintf(logMessage, " \nFunction : opticalZoomIn Msg : %d '%ws' ", hr, errMsg);
		printLogMessage(logMessage);
#endif
	}
	else
	{
#ifdef DEBUG
		sprintf(logMessage, " \nFunction : opticalZoomIn Msg : Camera Ternimal controls node not found. ");
		printLogMessage(logMessage);
#endif
	}

}

void opticalZoomOut()
{
	HRESULT hr = S_OK;
	//KSPROPERTY_CAMERACONTROL_ZOOM
	//getCameraTerminalControlPropertyCurrentValue(KSPROPERTY_CAMERACONTROL_ZOOM_RELATIVE, &currValue, &lCaps);
	if (ksNodeTree.isOKpCamControl)
	{
		//byte valueOpZoom[2];
		//valueOpZoom[0] = 0xff;
		//valueOpZoom[1] = 0;
		//long valueOpZoom = 0xff;
		long valueOpZoom = -1;
		hr = ksNodeTree.pCamControl->Set(KSPROPERTY_CAMERACONTROL_ZOOM_RELATIVE, (long)valueOpZoom, CameraControl_Flags_Manual);
#ifdef DEBUG
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		sprintf(logMessage, " \nFunction : opticalZoomOut Msg : %d '%ws' ", hr, errMsg);
		printLogMessage(logMessage);
#endif
	}
	else
	{
#ifdef DEBUG
		sprintf(logMessage, " \nFunction : opticalZoomOut Msg : Camera Ternimal controls node not found. ");
		printLogMessage(logMessage);
#endif
	}

}

void opticalZoomStop()
{
	HRESULT hr = S_OK;

	if (ksNodeTree.isOKpCamControl)
	{

		long testValue;
		testValue = (long)0;
		hr = ksNodeTree.pCamControl->Set(KSPROPERTY_CAMERACONTROL_ZOOM_RELATIVE, (long)testValue, KSPROPERTY_CAMERACONTROL_FLAGS_RELATIVE);
#ifdef DEBUG
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		sprintf(logMessage, " \nFunction :opticalZoomStop Msg : %d '%ws' ", hr, errMsg);
		printLogMessage(logMessage);
#endif

		// I2C command for optical zoom stop
		BYTE *pbPropertyValue;
		HRESULT hr = S_OK;
		ULONG ulSize = 11;
		int PropertyId = 16;
		pbPropertyValue = new BYTE[ulSize];
		pbPropertyValue[0] = 0x01; // I2C write comman first byte 1
		pbPropertyValue[1] = 0x04;
		pbPropertyValue[2] = 0x70;
		pbPropertyValue[3] = 0x82;
		pbPropertyValue[4] = 0x30;
		pbPropertyValue[5] = 0x10;
		pbPropertyValue[6] = 0x00;
		pbPropertyValue[7] = 0x00;
		pbPropertyValue[8] = 0x01;
		pbPropertyValue[9] = 0x00;
		pbPropertyValue[10] = 0x00;

		hr = setExtionControlProperty(PropertyId, ulSize, pbPropertyValue);
#ifdef DEBUG
		_com_error err1(hr);
		LPCTSTR errMsg1 = err1.ErrorMessage();
		sprintf(logMessage, " \nFunction :opticalZoomStop I2C command Msg : %d '%ws' ", hr, errMsg1);
		printLogMessage(logMessage);
#endif
		delete[] pbPropertyValue;
	}
	else
	{
#ifdef DEBUG
		sprintf(logMessage, " \nFunction : opticalZoomStop Msg : Camera Ternimal controls node not found. ");
		printLogMessage(logMessage);
#endif
	}

}

void    focusMode(HWND hwnd)
{

	BYTE *pbPropertyValue;
	HRESULT hr = S_OK;
	ULONG ulSize = 11;
	int PropertyId = 16;
	//HRESULT hr = S_OK;
	pbPropertyValue = new BYTE[ulSize];
	HWND hListFocusMode = GetDlgItem(hwnd, IDC_COMBO_FOCUS_MODE);


	int sel = ComboBox_GetCurSel(hListFocusMode);

	//HWND hNearBNT = GetDlgItem(hwnd, IDC_BTN_FOCUS_FAR);
	//Button_Enable(hNearBNT, TRUE);
	if (sel == 1){
		BOOL flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_FAR), TRUE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_NEAR), TRUE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_START), FALSE);

	}
	else if (sel == 2){
		BOOL flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_FAR), FALSE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_NEAR), FALSE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_START), TRUE);
	}
	else {
		BOOL flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_FAR), FALSE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_NEAR), FALSE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_START), FALSE);
	}

	//ON_UPDATE_COMMAND_UI();

	pbPropertyValue[0] = 0x01; // I2C write comman first byte 1
	pbPropertyValue[1] = 0x04;
	pbPropertyValue[2] = 0x70;
	pbPropertyValue[3] = 0x82;
	pbPropertyValue[4] = 0x30;
	pbPropertyValue[5] = 0x40;
	pbPropertyValue[6] = 0x00;
	pbPropertyValue[7] = 0x00;
	pbPropertyValue[8] = 0x01;
	switch (sel)
	{
	case 0:
		pbPropertyValue[9] = 1; //auto
		break;
	case 3:
		pbPropertyValue[9] = 3; //focus when zoom comp
		break;
	case 1:
		pbPropertyValue[9] = 0; //manual
		break;
	case 2:
		pbPropertyValue[9] = 2; //auto one time
		break;
	}
	pbPropertyValue[10] = 0x00;
	if (sel != initCtrlSetting.FocusMode){
		//initCtrlSetting.FocusMode_bak = initCtrlSetting.FocusMode;
		initCtrlSetting.FocusMode = sel;
		I2cCommandInt(pbPropertyValue);
	}
	delete[] pbPropertyValue;
}

void    focusZoomTrk(HWND hwnd)
{

	BYTE *pbPropertyValue;
	HRESULT hr = S_OK;
	ULONG ulSize = 11;
	int PropertyId = 16;
	//HRESULT hr = S_OK;
	pbPropertyValue = new BYTE[ulSize];
	HWND hListFocusTrack = GetDlgItem(hwnd, IDC_COMBO_FOCUS_ZOOMTRC);

	int sel = ComboBox_GetCurSel(hListFocusTrack);

	pbPropertyValue[0] = 0x01; // I2C write comman first byte 1
	pbPropertyValue[1] = 0x04;
	pbPropertyValue[2] = 0x70;
	pbPropertyValue[3] = 0x82;
	pbPropertyValue[4] = 0x30;
	pbPropertyValue[5] = 0x48;
	pbPropertyValue[6] = 0x00;
	pbPropertyValue[7] = 0x00;
	pbPropertyValue[8] = 0x01;
	pbPropertyValue[9] = (sel + 0x1) & 0x1;// 0x00;
	pbPropertyValue[10] = 0x00;

	if (sel != initCtrlSetting.FocusTrk){
		//initCtrlSetting.FocusTrk_bak = initCtrlSetting.FocusTrk;
		initCtrlSetting.FocusTrk = sel;

		I2cCommandInt(pbPropertyValue);
	}
	delete[] pbPropertyValue;
}

void    refocusMode(HWND hwnd, int sel)
{

	BYTE *pbPropertyValue;
	HRESULT hr = S_OK;
	ULONG ulSize = 11;
	int PropertyId = 16;
	//HRESULT hr = S_OK;
	pbPropertyValue = new BYTE[ulSize];

	if (sel == 1){
		BOOL flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_FAR), TRUE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_NEAR), TRUE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_START), FALSE);

	}
	else if (sel == 2){
		BOOL flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_FAR), FALSE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_NEAR), FALSE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_START), TRUE);
	}
	else {
		BOOL flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_FAR), FALSE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_NEAR), FALSE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_START), FALSE);
	}
	//ON_UPDATE_COMMAND_UI();

	pbPropertyValue[0] = 0x01; // I2C write comman first byte 1
	pbPropertyValue[1] = 0x04;
	pbPropertyValue[2] = 0x70;
	pbPropertyValue[3] = 0x82;
	pbPropertyValue[4] = 0x30;
	pbPropertyValue[5] = 0x40;
	pbPropertyValue[6] = 0x00;
	pbPropertyValue[7] = 0x00;
	pbPropertyValue[8] = 0x01;
	switch (sel)
	{
	case 0:
		pbPropertyValue[9] = 1; //auto
		break;
	case 3:
		pbPropertyValue[9] = 3; //focus when zoom comp
		break;
	case 1:
		pbPropertyValue[9] = 0; //manual
		break;
	case 2:
		pbPropertyValue[9] = 2; //auto one time
		break;
	}
	pbPropertyValue[10] = 0x00;
	//if (sel != initCtrlSetting.FocusMode){
	//initCtrlSetting.FocusMode_bak = initCtrlSetting.FocusMode;
	//initCtrlSetting.FocusMode = sel;
	I2cCommandInt(pbPropertyValue);
	//}
	delete[] pbPropertyValue;
}

void    refocusZoomTrk(HWND hwnd, int sel)
{

	BYTE *pbPropertyValue;
	HRESULT hr = S_OK;
	ULONG ulSize = 11;
	int PropertyId = 16;
	//HRESULT hr = S_OK;
	pbPropertyValue = new BYTE[ulSize];

	pbPropertyValue[0] = 0x01; // I2C write comman first byte 1
	pbPropertyValue[1] = 0x04;
	pbPropertyValue[2] = 0x70;
	pbPropertyValue[3] = 0x82;
	pbPropertyValue[4] = 0x30;
	pbPropertyValue[5] = 0x48;
	pbPropertyValue[6] = 0x00;
	pbPropertyValue[7] = 0x00;
	pbPropertyValue[8] = 0x01;
	pbPropertyValue[9] = (sel + 0x1) & 0x1;// 0x00;
	pbPropertyValue[10] = 0x00;

	if (1 || sel != initCtrlSetting.FocusTrk){
		//initCtrlSetting.FocusTrk_bak = initCtrlSetting.FocusTrk;
		//initCtrlSetting.FocusTrk = pbPropertyValue[9];

		I2cCommandInt(pbPropertyValue);
	}
	delete[] pbPropertyValue;
}

void    startFocusOne()
{

	BYTE *pbPropertyValue;
	HRESULT hr = S_OK;
	ULONG ulSize = 11;
	int PropertyId = 16;
	pbPropertyValue = new BYTE[ulSize];
	pbPropertyValue[0] = 0x01; // I2C write comman first byte 1
	pbPropertyValue[1] = 0x04;
	pbPropertyValue[2] = 0x70;
	pbPropertyValue[3] = 0x82;
	pbPropertyValue[4] = 0x30;
	pbPropertyValue[5] = 0x42;
	pbPropertyValue[6] = 0x00;
	pbPropertyValue[7] = 0x00;
	pbPropertyValue[8] = 0x01;
	pbPropertyValue[9] = 0x01;
	pbPropertyValue[10] = 0x00;

	I2cCommandInt(pbPropertyValue);
	delete[] pbPropertyValue;
}

void    manuFocusNear()
{

	BYTE *pbPropertyValue;
	HRESULT hr = S_OK;
	ULONG ulSize = 11;
	int PropertyId = 16;
	pbPropertyValue = new BYTE[ulSize];
	pbPropertyValue[0] = 0x01; // I2C write comman first byte 1
	pbPropertyValue[1] = 0x04;
	pbPropertyValue[2] = 0x70;
	pbPropertyValue[3] = 0x82;
	pbPropertyValue[4] = 0x30;
	pbPropertyValue[5] = 0x10;
	pbPropertyValue[6] = 0x00;
	pbPropertyValue[7] = 0x00;
	pbPropertyValue[8] = 0x01;
	pbPropertyValue[9] = 0x10;
	pbPropertyValue[10] = 0x00;

	I2cCommandInt(pbPropertyValue);
	delete[] pbPropertyValue;
}

void    manuFocusFar()
{

	BYTE *pbPropertyValue;
	HRESULT hr = S_OK;
	ULONG ulSize = 11;
	int PropertyId = 16;
	pbPropertyValue = new BYTE[ulSize];
	pbPropertyValue[0] = 0x01; // I2C write comman first byte 1
	pbPropertyValue[1] = 0x04;
	pbPropertyValue[2] = 0x70;
	pbPropertyValue[3] = 0x82;
	pbPropertyValue[4] = 0x30;
	pbPropertyValue[5] = 0x10;
	pbPropertyValue[6] = 0x00;
	pbPropertyValue[7] = 0x00;
	pbPropertyValue[8] = 0x01;
	pbPropertyValue[9] = 0x20;
	pbPropertyValue[10] = 0x00;

	I2cCommandInt(pbPropertyValue);
	delete[] pbPropertyValue;
}


void OnInitZoomControlDialog(HWND hwnd)
{
	HRESULT hr = S_OK;
	long lMin, lMax, lStep, lDefault;
	long currValue, lCaps;
	CString currValueStr;

	hr = getStandardControlPropertyRange(KSPROPERTY_VIDEOPROCAMP_DIGITAL_MULTIPLIER, &lMin, &lMax, &lStep, &lDefault, &lCaps);
	HWND hListSldDigZoomPos = GetDlgItem(hwnd, IDC_SLD_DIG_ZOOM_POS);

	if (!SUCCEEDED(hr))
	{
		EnableWindow(hListSldDigZoomPos, FALSE);
	}

	getStandardControlPropertyCurrentValue(KSPROPERTY_VIDEOPROCAMP_DIGITAL_MULTIPLIER, &currValue, &lCaps);
	SendMessageA(hListSldDigZoomPos, TBM_SETRANGEMAX, TRUE, lMax);
	SendMessageA(hListSldDigZoomPos, TBM_SETRANGEMIN, TRUE, lMin);
	SendMessageA(hListSldDigZoomPos, TBM_SETPAGESIZE, TRUE, 1);
	SendMessageA(hListSldDigZoomPos, TBM_SETPOS, TRUE, currValue);
	HWND hListDigZoomPos = GetDlgItem(hwnd, IDC_EDIT_DIG_ZOOM_POS);
	currValueStr.Format(L"%ld", currValue);
	Edit_SetText(hListDigZoomPos, currValueStr);
	EnableWindow(hListDigZoomPos, FALSE);

	HWND hListFocusMode = GetDlgItem(hwnd, IDC_COMBO_FOCUS_MODE);
	ComboBox_AddString(hListFocusMode, L"Continuous Auto Focus");
	ComboBox_AddString(hListFocusMode, L"Manual Focus");
	ComboBox_AddString(hListFocusMode, L"One Time Auto Focus");
	//ComboBox_AddString(hListFocusMode, L"Focus Once Zoom Complete");
	ComboBox_SetCurSel(hListFocusMode, initCtrlSetting.FocusMode);
	if (initCtrlSetting.FocusMode == 1){//manu
		BOOL flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_FAR), TRUE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_NEAR), TRUE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_START), FALSE);
	}
	else if (initCtrlSetting.FocusMode == 2){//one time
		BOOL flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_FAR), FALSE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_NEAR), FALSE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_START), TRUE);
	}
	else {
		BOOL flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_FAR), FALSE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_NEAR), FALSE);
		flag = EnableWindow(GetDlgItem(hwnd, IDC_BTN_FOCUS_START), FALSE);
	}


	HWND hListFocusTrack = GetDlgItem(hwnd, IDC_COMBO_FOCUS_ZOOMTRC);
	ComboBox_AddString(hListFocusTrack, L"Enable");
	ComboBox_AddString(hListFocusTrack, L"Disable");
	ComboBox_SetCurSel(hListFocusTrack, initCtrlSetting.FocusTrk);

}

void saveZoomControlInitSetting(HWND hwnd)
{
	long currValue, lCaps;

	// reset global struct
	initCtrlSetting.DigitalMultiplier = 0;
	initCtrlSetting.FocusMode_bak = initCtrlSetting.FocusMode;
	initCtrlSetting.FocusTrk_bak = initCtrlSetting.FocusTrk;

	// get current Value
	currValue = 0;
	getStandardControlPropertyCurrentValue(KSPROPERTY_VIDEOPROCAMP_DIGITAL_MULTIPLIER, &currValue, &lCaps);
	initCtrlSetting.DigitalMultiplier = currValue;

}

void resetZoomControlInitSetting(HWND hwnd)
{
	if (ksNodeTree.isOKpProcAmp)
	{
		HWND hListSldDigZoomPos = GetDlgItem(hwnd, IDC_SLD_DIG_ZOOM_POS);
		long SldPos = (long)SendMessageA(hListSldDigZoomPos, TBM_GETPOS, TRUE, SldPos);
		if (SldPos != initCtrlSetting.DigitalMultiplier)
			ksNodeTree.pProcAmp->Set(KSPROPERTY_VIDEOPROCAMP_DIGITAL_MULTIPLIER, (long)initCtrlSetting.DigitalMultiplier, VideoProcAmp_Flags_Manual);

		if (initCtrlSetting.FocusMode != initCtrlSetting.FocusMode_bak){
			initCtrlSetting.FocusMode = initCtrlSetting.FocusMode_bak;
			refocusMode(hwnd, initCtrlSetting.FocusMode);
		}
		if (initCtrlSetting.FocusTrk != initCtrlSetting.FocusTrk_bak){
			initCtrlSetting.FocusTrk = initCtrlSetting.FocusTrk_bak;
			refocusZoomTrk(hwnd, initCtrlSetting.FocusTrk);
		}

	}
}


HRESULT	onDigZoomControlPosChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	long SldPos = 0;
	CString strPos;

	HWND hListDigZoomPos = GetDlgItem(hwnd, IDC_EDIT_DIG_ZOOM_POS);
	HWND hListSldDigZoomPos = GetDlgItem(hwnd, IDC_SLD_DIG_ZOOM_POS);

	SldPos = (long)SendMessageA(hListSldDigZoomPos, TBM_GETPOS, TRUE, SldPos);
	strPos.Format(L"%ld", SldPos);
	Edit_SetText(hListDigZoomPos, strPos);

	if (ksNodeTree.isOKpProcAmp)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nbmRequestType:SET \t bRequest:SET_CUR \t wValue:digital zoom \t wIndex:0x02\t PutValue:%ld", SldPos);
		printLogMessage(logMessage);
#endif
		hr = ksNodeTree.pProcAmp->Set(KSPROPERTY_VIDEOPROCAMP_DIGITAL_MULTIPLIER, SldPos, VideoProcAmp_Flags_Manual);
#ifdef DEBUG
		sprintf(logMessage, "\nFunction : onDigZoomControlChange Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

/////////////////////////////////////////////////////////////////////

// Dialog functions
void    OnInitIrisControlDialog(HWND hwnd);
HRESULT onManualIrisChange(HWND hwnd);
HRESULT OnAEPriorityChange(HWND hwnd);
HRESULT onExposureTimeAbsoluteChange(HWND hwnd);
void saveCameraTerminalControlInitSetting(HWND hwnd);
void resetCameraTerminalControlInitSetting(HWND hwnd);
HRESULT onExposureModeChange(HWND hwnd);
HRESULT OnIrisModeChange(HWND hwnd);
void writeIrisMode(int irisModeValue);
int readIrisMode();

//-------------------------------------------------------------------
//  DayNightSettingsDlgProc
//
//  DayNightSettingsDlgProc procedure for the "Extension settings" dialog.
//-------------------------------------------------------------------

INT_PTR CALLBACK IrisControlDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
	case WM_INITDIALOG:
#ifdef DEBUG
		sprintf(logMessage, "\nCommand Fired : WM_INITDIALOG ID : IDD_IRIS_CONTROL");
		printLogMessage(logMessage);
#endif
		saveCameraTerminalControlInitSetting(hwnd);
		OnInitIrisControlDialog(hwnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		case IDC_COMBO_AE_PRIORITY:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case CBN_SELCHANGE:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_COMBO_AE_PRIORITY Event : CBN_SELCHANGE");
				printLogMessage(logMessage);
#endif
				OnAEPriorityChange(hwnd);
				break;
			}
			return TRUE;

		case IDC_CHECK_AUTO_EXP:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case BN_CLICKED:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_CHECK_AUTO_EXP Event : BN_CLICKED");
				printLogMessage(logMessage);
#endif
				onExposureModeChange(hwnd);
				break;
			}
			return TRUE;

		case IDC_CHECK_AUTO_IRIS:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case BN_CLICKED:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_CHECK_AUTO_IRIS Event : BN_CLICKED");
				printLogMessage(logMessage);
#endif
				OnIrisModeChange(hwnd);
				break;
			}
			return TRUE;

		case IDCANCEL:
			resetCameraTerminalControlInitSetting(hwnd);
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		}
		break;

	case WM_HSCROLL:
		HWND hListSldManualIris = GetDlgItem(hwnd, IDC_SLD_MANUAL_IRIS);
		if ((HWND)lParam == hListSldManualIris)
		{
			switch (LOWORD(wParam)) // Find out what message it was
			{
			case TB_ENDTRACK: // This means that the list is about to display
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_SLD_MANUAL_IRIS Event : TB_ENDTRACK");
				printLogMessage(logMessage);
#endif
				onManualIrisChange(hwnd);
				break;
			}
			return TRUE;
		}

		HWND hSLDExpTmAbs = GetDlgItem(hwnd, IDC_SLD_EXP_TIME_ABS);
		if ((HWND)lParam == hSLDExpTmAbs)
		{
			switch (LOWORD(wParam)) // Find out what message it was
			{
			case TB_ENDTRACK: // This means that the list is about to display
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_SLD_EXP_TIME_ABS Event : TB_ENDTRACK");
				printLogMessage(logMessage);
#endif
				onExposureTimeAbsoluteChange(hwnd);
				break;
			}
			return TRUE;
		}

	}

	return FALSE;
}

void saveCameraTerminalControlInitSetting(HWND hwnd)
{
	long currValue, lCaps;

	// reset global struct
	initCtrlSetting.AEPriority = 0;
	initCtrlSetting.ExposureTimeAbsolute = 0;

	// get current Value
	currValue = 0;
	getCameraTerminalControlPropertyCurrentValue(KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY, &currValue, &lCaps);
	initCtrlSetting.AEPriority = currValue;
	currValue = 0;
	getCameraTerminalControlPropertyCurrentValue(KSPROPERTY_CAMERACONTROL_EXPOSURE, &currValue, &lCaps);
	initCtrlSetting.ExposureTimeAbsolute = currValue;

}

void resetCameraTerminalControlInitSetting(HWND hwnd)
{
	if (ksNodeTree.isOKpCamControl)
	{
		HWND hListAEPriority = GetDlgItem(hwnd, IDC_COMBO_AE_PRIORITY);
		int sel = ComboBox_GetCurSel(hListAEPriority);
		if (sel != initCtrlSetting.AEPriority)
			ksNodeTree.pCamControl->Set(KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY, (long)initCtrlSetting.AEPriority, CameraControl_Flags_Manual);

		long SldPos = 0;
		HWND hListSldExpTmAbs = GetDlgItem(hwnd, IDC_SLD_EXP_TIME_ABS);
		SldPos = (long)SendMessageA(hListSldExpTmAbs, TBM_GETPOS, TRUE, SldPos);
		if (SldPos != initCtrlSetting.ExposureTimeAbsolute)
			ksNodeTree.pCamControl->Set(KSPROPERTY_CAMERACONTROL_EXPOSURE, (long)initCtrlSetting.ExposureTimeAbsolute, KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE);
	}
}

void OnInitIrisControlDialog(HWND hwnd)
{
	HRESULT hr = S_OK;
	long lMin, lMax, lStep, lDefault;
	long currValue, lCaps;
	CString strValue;

	//HWND hListCheckExpAuto = GetDlgItem(hwnd, IDC_CHECK_AUTO_EXP);
	//SendMessageA(hListCheckExpAuto, BM_SETCHECK, BST_CHECKED, NULL);

	getCameraTerminalControlPropertyCurrentValue(KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY, &currValue, &lCaps);
	HWND hListAEPriority = GetDlgItem(hwnd, IDC_COMBO_AE_PRIORITY);
	ComboBox_AddString(hListAEPriority, L"Constant frame rate");
	ComboBox_AddString(hListAEPriority, L"Dynamic frame rate");
	ComboBox_SetCurSel(hListAEPriority, currValue);

	getCameraTerminalControlPropertyRange(KSPROPERTY_CAMERACONTROL_EXPOSURE, &lMin, &lMax, &lStep, &lDefault, &lCaps);
	getCameraTerminalControlPropertyCurrentValue(KSPROPERTY_CAMERACONTROL_EXPOSURE, &currValue, &lCaps);
	HWND hListSldExpTmAbs = GetDlgItem(hwnd, IDC_SLD_EXP_TIME_ABS);
	SendMessageA(hListSldExpTmAbs, TBM_SETRANGEMAX, TRUE, lMax);
	SendMessageA(hListSldExpTmAbs, TBM_SETRANGEMIN, TRUE, lMin);
	SendMessageA(hListSldExpTmAbs, TBM_SETPAGESIZE, TRUE, 1);
	SendMessageA(hListSldExpTmAbs, TBM_SETPOS, TRUE, currValue);
	HWND hListEditExpTmAbs = GetDlgItem(hwnd, IDC_EDIT_EXP_TIME_ABS);
	strValue.Format(L"%ld", currValue);
	Edit_SetText(hListEditExpTmAbs, strValue);

	getCameraTerminalControlPropertyRange(KSPROPERTY_CAMERACONTROL_IRIS, &lMin, &lMax, &lStep, &lDefault, &lCaps);
	getCameraTerminalControlPropertyCurrentValue(KSPROPERTY_CAMERACONTROL_IRIS, &currValue, &lCaps);

	HWND hListManualIris = GetDlgItem(hwnd, IDC_SLD_MANUAL_IRIS);
	SendMessageA(hListManualIris, TBM_SETRANGEMAX, TRUE, (lMax / 10));
	SendMessageA(hListManualIris, TBM_SETRANGEMIN, TRUE, lMin);
	SendMessageA(hListManualIris, TBM_SETPAGESIZE, TRUE, 1);
	if (currValue == 0)
	{
		//EnableWindow(hListManualIris, FALSE);
		SendMessageA(hListManualIris, TBM_SETPOS, TRUE, currValue);
	}
	else
	{

		SendMessageA(hListManualIris, TBM_SETPOS, TRUE, (currValue / 10));
	}


	HWND hListEditManualIris = GetDlgItem(hwnd, IDC_EDIT_MANUAL_IRIS);
	strValue.Format(L"%ld", (currValue / 10));
	Edit_SetText(hListEditManualIris, strValue);
	EnableWindow(hListEditManualIris, FALSE);

	int isExpAuto;
	hr = getExtControlValue(1, &isExpAuto);
	if (SUCCEEDED(hr))
	{
		if (isExpAuto == 0)
		{
			HWND hListCheckExpAuto = GetDlgItem(hwnd, IDC_CHECK_AUTO_EXP);
			LRESULT autoExp = SendMessageA(hListCheckExpAuto, BM_SETCHECK, BST_CHECKED, 0);
			EnableWindow(hListSldExpTmAbs, FALSE);
		}

	}

	if (!readIrisMode())
	{
		HWND hListCheckIrisAuto = GetDlgItem(hwnd, IDC_CHECK_AUTO_IRIS);
		LRESULT autoExp = SendMessageA(hListCheckIrisAuto, BM_SETCHECK, BST_CHECKED, 0);
		EnableWindow(hListManualIris, FALSE);
	}
}

HRESULT	OnAEPriorityChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	HWND hListAEPriority = GetDlgItem(hwnd, IDC_COMBO_AE_PRIORITY);

	int sel = ComboBox_GetCurSel(hListAEPriority);

	if (sel != LB_ERR && ksNodeTree.isOKpCamControl)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nbmRequestType:SET \t bRequest:SET_CUR \t wValue:AE priority control \t wIndex:0x01\t PutValue:%d", sel);
		printLogMessage(logMessage);
#endif

		hr = ksNodeTree.pCamControl->Set(KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY, (long)sel, CameraControl_Flags_Manual);

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : OnAEPriorityChange Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT onExposureTimeAbsoluteChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	long SldPos = 0;
	CString strPos;
	HWND hListSldExpTmAbs = GetDlgItem(hwnd, IDC_SLD_EXP_TIME_ABS);
	SldPos = (long)SendMessageA(hListSldExpTmAbs, TBM_GETPOS, TRUE, SldPos);

	//SldPos = -3;

	HWND hListEditExpTmAbs = GetDlgItem(hwnd, IDC_EDIT_EXP_TIME_ABS);
	strPos.Format(L"%ld", SldPos);
	Edit_SetText(hListEditExpTmAbs, strPos);

	if (ksNodeTree.isOKpCamControl)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nbmRequestType:SET \t bRequest:SET_CUR \t wValue:Exposure time control (absolute) \t wIndex:0x01\t PutValue:%ld", SldPos);
		printLogMessage(logMessage);
#endif
		hr = ksNodeTree.pCamControl->Set(KSPROPERTY_CAMERACONTROL_EXPOSURE, SldPos, KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE);
#ifdef DEBUG
		sprintf(logMessage, "\nFunction : onExposureTimeAbsoluteChange Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT onManualIrisChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	long SldPos = 0;
	CString strPos;
	HWND hListSldManualIris = GetDlgItem(hwnd, IDC_SLD_MANUAL_IRIS);
	SldPos = (long)SendMessageA(hListSldManualIris, TBM_GETPOS, TRUE, SldPos);

	HWND hListEditManualIris = GetDlgItem(hwnd, IDC_EDIT_MANUAL_IRIS);
	strPos.Format(L"%ld", SldPos);
	Edit_SetText(hListEditManualIris, strPos);

	if (ksNodeTree.isOKpCamControl)
	{
		hr = ksNodeTree.pCamControl->Set(KSPROPERTY_CAMERACONTROL_IRIS, (SldPos * 10), KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE);
	}

	return hr;
}

HRESULT onExposureModeChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	long SldPos = 0;
	CString strPos;
	HWND hListSldExpTmAbs = GetDlgItem(hwnd, IDC_SLD_EXP_TIME_ABS);
	SldPos = (long)SendMessageA(hListSldExpTmAbs, TBM_GETPOS, TRUE, SldPos);

	HWND hListEditExpTmAbs = GetDlgItem(hwnd, IDC_EDIT_EXP_TIME_ABS);
	strPos.Format(L"%ld", SldPos);
	Edit_SetText(hListEditExpTmAbs, strPos);

	if (ksNodeTree.isOKpCamControl)
	{
		HWND hListCheckExpAuto = GetDlgItem(hwnd, IDC_CHECK_AUTO_EXP);
		LRESULT autoExp = SendMessageA(hListCheckExpAuto, BM_GETCHECK, 0, 0);
		if (BST_CHECKED == autoExp)
		{
			EnableWindow(hListSldExpTmAbs, FALSE);
			hr = ksNodeTree.pCamControl->Set(KSPROPERTY_CAMERACONTROL_EXPOSURE, 0, KSPROPERTY_CAMERACONTROL_FLAGS_AUTO);
		}
		else
		{
			EnableWindow(hListSldExpTmAbs, TRUE);
			//hr = ksNodeTree.pCamControl->Set(KSPROPERTY_CAMERACONTROL_EXPOSURE, SldPos, KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE);
			hr = ksNodeTree.pCamControl->Set(KSPROPERTY_CAMERACONTROL_EXPOSURE, SldPos, KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL);
		}
	}

	return hr;
}

HRESULT OnIrisModeChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	long SldPos = 0;
	CString strPos;
	HWND hListSldManualIris = GetDlgItem(hwnd, IDC_SLD_MANUAL_IRIS);
	SldPos = (long)SendMessageA(hListSldManualIris, TBM_GETPOS, TRUE, SldPos);

	HWND hListEditManualIris = GetDlgItem(hwnd, IDC_EDIT_MANUAL_IRIS);
	strPos.Format(L"%ld", SldPos);
	Edit_SetText(hListEditManualIris, strPos);

	long currValue, lCaps;

	if (ksNodeTree.isOKpCamControl)
	{
		HWND hListCheckIrisAuto = GetDlgItem(hwnd, IDC_CHECK_AUTO_IRIS);
		LRESULT autoIris = SendMessageA(hListCheckIrisAuto, BM_GETCHECK, 0, 0);
		if (BST_CHECKED == autoIris)
		{
			EnableWindow(hListSldManualIris, FALSE);
			hr = ksNodeTree.pCamControl->Set(KSPROPERTY_CAMERACONTROL_IRIS, 0, KSPROPERTY_CAMERACONTROL_FLAGS_AUTO);
			writeIrisMode(0);
		}
		else
		{
			EnableWindow(hListSldManualIris, TRUE);
			hr = ksNodeTree.pCamControl->Set(KSPROPERTY_CAMERACONTROL_IRIS, (SldPos * 10), KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL);
			writeIrisMode(1);
		}
	}

	return hr;
}

int readIrisMode()
{
	int irisModeValue = 1;
	char irisModeFileFull[260];
	strcpy(irisModeFileFull, pathExe);
	strcat(irisModeFileFull, irisModeFile);

	FILE *fpSnapCount = fopen(irisModeFileFull, "r");
	if (fpSnapCount)
	{
		fscanf(fpSnapCount, "%d", &irisModeValue);
		fclose(fpSnapCount);
	}
	else
	{
		snapshotIndex = 0;
	}
	return irisModeValue;
}

void writeIrisMode(int irisModeValue)
{
	char irisModeFileFull[260];
	strcpy(irisModeFileFull, pathExe);
	strcat(irisModeFileFull, irisModeFile);

	wchar_t wcsDetmpPath[1000];
	memset(wcsDetmpPath, 0, sizeof(wcsDetmpPath));
	MultiByteToWideChar(CP_UTF8, 0, &irisModeFileFull[0], (int)strlen(irisModeFileFull), &wcsDetmpPath[0], sizeof(wcsDetmpPath));

	SetFileAttributes(wcsDetmpPath, FILE_ATTRIBUTE_NORMAL);

	FILE *fpSnapCount = fopen(irisModeFileFull, "w");
	if (fpSnapCount)
	{
		fprintf(fpSnapCount, "%d", irisModeValue);
		fclose(fpSnapCount);

		SetFileAttributes(wcsDetmpPath, FILE_ATTRIBUTE_HIDDEN);
	}

}

/////////////////////////////////////////////////////////////////////

// Dialog functions

//-------------------------------------------------------------------
//  ImageCaptureDlgProc
//
//  ImageCaptureDlgProc procedure for the "Extension settings" dialog.
//-------------------------------------------------------------------

INT_PTR CALLBACK ImageCaptureDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{

	case WM_INITDIALOG:
#ifdef DEBUG
		sprintf(logMessage, "\nCommand Fired : WM_INITDIALOG ID : IDD_IMG_CAPTURE");
		printLogMessage(logMessage);
#endif
		OnInitImageCaptureDialog(hwnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		case IDC_BTN_DIR:
			//EndDialog(hwnd, LOWORD(wParam));
			GetFolderSelection(hwnd);
			return TRUE;

		case IDC_BTN_SNAPSHOT:
			//EndDialog(hwnd, LOWORD(wParam));
			//takeSnapShotClicked(hwnd); // replaced by a still trigger routine --wenye
			//stillTrigger();		// to start a still image capture.
			//writeSnapCount();
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		case IDCANCEL:
			// write last snap count number (snapshotIndex variable) in file.
			writeSnapCount();
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		}
		break;

	}

	return FALSE;
}

void OnInitImageSetResDialog(HWND hwnd)
{
	long currValue, lCaps;
	CString currValueStr;
	static int num_of_elms;
	static int Indx_Res = 0;
	static int num_Res = 0;
	StillFormats* pstillFmts;
	HRESULT hr = NULL;
	AM_MEDIA_TYPE *pmt = NULL;
	int iCount = 0, iSize = 0;
	BOOL yuy2Flag = FALSE;
	BOOL mjpgFlag = FALSE;

	// get format
	num_of_elms = 0;
	if (gcap.pSSC){
		gcap.pSSC->GetNumberOfCapabilities(&iCount, &iSize);
		//stillFmts = (StillFormats *)malloc(sizeof(StillFormats)*iCount);
		pstillFmts = gcap.stillFmts;
		if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS)){
			HWND hListStilSet = GetDlgItem(hwnd, IDC_COMBO_STIL_RES);
			HWND hListStilType = GetDlgItem(hwnd, IDC_COMBO_STIL_VIDTYPE);
			TCHAR  txt_display[20];

			for (int iFormat = 0; iFormat < iCount; iFormat++){
				VIDEO_STREAM_CONFIG_CAPS scc;
				VIDEOINFOHEADER *pvi;
				hr = gcap.pSSC->GetStreamCaps(iFormat, &pmt, (BYTE *)&scc);
				if (SUCCEEDED(hr)){
					if (pmt->subtype == MEDIASUBTYPE_YUY2){
						wcsncpy_s((pstillFmts + iFormat)->Comp, 5, _T("YUY2"), _TRUNCATE);
						yuy2Flag = TRUE;
					}
					else if (pmt->subtype == MEDIASUBTYPE_MJPG){
						wcsncpy_s((pstillFmts + iFormat)->Comp, 5, _T("MJPG"), _TRUNCATE);
						mjpgFlag = TRUE;
					}
					pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
					(pstillFmts + iFormat)->width = pvi->bmiHeader.biWidth;
					(pstillFmts + iFormat)->height = pvi->bmiHeader.biHeight;
				}
				StringCbPrintf(txt_display, 20, L"%dx%d", (pstillFmts + iFormat)->width, (pstillFmts + iFormat)->height);
				ComboBox_AddString(hListStilSet, txt_display);
			}
			if (yuy2Flag){
				StringCbPrintf(txt_display, 20, L"YUY2");
				ComboBox_AddString(hListStilType, txt_display);
			}
			if (mjpgFlag){
				StringCbPrintf(txt_display, 20, L"MJPG");
				ComboBox_AddString(hListStilType, txt_display);
			}
			BOOL setFlag = FALSE;  //set for the miss setting the still attibute. more need TODO 
			for (int i = 0; i < iCount; i++){
				if (((pstillFmts + i)->width == gcap.stillWidth) && ((pstillFmts + i)->height == gcap.stillHeight)){
					setFlag = TRUE;
					ComboBox_SetCurSel(hListStilSet, i);
					if (wcscmp((pstillFmts + i)->Comp, _T("YUY2")) == 0 && gcap.stillsubType == MEDIA_YUY2)
						ComboBox_SetCurSel(hListStilType, 0);
					else if (wcscmp((pstillFmts + i)->Comp, _T("MJPG")) == 0 && gcap.stillsubType == MEDIA_MJPEG)
						ComboBox_SetCurSel(hListStilType, 1);
				}
			}
			if (!setFlag){
				ComboBox_SetCurSel(hListStilType, 0);
				ComboBox_SetCurSel(hListStilSet, 0);
			}
		}

	}
	else{
		HWND hListStilSet = GetDlgItem(hwnd, IDC_COMBO_STIL_RES);
		ComboBox_AddString(hListStilSet, L"No Sanpshot Support");
		//ComboBox_AddString(hListFocusMode, L"1920x1080");
		//ComboBox_AddString(hListFocusMode, L"2592x1944");
		ComboBox_SetCurSel(hListStilSet, 0);

	}


}

INT_PTR CALLBACK ImageResSetDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{

	case WM_INITDIALOG:
#ifdef DEBUG
		sprintf(logMessage, "\nCommand Fired : WM_INITDIALOG ID : IDD_IMG_CAPTURE");
		printLogMessage(logMessage);
#endif
		StopPreview();
		TearDownGraph();
		OnInitImageSetResDialog(hwnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			setStilFmat(hwnd, gcap.stillWidth, gcap.stillHeight);

			gcap.pFg->AddFilter(gcap.pSampleGrabberFilter, L"SampleGrab");
			gcap.pFg->AddFilter(gcap.pNullRenderer, L"NullRender");
			//gcap.pFg->AddFilter(gcap.VideoRenderer, L"VideoRenderer");

			BuildPreviewGraph();
			StartPreview();

			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		case IDC_BTN_DIR:
			//EndDialog(hwnd, LOWORD(wParam));
			GetFolderSelection(hwnd);
			return TRUE;

		case IDC_BTN_SNAPSHOT:
			//EndDialog(hwnd, LOWORD(wParam));
			//takeSnapShotClicked(hwnd); // replaced by a still trigger routine --wenye
			//stillTrigger();		// to start a still image capture.
			//writeSnapCount();
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		case IDCANCEL:
			// write last snap count number (snapshotIndex variable) in file.
			writeSnapCount();

			gcap.pFg->AddFilter(gcap.pSampleGrabberFilter, L"SampleGrab");
			gcap.pFg->AddFilter(gcap.pNullRenderer, L"NullRender");
			//gcap.pFg->AddFilter(gcap.VideoRenderer, L"VideoRenderer");
			BuildPreviewGraph();
			StartPreview();

			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		}
		break;

	}

	return FALSE;
}

#if 1
void OnInitImageCaptureDialog(HWND hwnd)
{
	wchar_t wcsDefaultPath[1000];
	memset(wcsDefaultPath, 0, sizeof(wcsDefaultPath));
	MultiByteToWideChar(CP_UTF8, 0, &pathSnapImg[0], (int)strlen(pathSnapImg), &wcsDefaultPath[0], sizeof(wcsDefaultPath));

	HWND hListEditSnapDir = GetDlgItem(hwnd, IDC_EDIT_DIR);
	Edit_SetText(hListEditSnapDir, wcsDefaultPath);

	// read last snap count number in snapshotIndex variable
	readSnapCount();

}
#endif

BOOL GetFolderSelection(HWND hwnd)
{
	wchar_t szBuf[MAX_PATH], szTitle[MAX_PATH] = L"Select Directory";

	LPITEMIDLIST pidl = NULL;
	BROWSEINFO   bi = { 0 };
	BOOL         bResult = FALSE;

	bi.hwndOwner = hwnd;
	bi.pszDisplayName = szBuf;
	bi.pidlRoot = NULL;
	bi.lpszTitle = szTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;

	if ((pidl = SHBrowseForFolder(&bi)) != NULL)
	{
		bResult = SHGetPathFromIDList(pidl, szBuf);
		wcscat(szBuf, L"\\");
		HWND hListEditSnapDir = GetDlgItem(hwnd, IDC_EDIT_DIR);
		Edit_SetText(hListEditSnapDir, szBuf);

		char DefChar = ' ';
		WideCharToMultiByte(CP_ACP, 0, szBuf, -1, pathSnapImg, MAX_PATH, &DefChar, NULL);
		writeSnapPath();

		CoTaskMemFree(pidl);
	}

	return bResult;
}

HRESULT	takeSnapShotClicked(HWND hwnd)
{
	HRESULT hr = S_OK;
	const int MAX_FILENAME = 24;
	WCHAR filePath[MAX_PATH + MAX_FILENAME];
	WCHAR fileName[MAX_FILENAME];

	char *pBuffer = NULL;
	AM_MEDIA_TYPE mt;

	HWND hListEditSnapDir = GetDlgItem(hwnd, IDC_EDIT_DIR);
	Edit_GetText(hListEditSnapDir, filePath, MAX_PATH);

	hr = StringCbPrintf(fileName, sizeof(fileName), L"Snap%d.bmp", snapshotIndex);

	wcscat(filePath, fileName);

	// snap shot filename error
#ifdef DEBUG
	sprintf(logMessage, "\nFuction : OnInitImageCaptureDialog Msg : snap shot filename %ls", filePath);
	printLogMessage(logMessage);
#endif

	/*
	if (g_pPreview)
	{
	hr = g_pPreview->SnapShot(filePath);
	snapshotIndex++;
	}
	*/

	// Grab a sample
	// First, find the required buffer size
	long buffer_size = 0;
	int iCountTry = 0;
	while (iCountTry < 1000)
	{
		iCountTry++;
		// Passing in a NULL pointer signals that we're just checking
		// the required buffer size; not looking for actual data yet.
		if (gcap.pSampleGrabber)
			hr = gcap.pSampleGrabber->GetCurrentBuffer(&buffer_size, NULL);

		// Keep trying until buffer_size is set to non-zero value.
		if (hr == S_OK && buffer_size != 0) break;

		// If the return value isn't S_OK or VFW_E_WRONG_STATE
		// then something has gone wrong. VFW_E_WRONG_STATE just
		// means that the filter graph is still starting up and
		// no data has arrived yet in the sample grabber filter.
		//if (hr != S_OK && hr != VFW_E_WRONG_STATE)
		//exit_message("Could not get buffer size", 1);
	}

	// Stop the graph
	//pMediaControl->Stop();

	if (buffer_size != 0)
	{
		// Allocate buffer for image
		pBuffer = new char[buffer_size];
		if (!pBuffer)
		{
			//exit_message("Could not allocate data buffer for image", 1);
			hr = E_FAIL;
			return hr;
		}
	}
	else
	{
		//exit_message("Could not get buffer size for image", 1);
		hr = E_FAIL;
		return hr;
	}

	// Retrieve image data from sample grabber buffer
	hr = gcap.pSampleGrabber->GetCurrentBuffer(
		&buffer_size, (long*)pBuffer);
	if (hr != S_OK)
	{
		//exit_message("Could not get buffer data from sample grabber", 1);
		hr = E_FAIL;
		return hr;
	}


	// Get the media type from the sample grabber filter
	hr = gcap.pSampleGrabber->GetConnectedMediaType(
		(_AMMediaType *)&mt);
	if (hr != S_OK)
	{
		//exit_message("Could not get media type", 1);
		hr = E_FAIL;
		return hr;
	}

	// Retrieve format information
	VIDEOINFOHEADER *pVih = NULL;
	if ((mt.formattype == FORMAT_VideoInfo) &&
		(mt.cbFormat >= sizeof(VIDEOINFOHEADER)) &&
		(mt.pbFormat != NULL))
	{
		// Get video info header structure from media type
		pVih = (VIDEOINFOHEADER*)mt.pbFormat;

		// Print the resolution of the captured image
		fprintf(stderr, "Capture resolution: %dx%d\n",
			pVih->bmiHeader.biWidth,
			pVih->bmiHeader.biHeight);

		// Create bitmap structure
		long cbBitmapInfoSize = mt.cbFormat - SIZE_PREHEADER;
		BITMAPFILEHEADER bfh;
		ZeroMemory(&bfh, sizeof(bfh));
		bfh.bfType = 'MB'; // Little-endian for "BM".
		bfh.bfSize = sizeof(bfh) + buffer_size + cbBitmapInfoSize;
		bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + cbBitmapInfoSize;

		// Open output file
		HANDLE hf = CreateFile(filePath, GENERIC_WRITE,
			FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, 0, NULL);
		if (hf == INVALID_HANDLE_VALUE)
		{
			//exit_message("Error opening output file", 1);
			hr = E_FAIL;
			return hr;
		}

		// Write the file header.
		DWORD dwWritten = 0;
		WriteFile(hf, &bfh, sizeof(bfh), &dwWritten, NULL);
		WriteFile(hf, HEADER(pVih),
			cbBitmapInfoSize, &dwWritten, NULL);

		// Write pixel data to file
		WriteFile(hf, pBuffer, buffer_size, &dwWritten, NULL);
		CloseHandle(hf);

		snapshotIndex++;
	}
	else
	{
		//exit_message("Wrong media type", 1);
		hr = E_FAIL;
		return hr;
	}

	// Free the format block
	if (mt.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL)
	{
		// pUnk should not be used.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}


	return hr;
}

HRESULT stillTrigger() //--wenye
{
	HRESULT hr = NULL;
	AM_MEDIA_TYPE mediaType;
	if (gcap.pAMVideoCtrl != NULL)
	{
		hr = gcap.pSampleGrabber->GetConnectedMediaType(&mediaType);
		if (hr == S_OK)
		{
#if 1		
			// don't use the code for check the midea type as the only YUY2 format here.
			if (mediaType.subtype == MEDIASUBTYPE_YUY2)
			{
				OutputDebugStringW(TEXT("still subtype YUY2...\r\n"));
			}
			if (mediaType.subtype == MEDIASUBTYPE_MJPG)
			{
				OutputDebugStringW(TEXT("still subtype MJPG...\r\n"));
			}

#endif
		}
		gcap.pAMVideoCtrl->SetMode(gcap.pstilPin, VideoControlFlag_Trigger); //trigger a snapshot
	}
	return hr;
}

HRESULT setStilFmat(HWND hwnd, DWORD Width, DWORD Height)
{
	HRESULT hr = FALSE;
	AM_MEDIA_TYPE  *pmt = NULL;
	int iCount = 0, iSize = 0;
	int width = 0, height = 0, isheight = false;
	TCHAR txt_select_subtype[5] = _T("YUY2");
	TCHAR txt_select_res[255] = _T("");
	HWND hListStilSet = GetDlgItem(hwnd, IDC_COMBO_STIL_RES);
	ComboBox_GetText(hListStilSet, txt_select_res, 255);
	for (int i = 0; txt_select_res[i] != '\0'; i++){
		if (isdigit(txt_select_res[i]))
		{
			if (!isheight)
			{
				width = width * 10 + txt_select_res[i] - '0';
			}
			else
			{
				height = height * 10 + txt_select_res[i] - '0';
			}
		}
		else if (txt_select_res[i] == 'x'){
			isheight = true;
		}
	}
	gcap.stillWidth = width;
	gcap.stillHeight = height;

	HWND hListStilType = GetDlgItem(hwnd, IDC_COMBO_STIL_VIDTYPE);
	ComboBox_GetText(hListStilType, txt_select_subtype, 255);

	/*
	int sel = ComboBox_GetCurSel(hListStilSet);
	switch (sel)
	{
	case 1:  //1280x720
	gcap.stillWidth = 1280;
	gcap.stillHeight = 720;
	break;
	case 2:  //1920x1080
	gcap.stillWidth = 1920;
	gcap.stillHeight = 1080;
	break;
	case 3:  //2592x1944
	gcap.stillWidth = 2592;
	gcap.stillHeight = 1944;
	break;
	case 0:
	gcap.stillWidth = 640;
	gcap.stillHeight = 480;
	break;
	default:
	gcap.stillWidth = 1920;
	gcap.stillHeight = 1080;
	}
	*/
	hr = gcap.pSSC->GetNumberOfCapabilities(&iCount, &iSize);
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS)){
		for (int iFormat = 0; iFormat < iCount; iFormat++){
			VIDEO_STREAM_CONFIG_CAPS scc;
			VIDEOINFOHEADER *pvi;
			hr = gcap.pSSC->GetStreamCaps(iFormat, &pmt, (BYTE*)&scc);
			if (SUCCEEDED(hr)){
				if (pmt->formattype == FORMAT_VideoInfo){
					pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
					if ((pvi->bmiHeader.biWidth == gcap.stillWidth)
						&& (pvi->bmiHeader.biHeight == gcap.stillHeight)){
						if (wcscmp(txt_select_subtype, _T("YUY2")) == 0 && (pmt->subtype == MEDIASUBTYPE_YUY2)){
							gcap.stillsubType = MEDIA_YUY2;
							if (pmt != NULL){
								hr = gcap.pSSC->SetFormat(pmt);
								if (hr == S_OK || hr == NO_ERROR)
									break;
							}
						}
						else if (wcscmp(txt_select_subtype, _T("MJPG")) == 0 && (pmt->subtype == MEDIASUBTYPE_MJPG)){
							//pmt->subtype = MEDIASUBTYPE_MJPG;
							gcap.stillsubType = MEDIA_MJPEG;
							if (pmt != NULL){
								hr = gcap.pSSC->SetFormat(pmt);
								if (hr == S_OK || hr == NO_ERROR)
									break;
							}
						}
						//						if (pmt != NULL){
						//							hr = gcap.pSSC->SetFormat(pmt);
						//							if (hr == S_OK || hr == NO_ERROR)
						//								break;
						//						}
						//						break;
					}
					else{
						hr = FALSE;
					}
				}
			}
		}
		if (pmt != NULL){
			DeleteMediaType(pmt);
			pmt = NULL;
		}
	}
	return hr;
}

HRESULT StillPinCapture() // add the still pin find routine --wenye
{
	HRESULT hr = NULL;
	hr = gcap.pVCap->QueryInterface(IID_IAMVideoControl, (void **)&gcap.pAMVideoCtrl);
	if (SUCCEEDED(hr))
	{
		hr = gcap.pBuilder->FindPin(
			gcap.pVCap,				// filter
			PINDIR_OUTPUT,			// look for an output pin
			&PIN_CATEGORY_STILL,	// pin category
			NULL,					// media type (don't care)
			FALSE,					// pin must be unconnected
			0,						// get the 0's pin
			&gcap.pstilPin			// receives a pointer to the pin
			);
	}
	if (SUCCEEDED(hr))
	{
		;// hr = gcap.pAMVideoCtrl->SetMode(gcap.pstilPin, VideoControlFlag_Trigger);
	}
	else
		hr = E_FAIL;
	return hr;
}

void readSnapCount()
{
	char snapCountFileFull[260];
	strcpy(snapCountFileFull, pathExe);
	strcat(snapCountFileFull, snapCountFile);

	FILE *fpSnapCount = fopen(snapCountFileFull, "r");
	if (fpSnapCount)
	{
		fscanf(fpSnapCount, "%d", &snapshotIndex);
		fclose(fpSnapCount);
	}
	else
	{
		snapshotIndex = 0;
	}
}

void writeSnapCount()
{
	char snapCountFileFull[260];
	strcpy(snapCountFileFull, pathExe);
	strcat(snapCountFileFull, snapCountFile);

	wchar_t wcsDetmpPath[1000];
	memset(wcsDetmpPath, 0, sizeof(wcsDetmpPath));
	MultiByteToWideChar(CP_UTF8, 0, &snapCountFileFull[0], (int)strlen(snapCountFileFull), &wcsDetmpPath[0], sizeof(wcsDetmpPath));

	SetFileAttributes(wcsDetmpPath, FILE_ATTRIBUTE_NORMAL);

	FILE *fpSnapCount = fopen(snapCountFileFull, "w");
	if (fpSnapCount)
	{
		fprintf(fpSnapCount, "%d", snapshotIndex);
		fclose(fpSnapCount);

		SetFileAttributes(wcsDetmpPath, FILE_ATTRIBUTE_HIDDEN);
	}

}

void readSnapPath()
{
	char snapPathFileFull[260];
	strcpy(snapPathFileFull, pathExe);
	strcat(snapPathFileFull, snapPathFile);

	FILE *fpSnapCount = fopen(snapPathFileFull, "r");
	if (fpSnapCount)
	{
		fscanf(fpSnapCount, "%[^\t]s", pathSnapImg);
		fclose(fpSnapCount);
	}
	else
	{
		strcpy(pathSnapImg, pathExe);
	}
}

void writeSnapPath()
{
	char snapPathFileFull[260];
	strcpy(snapPathFileFull, pathExe);
	strcat(snapPathFileFull, snapPathFile);

	wchar_t wcsDetmpPath[1000];
	memset(wcsDetmpPath, 0, sizeof(wcsDetmpPath));
	MultiByteToWideChar(CP_UTF8, 0, &snapPathFileFull[0], (int)strlen(snapPathFileFull), &wcsDetmpPath[0], sizeof(wcsDetmpPath));

	SetFileAttributes(wcsDetmpPath, FILE_ATTRIBUTE_NORMAL);

	FILE *fpSnapPath = fopen(snapPathFileFull, "w");
	if (fpSnapPath)
	{
		fprintf(fpSnapPath, "%s", pathSnapImg);
		fclose(fpSnapPath);

		SetFileAttributes(wcsDetmpPath, FILE_ATTRIBUTE_HIDDEN);
	}

}


/////////////////////////////////////////////////////////////////////

// Dialog functions
void    OnInitCameraRecoveryParamDialog(HWND hwnd);
HRESULT	resetParamClicked(HWND hwnd);
HRESULT	saveParamClicked(HWND hwnd);

//-------------------------------------------------------------------
//  CameraRecoveryParamDlgProc
//
//  CameraRecoveryParamDlgProc procedure for the "Extension settings" dialog.
//-------------------------------------------------------------------

INT_PTR CALLBACK CameraRecoveryParamDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{

	case WM_INITDIALOG:
		OnInitCameraRecoveryParamDialog(hwnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			//EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		case IDC_BTN_RESET:
			resetParamClicked(hwnd);
			return TRUE;

		case IDC_BTN_SAVE:
			saveParamClicked(hwnd);
			return TRUE;

		case IDCANCEL:
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		}
		break;

	}

	return FALSE;
}

void OnInitCameraRecoveryParamDialog(HWND hwnd)
{
	//IDC_STR_STATUA_CAM_REC
}

HRESULT saveParamClicked(HWND hwnd)
{
	HRESULT hr = S_OK;

	if (ksNodeTree.isOK)
	{
		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 15;
		int save = 1;

		//INT_PTR result = MessageBox(hwnd, L"Are you sure to save camera param ?", WINDOW_NAME, MB_YESNO);
		if (1/*result == IDYES*/)
		{
			hr = getExtionControlPropertySize(PropertId, &ulSize);
			if (FAILED(hr))
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : saveParamClicked \t Msg : Unable to find property size : %x", hr);
				printLogMessage(logMessage);
#endif
			}
			else
			{
				pbPropertyValue = new BYTE[ulSize];
				if (!pbPropertyValue)
				{
#ifdef DEBUG
					sprintf(logMessage, "\nERROR \t Function : saveParamClicked \t Msg : Unable to allocate memory for property value");
					printLogMessage(logMessage);
#endif
				}
				else
				{
					memcpy(pbPropertyValue, (char*)&save, ulSize);
					hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
				}
				delete[] pbPropertyValue;
			}

#ifdef DEBUG
			sprintf(logMessage, "\nFunction : saveParamClicked \t Msg : Return Value:%ld", hr);
			printLogMessage(logMessage);
#endif

			HWND hListStatus = GetDlgItem(hwnd, IDC_STR_STATUA_CAM_REC);
			if (SUCCEEDED(hr))
			{
				Static_SetText(hListStatus, L"Reboot required.\nWait for 2 sec before reboot the device.");
				Sleep(2000);
				Static_SetText(hListStatus, L"");
			}
			else
				Static_SetText(hListStatus, L"Save param request failed.Please try again later.");
		}
	}

	return hr;
}

HRESULT	resetParamClicked(HWND hwnd)
{
	HRESULT hr = S_OK;
	HWND hListStatus = GetDlgItem(hwnd, IDC_STR_STATUA_CAM_REC);

	if (ksNodeTree.isOK)
	{
		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 15;
		int reset = 2;

		//INT_PTR result = MessageBox(hwnd, L"Are you sure to reset camera param ?", WINDOW_NAME, MB_YESNO);
		if (1/*result == IDYES*/)
		{
			hr = getExtionControlPropertySize(PropertId, &ulSize);
			if (FAILED(hr))
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : resetParamClicked \t Msg : Unable to find property size : %x", hr);
				printLogMessage(logMessage);
#endif
			}
			else
			{
				pbPropertyValue = new BYTE[ulSize];
				if (!pbPropertyValue)
				{
#ifdef DEBUG
					sprintf(logMessage, "\nERROR \t Function : resetParamClicked \t Msg : Unable to allocate memory for property value");
					printLogMessage(logMessage);
#endif
				}
				else
				{
					memcpy(pbPropertyValue, (char*)&reset, ulSize);
					hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
				}
				delete[] pbPropertyValue;
			}

#ifdef DEBUG
			sprintf(logMessage, "\nFunction : resetParamClicked \t Msg : Return Value:%ld", hr);
			printLogMessage(logMessage);
#endif

			if (SUCCEEDED(hr))
			{
				Static_SetText(hListStatus, L"Reboot required.\nWait for 2 sec before rebooting the device.");
				Sleep(2000);
				Static_SetText(hListStatus, L"");
			}
			else
				Static_SetText(hListStatus, L"Reset param request failed.Please try again later.");

		}
	}
	else
	{
		Static_SetText(hListStatus, L"Reset param request failed.");
	}

	return hr;
}


void ShowErrorMessage(PCWSTR format, HRESULT hrErr)
{
	HRESULT hr = S_OK;
	WCHAR msg[MAX_PATH];

	hr = StringCbPrintf(msg, sizeof(msg), L"%s (hr=0x%X)", format, hrErr);

	if (SUCCEEDED(hr))
	{
		MessageBox(NULL, msg, L"Error", MB_ICONERROR);
	}
	else
	{
		DebugBreak();
	}
}

// Start previewing
//
BOOL StartPreview()
{
	// way ahead of you
	if (gcap.fPreviewing)
		return TRUE;

	if (!gcap.fPreviewGraphBuilt)
		return FALSE;

	// run the graph
	IMediaControl *pMC = NULL;
	HRESULT hr = gcap.pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);
	if (SUCCEEDED(hr))
	{
		hr = pMC->Run();
		if (FAILED(hr))
		{
			// stop parts that ran
			pMC->Stop();
		}
		pMC->Release();
	}
	if (FAILED(hr))
	{
		//ErrMsg(TEXT("Error %x: Cannot run preview graph"), hr);
		return FALSE;
	}

	gcap.fPreviewing = TRUE;
	return TRUE;
}


// stop the preview graph
//
BOOL StopPreview()
{
	// way ahead of you
	if (!gcap.fPreviewing)
	{
		return FALSE;
	}

	// stop the graph
	IMediaControl *pMC = NULL;
	HRESULT hr = gcap.pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);
	if (SUCCEEDED(hr))
	{
		hr = pMC->Stop();
		pMC->Release();
	}
	if (FAILED(hr))
	{
		//ErrMsg(TEXT("Error %x: Cannot stop preview graph"), hr);
		return FALSE;
	}

	gcap.fPreviewing = FALSE;

	// get rid of menu garbage
	//InvalidateRect(ghwndApp, NULL, TRUE);

	return TRUE;
}

// Tear down everything downstream of the capture filters, so we can build
// a different capture graph.  Notice that we never destroy the capture filters
// and WDM filters upstream of them, because then all the capture settings
// we've set would be lost.
//
void TearDownGraph()
{
	SAFE_RELEASE(gcap.pSink);
	SAFE_RELEASE(gcap.pConfigAviMux);
	SAFE_RELEASE(gcap.pRender);
	SAFE_RELEASE(gcap.pME);
	SAFE_RELEASE(gcap.pDF);

	if (gcap.pVW)
	{
		// stop drawing in our window, or we may get wierd repaint effects
		gcap.pVW->put_Owner(NULL);
		gcap.pVW->put_Visible(OAFALSE);
		gcap.pVW->Release();
		gcap.pVW = NULL;
	}

	// destroy the graph downstream of our capture filters
	if (gcap.pVCap)
		RemoveDownstream(gcap.pVCap);
	if (gcap.pACap)
		RemoveDownstream(gcap.pACap);
	if (gcap.pVCap)
		gcap.pBuilder->ReleaseFilters();

	// potential debug output - what the graph looks like
	// if (gcap.pFg) DumpGraph(gcap.pFg, 1);

#ifdef REGISTER_FILTERGRAPH
	// Remove filter graph from the running object table
	if (g_dwGraphRegister)
	{
		RemoveGraphFromRot(g_dwGraphRegister);
		g_dwGraphRegister = 0;
	}
#endif

	gcap.fCaptureGraphBuilt = FALSE;
	gcap.fPreviewGraphBuilt = FALSE;
	gcap.fPreviewFaked = FALSE;
}

// Tear down everything downstream of a given filter
void RemoveDownstream(IBaseFilter *pf)
{
	IPin *pP = 0, *pTo = 0;
	ULONG u;
	IEnumPins *pins = NULL;
	PIN_INFO pininfo;

	if (!pf)
		return;

	HRESULT hr = pf->EnumPins(&pins);
	pins->Reset();

	while (hr == NOERROR)
	{
		hr = pins->Next(1, &pP, &u);
		if (hr == S_OK && pP)
		{
			pP->ConnectedTo(&pTo);
			if (pTo)
			{
				hr = pTo->QueryPinInfo(&pininfo);
				if (hr == NOERROR)
				{
					if (pininfo.dir == PINDIR_INPUT)
					{
						RemoveDownstream(pininfo.pFilter);
						gcap.pFg->Disconnect(pTo);
						gcap.pFg->Disconnect(pP);
						gcap.pFg->RemoveFilter(pininfo.pFilter);
					}
					pininfo.pFilter->Release();
				}
				pTo->Release();
			}
			pP->Release();
		}
	}

	if (pins)
		pins->Release();
}

// all done with the capture filters and the graph builder
//
void FreeCapFilters()
{
	SAFE_RELEASE(gcap.pFg);
	if (gcap.pBuilder)
	{
		delete gcap.pBuilder;
		gcap.pBuilder = NULL;
	}
	SAFE_RELEASE(gcap.pVCap);
	SAFE_RELEASE(gcap.pACap);
	SAFE_RELEASE(gcap.pASC);
	SAFE_RELEASE(gcap.pVSC);
	SAFE_RELEASE(gcap.pVC);
	SAFE_RELEASE(gcap.pDlg);
}

// create the capture filters of the graph.  We need to keep them loaded from
// the beginning, so we can set parameters on them and have them remembered
//
BOOL InitCapFilters()
{
	HRESULT hr = S_OK;
	BOOL f;

	gcap.fCCAvail = FALSE;  // assume no closed captioning support

	f = MakeBuilder();
	if (!f)
	{
		//ErrMsg(TEXT("Cannot instantiate graph builder"));
		return FALSE;
	}

	//
	// First, we need a Video Capture filter, and some interfaces
	//
	gcap.pVCap = NULL;

	if (gcap.pmVideo != 0)
	{
		IPropertyBag *pBag;
		gcap.wachFriendlyName[0] = 0;

		hr = gcap.pmVideo->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if (SUCCEEDED(hr))
		{
			VARIANT var;
			var.vt = VT_BSTR;

			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR)
			{
				hr = StringCchCopyW(gcap.wachFriendlyName, sizeof(gcap.wachFriendlyName) / sizeof(gcap.wachFriendlyName[0]), var.bstrVal);
				SysFreeString(var.bstrVal);
			}

			pBag->Release();
		}

		hr = gcap.pmVideo->BindToObject(0, 0, IID_IBaseFilter, (void**)&gcap.pVCap);
	}

	if (gcap.pVCap == NULL)
	{
		//ErrMsg(TEXT("Error %x: Cannot create video capture filter"), hr);
		goto InitCapFiltersFail;
	}

	// create some interface object to configure video setting
	createMFInterfaceVideoSettings();

	//
	// make a filtergraph, give it to the graph builder and put the video
	// capture filter in the graph
	//

	gcap.VideoRenderer = NULL;
	//hr = CoCreateInstance(CLSID_VideoRenderer, NULL, CLSCTX_INPROC,
		//IID_IBaseFilter, (void **)&gcap.VideoRenderer); //create the video renderer filter

	//SmartPtr<IBaseFilter> pVMR;
	//hr = pVMR.CoCreateInstance(CLSID_VideoMixingRenderer);
	hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC,
		IID_IBaseFilter, (void **)&gcap.VideoRenderer); //create the video renderer filter



	f = MakeGraph();
	if (!f)
	{
		//ErrMsg(TEXT("Cannot instantiate filtergraph"));
		goto InitCapFiltersFail;
	}

	hr = gcap.pBuilder->SetFiltergraph(gcap.pFg);
	if (hr != NOERROR)
	{
		//ErrMsg(TEXT("Cannot give graph to builder"));
		goto InitCapFiltersFail;
	}

	// Add the video capture filter to the graph with its friendly name
	hr = gcap.pFg->AddFilter(gcap.pVCap, gcap.wachFriendlyName);
	if (hr != NOERROR)
	{
		//ErrMsg(TEXT("Error %x: Cannot add vidcap to filtergraph"), hr);
		goto InitCapFiltersFail;
	}

#if 1 // delete --wenye
	// Create sample grabber filter
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL,
		CLSCTX_INPROC_SERVER, IID_IBaseFilter,
		(void**)&gcap.pSampleGrabberFilter);

	//hr = CoCreateInstance(CLSID_SampleGrabber, NULL, // deleted -- wenye
	//CLSCTX_INPROC_SERVER, IID_IBaseFilter,
	//(void**)&gcap.pSampleGrabberFilterStill);
#endif

#if 1 // delete all the capture setting here --wenye
	//if (hr != S_OK)
	//exit_message("Could not create Sample Grabber filter", 1);

	// Query the ISampleGrabber interface of the sample grabber filter
	hr = gcap.pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&gcap.pSampleGrabber);
	//hr = gcap.pSampleGrabberFilterStill->QueryInterface(IID_ISampleGrabber, (void**)&gcap.pSampleGrabberStill);
	//if (hr != S_OK)
	//exit_message("Could not get ISampleGrabber interface to sample grabber filter", 1);

	// Enable sample buffering in the sample grabber filter
	hr = gcap.pSampleGrabber->SetOneShot(FALSE); //--wenye
	hr = gcap.pSampleGrabber->SetBufferSamples(TRUE);
	//hr = gcap.pSampleGrabberStill->SetBufferSamples(TRUE);
	//hr = gcap.pSampleGrabberStill->SetOneShot(FALSE); //--wenye

	//if (hr != S_OK)
	//exit_message("Could not enable sample buffering in the sample grabber", 1);
	// default capture format
	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));

	// Set media type in sample grabber filter
	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB24;
	//mt.lSampleSize = 10000000;
	hr = gcap.pSampleGrabber->SetMediaType((_AMMediaType *)&mt);
	//hr = gcap.pSampleGrabberStill->SetMediaType((_AMMediaType *)&mt); delete by wenye


	//gcap.pSampleGrabberStill->Release();  //what is it for ? --wenye

	//hr = gcap.pSampleGrabberStill->SetCallback(&g_StillCapCB, 1); // 0 = Use the SampleCB callback method.
	hr = gcap.pSampleGrabber->SetCallback(&g_StillCapCB, 1); // 0 = Use the SampleCB callback method.

	//if (hr != S_OK)
	//exit_message("Could not set media type in sample grabber", 1);

	// Add sample grabber filter to filter graph
	hr = gcap.pFg->AddFilter(gcap.pSampleGrabberFilter, L"SampleGrab");
	//hr = gcap.pFg->AddFilter(gcap.pSampleGrabberFilterStill, L"SampleGrabStill");
	//if (hr != S_OK)
	//exit_message("Could not add Sample Grabber to filter graph", 1);
#endif // end of the still setting deletion --wenye

#if 1 // the additional statment to delete --wenye
	// Create Null Renderer filter
	hr = CoCreateInstance(CLSID_NullRenderer, NULL,
		CLSCTX_INPROC_SERVER, IID_IBaseFilter,
		(void**)&gcap.pNullRenderer);
	hr = CoCreateInstance(CLSID_NullRenderer, NULL,
		CLSCTX_INPROC_SERVER, IID_IBaseFilter,
		(void**)&gcap.pNullRendererStill);

	//if (hr != S_OK)
	//exit_message("Could not create Null Renderer filter", 1);

	// Add Null Renderer filter to filter graph
	hr = gcap.pFg->AddFilter(gcap.pNullRenderer, L"NullRender");
	hr = gcap.pFg->AddFilter(gcap.pNullRendererStill, L"NullRenderStill");
	//hr = gcap.pFg->AddFilter(gcap.VideoRenderer, L"VideoRenderer");
	//if (hr != S_OK)
	//exit_message("Could not add Null Renderer to filter graph", 1);


	// Connect up the filter graph's capture stream
	//hr = pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,pCap, NULL, NULL);
	//hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_STILL, &MEDIATYPE_Video, gcap.pVCap, gcap.pSampleGrabberFilter, gcap.pNullRenderer);

	// Calling FindInterface below will result in building the upstream
	// section of the capture graph (any WDM TVTuners or Crossbars we might
	// need).

	// we use this interface to get the name of the driver
	// Don't worry if it doesn't work:  This interface may not be available
	// until the pin is connected, or it may not be available at all.
	// (eg: interface may not be available for some DV capture)
	hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Interleaved, gcap.pVCap,
		IID_IAMVideoCompression, (void **)&gcap.pVC);
	if (hr != S_OK)
	{
		hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
			&MEDIATYPE_Video, gcap.pVCap,
			IID_IAMVideoCompression, (void **)&gcap.pVC);
	}

	// !!! What if this interface isn't supported?
	// we use this interface to set the frame rate and get the capture size
	hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Interleaved,
		gcap.pVCap, IID_IAMStreamConfig, (void **)&gcap.pVSC);

	if (hr != NOERROR)
	{
		hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
			&MEDIATYPE_Video, gcap.pVCap,
			IID_IAMStreamConfig, (void **)&gcap.pVSC);
		if (hr != NOERROR)
		{
			// this means we can't set frame rate (non-DV only)
			//ErrMsg(TEXT("Error %x: Cannot find VCapture:IAMStreamConfig"), hr);
		}
	}
#endif // end of the additional statment delete --wenye

#if 1
	/* add the still stream config --wenye */
	gcap.pSSC = NULL;
	hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_STILL, &MEDIATYPE_Video,
		gcap.pVCap, IID_IAMStreamConfig, (void **)&gcap.pSSC);

	if (FAILED(hr))
		gcap.isStillSup = FALSE;
	else
	{
		gcap.isStillSup = TRUE;
		gcap.stillWidth = 2592;// 1280;
		gcap.stillHeight = 1944;// 720;
		//getStillRes();
		/* set still pin paramter */
		AM_MEDIA_TYPE  *pmt = NULL;
		int iCount = 0, iSize = 0;
		VIDEOINFOHEADER *pvi;
		BOOL setFlag = FALSE;
		hr = gcap.pSSC->GetNumberOfCapabilities(&iCount, &iSize);
		if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS)){
			for (int iFormat = 0; iFormat < iCount; iFormat++){
				VIDEO_STREAM_CONFIG_CAPS scc;
				hr = gcap.pSSC->GetStreamCaps(iFormat, &pmt, (BYTE*)&scc);
				if (SUCCEEDED(hr)){
					if (pmt->formattype == FORMAT_VideoInfo){
						pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
						if ((pvi->bmiHeader.biWidth == gcap.stillWidth)
							&& (pvi->bmiHeader.biHeight == gcap.stillHeight)){
							setFlag = TRUE;
							if (gcap.stillsubType == MEDIA_YUY2){
								pmt->subtype = MEDIASUBTYPE_YUY2;
							}
							else if (gcap.stillsubType == MEDIA_MJPEG){//TODO: it's bug to pars still image with bot YUY2 & MJPEG
								pmt->subtype = MEDIASUBTYPE_MJPG;;
							}
							if (pmt != NULL){
								hr = gcap.pSSC->SetFormat(pmt);
								if (hr == S_OK || hr == NO_ERROR)
									break;
							}
							break;
						}
						else{
							hr = FALSE;
						}
					}
				}
			}
			if (!setFlag && (pmt != NULL))//if the initial still resolution is not in the stream, set one in the stream
			{
				gcap.stillWidth = pvi->bmiHeader.biWidth;
				gcap.stillHeight = pvi->bmiHeader.biHeight;
				gcap.stillsubType = MEDIA_YUY2;
				pmt->subtype = MEDIASUBTYPE_YUY2;
				hr = gcap.pSSC->SetFormat(pmt);
			}
			if (pmt != NULL){
				DeleteMediaType(pmt);
				pmt = NULL;
			}
		}
		/* end the still pin set */
	}
	/* end of the still grabber filter addition */
#endif
	gcap.fCapAudioIsRelevant = FALSE;// TRUE; --wenye

	AM_MEDIA_TYPE *pmt;

	// default capture format
	if (gcap.pVSC && gcap.pVSC->GetFormat(&pmt) == S_OK)
	{
		// DV capture does not use a VIDEOINFOHEADER
		if (pmt->formattype == FORMAT_VideoInfo)
		{
			// resize our window to the default capture size
			aspRetio = (float)(HEADER(pmt->pbFormat)->biWidth) / (float)(ABS(HEADER(pmt->pbFormat)->biHeight));
			image_w = HEADER(pmt->pbFormat)->biWidth;
			image_h = ABS(HEADER(pmt->pbFormat)->biHeight);
			ResizeWindow(HEADER(pmt->pbFormat)->biWidth, ABS(HEADER(pmt->pbFormat)->biHeight));//-TODO
		}
		if (pmt->majortype != MEDIATYPE_Video)
		{
			// This capture filter captures something other that pure video.
			// Maybe it's DV or something?  Anyway, chances are we shouldn't
			// allow capturing audio separately, since our video capture
			// filter may have audio combined in it already!
			gcap.fCapAudioIsRelevant = FALSE;
			gcap.fCapAudio = FALSE;
		}
		DeleteMediaType(pmt);
	}

	// we use this interface to bring up the 3 dialogs
	// NOTE:  Only the VfW capture filter supports this.  This app only brings
	// up dialogs for legacy VfW capture drivers, since only those have dialogs
	hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
		&MEDIATYPE_Video, gcap.pVCap,
		IID_IAMVfwCaptureDialogs, (void **)&gcap.pDlg);

	// Use the crossbar class to help us sort out all the possible video inputs
	// The class needs to be given the capture filters ANALOGVIDEO input pin
	{
		IPin        *pP = 0;
		IEnumPins   *pins = 0;
		ULONG        n;
		PIN_INFO     pinInfo;
		BOOL         Found = FALSE;
		IKsPropertySet *pKs = 0;
		GUID guid;
		DWORD dw;
		BOOL fMatch = FALSE;

		if (SUCCEEDED(gcap.pVCap->EnumPins(&pins)))
		{
			while (!Found && (S_OK == pins->Next(1, &pP, &n)))
			{
				if (S_OK == pP->QueryPinInfo(&pinInfo))
				{
					if (pinInfo.dir == PINDIR_INPUT)
					{
						// is this pin an ANALOGVIDEOIN input pin?
						if (pP->QueryInterface(IID_IKsPropertySet,
							(void **)&pKs) == S_OK)
						{
							if (pKs->Get(AMPROPSETID_Pin,
								AMPROPERTY_PIN_CATEGORY, NULL, 0,
								&guid, sizeof(GUID), &dw) == S_OK)
							{
								if (guid == PIN_CATEGORY_ANALOGVIDEOIN)
									fMatch = TRUE;
							}
							pKs->Release();
						}
					}
					pinInfo.pFilter->Release();
				}
				pP->Release();
			}
			pins->Release();
		}
	}

	// there's no point making an audio capture filter
	if (gcap.fCapAudioIsRelevant == FALSE)
		goto SkipAudio;

	// create the audio capture filter, even if we are not capturing audio right
	// now, so we have all the filters around all the time.

	//
	// We want an audio capture filter and some interfaces
	//

	if (gcap.pmAudio == 0)
	{
		// there are no audio capture devices. We'll only allow video capture
		gcap.fCapAudio = FALSE;
		goto SkipAudio;
	}
	gcap.pACap = NULL;


	hr = gcap.pmAudio->BindToObject(0, 0, IID_IBaseFilter, (void**)&gcap.pACap);

	if (gcap.pACap == NULL)
	{
		// there are no audio capture devices. We'll only allow video capture
		gcap.fCapAudio = FALSE;
		//ErrMsg(TEXT("Cannot create audio capture filter"));
		goto SkipAudio;
	}

	//
	// put the audio capture filter in the graph
	//
	{
		WCHAR wachAudioFriendlyName[256];
		IPropertyBag *pBag;

		wachAudioFriendlyName[0] = 0;

		// Read the friendly name of the filter to assist with remote graph
		// viewing through GraphEdit
		hr = gcap.pmAudio->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if (SUCCEEDED(hr))
		{
			VARIANT var;
			var.vt = VT_BSTR;

			hr = pBag->Read(L"FriendlyName", &var, NULL);
			if (hr == NOERROR)
			{
				hr = StringCchCopyW(wachAudioFriendlyName, 256, var.bstrVal);
				SysFreeString(var.bstrVal);
			}

			pBag->Release();
		}

		// We'll need this in the graph to get audio property pages
		hr = gcap.pFg->AddFilter(gcap.pACap, wachAudioFriendlyName);
		if (hr != NOERROR)
		{
			//ErrMsg(TEXT("Error %x: Cannot add audio capture filter to filtergraph"), hr);
			goto InitCapFiltersFail;
		}
	}

	// Calling FindInterface below will result in building the upstream
	// section of the capture graph (any WDM TVAudio's or Crossbars we might
	// need).

	// !!! What if this interface isn't supported?
	// we use this interface to set the captured wave format
	hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, gcap.pACap,
		IID_IAMStreamConfig, (void **)&gcap.pASC);

	if (hr != NOERROR)
	{
		//ErrMsg(TEXT("Cannot find ACapture:IAMStreamConfig"));
	}

SkipAudio:

	// Can this filter do closed captioning? -- will modified for still pin find. -- wenye
	IPin *pPin;
	hr = gcap.pBuilder->FindPin(gcap.pVCap, PINDIR_OUTPUT, &PIN_CATEGORY_VBI,
		NULL, FALSE, 0, &pPin);
	if (hr != S_OK)
		hr = gcap.pBuilder->FindPin(gcap.pVCap, PINDIR_OUTPUT, &PIN_CATEGORY_CC,
		NULL, FALSE, 0, &pPin);
	if (hr == S_OK)
	{
		pPin->Release();
		gcap.fCCAvail = TRUE;
	}
	else
	{
		gcap.fCapCC = FALSE;    // can't capture it, then
	}

	// potential debug output - what the graph looks like
	// DumpGraph(gcap.pFg, 1);

	return TRUE;

InitCapFiltersFail:
	FreeCapFilters();
	return FALSE;
}

// Make a graph builder object we can use for capture graph building
//
BOOL MakeBuilder()
{
	// we have one already
	if (gcap.pBuilder)
		return TRUE;

	gcap.pBuilder = new ISampleCaptureGraphBuilder();
	if (NULL == gcap.pBuilder)
	{
		return FALSE;
	}

	return TRUE;
}


// Make a graph object we can use for capture graph building
//
BOOL MakeGraph()
{
	// we have one already
	if (gcap.pFg)
		return TRUE;

	HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
		IID_IGraphBuilder, (LPVOID *)&gcap.pFg);

	return (hr == NOERROR) ? TRUE : FALSE;
}

// make sure the preview window inside our window is as big as the
// dimensions of captured video, or some capture cards won't show a preview.
// (Also, it helps people tell what size video they're capturing)
// We will resize our app's window big enough so that once the status bar
// is positioned at the bottom there will be enough room for the preview
// window to be w x h
//
int gnRecurse = 0;


void ResizeWindow(int w, int h)
{
	RECT rcW, rcC;
	int xExtra, yExtra;
	int cyBorder = GetSystemMetrics(SM_CYBORDER);

	//SendMessage(ghwndApp, WM_SIZE, SIZE_RESTORED, 0);// force a WM_SIZE message
	//return;

	//gnRecurse++;

	GetWindowRect(ghwndAppMain, &rcW);
	GetClientRect(ghwndAppMain, &rcC);
	long setSize;
	setSize = (long)(rcC.bottom/*h*/) << 16;
	setSize = setSize | ((long)(rcC.right/*w*/) & 0xffff);
	//wWid = ((int)lParam & 0xffff);
	//wHig = ((int)lParam >> 16);


	SendMessage(ghwndAppMain, WM_SIZE, SIZE_RESTORED, setSize);// force a WM_SIZE message
	return;

	xExtra = rcW.right - rcW.left - rcC.right;
	yExtra = rcW.bottom - rcW.top - rcC.bottom + cyBorder;

	if (w >= rcC.right)
		rcC.bottom = rcC.right / aspRetio;
	else{
		rcC.right = w;
		rcC.bottom = h;
	}
	SetWindowPos(ghwndAppMain, NULL, 0, 0, rcC.right + xExtra,
		rcC.bottom + yExtra, SWP_NOZORDER | SWP_NOMOVE);

	// we may need to recurse once.  But more than that means the window cannot
	// be made the size we want, trying will just stack fault.
	//
	if (gnRecurse == 1 && ((rcC.right + xExtra != rcW.right - rcW.left && w > GetSystemMetrics(SM_CXMIN)) ||
		(rcC.bottom + yExtra != rcW.bottom - rcW.top)))
		ResizeWindow(w, h);

	gnRecurse--;
}

// build the preview graph!
//
// !!! PLEASE NOTE !!!  Some new WDM devices have totally separate capture
// and preview settings.  An application that wishes to preview and then
// capture may have to set the preview pin format using IAMStreamConfig on the
// preview pin, and then again on the capture pin to capture with that format.
// In this sample app, there is a separate page to set the settings on the
// capture pin and one for the preview pin.  To avoid the user
// having to enter the same settings in 2 dialog boxes, an app can have its own
// UI for choosing a format (the possible formats can be enumerated using
// IAMStreamConfig) and then the app can programmatically call IAMStreamConfig
// to set the format on both pins.
//
BOOL BuildPreviewGraph()
{
	int cy, cyBorder;
	HRESULT hr = S_OK;
	AM_MEDIA_TYPE *pmt;

	// we have one already
	if (gcap.fPreviewGraphBuilt)
		return TRUE;

	// No rebuilding while we're running
	if (gcap.fCapturing || gcap.fPreviewing)
		return FALSE;

	// We don't have the necessary capture filters
	if (gcap.pVCap == NULL)
		return FALSE;
	if (gcap.pACap == NULL && gcap.fCapAudio)
		return FALSE;

	// we already have another graph built... tear down the old one
	if (gcap.fCaptureGraphBuilt)
		TearDownGraph();

	//
	// Render the preview pin - even if there is not preview pin, the capture
	// graph builder will use a smart tee filter and provide a preview.
	//
	// !!! what about latency/buffer issues?

	// NOTE that we try to render the interleaved pin before the video pin, because
	// if BOTH exist, it's a DV filter and the only way to get the audio is to use
	// the interleaved pin.  Using the Video pin on a DV filter is only useful if
	// you don't want the audio.

	/*
	// capture video in windows file
	IBaseFilter* pASFWriter = 0;
	hr = gcap.pBuilder->SetOutputFileName(
	&MEDIASUBTYPE_Asf,   // Create a Windows Media file.
	NULL,//L"D:\\gnomic\\UVC\\VidCap.wmv",   // File name.
	&pASFWriter,         // Receives a pointer to the filter.
	NULL);  // Receives an IFileSinkFilter interface pointer (optional).

	hr = gcap.pBuilder->RenderStream(
	&PIN_CATEGORY_CAPTURE,   // Capture pin.
	&MEDIATYPE_Video,        // Video. Use MEDIATYPE_Audio for audio.
	gcap.pVCap,                    // Pointer to the capture filter.
	0,
	pASFWriter);             // Pointer to the sink filter (ASF Writer).
	*/

	int isIt640 = 0;
	if (gcap.pVSC)
	{
		hr = gcap.pVSC->GetFormat(&pmt);

		// DV capture does not use a VIDEOINFOHEADER
		if (hr == NOERROR)
		{
			if (pmt->formattype == FORMAT_VideoInfo)
			{
				VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
				//pvi->AvgTimePerFrame = (LONGLONG)(10000000 / gcap.FrameRate);
				//pvi->dwBitRate = 2000;
				hr = gcap.pVSC->SetFormat(pmt);
				//if (hr != NOERROR)
				//ErrMsg(TEXT("%x: Cannot set frame rate for preview"), hr);
#if 1
				if (HEADER(pmt->pbFormat)->biWidth == 640)
				{
					isIt640 = 1;
				}
#endif
			}
			DeleteMediaType(pmt);
		}
	}

	//hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_STILL, &MEDIATYPE_Video, gcap.pVCap, gcap.pSampleGrabberFilterStill, gcap.pNullRendererStill);
	if (0 && isIt640)
	{
		hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, gcap.pVCap, /*gcap.pSampleGrabberFilter*/NULL, /*gcap.pNullRenderer*/NULL);
		//hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, gcap.pVCap, /*gcap.pSampleGrabberFilter*/NULL, /*gcap.pNullRenderer*/NULL);
		hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_STILL, &MEDIATYPE_Video, gcap.pVCap, gcap.pSampleGrabberFilter, gcap.pNullRendererStill); // add --wenye
	}
	else
	{
		//hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, gcap.pVCap, NULL/*gcap.pSampleGrabberFilter*/, gcap.pNullRenderer);
		//hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_STILL, &MEDIATYPE_Video, gcap.pVCap, gcap.pSampleGrabberFilter, gcap.pNullRendererStill); // add --wenye

		//hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
		//	&MEDIATYPE_Video, gcap.pVCap, NULL, NULL);

		hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
			&MEDIATYPE_Video, gcap.pVCap, NULL, NULL/*gcap.VideoRenderer*/);// using the gcap.VideoRenderer causes the out of video memory issue.
		//ErrMsg(TEXT("This graph preview test!"));
		if (hr == VFW_S_NOPREVIEWPIN)
		{
			// preview was faked up for us using the (only) capture pin
			gcap.fPreviewFaked = TRUE;
		}
		else if (hr == VFW_E_OUT_OF_VIDEO_MEMORY){
			ErrMsg(TEXT("Out of video memory!"));
		}
		//		else if (hr == VFW_E_CANNOT_CONNECT){
		//			hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
		//				&MEDIATYPE_Video, gcap.pVCap, NULL, gcap.VideoRenderer);// the issue running on the misumi camera.
		//		}
		else if (hr != S_OK)
		{
			// maybe it's DV?
			hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
				&MEDIATYPE_Video, gcap.pVCap, NULL, NULL);
			//			ErrMsg(TEXT("This graph cannot preview2!"));
			if (hr == VFW_S_NOPREVIEWPIN)
			{
				// preview was faked up for us using the (only) capture pin
				//				ErrMsg(TEXT("This graph cannot preview1!"));
				gcap.fPreviewFaked = TRUE;
			}
			else if (hr != S_OK)
			{
				//				ErrMsg(TEXT("This graph cannot preview!"));
				gcap.fPreviewGraphBuilt = FALSE;
				return FALSE;
			}
		}
		hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_STILL, &MEDIATYPE_Video, gcap.pVCap, gcap.pSampleGrabberFilter, gcap.pNullRendererStill); // add --wenye

	}
	//
	// Get the preview window to be a child of our app's window
	//

	// This will find the IVideoWindow interface on the renderer.  It is
	// important to ask the filtergraph for this interface... do NOT use
	// ICaptureGraphBuilder2::FindInterface, because the filtergraph needs to
	// know we own the window so it can give us display changed messages, etc.

	hr = gcap.pFg->QueryInterface(IID_IVideoWindow, (void **)&gcap.pVW);
	if (hr != NOERROR)
	{
		//ErrMsg(TEXT("This graph cannot preview properly"));
	}
	else  //the following hasn't been catched!!
	{
		//Find out if this is a DV stream
		AM_MEDIA_TYPE * pmtDV;
		long lWidth, lHeight;

		if (gcap.pVSC && SUCCEEDED(gcap.pVSC->GetFormat(&pmtDV)))
		{
			if (pmtDV->formattype == FORMAT_DvInfo)
			{
				// in this case we want to set the size of the parent window to that of
				// current DV resolution.
				// We get that resolution from the IVideoWindow.
				SmartPtr<IBasicVideo> pBV;

				// If we got here, gcap.pVW is not NULL 
				ASSERT(gcap.pVW != NULL);
				hr = gcap.pVW->QueryInterface(IID_IBasicVideo, (void**)&pBV);

				if (SUCCEEDED(hr))
				{
					HRESULT hr1, hr2;
					//long lWidth, lHeight;

					hr1 = pBV->get_VideoHeight(&lHeight);
					hr2 = pBV->get_VideoWidth(&lWidth);
					if (SUCCEEDED(hr1) && SUCCEEDED(hr2))
					{
						ResizeWindow(lWidth, abs(lHeight));
					}
				}
			}
		}

		RECT rc;
		gcap.pVW->put_Owner((OAHWND)ghwndAppSub/*Main*/);    // We own the window now
		gcap.pVW->put_WindowStyle(WS_CHILD);    // you are now a child

		// give the preview window all our space but where the status bar is
		GetClientRect(ghwndAppSub/*Main*/, &rc);
		//cyBorder = GetSystemMetrics(SM_CYBORDER);
		//cy = cyBorder;
		//rc.bottom -= cy;

		long setSize;
		setSize = (long)(rc.bottom) << 16;
		setSize = setSize | ((long)(rc.right) & 0xffff);
		//wWid = ((int)lParam & 0xffff);
		//wHig = ((int)lParam >> 16);


		SendMessage(ghwndAppMain, WM_SIZE, SIZE_RESTORED, setSize);// force a WM_SIZE message
		//gcap.pVW->SetWindowPosition(0, 0, rc.right, rc.bottom); // be this big

		gcap.pVW->put_Visible(OATRUE);
	}

	// make sure we process events while we're previewing!
	hr = gcap.pFg->QueryInterface(IID_IMediaEventEx, (void **)&gcap.pME);
	if (hr == NOERROR)
	{
		gcap.pME->SetNotifyWindow((OAHWND)ghwndAppMain, WM_FGNOTIFY, 0);
	}

	// potential debug output - what the graph looks like
	// DumpGraph(gcap.pFg, 1);

	// Add our graph to the running object table, which will allow
	// the GraphEdit application to "spy" on our graph
#ifdef REGISTER_FILTERGRAPH
	hr = AddGraphToRot(gcap.pFg, &g_dwGraphRegister);
	if (FAILED(hr))
	{
		ErrMsg(TEXT("Failed to register filter graph with ROT!  hr=0x%x"), hr);
		g_dwGraphRegister = 0;
	}
#endif
	StillPinCapture(); // add the still pin find routine --wenye
	// All done.
	gcap.fPreviewGraphBuilt = TRUE;
	return TRUE;
}

// stop the capture graph
//
BOOL StopCapture()
{
	// way ahead of you
	if (!gcap.fCapturing)
	{
		return FALSE;
	}

	// stop the graph
	IMediaControl *pMC = NULL;
	HRESULT hr = gcap.pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);
	if (SUCCEEDED(hr))
	{
		hr = pMC->Stop();
		pMC->Release();
	}
	if (FAILED(hr))
	{
		//ErrMsg(TEXT("Error %x: Cannot stop graph"), hr);
		return FALSE;
	}

	// when the graph was stopped
	gcap.lCapStopTime = timeGetTime();

	// no more status bar updates
	//KillTimer(ghwndApp, 1);

	// one last time for the final count and all the stats
	//UpdateStatus(TRUE);

	gcap.fCapturing = FALSE;

	// get rid of menu garbage
	//InvalidateRect(ghwndApp, NULL, TRUE);

	return TRUE;
}

INT_PTR CALLBACK ROIControlDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	POINT point;
	switch (msg)
	{
	case WM_INITDIALOG:
#ifdef DEBUG
		sprintf(logMessage, "\nCommand Fired : WM_INITDIALOG ID : IDD_ZOOM_CONTROL");
		printLogMessage(logMessage);
#endif
		//initCtrlSetting.ROIMode = 0;//initCtrlSetting.ROIMode_bak; //default is full pixel binning
		saveROIInitSetting(hwnd);
		OnInitROIDialog(hwnd);
		return TRUE;
		break;


	case WM_PARENTNOTIFY:
		return TRUE;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		case IDCANCEL:
			resetROIInitSetting(hwnd);
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		case IDC_COMBO_ROI_MODE:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case CBN_SELCHANGE:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_COMBO_ROI_MODE Event : CBN_SELCHANGE");
				printLogMessage(logMessage);
#endif
				//OnShutCtrlChange(hwnd);
				ROIMode(hwnd);
				break;
			}
			return TRUE;

		}
		break;
	}

	return FALSE;
}

void    ROIMode(HWND hwnd)
{

	BYTE *pbPropertyValue;
	HRESULT hr = S_OK;
	ULONG ulSize = 11;
	//int PropertyId = 16;
	//HRESULT hr = S_OK;
	BOOL is720 = TRUE;
	AM_MEDIA_TYPE *pmt;

	// default capture format
	if (gcap.pVSC && gcap.pVSC->GetFormat(&pmt) == S_OK)
	{
		if ((HEADER(pmt->pbFormat)->biHeight) == 480)
			is720 = FALSE;
		initCtrlSetting.ROIModeVlu = is720;
		pbPropertyValue = new BYTE[ulSize];
		HWND hListFocusMode = GetDlgItem(hwnd, IDC_COMBO_ROI_MODE);


		int sel = ComboBox_GetCurSel(hListFocusMode);

		//HWND hNearBNT = GetDlgItem(hwnd, IDC_BTN_FOCUS_FAR);
		//Button_Enable(hNearBNT, TRUE);
		//ON_UPDATE_COMMAND_UI();

		pbPropertyValue[0] = 0x01; // I2C write comman first byte 1
		pbPropertyValue[1] = 0x04;
		pbPropertyValue[2] = 0x70;
		pbPropertyValue[3] = 0x52;
		pbPropertyValue[4] = 0x30;
		pbPropertyValue[5] = 0x01;
		pbPropertyValue[6] = 0x00;
		pbPropertyValue[7] = 0x00;
		pbPropertyValue[8] = 0x01;
		switch (sel)
		{
		case 0:
			if (is720)
				pbPropertyValue[9] = 0x01; //full pixel binning
			else
				pbPropertyValue[9] = 0x31;
			break;
		case 1:
			if (is720)
				pbPropertyValue[9] = 0x02; //full pixel skip
			else
				pbPropertyValue[9] = 0x32;
			break;
		case 2:
			if (is720)
				pbPropertyValue[9] = 0x00; //ROI
			else
				pbPropertyValue[9] = 0x30;
			break;
		}
		pbPropertyValue[10] = 0x00;
		if (sel != initCtrlSetting.ROIMode){
			//initCtrlSetting.FocusMode_bak = initCtrlSetting.FocusMode;
			initCtrlSetting.ROIMode = sel;
			I2cCommandInt(pbPropertyValue);
		}
		delete[] pbPropertyValue;

	}

}

void    reROIMode(HWND hwnd, int sel)
{

	BYTE *pbPropertyValue;
	HRESULT hr = S_OK;
	ULONG ulSize = 11;
	//int PropertyId = 16;
	//HRESULT hr = S_OK;
	pbPropertyValue = new BYTE[ulSize];
	//HWND hListFocusMode = GetDlgItem(hwnd, IDC_COMBO_ROI_MODE);


	//int sel = ComboBox_GetCurSel(hListFocusMode);

	pbPropertyValue[0] = 0x01; // I2C write comman first byte 1
	pbPropertyValue[1] = 0x04;
	pbPropertyValue[2] = 0x70;
	pbPropertyValue[3] = 0x52;
	pbPropertyValue[4] = 0x30;
	pbPropertyValue[5] = 0x01;
	pbPropertyValue[6] = 0x00;
	pbPropertyValue[7] = 0x00;
	pbPropertyValue[8] = 0x01;
	switch (sel)
	{
	case 0:
		if (initCtrlSetting.ROIModeVlu)
			pbPropertyValue[9] = 0x01; //full pixel binning
		else
			pbPropertyValue[9] = 0x31;
		break;
	case 1:
		if (initCtrlSetting.ROIModeVlu)
			pbPropertyValue[9] = 0x02; //full pixel skip
		else
			pbPropertyValue[9] = 0x32;
		break;
	case 2:
		if (initCtrlSetting.ROIModeVlu)
			pbPropertyValue[9] = 0x00; //ROI
		else
			pbPropertyValue[9] = 0x30;
		break;
	}
	pbPropertyValue[10] = 0x00;
	//if (sel != initCtrlSetting.FocusMode){
	//initCtrlSetting.FocusMode_bak = initCtrlSetting.FocusMode;
	//initCtrlSetting.FocusMode = sel;
	I2cCommandInt(pbPropertyValue);
	//}
	delete[] pbPropertyValue;
}

void	saveROIInitSetting(HWND hwnd)
{
	long currValue, lCaps;

	// reset global struct
	//initCtrlSetting.DigitalMultiplier = 0;
	initCtrlSetting.ROIMode_bak = initCtrlSetting.ROIMode;
	//initCtrlSetting.FocusTrk_bak = initCtrlSetting.FocusTrk;

	// get current Value
	//currValue = 0;
	//getStandardControlPropertyCurrentValue(KSPROPERTY_VIDEOPROCAMP_DIGITAL_MULTIPLIER, &currValue, &lCaps);
	//initCtrlSetting.DigitalMultiplier = currValue;

}

void    OnInitROIDialog(HWND hwnd)
{
	//HRESULT hr = S_OK;
	//long lMin, lMax, lStep, lDefault;
	long currValue, lCaps;
	CString currValueStr;

	//hr = getStandardControlPropertyRange(KSPROPERTY_VIDEOPROCAMP_DIGITAL_MULTIPLIER, &lMin, &lMax, &lStep, &lDefault, &lCaps);
	//HWND hListSldDigZoomPos = GetDlgItem(hwnd, IDC_SLD_DIG_ZOOM_POS);

	//if (!SUCCEEDED(hr))
	//{
	//	EnableWindow(hListSldDigZoomPos, FALSE);
	//}

	//getStandardControlPropertyCurrentValue(KSPROPERTY_VIDEOPROCAMP_DIGITAL_MULTIPLIER, &currValue, &lCaps);
	//SendMessageA(hListSldDigZoomPos, TBM_SETRANGEMAX, TRUE, lMax);
	//SendMessageA(hListSldDigZoomPos, TBM_SETRANGEMIN, TRUE, lMin);
	//SendMessageA(hListSldDigZoomPos, TBM_SETPAGESIZE, TRUE, 1);
	//SendMessageA(hListSldDigZoomPos, TBM_SETPOS, TRUE, currValue);
	//HWND hListDigZoomPos = GetDlgItem(hwnd, IDC_EDIT_DIG_ZOOM_POS);
	//currValueStr.Format(L"%ld", currValue);
	//Edit_SetText(hListDigZoomPos, currValueStr);
	//EnableWindow(hListDigZoomPos, FALSE);

	HWND hListFocusMode = GetDlgItem(hwnd, IDC_COMBO_ROI_MODE);
	ComboBox_AddString(hListFocusMode, L"full pixel skip");
	ComboBox_AddString(hListFocusMode, L"full pixel binning");
	ComboBox_AddString(hListFocusMode, L"ROI");
	ComboBox_SetCurSel(hListFocusMode, initCtrlSetting.ROIMode);
}

void	resetROIInitSetting(HWND hwnd)
{
	//if (ksNodeTree.isOKpProcAmp)
	//{
	//HWND hListSldDigZoomPos = GetDlgItem(hwnd, IDC_SLD_DIG_ZOOM_POS);
	//long SldPos = (long)SendMessageA(hListSldDigZoomPos, TBM_GETPOS, TRUE, SldPos);
	//if (SldPos != initCtrlSetting.DigitalMultiplier)
	//	ksNodeTree.pProcAmp->Set(KSPROPERTY_VIDEOPROCAMP_DIGITAL_MULTIPLIER, (long)initCtrlSetting.DigitalMultiplier, VideoProcAmp_Flags_Manual);

	if (initCtrlSetting.ROIMode != initCtrlSetting.ROIMode_bak){
		initCtrlSetting.ROIMode = initCtrlSetting.ROIMode_bak;
		reROIMode(hwnd, initCtrlSetting.ROIMode);
	}
	//if (initCtrlSetting.FocusTrk != initCtrlSetting.FocusTrk_bak){
	//	initCtrlSetting.FocusTrk = initCtrlSetting.FocusTrk_bak;
	//	refocusZoomTrk(hwnd, initCtrlSetting.FocusTrk);
	//}

	//}
}
/*----------------------------------------------------------------------------*\
|   ErrMsg - Opens a Message box with a error message in it.  The user can     |
|            select the OK button to continue                                  |
\*----------------------------------------------------------------------------*/
void ErrMsg(LPTSTR szFormat, ...)
{
	static TCHAR szBuffer[2048] = { 0 };
	const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
	const int LASTCHAR = NUMCHARS - 1;

	// Format the input string
	va_list pArgs;
	va_start(pArgs, szFormat);

	// Use a bounded buffer size to prevent buffer overruns.  Limit count to
	// character size minus one to allow for a NULL terminating character.
	_vsntprintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
	va_end(pArgs);

	// Ensure that the formatted string is NULL-terminated
	szBuffer[LASTCHAR] = TEXT('\0');

	MessageBox(ghwndAppMain, szBuffer, NULL,
		MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
}

void ShowCapFilterPropPage(HWND hwnd)
{
	ISpecifyPropertyPages *pSpec;
	CAUUID cauuid;

	HRESULT hr = S_OK;
	if (gcap.pVCap)
	{
		hr = gcap.pVCap->QueryInterface(IID_ISpecifyPropertyPages,
			(void **)&pSpec);

		if (hr == S_OK)
		{
			hr = pSpec->GetPages(&cauuid);

			hr = OleCreatePropertyFrame(hwnd, 30, 30, NULL, 1,
				(IUnknown **)&gcap.pVCap, cauuid.cElems,
				(GUID *)cauuid.pElems, 0, 0, NULL);

			CoTaskMemFree(cauuid.pElems);
			pSpec->Release();
		}
	}
}



void GetCurrentStillFormat(eMediaType *MediaType, TCHAR* Format)
{
	// DV capture does not use a VIDEOINFOHEADER
	if (gcap.pSSC != NULL)
	{
		AM_MEDIA_TYPE *pmt;
		HRESULT hr = gcap.pSSC->GetFormat(&pmt);
		eMediaType m_MediaSubType;
		if (pmt->formattype == FORMAT_VideoInfo)
		{
			if (pmt->subtype == MEDIASUBTYPE_YUY2)
			{
				//PrintMessage(L"MEDIASUBTYPE_YUY2 \r\n");
				m_MediaSubType = MEDIA_YUY2;
			}
			else if (pmt->subtype == MEDIASUBTYPE_MJPG)
			{
				//PrintMessage(L"MEDIASUBTYPE_MJPG \r\n");
				m_MediaSubType = MEDIA_MJPEG;
			}
		}
		if (m_MediaSubType == MEDIA_MJPEG)
		{
			wsprintf(Format, TEXT("%s"), TEXT("MJPG"));
			OutputDebugStringW(Format);

		}
		else
		{
			wsprintf(Format, TEXT("%s"), TEXT("YUY2"));
			OutputDebugStringW(Format);
		}
	}
}

/* the 2/1.2 mp color camera control dialog */
/////////////////////////////////////////////////////////////////////

// Dialog functions

void    OnInitCameraControlDialog(HWND hwnd);
void	saveCameraControlInitSetting(HWND hwnd);
void	resetCameraControlInitSetting(HWND hwnd);
HRESULT	OnShutCtrlChange(HWND hwnd);
//HRESULT	getExtControlValue(int PropertyId, int *curValue);
HRESULT	OnSenUpModeChange(HWND hwnd);
HRESULT	onAEReferenceLevelChange(HWND hwnd);
HRESULT OnExposureModeChange(HWND hwnd);
HRESULT	onAgcLvlChange(HWND hwnd);
HRESULT	onBLCRangeChange(HWND hwnd);
HRESULT OnSelectedIndexChangeMirror(HWND hwnd);
//int		getMainsFrequency();
HRESULT OnMainsFrequencyChange(HWND hwnd);
HRESULT	onBacklightCompensationChange(HWND hwnd);
HRESULT OnCameraModeChange(HWND hwnd);
HRESULT OnBLCWeightFactorChange(HWND hwnd);
HRESULT OnBLCGridChange(HWND hwnd);

//HRESULT getBLCRangeValue(int PropertyId, int *hpos, int *hsize, int *vpos, int *vsize);
HRESULT getExposureAGCLvlValue(int PropertyId, int *ExpValue, int *AgcLvlValue);
HRESULT setExposureAGCLvlValue(HWND hwnd, int PropertyId, int ExpValue, int AgcLvlValue);
//HRESULT setExtControls(int PropertyId, int PropertyValue);

//-------------------------------------------------------------------
//  CameraControlDlgProc
//
//  CameraControlDlgProc procedure for the "Extension settings" dialog.
//-------------------------------------------------------------------

INT_PTR CALLBACK CameraControlDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
	case WM_INITDIALOG:
#ifdef DEBUG
		sprintf(logMessage, "\nCommand Fired : WM_INITDIALOG ID : IDD_CAM_CONTROL");
		printLogMessage(logMessage);
#endif
		saveCameraControlInitSetting(hwnd);  // save initial value
		OnInitCameraControlDialog(hwnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			//OnSelectedIndexChangeMirror(hwnd, pParam);
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		case IDC_COMBO_SHUT_CONTL:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case CBN_SELCHANGE:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_COMBO_SHUT_CONTL Event : CBN_SELCHANGE");
				printLogMessage(logMessage);
#endif
				OnShutCtrlChange(hwnd);
				break;
			}
			return TRUE;

		case IDC_COMBO_SEN_UP_MODE:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case CBN_SELCHANGE:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_COMBO_SEN_UP_MODE Event : CBN_SELCHANGE");
				printLogMessage(logMessage);
#endif
				OnSenUpModeChange(hwnd);
				break;
			}
			return TRUE;

		case IDC_COMBO_EXPOSURE_MODE:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case CBN_SELCHANGE:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_COMBO_EXPOSURE_MODE Event : CBN_SELCHANGE");
				printLogMessage(logMessage);
#endif
				OnExposureModeChange(hwnd);
				break;
			}
			return TRUE;

		case IDC_COMBO_MIRROR_MODE:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case CBN_SELCHANGE:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_COMBO_MIRROR_MODE Event : CBN_SELCHANGE");
				printLogMessage(logMessage);
#endif
				OnSelectedIndexChangeMirror(hwnd);
				break;
			}
			return TRUE;

		case IDC_COMBO_MAIN_FEQ:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case CBN_SELCHANGE:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_COMBO_MAIN_FEQ Event : CBN_SELCHANGE");
				printLogMessage(logMessage);
#endif
				OnMainsFrequencyChange(hwnd);
				break;
			}
			return TRUE;

		case IDC_COMBO_BACK_LIGHT:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case CBN_SELCHANGE:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_COMBO_BACK_LIGHT Event : CBN_SELCHANGE");
				printLogMessage(logMessage);
#endif
				onBacklightCompensationChange(hwnd);
				break;
			}
			return TRUE;

		case IDC_COMBO_CAM_MODE:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case CBN_SELCHANGE:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_COMBO_CAM_MODE Event : CBN_SELCHANGE");
				printLogMessage(logMessage);
#endif
				OnCameraModeChange(hwnd);
				break;
			}
			return TRUE;

		case IDC_COMBO_BLC_WGHT_FACT:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case CBN_SELCHANGE:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_COMBO_BLC_WGHT_FACT Event : CBN_SELCHANGE");
				printLogMessage(logMessage);
#endif
				OnBLCWeightFactorChange(hwnd);
				break;
			}
			return TRUE;

		case IDC_COMBO_BLC_GRID:
			switch (HIWORD(wParam)) // Find out what message it was
			{
			case CBN_SELCHANGE:
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_COMBO_BLC_GRID Event : CBN_SELCHANGE");
				printLogMessage(logMessage);
#endif
				OnBLCGridChange(hwnd);
				break;
			}
			return TRUE;


		case IDCANCEL:
			resetCameraControlInitSetting(hwnd);  // reset initial value
			EndDialog(hwnd, LOWORD(wParam));
			return TRUE;

		}
		break;

	case WM_HSCROLL:
		HWND hSLDArRefLvl = GetDlgItem(hwnd, IDC_SLD_AE_REF_LVL);
		if ((HWND)lParam == hSLDArRefLvl)
		{
			switch (LOWORD(wParam)) // Find out what message it was
			{
			case TB_ENDTRACK: // This means that the list is about to display
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_SLD_AE_REF_LVL Event : TB_ENDTRACK");
				printLogMessage(logMessage);
#endif
				onAEReferenceLevelChange(hwnd);
				break;
			}
			return TRUE;
		}

		HWND hSLDAgcLvl = GetDlgItem(hwnd, IDC_SLD_AGC_LVL);
		if ((HWND)lParam == hSLDAgcLvl)
		{
			switch (LOWORD(wParam)) // Find out what message it was
			{
			case TB_ENDTRACK: // This means that the list is about to display
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_SLD_AGC_LVL Event : TB_ENDTRACK");
				printLogMessage(logMessage);

#endif
				onAgcLvlChange(hwnd);
				break;
			}
			return TRUE;
		}

		HWND hSLDHPos = GetDlgItem(hwnd, IDC_SLD_HPos);
		HWND hSLDHSize = GetDlgItem(hwnd, IDC_SLD_HSize);
		HWND hSLDVPos = GetDlgItem(hwnd, IDC_SLD_VPos);
		HWND hSLDVSize = GetDlgItem(hwnd, IDC_SLD_VSize);
		if ((HWND)lParam == hSLDHPos || (HWND)lParam == hSLDHSize || (HWND)lParam == hSLDVPos || (HWND)lParam == hSLDVSize)
		{
			switch (LOWORD(wParam)) // Find out what message it was
			{
			case TB_ENDTRACK: // This means that the list is about to display
#ifdef DEBUG
				sprintf(logMessage, " \nCommand Fired : IDC_SLD_HPos Event : TB_ENDTRACK");
				printLogMessage(logMessage);
#endif
				onBLCRangeChange(hwnd);
				break;
			}
			return TRUE;
		}

		break;

	}

	return FALSE;
}


void OnInitCameraControlDialog(HWND hwnd)
{
	HRESULT hr = S_OK;
	CString retValueStr;
	int retValue = 0;
	long currValue, lCaps;
	long lMin, lMax, lStep, lDefault;

	hr = getExtControlValue(1, &retValue);
	hr = getExtControlValue(17, &retValue);

	HWND hListShutCtrl = GetDlgItem(hwnd, IDC_COMBO_SHUT_CONTL);
	hr = getExtControlValue(1, &retValue);
	if (FAILED(hr))
	{
		EnableWindow(hListShutCtrl, FALSE);
	}
	ComboBox_AddString(hListShutCtrl, L"0x00:AUTO");
	ComboBox_AddString(hListShutCtrl, L"0x01:1/50 - 1/60");
	ComboBox_AddString(hListShutCtrl, L"0x02:Flickerless");
	ComboBox_AddString(hListShutCtrl, L"0x03:1/250");
	ComboBox_AddString(hListShutCtrl, L"0x04:1/500");
	ComboBox_AddString(hListShutCtrl, L"0x05:1/1000");
	ComboBox_AddString(hListShutCtrl, L"0x06:1/2000");
	ComboBox_AddString(hListShutCtrl, L"0x07:1/5000");
	ComboBox_AddString(hListShutCtrl, L"0x08:1/10000");
	ComboBox_AddString(hListShutCtrl, L"0x09:1/100000");
	ComboBox_AddString(hListShutCtrl, L"0x0a:extended X2");
	ComboBox_AddString(hListShutCtrl, L"0x0b:extended X4");
	ComboBox_AddString(hListShutCtrl, L"0x0c:extended X6");
	ComboBox_AddString(hListShutCtrl, L"0x0d:extended X8");
	ComboBox_AddString(hListShutCtrl, L"0x0e:extended X10");
	ComboBox_AddString(hListShutCtrl, L"0x0f:extended X15");
	ComboBox_AddString(hListShutCtrl, L"0x10:extended X20");
	ComboBox_AddString(hListShutCtrl, L"0x11:extended X25");
	ComboBox_AddString(hListShutCtrl, L"0x12:extended X30");
	ComboBox_SetCurSel(hListShutCtrl, retValue);


	HWND hListSenseUpMode = GetDlgItem(hwnd, IDC_COMBO_SEN_UP_MODE);
	hr = getExtControlValue(2, &retValue);
	if (FAILED(hr))
	{
		EnableWindow(hListSenseUpMode, FALSE);
	}

	ComboBox_AddString(hListSenseUpMode, L"00:OFF");
	ComboBox_AddString(hListSenseUpMode, L"01:X2");
	ComboBox_AddString(hListSenseUpMode, L"02:X4");
	ComboBox_AddString(hListSenseUpMode, L"03:X6");
	ComboBox_AddString(hListSenseUpMode, L"04:X8");
	ComboBox_AddString(hListSenseUpMode, L"05:X10");
	ComboBox_AddString(hListSenseUpMode, L"06:X15");
	ComboBox_AddString(hListSenseUpMode, L"07:X20");
	ComboBox_AddString(hListSenseUpMode, L"08:X25");
	ComboBox_AddString(hListSenseUpMode, L"09:X30");
	ComboBox_SetCurSel(hListSenseUpMode, retValue);

	HWND hListSldAELVL = GetDlgItem(hwnd, IDC_SLD_AE_REF_LVL);
	hr = getExtControlValue(11, &retValue);
	if (FAILED(hr))
	{
		EnableWindow(hListSldAELVL, FALSE);
	}

	SendMessageA(hListSldAELVL, TBM_SETRANGEMAX, TRUE, 64);
	SendMessageA(hListSldAELVL, TBM_SETRANGEMIN, TRUE, 0);
	SendMessageA(hListSldAELVL, TBM_SETPAGESIZE, TRUE, 1);
	SendMessageA(hListSldAELVL, TBM_SETPOS, TRUE, retValue);
	HWND hListAELVL = GetDlgItem(hwnd, IDC_EDIT_AE_REF_LVL);
	retValueStr.Format(L"%d", retValue);
	EnableWindow(hListAELVL, FALSE);
	Edit_SetText(hListAELVL, retValueStr);

	int expMode = 0, AGCLvl = 0;
	hr = getExposureAGCLvlValue(10, &expMode, &AGCLvl);

	HWND hListExpoMode = GetDlgItem(hwnd, IDC_COMBO_EXPOSURE_MODE);
	if (FAILED(hr))
	{
		EnableWindow(hListExpoMode, FALSE);
	}
	ComboBox_AddString(hListExpoMode, L"0x00:AUTO");
	ComboBox_AddString(hListExpoMode, L"0x01");
	ComboBox_AddString(hListExpoMode, L"0x02");
	ComboBox_AddString(hListExpoMode, L"0x03");
	ComboBox_AddString(hListExpoMode, L"0x04");
	ComboBox_AddString(hListExpoMode, L"0x05");
	ComboBox_AddString(hListExpoMode, L"0x06");
	ComboBox_AddString(hListExpoMode, L"0x07");
	ComboBox_AddString(hListExpoMode, L"0x08");
	ComboBox_AddString(hListExpoMode, L"0x09");
	ComboBox_AddString(hListExpoMode, L"0x0a");
	ComboBox_AddString(hListExpoMode, L"0x0b");
	ComboBox_AddString(hListExpoMode, L"0x0c");
	ComboBox_AddString(hListExpoMode, L"0x0d");
	ComboBox_AddString(hListExpoMode, L"0x0e");
	ComboBox_AddString(hListExpoMode, L"0x0f");
	ComboBox_AddString(hListExpoMode, L"0x10");
	ComboBox_AddString(hListExpoMode, L"0x11");
	ComboBox_AddString(hListExpoMode, L"0x12");
	ComboBox_SetCurSel(hListExpoMode, expMode);

	HWND hListSldAGCLVL = GetDlgItem(hwnd, IDC_SLD_AGC_LVL);
	if (FAILED(hr) || expMode == 0)
	{
		EnableWindow(hListSldAGCLVL, FALSE);
	}

	SendMessageA(hListSldAGCLVL, TBM_SETRANGEMAX, TRUE, 255);
	SendMessageA(hListSldAGCLVL, TBM_SETRANGEMIN, TRUE, 0);
	SendMessageA(hListSldAGCLVL, TBM_SETPAGESIZE, TRUE, 1);
	SendMessageA(hListSldAGCLVL, TBM_SETPOS, TRUE, AGCLvl);

	HWND hListAGCLVL = GetDlgItem(hwnd, IDC_EDIT_AGC_LVL);
	//Edit_SetText(hListAGCLVL, L"0");
	EnableWindow(hListAGCLVL, FALSE);
	retValueStr.Format(L"%d", AGCLvl);
	Edit_SetText(hListAGCLVL, retValueStr);


	HWND hListMirrorMode = GetDlgItem(hwnd, IDC_COMBO_MIRROR_MODE);
	hr = getExtControlValue(3, &retValue);
	if (FAILED(hr))
	{
		EnableWindow(hListMirrorMode, FALSE);
	}
	ComboBox_AddString(hListMirrorMode, L"OFF");
	ComboBox_AddString(hListMirrorMode, L"MIRROR");
	ComboBox_AddString(hListMirrorMode, L"V_FLIP");
	ComboBox_AddString(hListMirrorMode, L"ROTATE ");
	ComboBox_SetCurSel(hListMirrorMode, retValue);


	HWND hListMainFeq = GetDlgItem(hwnd, IDC_COMBO_MAIN_FEQ);
	ComboBox_AddString(hListMainFeq, L"Disabled");
	ComboBox_AddString(hListMainFeq, L"50 Hz");
	ComboBox_AddString(hListMainFeq, L"60 Hz");
	ComboBox_SetCurSel(hListMainFeq, getMainsFrequency());

	/* check the res */
	AM_MEDIA_TYPE *pmt;
	gcap.pVSC->GetFormat(&pmt);
	int setV = 0;
	getStandardControlPropertyCurrentValue(KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION, &currValue, &lCaps);
	HWND hListBackLight = GetDlgItem(hwnd, IDC_COMBO_BACK_LIGHT);
	if ((pmt->lSampleSize == 4147200) || (pmt->lSampleSize == 1036800)){
		ComboBox_AddString(hListBackLight, L"Wide Dyn Range");
		switch (currValue)
		{
		case 0:
			setV = 1;// 3;
			break;
		case 1:
			setV = 3;// 0;
			break;
		case 2:
			setV = 2;
			break;
		case 3:
			setV = 0;// 1;
			break;
		}
	}
	else{
		switch (currValue)
		{
		case 0:
			setV = 0;
			break;
		case 1:
			setV = 1;
			break;
		}
	}
	ComboBox_AddString(hListBackLight, L"BLC off");
	if ((pmt->lSampleSize == 4147200) || (pmt->lSampleSize == 1036800)){
		ComboBox_AddString(hListBackLight, L"HBLC");
	}
	ComboBox_AddString(hListBackLight, L"BLC on");
	ComboBox_SetCurSel(hListBackLight, setV);

	HWND hListCameraMode = GetDlgItem(hwnd, IDC_COMBO_CAM_MODE);
	hr = getExtControlValue(13, &retValue);
	if (FAILED(hr))
	{
		EnableWindow(hListCameraMode, FALSE);
	}

	// default capture format

	// DV capture does not use a VIDEOINFOHEADER
	if (pmt->lSampleSize == 4147200) //1080p
	{
		ComboBox_AddString(hListCameraMode, L"1080P(25/30fps)");
		//ComboBox_AddString(hListCameraMode, L"Linear 1080P(25/30fps),BLC OFF");
		//ComboBox_AddString(hListCameraMode, L"Linear 1080P(25/30fps),BLC ON");
		//ComboBox_AddString(hListCameraMode, L"Linear 1080P(25/30fps),HBLC OFF");
		//ComboBox_AddString(hListCameraMode, L"WDR 1080P(25/30fps),BLC OFF");
	}
	if (pmt->lSampleSize == 1036800) //1080p/2
	{
		ComboBox_AddString(hListCameraMode, L"1080P(25/30fps) USB2");
		//ComboBox_AddString(hListCameraMode, L"Linear 1080P(25/30fps),BLC OFF");
		//ComboBox_AddString(hListCameraMode, L"Linear 1080P(25/30fps),BLC ON");
		//ComboBox_AddString(hListCameraMode, L"Linear 1080P(25/30fps),HBLC OFF");
		//ComboBox_AddString(hListCameraMode, L"WDR 1080P(25/30fps),BLC OFF");
	}

	if (pmt->lSampleSize == 1843200) //720p
	{
		ComboBox_AddString(hListCameraMode, L"720P(50/60fps)");
		// This capture filter captures something other that pure video.
		//ComboBox_AddString(hListCameraMode, L"Linear 720P(50 / 60fps), BLC OFF");
		//ComboBox_AddString(hListCameraMode, L"Linear 720P(50/60fps), BLC ON");
		//ComboBox_AddString(hListCameraMode, L"Linear 720P(50/60fps), HBLC ON");
	}
	if (pmt->lSampleSize == 460800) //720p/2
	{
		ComboBox_AddString(hListCameraMode, L"720P(50/60fps) USB2");
		// This capture filter captures something other that pure video.
		//ComboBox_AddString(hListCameraMode, L"Linear 720P(50 / 60fps), BLC OFF");
		//ComboBox_AddString(hListCameraMode, L"Linear 720P(50/60fps), BLC ON");
		//ComboBox_AddString(hListCameraMode, L"Linear 720P(50/60fps), HBLC ON");
	}

	/*
	ComboBox_AddString(hListCameraMode, L"Linear 1080P(25/30fps),BLC OFF");
	ComboBox_AddString(hListCameraMode, L"Linear 1080P(25/30fps),BLC ON");
	ComboBox_AddString(hListCameraMode, L"Linear 1080P(25/30fps),HBLC OFF");
	ComboBox_AddString(hListCameraMode, L"WDR 1080P(25/30fps),BLC OFF");
	ComboBox_AddString(hListCameraMode, L"Linear 720P(50 / 60fps), BLC OFF");
	ComboBox_AddString(hListCameraMode, L"Linear 720P(50/60fps), BLC ON");
	ComboBox_AddString(hListCameraMode, L"Linear 720P(50/60fps), HBLC ON");*/
	ComboBox_SetCurSel(hListCameraMode, 0/*retValue*/);

	HWND hListBLCGrid = GetDlgItem(hwnd, IDC_COMBO_BLC_GRID);
	hr = getExtControlValue(19, &retValue);
	if (FAILED(hr))
	{
		EnableWindow(hListBLCGrid, FALSE);
	}
	ComboBox_AddString(hListBLCGrid, L"disable");
	ComboBox_AddString(hListBLCGrid, L"enable");
	ComboBox_SetCurSel(hListBLCGrid, retValue);

	HWND hListBLCWF = GetDlgItem(hwnd, IDC_COMBO_BLC_WGHT_FACT);
	hr = getExtControlValue(18, &retValue);
	if (FAILED(hr))
	{
		EnableWindow(hListBLCWF, FALSE);
	}
	ComboBox_AddString(hListBLCWF, L"low-gain");
	ComboBox_AddString(hListBLCWF, L"medium-gain");
	ComboBox_AddString(hListBLCWF, L"high-gain");
	ComboBox_SetCurSel(hListBLCWF, retValue);

	int hpos = 0, hsize = 0, vpos = 0, vsize = 0;
	hr = getBLCRangeValue(17, &hpos, &hsize, &vpos, &vsize);

	HWND hListSldHPos = GetDlgItem(hwnd, IDC_SLD_HPos);
	HWND hListSldHSize = GetDlgItem(hwnd, IDC_SLD_HSize);
	HWND hListSldVPos = GetDlgItem(hwnd, IDC_SLD_VPos);
	HWND hListSldVSize = GetDlgItem(hwnd, IDC_SLD_VSize);
	if (FAILED(hr))
	{
		EnableWindow(hListSldHPos, FALSE);
		EnableWindow(hListSldHSize, FALSE);
		EnableWindow(hListSldVPos, FALSE);
		EnableWindow(hListSldVSize, FALSE);
	}

	SendMessageA(hListSldHPos, TBM_SETRANGEMAX, TRUE, 12);
	SendMessageA(hListSldHPos, TBM_SETRANGEMIN, TRUE, 0);
	SendMessageA(hListSldHPos, TBM_SETPAGESIZE, TRUE, 1);
	SendMessageA(hListSldHPos, TBM_SETPOS, TRUE, hpos);

	HWND hListEditHPos = GetDlgItem(hwnd, IDC_EDIT_HPos);
	EnableWindow(hListEditHPos, FALSE);
	retValueStr.Format(L"%d", hpos);
	Edit_SetText(hListEditHPos, retValueStr);


	SendMessageA(hListSldHSize, TBM_SETRANGEMAX, TRUE, 12);
	SendMessageA(hListSldHSize, TBM_SETRANGEMIN, TRUE, 0);
	SendMessageA(hListSldHSize, TBM_SETPAGESIZE, TRUE, 1);
	SendMessageA(hListSldHSize, TBM_SETPOS, TRUE, hsize);

	HWND hListEditHSize = GetDlgItem(hwnd, IDC_EDIT_HSize);
	EnableWindow(hListEditHSize, FALSE);
	retValueStr.Format(L"%d", hsize);
	Edit_SetText(hListEditHSize, retValueStr);

	SendMessageA(hListSldVPos, TBM_SETRANGEMAX, TRUE, 7);
	SendMessageA(hListSldVPos, TBM_SETRANGEMIN, TRUE, 0);
	SendMessageA(hListSldVPos, TBM_SETPAGESIZE, TRUE, 1);
	SendMessageA(hListSldVPos, TBM_SETPOS, TRUE, vpos);

	HWND hListEditVPos = GetDlgItem(hwnd, IDC_EDIT_VPos);
	EnableWindow(hListEditVPos, FALSE);
	retValueStr.Format(L"%d", vpos);
	Edit_SetText(hListEditVPos, retValueStr);

	SendMessageA(hListSldVSize, TBM_SETRANGEMAX, TRUE, 7);
	SendMessageA(hListSldVSize, TBM_SETRANGEMIN, TRUE, 0);
	SendMessageA(hListSldVSize, TBM_SETPAGESIZE, TRUE, 1);
	SendMessageA(hListSldVSize, TBM_SETPOS, TRUE, vsize);

	HWND hListEditVSize = GetDlgItem(hwnd, IDC_EDIT_VSize);
	EnableWindow(hListEditVSize, FALSE);
	retValueStr.Format(L"%d", vsize);
	Edit_SetText(hListEditVSize, retValueStr);

}

void saveCameraControlInitSetting(HWND hwnd)
{
	long currValue, lCaps;
	int setV = 0;

	// reset global struct
	initCtrlSetting.ShutterControl = 0;
	initCtrlSetting.SenseUpMode = 0;
	initCtrlSetting.AEReferenceLevel = 0;
	initCtrlSetting.ExposureMode = 0;
	initCtrlSetting.AGCLevel = 0;
	initCtrlSetting.MirrorMode = 0;
	initCtrlSetting.MainsFrequency = 0;
	initCtrlSetting.BacklightCompensation = 0;
	initCtrlSetting.CameraMode = 0;

	// get current Value
	getExtControlValue(1, &initCtrlSetting.ShutterControl);
	getExtControlValue(2, &initCtrlSetting.SenseUpMode);
	getExtControlValue(11, &initCtrlSetting.AEReferenceLevel);

	getExposureAGCLvlValue(10, &initCtrlSetting.ExposureMode, &initCtrlSetting.AGCLevel);

	getExtControlValue(3, &initCtrlSetting.MirrorMode);
	//getExtControlValue(13, &initCtrlSetting.CameraMode);
	//initCtrlSetting.CameraMode = 0;
	getExtControlValue(18, &initCtrlSetting.BLCWeightFactor);
	getExtControlValue(19, &initCtrlSetting.BLCGrid);

	getStandardControlPropertyCurrentValue(KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY, &currValue, &lCaps);
	initCtrlSetting.MainsFrequency = currValue;
	currValue = 0;
	getStandardControlPropertyCurrentValue(KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION, &currValue, &lCaps);
#if 0	
	/* check the res */
	AM_MEDIA_TYPE *pmt;
	gcap.pVSC->GetFormat(&pmt);

	if ((pmt->lSampleSize == 4147200) || (pmt->lSampleSize == 1036800)) //1080p
	{
		switch (currValue)
		{
		case 0:
			setV = 1;
			break;
		case 1:
			setV = 3;
			break;
		case 2:
			setV = 2;
			break;
		case 3:
			setV = 0;
			break;
		}
	}
	else{ //720p
		switch (currValue)
		{
		case 0:
			setV = 0;
			break;
		case 1:
			setV = 1;
			break;
		}

	}

#endif	
	initCtrlSetting.BacklightCompensation = currValue;

}

void resetCameraControlInitSetting(HWND hwnd)
{
	HWND hListComboShutCtrl = GetDlgItem(hwnd, IDC_COMBO_SHUT_CONTL);
	int sel = ComboBox_GetCurSel(hListComboShutCtrl);
	int setV = 0;
	if (sel != initCtrlSetting.ShutterControl)
		setExtControls(1, initCtrlSetting.ShutterControl);

	HWND hListSenseUpMode = GetDlgItem(hwnd, IDC_COMBO_SEN_UP_MODE);
	sel = ComboBox_GetCurSel(hListSenseUpMode);
	if (sel != initCtrlSetting.SenseUpMode)
		setExtControls(2, initCtrlSetting.SenseUpMode);

	HWND hListComboMirror = GetDlgItem(hwnd, IDC_COMBO_MIRROR_MODE);
	sel = ComboBox_GetCurSel(hListComboMirror);
	if (sel != initCtrlSetting.MirrorMode)
		setExtControls(3, initCtrlSetting.MirrorMode);

	HWND hListSldAELVL = GetDlgItem(hwnd, IDC_SLD_AE_REF_LVL);
	long arSldPos = (long)SendMessageA(hListSldAELVL, TBM_GETPOS, TRUE, arSldPos);
	if (arSldPos != initCtrlSetting.AEReferenceLevel)
		setExtControls(11, initCtrlSetting.AEReferenceLevel);

	HWND hListComboCameraMode = GetDlgItem(hwnd, IDC_COMBO_CAM_MODE);
	sel = ComboBox_GetCurSel(hListComboCameraMode);
	if (sel != initCtrlSetting.CameraMode)
		setExtControls(13, initCtrlSetting.CameraMode);

	HWND hListComboBLCWF = GetDlgItem(hwnd, IDC_COMBO_BLC_WGHT_FACT);
	sel = ComboBox_GetCurSel(hListComboBLCWF);
	if (sel != initCtrlSetting.BLCWeightFactor)
		setExtControls(18, initCtrlSetting.BLCWeightFactor);

	HWND hListComboBLCGrid = GetDlgItem(hwnd, IDC_COMBO_BLC_GRID);
	sel = ComboBox_GetCurSel(hListComboBLCGrid);
	if (sel != initCtrlSetting.BLCGrid)
		setExtControls(19, initCtrlSetting.BLCGrid);


	HWND hListExpoMode = GetDlgItem(hwnd, IDC_COMBO_EXPOSURE_MODE);
	sel = ComboBox_GetCurSel(hListExpoMode);
	HWND hListSldAGCLVL = GetDlgItem(hwnd, IDC_SLD_AGC_LVL);
	int agcSldPos = (int)SendMessageA(hListSldAGCLVL, TBM_GETPOS, TRUE, agcSldPos);

	if (sel != initCtrlSetting.ExposureMode || agcSldPos != initCtrlSetting.AGCLevel)
	{
		setExposureAGCLvlValue(hwnd, 10, initCtrlSetting.ExposureMode, initCtrlSetting.AGCLevel);
	}


	if (ksNodeTree.isOKpProcAmp)
	{
		HWND hListMainFeq = GetDlgItem(hwnd, IDC_COMBO_MAIN_FEQ);
		sel = ComboBox_GetCurSel(hListMainFeq);
		if (sel != initCtrlSetting.MainsFrequency)
			ksNodeTree.pProcAmp->Set(KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY, (long)initCtrlSetting.MainsFrequency, VideoProcAmp_Flags_Manual);

		HWND hListBackLight = GetDlgItem(hwnd, IDC_COMBO_BACK_LIGHT);
		sel = ComboBox_GetCurSel(hListBackLight);

		/* check the res */
		AM_MEDIA_TYPE *pmt;
		gcap.pVSC->GetFormat(&pmt);

		if ((pmt->lSampleSize == 4147200) || (pmt->lSampleSize == 1036800)) //1080p
		{
			switch (sel)
			{
			case 0:
				setV = 3;
				break;
			case 1:
				setV = 0;
				break;
			case 2:
				setV = 2;
				break;
			case 3:
				setV = 1;
				break;
			}
		}
		else{ //720p
			switch (sel)
			{
			case 0:
				setV = 0;
				break;
			case 1:
				setV = 1;
				break;
			}

		}


		if (setV != initCtrlSetting.BacklightCompensation)
			ksNodeTree.pProcAmp->Set(KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION, (long)initCtrlSetting.BacklightCompensation, VideoProcAmp_Flags_Manual);

	}

}
#if 0
HRESULT	getExtControlValue(int PropertyId, int *curValue)
{
	HRESULT hr = S_OK;
	ULONG ulSize;
	BYTE *pbPropertyValue;
	int retValue = 0;

	hr = getExtionControlPropertySize(PropertyId, &ulSize);
	if (FAILED(hr))
	{
#ifdef DEBUG
		sprintf(logMessage, " \nERROR \t Function : getExtControlValue \t Msg : Unable to find property[%d] size : %x", PropertyId, hr);
		printLogMessage(logMessage);
#endif
	}
	else
	{
		pbPropertyValue = new BYTE[ulSize];
		if (!pbPropertyValue)
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : getExtControlValue \t Msg : Unable to allocate memory for property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}

		hr = getExtionControlProperty(PropertyId, ulSize, pbPropertyValue);

		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : getExtControlValue \t Msg : Unable to get property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}

		memcpy(&retValue, (char *)pbPropertyValue, ulSize);

#ifdef DEBUG
		sprintf(logMessage, " \nFunction : getExtControlValue \t Msg : property[%d] value[%d]", PropertyId, retValue);
		printLogMessage(logMessage);
#endif

	}

	*curValue = retValue;
	return hr;

}
#endif
#if 0
HRESULT getBLCRangeValue(int PropertyId, int *hpos, int *hsize, int *vpos, int *vsize)
{
	HRESULT hr = S_OK;
	ULONG ulSize;
	BYTE *pbPropertyValue;
	int retValue = 0;

	*hpos = 0;
	*hsize = 0;
	*vpos = 0;
	*vsize = 0;

	hr = getExtionControlPropertySize(PropertyId, &ulSize);
	//ulSize = 2;
	if (FAILED(hr) || (ulSize != 2))
	{
#ifdef DEBUG
		sprintf(logMessage, " \nERROR \t Function : getBLCRangeValue \t Msg : Unable to find property[%d] size : %x", PropertyId, hr);
		printLogMessage(logMessage);
#endif
	}
	else
	{
		pbPropertyValue = new BYTE[ulSize];
		if (!pbPropertyValue)
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : getBLCRangeValue \t Msg : Unable to allocate memory for property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}

		hr = getExtionControlProperty(PropertyId, ulSize, pbPropertyValue);
		//pbPropertyValue[0] = 0x05;
		//pbPropertyValue[1] = 0x06;

		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : getBLCRangeValue \t Msg : Unable to get property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}

		// *pbPropertyValue = (*pbPropertyValue & 0x0000) | ((vpos & 0x000F) << 12) | ((hpos & 0x000F) << 8) | ((vsize & 0x000F) << 4) | ((hsize & 0x000F));

		/*
		*hsize = ((*pbPropertyValue) & 0x0F);
		*vsize = ((*pbPropertyValue) >> 4 & 0x0F);
		*hpos = ((*(pbPropertyValue+1)) & 0x0F);
		*vpos = ((*(pbPropertyValue+1) >> 4) & 0x0F);
		*/

		*hpos = ((*pbPropertyValue) & 0x0F);
		*vpos = ((*pbPropertyValue) >> 4 & 0x0F);
		*hsize = ((*(pbPropertyValue + 1)) & 0x0F);
		*vsize = ((*(pbPropertyValue + 1) >> 4) & 0x0F);


#ifdef DEBUG
		sprintf(logMessage, " \nFunction : getBLCRangeValue \t Msg : property[%d] value[%d]", PropertyId, retValue);
		printLogMessage(logMessage);
#endif

	}

	return hr;
}
#endif
HRESULT getExposureAGCLvlValue(int PropertyId, int *ExpValue, int *AgcLvlValue)
{
	HRESULT hr = S_OK;
	ULONG ulSize;
	BYTE *pbPropertyValue;
	int retValue = 0;

	*ExpValue = 0;
	*AgcLvlValue = 0;

	hr = getExtionControlPropertySize(PropertyId, &ulSize);
	if (FAILED(hr) || (ulSize != 4))
	{
#ifdef DEBUG
		sprintf(logMessage, " \nERROR \t Function : getExposureAGCLvlValue \t Msg : Unable to find property[%d] size : %x", PropertyId, hr);
		printLogMessage(logMessage);
#endif
	}
	else
	{
		pbPropertyValue = new BYTE[ulSize];
		if (!pbPropertyValue)
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : getExposureAGCLvlValue \t Msg : Unable to allocate memory for property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}

		hr = getExtionControlProperty(PropertyId, ulSize, pbPropertyValue);

		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, " \nERROR \t Function : getExposureAGCLvlValue \t Msg : Unable to get property[%d] value", PropertyId);
			printLogMessage(logMessage);
#endif
		}

		memcpy(ExpValue, (char *)&pbPropertyValue[0], 2);
		memcpy(AgcLvlValue, (char *)&pbPropertyValue[2], 2);

#ifdef DEBUG
		sprintf(logMessage, " \nFunction : getExposureAGCLvlValue \t Msg : property[%d] value[%d]", PropertyId, retValue);
		printLogMessage(logMessage);
#endif

	}

	return hr;
}

HRESULT setExposureAGCLvlValue(HWND hwnd, int PropertyId, int ExpValue, int AgcLvlValue)
{
	HRESULT hr = S_OK;

	try
	{
		if (ExpValue == 0)
		{
			HWND hListSldAGCLVL = GetDlgItem(hwnd, IDC_SLD_AGC_LVL);
			EnableWindow(hListSldAGCLVL, FALSE);
		}
		else
		{
			HWND hListSldAGCLVL = GetDlgItem(hwnd, IDC_SLD_AGC_LVL);
			EnableWindow(hListSldAGCLVL, TRUE);
		}

		if (ExpValue != LB_ERR && ksNodeTree.isOK)
		{

			ULONG ulSize;
			BYTE *pbPropertyValue;

			hr = getExtionControlPropertySize(PropertyId, &ulSize);
			if (FAILED(hr) || (ulSize != 4)) // first two byte Exposure Mode & last two byte AGC level mast be 4 byte
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : setExposureAGCLvlValue \t Msg : Unable to find property size : %x", hr);
				printLogMessage(logMessage);
#endif
			}
			else
			{
				pbPropertyValue = new BYTE[ulSize];
				if (!pbPropertyValue)
				{
#ifdef DEBUG
					sprintf(logMessage, "\nERROR \t Function : setExposureAGCLvlValue \t Msg : Unable to allocate memory for property value");
					printLogMessage(logMessage);
#endif
				}
				else
				{
					int ExposureByte = 2;
					int AgcLvlByte = 2;
					memcpy(&pbPropertyValue[0], (char*)&ExpValue, ExposureByte); // first two byte Exposure Mode & last two byte AGC level
					memcpy(&pbPropertyValue[ExposureByte], (char*)&AgcLvlValue, AgcLvlByte); // first two byte Exposure Mode & last two byte AGC level

					hr = setExtionControlProperty(PropertyId, ulSize, pbPropertyValue);
				}
			}

#ifdef DEBUG
			sprintf(logMessage, "\nFunction : setExposureAGCLvlValue \t Msg : Return Value:%ld", hr);
			printLogMessage(logMessage);
#endif
		}

	}
	catch (...)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nERROR : In Function : setExposureAGCLvlValue");
		printLogMessage(logMessage);
#endif
	}

	return hr;

}
#if 0
HRESULT setExtControls(int PropertyId, int PropertyValue)
{
	HRESULT hr = S_OK;
	ULONG ulSize;
	BYTE *pbPropertyValue;

	hr = getExtionControlPropertySize(PropertyId, &ulSize);
	if (SUCCEEDED(hr))
	{

		pbPropertyValue = new BYTE[ulSize];
		if (!pbPropertyValue)
		{
#ifdef DEBUG
			sprintf(logMessage, "\nError \t Function : resetCameraControlInitSetting \t Msg : Unable to allocate memory for property value");
			printLogMessage(logMessage);
#endif
		}
		else
		{
			memcpy(pbPropertyValue, (char*)&PropertyValue, ulSize);
			hr = setExtionControlProperty(PropertyId, ulSize, pbPropertyValue);
		}
	}
	return hr;
}
#endif
HRESULT OnShutCtrlChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	HWND hListComboShutCtrl = GetDlgItem(hwnd, IDC_COMBO_SHUT_CONTL);

	int sel = ComboBox_GetCurSel(hListComboShutCtrl);

	if (sel != LB_ERR && ksNodeTree.isOK)
	{

		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 1;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, "\nFunction : OnShutCtrlChange \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nFunction : OnShutCtrlChange \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				memcpy(pbPropertyValue, (char*)&sel, ulSize);
				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
			}
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : OnShutCtrlChange \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT OnSenUpModeChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	HWND hListSenseUpMode = GetDlgItem(hwnd, IDC_COMBO_SEN_UP_MODE);

	int sel = ComboBox_GetCurSel(hListSenseUpMode);

	if (sel != LB_ERR && ksNodeTree.isOK)
	{
		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 2;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : OnSenUpModeChange \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : OnSenUpModeChange \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				memcpy(pbPropertyValue, (char*)&sel, ulSize);
				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
			}
		}

#ifdef DEBUG
		sprintf(logMessage, "\nERROR \t Function : OnSenUpModeChange \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT onAEReferenceLevelChange(HWND hwnd)
{
	HRESULT hr = S_OK;

	try
	{
		long arSldPos = 0;
		CString strPos;
		HWND hListSldAELVL = GetDlgItem(hwnd, IDC_SLD_AE_REF_LVL);
		arSldPos = (long)SendMessageA(hListSldAELVL, TBM_GETPOS, TRUE, arSldPos);

		HWND hListAELVL = GetDlgItem(hwnd, IDC_EDIT_AE_REF_LVL);
		strPos.Format(L"%ld", arSldPos);
		Edit_SetText(hListAELVL, strPos);

		if (ksNodeTree.isOK)
		{
			ULONG ulSize;
			BYTE *pbPropertyValue;
			int PropertId = 11;

			hr = getExtionControlPropertySize(PropertId, &ulSize);
			if (FAILED(hr))
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : onAEReferenceLevelChange \t Msg : Unable to find property size : %x", hr);
				printLogMessage(logMessage);
#endif
			}
			else
			{
				pbPropertyValue = new BYTE[ulSize];
				if (!pbPropertyValue)
				{
#ifdef DEBUG
					sprintf(logMessage, "\nERROR \t Function : onAEReferenceLevelChange \t Msg : Unable to allocate memory for property value");
					printLogMessage(logMessage);
#endif
				}
				else
				{
					memcpy(pbPropertyValue, (char*)&arSldPos, ulSize);
					hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
				}
			}

#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : onAEReferenceLevelChange \t Msg : Return Value:%ld", hr);
			printLogMessage(logMessage);
#endif
		}

	}
	catch (...)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nERROR : In Function : onAEReferenceLevelChange");
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT OnExposureModeChange(HWND hwnd)
{
	HRESULT hr = S_OK;

	try
	{
		HWND hListExpoMode = GetDlgItem(hwnd, IDC_COMBO_EXPOSURE_MODE);

		int sel = ComboBox_GetCurSel(hListExpoMode);
		if (sel == 0)
		{
			HWND hListSldAGCLVL = GetDlgItem(hwnd, IDC_SLD_AGC_LVL);
			EnableWindow(hListSldAGCLVL, FALSE);
			//HWND hListAGCLVL = GetDlgItem(hwnd, IDC_EDIT_AGC_LVL);
			//EnableWindow(hListAGCLVL, FALSE);
		}
		else
		{
			HWND hListSldAGCLVL = GetDlgItem(hwnd, IDC_SLD_AGC_LVL);
			EnableWindow(hListSldAGCLVL, TRUE);
			//HWND hListAGCLVL = GetDlgItem(hwnd, IDC_EDIT_AGC_LVL);
			//EnableWindow(hListAGCLVL, TRUE);
		}

		if (sel != LB_ERR && ksNodeTree.isOK)
		{

			ULONG ulSize;
			BYTE *pbPropertyValue;
			int PropertId = 10;

			hr = getExtionControlPropertySize(PropertId, &ulSize);
			if (FAILED(hr) || (ulSize != 4)) // first two byte Exposure Mode & last two byte AGC level mast be 4 byte
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : OnExposureModeChange \t Msg : Unable to find property size : %x", hr);
				printLogMessage(logMessage);
#endif
			}
			else
			{
				pbPropertyValue = new BYTE[ulSize];
				if (!pbPropertyValue)
				{
#ifdef DEBUG
					sprintf(logMessage, "\nERROR \t Function : OnExposureModeChange \t Msg : Unable to allocate memory for property value");
					printLogMessage(logMessage);
#endif
				}
				else
				{
					long agcSldPos = 0;
					CString strPos;
					HWND hListSldAGCLVL = GetDlgItem(hwnd, IDC_SLD_AGC_LVL);
					agcSldPos = (long)SendMessageA(hListSldAGCLVL, TBM_GETPOS, TRUE, agcSldPos);

					int ExposureByte = 2;
					int AgcLvlByte = 2;
					memcpy(&pbPropertyValue[0], (char*)&sel, ExposureByte); // first two byte Exposure Mode & last two byte AGC level
					memcpy(&pbPropertyValue[ExposureByte], (char*)&agcSldPos, AgcLvlByte); // first two byte Exposure Mode & last two byte AGC level

					hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
				}
			}

#ifdef DEBUG
			sprintf(logMessage, "\nFunction : OnExposureModeChange \t Msg : Return Value:%ld", hr);
			printLogMessage(logMessage);
#endif
		}

	}
	catch (...)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nERROR : In Function : OnExposureModeChange");
		printLogMessage(logMessage);
#endif
	}

	return hr;

}

HRESULT onAgcLvlChange(HWND hwnd)
{

	HRESULT hr = S_OK;
	long agcSldPos = 0;
	CString strPos;
	HWND hListSldAGCLVL = GetDlgItem(hwnd, IDC_SLD_AGC_LVL);
	agcSldPos = (long)SendMessageA(hListSldAGCLVL, TBM_GETPOS, TRUE, agcSldPos);

	HWND hListExpoMode = GetDlgItem(hwnd, IDC_COMBO_EXPOSURE_MODE);
	int sel = ComboBox_GetCurSel(hListExpoMode);

	HWND hListAGCLVL = GetDlgItem(hwnd, IDC_EDIT_AGC_LVL);
	strPos.Format(L"%ld", agcSldPos);
	Edit_SetText(hListAGCLVL, strPos);

	if (ksNodeTree.isOK && (sel > 0))
	{
		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 10;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		if (FAILED(hr) || (ulSize != 4)) // first two byte Exposure Mode & last two byte AGC level mast be 4 byte
		{
#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : onAgcLvlChange \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : onAgcLvlChange \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				int ExposureByte = 2;
				int AgcLvlByte = 2;
				memcpy(&pbPropertyValue[0], (char*)&sel, ExposureByte); // first two byte Exposure Mode & last two byte AGC level
				memcpy(&pbPropertyValue[ExposureByte], (char*)&agcSldPos, AgcLvlByte); // first two byte Exposure Mode & last two byte AGC level

				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
			}
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : onAgcLvlChange \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}


HRESULT onBLCRangeChange(HWND hwnd)
{

	HRESULT hr = S_OK;
	unsigned short hpos = 0, hsize = 0, vpos = 0, vsize = 0;
	CString strPos;

	HWND hListSldHPos = GetDlgItem(hwnd, IDC_SLD_HPos);
	HWND hListSldHSize = GetDlgItem(hwnd, IDC_SLD_HSize);
	HWND hListSldVPos = GetDlgItem(hwnd, IDC_SLD_VPos);
	HWND hListSldVSize = GetDlgItem(hwnd, IDC_SLD_VSize);

	hpos = (unsigned short)SendMessageA(hListSldHPos, TBM_GETPOS, TRUE, hpos);
	hsize = (unsigned short)SendMessageA(hListSldHSize, TBM_GETPOS, TRUE, hsize);
	vpos = (unsigned short)SendMessageA(hListSldVPos, TBM_GETPOS, TRUE, vpos);
	vsize = (unsigned short)SendMessageA(hListSldVSize, TBM_GETPOS, TRUE, vsize);

	HWND hListEditHPos = GetDlgItem(hwnd, IDC_EDIT_HPos);
	strPos.Format(L"%ld", hpos);
	Edit_SetText(hListEditHPos, strPos);

	HWND hListEditHSize = GetDlgItem(hwnd, IDC_EDIT_HSize);
	strPos.Format(L"%ld", hsize);
	Edit_SetText(hListEditHSize, strPos);

	HWND hListEditVPos = GetDlgItem(hwnd, IDC_EDIT_VPos);
	strPos.Format(L"%ld", vpos);
	Edit_SetText(hListEditVPos, strPos);

	HWND hListEditVSize = GetDlgItem(hwnd, IDC_EDIT_VSize);
	strPos.Format(L"%ld", vsize);
	Edit_SetText(hListEditVSize, strPos);

	if (ksNodeTree.isOK)
	{
		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 17;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		//ulSize = 2;
		if (FAILED(hr) || (ulSize != 2))
		{
#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : onBLCRangeChange \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : onBLCRangeChange \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				/*
				*vpos = ((*pbPropertyValue) & 0x0F);
				*vsize = ((*pbPropertyValue) >> 4 & 0x0F);
				*hpos = ((*(pbPropertyValue + 1)) & 0x0F);
				*hsize = ((*(pbPropertyValue + 1) >> 4) & 0x0F);
				*/
				/*
				*pbPropertyValue = (*pbPropertyValue & 0x00) | ((vsize & 0x000F) << 4) | (hsize & 0x000F);
				*(pbPropertyValue+1) = (*(pbPropertyValue+1) & 0x00) | ((vpos & 0x000F) << 4) | (hpos & 0x000F);
				*/
				*pbPropertyValue = (*pbPropertyValue & 0x00) | ((vpos & 0x000F) << 4) | (hpos & 0x000F);
				*(pbPropertyValue + 1) = (*(pbPropertyValue + 1) & 0x00) | ((vsize & 0x000F) << 4) | (hsize & 0x000F);
				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
			}
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : onBLCRangeChange \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}


HRESULT OnCameraModeChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	HWND hListComboCameraMode = GetDlgItem(hwnd, IDC_COMBO_CAM_MODE);

	int sel = ComboBox_GetCurSel(hListComboCameraMode);

	if (sel != LB_ERR && ksNodeTree.isOK)
	{

		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 13;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : OnCameraModeChange \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : OnCameraModeChange \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				memcpy(pbPropertyValue, (char*)&sel, ulSize);
				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);

				if (0 && SUCCEEDED(hr))
				{
					long currValue, lCaps;
					getStandardControlPropertyCurrentValue(KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION, &currValue, &lCaps);
					HWND hListBackLight = GetDlgItem(hwnd, IDC_COMBO_BACK_LIGHT);
					ComboBox_SetCurSel(hListBackLight, currValue);

				}
			}
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : OnCameraModeChange \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}


HRESULT OnBLCGridChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	HWND hListComboBLCGrid = GetDlgItem(hwnd, IDC_COMBO_BLC_GRID);

	int sel = ComboBox_GetCurSel(hListComboBLCGrid);

	if (sel != LB_ERR && ksNodeTree.isOK)
	{

		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 19;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : OnBLCGridChange \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : OnBLCGridChange \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				memcpy(pbPropertyValue, (char*)&sel, ulSize);
				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
			}
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : OnBLCGridChange \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}


HRESULT OnBLCWeightFactorChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	HWND hListComboBLCWF = GetDlgItem(hwnd, IDC_COMBO_BLC_WGHT_FACT);

	int sel = ComboBox_GetCurSel(hListComboBLCWF);

	if (sel != LB_ERR && ksNodeTree.isOK)
	{

		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 18;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : OnBLCWeightFactorChange \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : OnBLCWeightFactorChange \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				memcpy(pbPropertyValue, (char*)&sel, ulSize);
				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
			}
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : OnBLCWeightFactorChange \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT OnSelectedIndexChangeMirror(HWND hwnd)
{
	HRESULT hr = S_OK;
	HWND hListComboMirror = GetDlgItem(hwnd, IDC_COMBO_MIRROR_MODE);

	int sel = ComboBox_GetCurSel(hListComboMirror);

	if (sel != LB_ERR && ksNodeTree.isOK)
	{

		ULONG ulSize;
		BYTE *pbPropertyValue;
		int PropertId = 3;

		hr = getExtionControlPropertySize(PropertId, &ulSize);
		if (FAILED(hr))
		{
#ifdef DEBUG
			sprintf(logMessage, "\nERROR \t Function : OnSelectedIndexChangeMirror \t Msg : Unable to find property size : %x", hr);
			printLogMessage(logMessage);
#endif
		}
		else
		{
			pbPropertyValue = new BYTE[ulSize];
			if (!pbPropertyValue)
			{
#ifdef DEBUG
				sprintf(logMessage, "\nERROR \t Function : OnSelectedIndexChangeMirror \t Msg : Unable to allocate memory for property value");
				printLogMessage(logMessage);
#endif
			}
			else
			{
				memcpy(pbPropertyValue, (char*)&sel, ulSize);
				hr = setExtionControlProperty(PropertId, ulSize, pbPropertyValue);
			}
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : OnSelectedIndexChangeMirror \t Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}
#if 0
int getMainsFrequency()
{
	long retValue;
	long lCap;
	HRESULT hr = S_OK;

	if (ksNodeTree.isOKpProcAmp)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nbmRequestType:GET \t bRequest:GET_CUR \t wValue:Mains frequency \t wIndex:0x02");
		printLogMessage(logMessage);
#endif
		hr = ksNodeTree.pProcAmp->Get(KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY, &retValue, &lCap);
#ifdef DEBUG
		sprintf(logMessage, "\nFunction : OnMainsFrequencyChange Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	if (!SUCCEEDED(hr))
		retValue = 0;

	return (int)retValue;

}
#endif
HRESULT OnMainsFrequencyChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	HWND hListMainFeq = GetDlgItem(hwnd, IDC_COMBO_MAIN_FEQ);

	int sel = ComboBox_GetCurSel(hListMainFeq);
	if (sel != LB_ERR && ksNodeTree.isOKpProcAmp)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nbmRequestType:SET \t bRequest:SET_CUR \t wValue:Mains frequency \t wIndex:0x02\t PutValue:%d", sel);
		printLogMessage(logMessage);
#endif
		hr = ksNodeTree.pProcAmp->Set(KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY, (long)sel, VideoProcAmp_Flags_Manual);
#ifdef DEBUG
		sprintf(logMessage, "\nFunction : OnMainsFrequencyChange Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

HRESULT	onBacklightCompensationChange(HWND hwnd)
{
	HRESULT hr = S_OK;
	HWND hListBackLight = GetDlgItem(hwnd, IDC_COMBO_BACK_LIGHT);
	int setV = 0;

	int sel = ComboBox_GetCurSel(hListBackLight);

	/* check the res */
	AM_MEDIA_TYPE *pmt;
	gcap.pVSC->GetFormat(&pmt);

	if (sel != LB_ERR && ksNodeTree.isOKpProcAmp)
	{
#ifdef DEBUG
		sprintf(logMessage, "\nbmRequestType:SET \t bRequest:SET_CUR \t wValue:BacklightCompensation \t wIndex:0x02\t PutValue:%d", sel);
		printLogMessage(logMessage);
#endif
		if ((pmt->lSampleSize == 4147200) || (pmt->lSampleSize == 1036800)) //1080p
		{
			switch (sel)
			{
			case 0:
				setV = 3;
				break;
			case 1:
				setV = 0;
				break;
			case 2:
				setV = 2;
				break;
			case 3:
				setV = 1;
				break;
			}
		}
		else{ //720p
			switch (sel)
			{
			case 0:
				setV = 0;
				break;
			case 1:
				setV = 1;
				break;
			}

		}

		hr = ksNodeTree.pProcAmp->Set(KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION, (long)setV, VideoProcAmp_Flags_Manual);

		if (SUCCEEDED(hr))
		{
			HWND hListCameraMode = GetDlgItem(hwnd, IDC_COMBO_CAM_MODE);
			int retValue = 0;
			getExtControlValue(13, &retValue);
			ComboBox_SetCurSel(hListCameraMode, retValue);
		}

#ifdef DEBUG
		sprintf(logMessage, "\nFunction : onBacklightCompensationChange Msg : Return Value:%ld", hr);
		printLogMessage(logMessage);
#endif
	}

	return hr;
}

/* convert the capture image data to the 8-bit grayscale image data */
BYTE* cvtImage2gray(BYTE* pData)
{
	//BITMAPINFO BmpInfo = *pBitmapInfo;
	long	lRawImageSize;
	long    ImageWidth, ImageHeight;
	const long X1 = 550, Y1 = 850, Width = 640, Height = 480;

	ImageWidth = gcap.stillWidth;
	ImageHeight = gcap.stillHeight;

	/* Get the size of the image */
	lRawImageSize = ImageWidth * ImageHeight;

	/* Copy the raw image on the memory */
#if 0
	for (long lIndexH = 0; lIndexH < ImageHeight; lIndexH++)
	{
		for (long lIndexW = 0; lIndexW < ImageWidth; lIndexW++)
		{
			pData[lIndexH * ImageWidth + lIndexW]
				= pData[(lIndexH * ImageWidth + lIndexW)*3];
		}
	}
#endif
	for (long lIndexH = Y1; lIndexH < Y1 + Height; lIndexH++)
	{
		for (long lIndexW = X1; lIndexW < X1 + Width; lIndexW++)
		{
			pData[(lIndexH - Y1) * Width + (lIndexW - X1)]
				= pData[(lIndexH * ImageWidth + lIndexW) * 3];
		}
	}

	return 0;// pRawImage;
}

