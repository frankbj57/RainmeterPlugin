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

	// Delay windows until first reload



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

	// Create a window class for a desktop bar
	if (g_WindowsRegistered <= 0)
	{
		WNDCLASSEX wcex = { sizeof(wcex) };
		wcex.lpfnWndProc = WndProc;
		wcex.cbWndExtra = sizeof(LPVOID);  // Room for the measure pointer
		wcex.lpszClassName = DESKTOP_BAR_WINDOW_CLASS;
		wcex.style = CS_NOCLOSE | CS_DBLCLKS;
		wcex.hbrBackground = nullptr;

		// wcex.hInstance = GetRainmeter().GetModuleInstance();

		RegisterClassEx(&wcex);

		g_WindowsRegistered = (g_WindowsRegistered <= 0) ? 1 : g_WindowsRegistered + 1;
	}

	if (!measure->m_Initialized)
	{
		// Create a window for the windows procedure
		// We are actually never using this
		HWND hwnd = CreateWindowEx(
			WS_EX_TOOLWINDOW,
			DESKTOP_BAR_WINDOW_CLASS,
			L"Rainmeter Desktop Bar Window",
			WS_POPUP | WS_CLIPCHILDREN,
			0,
			0,
			1,
			1,
			NULL,
			NULL,
			NULL,
			measure  // This is passed on with the NCCREATE message, to link window and measure
		);

		if (hwnd)
		{
			measure->m_hWnd = hwnd;  // Links window and measure

			ShowWindow(hwnd, SW_HIDE); // The actual window is never visible
			UpdateWindow(hwnd);        // Tell window to paint client area, to satisfy Windows
		}

		// Remember we have created the window now
		measure->m_Initialized = true;
	}

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

	ss << "L: " << measure->m_Left 
		<< " T: " << measure->m_Top 
		<< " R: " << measure->m_Right 
		<< " B: " << measure->m_Bottom 
		<< " H: " << (int)measure->m_hWnd 
		<< " E: " << measure->m_Edge;

	measure->m_stringResult = ss.str();

	return (double)r;
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
	switch (uMsg)
	{
	case WM_NCCREATE:
		{
			Measure* measure = (Measure*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)measure);

			DesktopBar::Register(hWnd, DESKTOP_BAR_CALLBACK);

			// Now we should set the size and side of the desktop bar
			DesktopBar::SetSide(hWnd, measure->m_Edge, measure->m_Width);

			//// Change the window procedure over to MainWndProc now that GWLP_USERDATA is set
			//SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			return TRUE;  // Indicating creation should continue
		}
		break;
	}

	Measure *measure = DesktopBar::GetDesktopMeasure(hWnd);
	if (measure)
	{
		switch (uMsg)
		{
		case DESKTOP_BAR_CALLBACK:
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
			break;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Special plugin functions to return subvalues
// The use of local static variables makes this plugin non-reenterable
// Only one desktop bar can be active at one time
// Should be fixed using allocation in measure object/class
PLUGIN_EXPORT LPCWSTR GetTop(void* data, const int argc, const WCHAR* argv[])
{
	Measure *measure = (Measure *)data;

	static wchar_t buffer[25];

	_itow_s(measure->m_Top, buffer, 10);

	return buffer;
}

PLUGIN_EXPORT LPCWSTR GetBottom(void* data, const int argc, const WCHAR* argv[])
{
	Measure *measure = (Measure *)data;

	static wchar_t buffer[25];

	_itow_s(measure->m_Bottom, buffer, 10);

	return buffer;
}

PLUGIN_EXPORT LPCWSTR GetLeft(void* data, const int argc, const WCHAR* argv[])
{
	Measure *measure = (Measure *)data;

	static wchar_t buffer[25];

	_itow_s(measure->m_Left, buffer, 10);

	return buffer;
}

PLUGIN_EXPORT LPCWSTR GetRight(void* data, const int argc, const WCHAR* argv[])
{
	Measure *measure = (Measure *)data;

	static wchar_t buffer[25];

	_itow_s(measure->m_Top, buffer, 10);

	return buffer;
}

PLUGIN_EXPORT LPCWSTR GetWidth(void* data, const int argc, const WCHAR* argv[])
{
	Measure *measure = (Measure *)data;

	static wchar_t buffer[25];

	_itow_s((measure->m_Right-measure->m_Left), buffer, 10);

	return buffer;
}

PLUGIN_EXPORT LPCWSTR GetHeight(void* data, const int argc, const WCHAR* argv[])
{
	Measure *measure = (Measure *)data;

	static wchar_t buffer[25];

	_itow_s((measure->m_Bottom-measure->m_Top), buffer, 10);

	return buffer;
}