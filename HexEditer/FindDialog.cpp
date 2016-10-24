#include "StdAfx.h"
#include "FindDialog.h"
#include "resource.h"
#include "HexEditer.h"

extern HexEditer hexEditer;

int CHECK_CH(char a) {
	int x = -1;
	if (a >= '0' && a <= '9') x = a - '0';
	else if (a >= 'A' && a <= 'F') x = a - 'A' + 10;
	else if (a >= 'a' && a <= 'f') x = a - 'a' + 10;
	return x;
}

int ParseInputBytes(char *buf) {
	char *p = buf, *pp = buf;
	while (*pp) {
		if (CHECK_CH(pp[0]) < 0 || CHECK_CH(pp[1]) < 0) {
			return 0;
		}
		*p = (CHECK_CH(pp[0]) << 4 | CHECK_CH(pp[1]));
		++p;
		pp += 2;
		if (*pp == ' ') {
			while (*pp == ' ')
				++pp; 
		} else if (*pp != 0) {
			return 0;
		}
	}
	return p - buf;
}

static int findIdx = -1;
static char lastBytes[120];
INT_PTR CALLBACK FDWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			char buf[200] = {0};
			HWND edit = GetDlgItem(hDlg, IDC_FDLG_BYTES_EDIT);
			GetWindowText(edit, buf, sizeof buf);
			int len = ParseInputBytes(buf);
			if (! len) {
				MessageBox(hDlg, "Input text format invalid", "Error", MB_OK|MB_ICONERROR);
				return TRUE;
			}
			if (memcmp(lastBytes, buf, len) == 0) {
				findIdx = hexEditer.find(findIdx == -1 ? 0 : findIdx + 1, buf, len);
			} else {
				findIdx = hexEditer.find(0, buf, len);
				memcpy(lastBytes, buf, len);
			}
			
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

FindDialog::FindDialog(void)
{
}

void FindDialog::createWnd(HWND parent, int x, int y, int w, int h) {
	HINSTANCE ins = (HINSTANCE)GetWindowLong(parent, GWL_HINSTANCE);
	mWnd = CreateDialog(ins, MAKEINTRESOURCE(IDD_DIALOG_FIND), parent, FDWndProc);
	RECT r;
	w = GetClientRect(mWnd, &r);
	MoveWindow(mWnd, x, y, r.right, r.bottom, FALSE);
	ShowWindow(mWnd, SW_SHOW);
}

FindDialog::~FindDialog(void)
{
}
