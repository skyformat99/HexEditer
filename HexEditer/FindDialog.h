#pragma once
#include <Windows.h>

class FindDialog
{
public:
	FindDialog(void);

	void createWnd(HWND parent, int x, int y, int w, int h);

	~FindDialog(void);

public:
	HWND mWnd;
};

