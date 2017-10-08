#pragma once
#include <string>

#define DESKTOP_BAR_CALLBACK (WM_USER + 0X1000)

struct Measure
{
	Measure() : m_hWnd(NULL), m_Width(100), m_Edge(ABE_TOP) {};

	HWND m_hWnd;
	int m_Width;
	UINT m_Edge;

	std::wstring m_stringResult;
};

