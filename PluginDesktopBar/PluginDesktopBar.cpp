#include <Windows.h>
#include "../../API/RainmeterAPI.h"

#include <sstream>

#include "DesktopBarMeasure.h"
#include "DesktopBarFunctions.h"

// Overview: This is a blank canvas on which to build your plugin.

// Note: GetString and ExecuteBang have been commented out. If you need
// GetString and/or ExecuteBang and you have read what they are used for
// from the SDK docs, uncomment the function(s). Otherwise leave them
// commented out (or get rid of them)!

static int g_WindowsRegistered = 0;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define DESKTOP_BAR_WINDOW_CLASS L"RainmeterDesktopBarWindowClass"

PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
	Measure* measure = new Measure;
	*data = measure;

	// Create a window class for a desktop bar
	if (g_WindowsRegistered <= 0)
	{
		HBRUSH hBrush = CreateSolidBrush(RGB(64, 64, 64));

		WNDCLASSEX wcex = { sizeof(wcex) };
		wcex.lpfnWndProc = WndProc;
		wcex.cbWndExtra = sizeof(LPVOID);  // Room for the measure pointer
		wcex.lpszClassName = DESKTOP_BAR_WINDOW_CLASS;
		wcex.style = CS_NOCLOSE | CS_DBLCLKS;
		// wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);  // Background, could be replaced by customisable background
		wcex.hbrBackground = hBrush;  // Background, could be replaced by customisable background

		// wcex.hInstance = GetRainmeter().GetModuleInstance();

		RegisterClassEx(&wcex);

		g_WindowsRegistered = (g_WindowsRegistered <= 0) ? 1 : g_WindowsRegistered + 1;


	}

	// Create an actual window for the desktop bar
	HWND hwnd = CreateWindowEx(
		WS_EX_TOOLWINDOW,
		DESKTOP_BAR_WINDOW_CLASS,
		L"Rainmeter Desktop Bar Window",
		WS_POPUP | WS_CLIPCHILDREN,
		0,
		0,
		400,
		measure->m_Width,
		NULL,
		NULL,
		NULL,
		measure  // This is passed on with the NCCREATE message
	);

	// If the window was successfully created, make the window visible,
	// update its client area
	if (hwnd)
	{
		measure->m_hWnd = hwnd;

		ShowWindow(hwnd, SW_SHOWDEFAULT); // Set to visible & paint non-client area
		UpdateWindow(hwnd);         // Tell window to paint client area
	}


}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue)
{
	Measure* measure = (Measure*)data;

	// Get and set the position and width
	LPCWSTR strEdge = RmReadString(rm, L"Edge", L"Top");

	int newEdge = measure->m_Edge;
	if (_wcsicmp(strEdge, L"Top") == 0)
	{
		newEdge = ABE_TOP;
	}
	else if (_wcsicmp(strEdge, L"Left") == 0)
	{
		newEdge = ABE_LEFT;
	}
	else if (_wcsicmp(strEdge, L"Right") == 0)
	{
		newEdge = ABE_RIGHT;
	}
	else if (_wcsicmp(strEdge, L"Bottom") == 0)
	{
		newEdge = ABE_BOTTOM;
	}

	int newWidth = measure->m_Width;
	
	newWidth = RmReadInt(rm, L"Width", 100);

	if (newWidth != measure->m_Width || newEdge != measure->m_Edge)
	{
		measure->m_Edge = newEdge;
		measure->m_Width = newWidth;

		DesktopBar::SetSide(measure->m_hWnd, measure->m_Edge, measure->m_Width);
	}

}

PLUGIN_EXPORT double Update(void* data)
{
	Measure* measure = (Measure*)data;
	LONG r = (LONG)measure->m_hWnd;

	std::wostringstream ss;

	ss << (int) measure->m_hWnd << ": " << measure->m_Width << " (W) " << measure->m_Edge << " (E)";

	measure->m_stringResult = ss.str();

	return (double) r;
}

PLUGIN_EXPORT LPCWSTR GetString(void* data)
{
	Measure* measure = (Measure*)data;
	return measure->m_stringResult.c_str();
}

//PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args)
//{
//	Measure* measure = (Measure*)data;
//}

PLUGIN_EXPORT void Finalize(void* data)
{
	Measure* measure = (Measure*)data;

	if (measure->m_hWnd)
	{
		DesktopBar::Unregister(measure->m_hWnd);

		SetWindowLongPtr(measure->m_hWnd, GWLP_USERDATA, (LONG_PTR)nullptr);
		DestroyWindow(measure->m_hWnd);
	}

	delete measure;

	if (--g_WindowsRegistered <= 0)
	{
		// Unregister windows class
		UnregisterClass(DESKTOP_BAR_WINDOW_CLASS, nullptr);
	}
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_NCCREATE)
	{
		Measure* measure = (Measure*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) measure);

		DesktopBar::Register(hWnd, DESKTOP_BAR_CALLBACK);

		// Now we should set the size and side of the desktop bar
		DesktopBar::SetSide(hWnd, measure->m_Edge, measure->m_Width);

		//// Change the window procedure over to MainWndProc now that GWLP_USERDATA is set
		//SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
		return TRUE;  // Indicating creation should continue
	}
	else if (uMsg == DESKTOP_BAR_CALLBACK)
	{
		// Callback from the Shell application bar API
		switch (wParam)
		{
		// Notifies the appbar that the taskbar's autohide or always-on-top
		// state has changed.  The appbar can use this to conform to the settings
		// of the system taskbar.
		case ABN_STATECHANGE:
			break;

		// Notifies the appbar when a full screen application is opening or
		// closing.  When a full screen app is opening, the appbar must drop
		// to the bottom of the Z-Order.  When the app is closing, we should
		// restore our Z-order position.
		// Since we are always at the bottom, don't do anything
		case ABN_FULLSCREENAPP:
			break;

		// Notifies the appbar when an event has occured that may effect the
		// appbar's size and position.  These events include changes in the
		// taskbar's size, position, and visiblity as well as adding, removing,
		// or resizing another appbar on the same side of the screen.
		case ABN_POSCHANGED:
			// Update our position in response to the system change
			{
				Measure *measure = DesktopBar::GetDesktopMeasure(hWnd);
				DesktopBar::SetSide(hWnd, measure->m_Edge, measure->m_Width);
			}
			break;

		// Notifies when windows are tiled, cascaded, etc.
		// Do nothing
		case ABN_WINDOWARRANGE:
			break;
		}

		return 0;
	}
	else if (uMsg == WM_ERASEBKGND)
	{
		// Paint with customized background brush (or even bitmap)
		HDC hDc = (HDC) wParam;

		SelectObject(hDc, GetStockObject(DC_BRUSH));

		RECT clientRect;

		GetClientRect(hWnd, &clientRect);

		SetDCBrushColor(hDc, RGB(1, 1, 1));

		Rectangle(hDc, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);

		return 1; // Non-false indicating erase background was done
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);

}

