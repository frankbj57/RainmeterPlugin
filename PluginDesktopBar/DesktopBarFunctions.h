#pragma once
#include "DesktopBarMeasure.h"

namespace DesktopBar
{
	Measure * GetDesktopMeasure(HWND hWnd);
	bool Register(HWND hWnd, UINT uCallbackMessage);
	void Unregister(HWND hWnd);
	void SetSide(HWND hWnd, int edge, int width);
	void QueryPos(HWND hWnd, int edge, RECT & rc, int width);
	void SetPos(HWND hWnd, int edge, RECT rc);

};
