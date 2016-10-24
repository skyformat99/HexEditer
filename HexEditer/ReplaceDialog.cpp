#include "StdAfx.h"
#include "ReplaceDialog.h"
#include "resource.h"
#include "HexEditer.h"

extern HexEditer hexEditer;

extern int ParseInputBytes(char *buf);

static void DoReplace(HWND dlg) {
	char txt[100] = {0};
	GetDlgItemText(dlg, IDC_RDLG_BYTES_EDIT, txt, sizeof txt);
	int len = ParseInputBytes(txt);
	if (! len) {
		MessageBox(dlg, "Input text format invalid", "Error", MB_OK|MB_ICONERROR);
		return;
	}
	int num = hexEditer.mBeginSel - hexEditer.mEndSel;
	if (num < 0) num = - num;
	++num;
	if (len != num) {
		MessageBox(dlg, "Input text lenth != select", "Error", MB_OK|MB_ICONERROR);
		return;
	}
	int pos = hexEditer.mBeginSel < hexEditer.mEndSel ? hexEditer.mBeginSel : hexEditer.mEndSel;
	hexEditer.replace(pos, txt, len);
	RECT r = hexEditer.changeSelRange(hexEditer.mBeginSel, hexEditer.mEndSel);
	InvalidateRect(hexEditer.mWnd, &r, TRUE);
}

INT_PTR CALLBACK RDWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			DoReplace(hDlg);
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

ReplaceDialog::ReplaceDialog(void)
{
}

void ReplaceDialog::createWnd(HWND parent, int x, int y, int w, int h) {
	HINSTANCE ins = (HINSTANCE)GetWindowLong(parent, GWL_HINSTANCE);
	mWnd = CreateDialog(ins, MAKEINTRESOURCE(IDD_DIALOG_REPLACE), parent, RDWndProc);
	RECT r;
	w = GetClientRect(mWnd, &r);
	MoveWindow(mWnd, x, y, r.right, r.bottom, FALSE);
	ShowWindow(mWnd, SW_SHOW);
}

ReplaceDialog::~ReplaceDialog(void)
{
}
