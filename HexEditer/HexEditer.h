#pragma once

#include <Windows.h>

class HexEditer
{
public:
	HexEditer(void);
	~HexEditer(void);

	void createWnd(HWND parent, int x, int y, int w, int h);
	void onPaint(HDC hdc);
	void onLButtonDown(int keyState, int x, int y);
	void onLButtonUp(int keyState, int x, int y);
	void onMouseMove(int keyState, int x, int y);
	void showCaret(HDC hdc, int x, int y);
	void hideCaret();
	void destroyCaret();
	HDC getDC();

	struct EditAction {
		char *mPos;
		int mLen;
		char mOldData[50];
	};

	void loadFile(const char *name);
	void saveFile();

	void drawTitleBar(HDC hdc);
	void drawOffsetArea(HDC hdc);
	void drawContent(HDC hdc);
	void drawAscii(HDC hdc);
	void pressContent(int x, int y);
	int getByteIdxAt(int cntX, int cntY);
	void getByteRect(int idx, RECT *r);
	void getSelRect(RECT *r);
	void replace(int targetIdx, char *src, int len);

	int getPageRows();
	int getTotalRows();
	int getTotalViewRows();
	void makeShow(int beginIdx, int endIdx);

	int find(int begin, char *bytes, int len);
	RECT changeSelRange(int begin, int end);
	void setViewRange(int beginAddr, int endAddr);
public:
	HWND mWnd;
	HFONT mFont;
	int mTranslateY;
	BOOL mCaretCreated;
	BOOL mCaretVisible;
	int mDataLen;
	EditAction mActions[500];
	int mActionNum;
	char *mData;
	int mWidth, mHeight;
	int mBeginSel, mEndSel;
	HBRUSH mSelBrush;
	BOOL mPressedCnt;
	int mBeginViewAddr, mEndViewAddr;
	char mFilePath[260];

	static const  int OFFSET_WIDTH = 100;
	static const  int CELL_WIDTH = 30;
	static const  int CELL_HEIGHT = 25;
	static const  int TABLE_WIDTH = CELL_WIDTH * 16 + 20;
	static const  int CELL_8_INNER = 10;
	static const  int TITLE_BAR_HEIGHT = 25;
	static const  int ASCII_WIDTH = 10;
};

