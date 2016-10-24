// HexEditer.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "Main.h"
#include "HexEditer.h"
#include "FindDialog.h"
#include "ReplaceDialog.h"
#include <Commdlg.h>
#include <CommCtrl.h>
#pragma comment(lib,"comctl32.lib")

// 开启视觉效果 Copy from MSDN  
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")  

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	RGWndProc(HWND, UINT, WPARAM, LPARAM);

HexEditer hexEditer;
FindDialog findDialog;
ReplaceDialog replaceDialog;
HWND rangeWnd;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_HEXEDITER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HEXEDITER));

	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HEXEDITER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_HEXEDITER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, SW_SHOWMAXIMIZED);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;
	static char lastPath[260] = {"."};

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_OPEN: {
			char fname[260] = {0};
			OPENFILENAME ofn = {0};
			ofn.lStructSize = sizeof ofn;
			ofn.hwndOwner = hWnd;
			ofn.lpstrFile = fname;
			ofn.nMaxFile = sizeof fname;
			ofn.lpstrFilter = "*.*\0";
			ofn.lpstrInitialDir = lastPath;
			ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
			if (GetOpenFileName(&ofn)) {
				hexEditer.loadFile(fname);
				SetWindowText(hWnd, fname);
				char *lp = strrchr(fname, '\\');
				*lp = 0;
				strcpy(lastPath, fname);
			}
			break;}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_CREATE: {
		hexEditer.createWnd(hWnd, 100, 30, 800, 100);
		hexEditer.loadFile("D:\\CPP\\IES\\IES\\IES.h");
		findDialog.createWnd(hWnd, 850, 30, 0, 0);
		replaceDialog.createWnd(hWnd, 850, 210, 0, 0);
		rangeWnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_RANGE), hWnd, RGWndProc);
		RECT r;
		GetClientRect(rangeWnd, &r);
		MoveWindow(rangeWnd, 850, 380, r.right, r.bottom, FALSE);
		ShowWindow(rangeWnd, SW_SHOW);
		break;}
	case WM_SIZE:
		GetClientRect(hWnd, &rect);
		MoveWindow(hexEditer.mWnd, 20, 10, 800, rect.bottom - 30, TRUE);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void TrimRight(char *t) {
	if (strlen(t) == 0) return;
	t += strlen(t) - 1;
	while (*t == ' ') {*t = 0; --t;}
}

extern int CHECK_CH(char a);

int GetAddr(char *t) {
	char dd[20] = {0};
	char *p = dd;
	--p;
	for (int i = strlen(t) - 1, j = 0; i >= 0; --i, ++j) {
		int v = CHECK_CH(t[i]);
		if (v < 0) return -1;
		if ((j % 2) == 0) {++p; *p = v;}
		else {*p = *p | (v << 4);}
	}
	int *x = (int*)dd;
	return *x;
}

INT_PTR CALLBACK RGWndProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (WM_INITDIALOG == msg) return TRUE;
	if (WM_COMMAND == msg && LOWORD(wParam) == ID_OK) {
		char bgTxt[30] = {0};
		char edTxt[30] = {0};
		GetDlgItemText(hDlg, IDC_RD_BEGIN_EDIT, bgTxt, sizeof bgTxt);
		GetDlgItemText(hDlg, IDC_RD_END_EDIT, edTxt, sizeof edTxt);
		TrimRight(bgTxt);
		TrimRight(edTxt);
		int bgAdr = 0, edAdr = 0;
		if (bgTxt[0] != 0) {
			bgAdr = GetAddr(bgTxt);
			if (bgAdr < 0) {
				MessageBox(hDlg, "Begin address invalde format", "Error", MB_OK | MB_ICONERROR);
				return TRUE;
			}
		}
		if (edTxt[0] != 0) {
			edAdr = GetAddr(edTxt);
			if (bgAdr < 0) {
				MessageBox(hDlg, "End address invalde format", "Error", MB_OK | MB_ICONERROR);
				return TRUE;
			}
		} else {
			edAdr = hexEditer.mDataLen;
		}
		hexEditer.setViewRange(bgAdr, edAdr);
	}
	return (INT_PTR)FALSE;
}