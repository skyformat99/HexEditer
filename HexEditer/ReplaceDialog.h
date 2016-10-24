#pragma once
#include <Windows.h>

class ReplaceDialog
{
public:
	ReplaceDialog(void);

	void createWnd(HWND parent, int x, int y, int w, int h);

	~ReplaceDialog(void);

public:
	HWND mWnd;
};

