#include "StdAfx.h"
#include "HexEditer.h"
#include <string.h>
#include <stdio.h>

static char *CLASS_NAME = "XHexEditer";
static LRESULT CALLBACK HexEditerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static HexEditer *self;

HANDLE os;
char LB[200];

HexEditer::HexEditer(void) : mWnd(0), mTranslateY(0), mWidth(0), mHeight(0),
	mCaretVisible(0),mCaretCreated(0), mDataLen(0), mActionNum(0), mData(0),
	mBeginSel(0), mEndSel(0), mPressedCnt(FALSE), mBeginViewAddr(0), mEndViewAddr(0)
{
	LOGFONT lf = {0};
	strcpy(lf.lfFaceName, "Arial");
	lf.lfWeight = FW_BLACK;
	lf.lfHeight = -16; // 字体大小
	lf.lfCharSet = 134; 
	lf.lfOutPrecision = 3; 
	lf.lfClipPrecision = 2; 
	lf.lfOrientation = 45; 
	lf.lfQuality = 1; 
	lf.lfPitchAndFamily = 2; 
	// 创建字体 
	mFont = CreateFontIndirect(&lf);
	mSelBrush = CreateSolidBrush(RGB(0xCD, 0x69, 0xC9));
	self = this;
	memset(mFilePath, 0, sizeof mFilePath);
	//AllocConsole();
	//os = GetStdHandle(STD_OUTPUT_HANDLE);
}

void HexEditerRegistClass() {
	static BOOL inited = FALSE;
	if (inited) return;
	inited = TRUE;
	WNDCLASSEX wcex = {0};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS ;
	wcex.lpfnWndProc	= HexEditerWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		=  0 ; //GetWindowLong( GWL_HINSTANCE);
	//wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RICHNOTE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	=  ::CreateSolidBrush(RGB(0xf0, 0xf0, 0xf0));  //(HBRUSH)(COLOR_WINDOW+1)
	//wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_RICHNOTE);
	wcex.lpszClassName	= CLASS_NAME;
	//wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassEx(&wcex);
}

void HexEditer::createWnd(HWND parent, int x, int y, int w, int h) {
	HINSTANCE ins = (HINSTANCE)GetWindowLong(parent, GWL_HINSTANCE);
	HexEditerRegistClass();
	mWnd = CreateWindow(CLASS_NAME, "", WS_CHILDWINDOW|WS_VISIBLE | WS_VSCROLL,
      x, y, w, h, parent, NULL, ins, NULL);
	// SetProp(mWnd, "_SELF_", (HANDLE)this);
}

static const char *HEX = "0123456789ABCDEF";
void HexEditer::drawTitleBar(HDC hdc) {
	SetTextColor(hdc, RGB(0, 0, 250));
	int x = OFFSET_WIDTH + 5;
	for (int i = 0; i < strlen(HEX); ++i) {
		if (i == 8) x += CELL_8_INNER;
		TextOut(hdc, x + i * CELL_WIDTH, 5, HEX + i, 1);
	}
	TextOut(hdc, 15, 5, "Offset", 6);
	MoveToEx(hdc, 0, TITLE_BAR_HEIGHT - 1, NULL);
	LineTo(hdc, mWidth, TITLE_BAR_HEIGHT - 1);
}

void HexEditer::drawOffsetArea(HDC hdc) {
	int bg = mTranslateY * 16 + mBeginViewAddr;
	int lines = getPageRows();
	char linfo[10];
	SetTextColor(hdc, RGB(0, 0, 150));
	int y = TITLE_BAR_HEIGHT;
	for (int i = 0; i < lines; ++i, bg += 16) {
		sprintf(linfo, "%08X", bg);
		TextOut(hdc, 10, 5 + y, linfo, 8);
		y += CELL_HEIGHT;
	}
	MoveToEx(hdc, OFFSET_WIDTH - 1,0 , 0);
	LineTo(hdc, OFFSET_WIDTH - 1, mHeight);
}

#define MMIN(a, b) ((a) < (b) ? (a) : (b))
#define MMAX(a, b) ((a) > (b) ? (a) : (b))

static BOOL BetweenIn(int v, int from, int to) {
	int va = MMIN(from, to);
	int vb = MMAX(from, to);
	return v >= va && v <= vb;
}

