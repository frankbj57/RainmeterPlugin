#pragma once
#include <string>

#define DESKTOP_BAR_CALLBACK (WM_USER + 0X1000)

struct Measure
{
	Measure() : m_hWnd(NULL), m_Width(-1), m_Edge(ABE_TOP), m_Initialized(false) {};

	HWND m_hWnd;
	int m_Width;
	UINT m_Edge;

	bool m_Initialized;


	// Actual coordinates of the toolbar area
	int m_Top = 0;
	int m_Left = 0;
	int m_Bottom = 0;
	int m_Right = 0;

	std::wstring m_stringResult;
};