void HexEditer::drawContent(HDC hdc) {
	int bg = mTranslateY * 16 + mBeginViewAddr;
	int end = mEndViewAddr;
	int lines = getPageRows();
	char linfo[4];
	int y = TITLE_BAR_HEIGHT + 3;
	SetTextColor(hdc, RGB(40, 40, 40));
	for (int i = 0; i < lines && bg < end; ++i, bg += 16) {
		int x = OFFSET_WIDTH + 5;
		for (int j = 0; j < 16 && bg + j < end; ++j) {
			if (j == 8) x += CELL_8_INNER;
			sprintf(linfo, "%02X", (unsigned char)mData[bg + j]);
			if (BetweenIn(bg + j, mBeginSel, mEndSel)) {
				RECT r;
				getByteRect(bg + j, &r);
				FillRect(hdc, &r, mSelBrush);
			}
			TextOut(hdc, x, y, linfo, 2);
			x += CELL_WIDTH;
		}
		y += CELL_HEIGHT;
	}
}

void HexEditer::drawAscii(HDC hdc) {
	int bg = mTranslateY * 16 + mBeginViewAddr;
	int end = mEndViewAddr;
	int lines = getPageRows();
	char linfo[4];
	int y = TITLE_BAR_HEIGHT;
	SetTextColor(hdc, RGB(40, 40, 40));
	for (int i = 0; i < lines && bg < end; ++i, bg += 16) {
		int x = OFFSET_WIDTH + TABLE_WIDTH + 5;
		for (int j = 0; j < 16 && bg + j < end; ++j) {
			char ch = mData[bg + j];
			if (BetweenIn(bg + j, mBeginSel, mEndSel)) {
				RECT r = {x, y, x + ASCII_WIDTH, y + CELL_HEIGHT};
				FillRect(hdc, &r, mSelBrush);
			}
			if (ch >= 32 && ch <= 126) {
				sprintf(linfo, "%c", ch);
				TextOut(hdc, x, y, linfo, 1);
			} else {
				TextOut(hdc, x, y, ".", 1);
			}
			x += ASCII_WIDTH;
		}
		y += CELL_HEIGHT;
	}

	int x = OFFSET_WIDTH + TABLE_WIDTH + 1;
	MoveToEx(hdc, x, 0 , 0);
	LineTo(hdc, x, mHeight);
}

int HexEditer::getPageRows() {
	int lines = (mHeight - TITLE_BAR_HEIGHT) / CELL_HEIGHT;
	return lines;
}

int HexEditer::getTotalRows() {
	return (mDataLen + 15) / 16;
}

int HexEditer::getTotalViewRows() {
	return (mEndViewAddr - mBeginViewAddr + 15) / 16;
}

void HexEditer::makeShow(int beginIdx, int endIdx) {
	int ed = MMAX(beginIdx, endIdx);
	ed -= mBeginViewAddr;
	int ln = ed / 16;
	int scroll = 0;
	if (ln >= mTranslateY && ln < mTranslateY + getPageRows()) {
		return; // it is visible
	}
	SCROLLINFO si = {0};
	si.cbSize = sizeof si;
	si.fMask = SIF_ALL;
	GetScrollInfo(mWnd, SB_VERT, &si);
	ln = MMAX(ln - 8, 0);
	ln = MMIN(ln, si.nMax);
	si.fMask = SIF_POS;
	si.nPos = ln;
	mTranslateY = ln;
	SetScrollInfo(mWnd, SB_VERT, &si, TRUE);
	InvalidateRect(mWnd, NULL, TRUE);
}

void HexEditer::onPaint(HDC hdc) {
	HGDIOBJ of = SelectObject(hdc, mFont);
	SetBkMode(hdc, TRANSPARENT);
	drawTitleBar(hdc);
	drawOffsetArea(hdc);
	drawContent(hdc);
	drawAscii(hdc);
	SelectObject(hdc, of);
}

void HexEditer::onLButtonDown(int keyState, int x, int y) {
	mPressedCnt = FALSE;
	if (x > OFFSET_WIDTH && x < OFFSET_WIDTH + TABLE_WIDTH && y > TITLE_BAR_HEIGHT) {
		pressContent(x, y);
	}
}

void HexEditer::onLButtonUp(int keyState, int x, int y) {
	mPressedCnt = FALSE;
}

void HexEditer::pressContent(int x, int y) {
	x -= OFFSET_WIDTH;
	y -= TITLE_BAR_HEIGHT;
	int idx = getByteIdxAt(x, y);
	if (idx < 0) return;
	RECT m = changeSelRange(idx, idx);
	mPressedCnt = TRUE;
	InvalidateRect(mWnd, &m, TRUE);
}

int HexEditer::find(int begin, char *bytes, int len) {
	int fd = -1;
	for (int i = begin + mBeginViewAddr; i < MMIN(mDataLen - len + 1, mEndViewAddr); ++i) {
		if (memcmp(bytes, mData + i, len) == 0) {
			fd = i;
			break;
		}
	}
	if (fd == -1) return -1;
	RECT r = changeSelRange(fd, fd + len - 1);
	InvalidateRect(mWnd, &r, TRUE);
	makeShow(mBeginSel, mEndSel);
	return fd;
}

void HexEditer::replace(int targetIdx, char *src, int len) {
	memcpy(mData + targetIdx, src, len);
}

void HexEditer::setViewRange(int beginAddr, int endAddr) {
	mBeginViewAddr = (MMAX(beginAddr, 0)) & ~15;
	mEndViewAddr = MMIN(endAddr, mDataLen);
	SendMessage(mWnd, WM_SIZE, 0, 0);
	InvalidateRect(mWnd, NULL, TRUE);
}

RECT HexEditer::changeSelRange(int begin, int end) {
	RECT r1, r2;
	getSelRect(&r1);
	mBeginSel = begin;
	mEndSel = end;
	getSelRect(&r2);
	r1.left = OFFSET_WIDTH;//MMIN(r1.left, r2.left);
	r1.top = MMIN(r1.top, r2.top);
	r1.right = mWidth;//MMAX(r1.right, r2.right);
	r1.bottom = MMAX(r1.bottom, r2.bottom);
	return r1;
}

int HexEditer::getByteIdxAt(int cntX, int cntY) {
	if (cntX > CELL_WIDTH * 16) return -1;
	else if (cntX > CELL_WIDTH * 9) cntX -= CELL_8_INNER;
	else if (cntX >= CELL_WIDTH * 9) return -1;
	int c = cntX / CELL_WIDTH;
	int r = cntY / CELL_HEIGHT;
	int idx = (mTranslateY + r) * 16 + c + mBeginViewAddr;
	if (idx > mDataLen) idx = -1;
	return idx;
}

void HexEditer::getByteRect(int idx, RECT *r) {
	r->left = r->bottom = r->right = r->top = 0;
	if (idx < 0 || idx > mDataLen) return;
	idx -= mTranslateY * 16 + mBeginViewAddr;
	int row = idx / 16;
	int c = idx % 16;
	if (row >= getPageRows()) return;
	r->left = c * CELL_WIDTH + OFFSET_WIDTH;
	if (c >= 8) r->left += CELL_8_INNER;
	r->top = row * CELL_HEIGHT + TITLE_BAR_HEIGHT;
	r->right = r->left + CELL_WIDTH;
	r->bottom = r->top + CELL_HEIGHT;
}

void HexEditer::getSelRect(RECT *r) {
	RECT r1, r2;
	getByteRect(mBeginSel, &r1);
	getByteRect(mEndSel, &r2);
	RECT m = {MMIN(r1.left, r2.left), MMIN(r1.top, r2.top),
				MMAX(r1.right, r2.right), MMAX(r1.bottom, r2.bottom)};
	*r = m;
}

void HexEditer::onMouseMove(int keyState, int x, int y) {
	if (mPressedCnt) {
		x -= OFFSET_WIDTH;
		y -= TITLE_BAR_HEIGHT;
		int idx = getByteIdxAt(x, y);
		if (idx < 0 || mEndSel == idx) return;
		RECT m = changeSelRange(mBeginSel, idx);
		InvalidateRect(mWnd, &m, TRUE);
	}
}

void HexEditer::showCaret(HDC hdc, int x, int y) {
	y -= mTranslateY;
	if (! mCaretCreated) {
		//SIZE sz;
		//MultiByteToWideChar(CP_ACP, 0, och, 2, &ch, 1);
		//GetTextExtentPoint32W(hdc, L"中", 1, &sz);
		CreateCaret(mWnd, NULL, 5, 15);
		mCaretCreated = TRUE;
	}
	SetCaretPos(x, y);
	if (! mCaretVisible) {
		ShowCaret(mWnd);
		mCaretVisible = TRUE;
	}
}
void HexEditer::hideCaret() {
	if (mCaretVisible) {
		HideCaret(mWnd);
		mCaretVisible = FALSE;
	}
}

void HexEditer::destroyCaret() {
	if (mCaretCreated) {
		::DestroyCaret();
		mCaretCreated = FALSE;
	}
	mCaretVisible = FALSE;
}

HDC HexEditer::getDC() {
	HDC hdc = GetDC(mWnd);
	SelectObject(hdc, mFont);
	return hdc;
}

void HexEditer::loadFile(const char *name) {
	FILE *f = fopen(name, "rb");
	int len = 0;
	fseek(f, 0, SEEK_END);
	len = (int)ftell(f);
	fseek(f, 0, SEEK_SET);

	mDataLen = 0;
	if (mData) free(mData);
	mData = 0;

	if (len <= 0) {
		return;
	}
	mDataLen = len;
	mData = (char*)malloc(mDataLen);
	char *p = mData;
	while (1) {
		len = fread(p, 1, 4096, f);
		if (len <= 0) break;
		p += len;
	}
	fclose(f);

	mBeginViewAddr = 0;
	mEndViewAddr = mDataLen;
	strcpy(mFilePath, name);
	SendMessage(mWnd, WM_SIZE, 0, 0);
	InvalidateRect(mWnd, NULL, TRUE);
}

void HexEditer::saveFile() {
	if (mFilePath[0] == 0) return;
	FILE *f = fopen(mFilePath, "rb+");
	// TODO:
	fclose(f);
}

HexEditer::~HexEditer(void)
{
	if (mCaretCreated) DestroyCaret();
	DeleteObject(mFont);
}


LRESULT CALLBACK HexEditerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;

	switch (message) {
	case WM_KILLFOCUS:
		self->destroyCaret();
		break;
	case WM_LBUTTONDOWN:
		SetFocus(hWnd);
		self->onLButtonDown(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONUP:
		self->onLButtonUp(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOUSEMOVE:
		self->onMouseMove(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOUSEWHEEL: {
		short delta = (short)HIWORD(wParam);
		SCROLLINFO si = {0};
		si.cbSize = sizeof si;
		si.fMask = SIF_ALL;
		GetScrollInfo(hWnd, SB_VERT, &si);
		int oldTY = self->mTranslateY;
		self->mTranslateY += (-delta) / 120 * 3;
		if (self->mTranslateY  < si.nMin) self->mTranslateY = si.nMin;
		if (self->mTranslateY > si.nMax - (int)si.nPage) self->mTranslateY = si.nMax - (int)si.nPage;
		if (self->mTranslateY < 0) self->mTranslateY = 0;
		if (oldTY != self->mTranslateY) {
			si.fMask = SIF_POS;
			si.nPos = self->mTranslateY;
			SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;}
	case WM_LBUTTONDBLCLK:
		//self->onDbClick(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_KEYDOWN:
		break;
	case WM_SIZE: {
		GetClientRect(hWnd, &rect);
		self->mWidth = rect.right;
		self->mHeight = rect.bottom;
		SCROLLINFO si = {0};
		si.cbSize = sizeof si;
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
		si.nPos = 0;
		si.nMin = 0;
		si.nMax = self->getTotalViewRows();
		si.nPage = self->getPageRows();
		SetScrollInfo(hWnd, SB_VERT, &si, FALSE);
		self->mTranslateY = 0;
		break;}
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		break;
	case WM_PAINT:
		::GetClientRect(hWnd, &rect);
		hdc = BeginPaint(hWnd, &ps);
		self->onPaint(hdc);
		EndPaint(hWnd, &ps);
		break;
	case WM_VSCROLL: {
		WORD sb = LOWORD(wParam);
		WORD pos = HIWORD(wParam);
		// if (sb == SB_LINEUP) SB_LINEDOWN  SB_THUMBPOSITION SB_THUMBTRACK 
		// if (sb != SB_THUMBPOSITION) break;

		SCROLLINFO si = {0};
		si.cbSize = sizeof si;
		si.fMask = SIF_ALL;
		GetScrollInfo(hWnd, SB_VERT, &si);
		self->mTranslateY = si.nTrackPos;
		si.fMask = SIF_POS;
		si.nPos = si.nTrackPos;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		::InvalidateRect(hWnd, NULL, TRUE);
		break;}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
