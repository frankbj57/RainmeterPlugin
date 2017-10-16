#include <Windows.h>

#include "DesktopBarFunctions.h"
#include "DesktopBarMeasure.h"

namespace DesktopBar
{
	bool Register(HWND hWnd, UINT uCallbackMessage)
	{
		bool Result = false;

		APPBARDATA abd;
		abd.cbSize = sizeof(APPBARDATA);
		abd.hWnd = hWnd;
		abd.uCallbackMessage = uCallbackMessage;

		Result = (BOOL)SHAppBarMessage(ABM_NEW, &abd);

		return Result;
	}

	void Unregister(HWND hWnd)
	{
		APPBARDATA abd;
		abd.cbSize = sizeof(APPBARDATA);
		abd.hWnd = hWnd;

		SHAppBarMessage(ABM_REMOVE, &abd);

		return;
	}

	Measure * GetDesktopMeasure(HWND hWnd)
	{
		return (Measure *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}

	void SetSide(HWND hWnd, int edge, int width)
	{
		// Get the size of the window from side and option values
		RECT rc;
		rc.top = 0;
		rc.left = 0;
		rc.right = GetSystemMetrics(SM_CXSCREEN);
		rc.bottom = GetSystemMetrics(SM_CYSCREEN);

		// Adjust the rectangle to set our height or width depending on the
		// side we want.
		switch (edge)
		{
		case ABE_TOP:
			rc.bottom = rc.top + width;
			break;
		case ABE_BOTTOM:
			rc.top = rc.bottom - width;
			break;
		case ABE_LEFT:
			rc.right = rc.left + width;
			break;
		case ABE_RIGHT:
			rc.left = rc.right - width;
			break;
		}

		// Get the adjusted coordinates
		QueryPos(hWnd, edge, rc, width);

		SetPos(hWnd, edge, rc);

	}

	void QueryPos(HWND hWnd, int edge, RECT & rc, int width)
	{
		// Fill out the APPBARDATA struct
		APPBARDATA abd{ sizeof(APPBARDATA) };
		abd.hWnd = hWnd;
		abd.rc = rc;
		abd.uEdge = edge;

		SHAppBarMessage(ABM_QUERYPOS, &abd);

		// The system will return an approved position along the edge we're asking
		// for.  However, if we can't get the exact position requested, the system
		// only updates the edge that's incorrect.  For example, if we want to
		// attach to the bottom of the screen and the taskbar is already there,
		// we'll pass in a rect like 0, 964, 1280, 1024 and the system will return
		// 0, 964, 1280, 996.  Since the appbar has to be above the taskbar, the
		// bottom of the rect was adjusted to 996.  We need to adjust the opposite
		// edge of the rectangle to preserve the height we want.

		rc = abd.rc;

		// Adjust with width
		switch (abd.uEdge)
		{
		case ABE_LEFT:
			rc.right = rc.left + width;
			break;

		case ABE_RIGHT:
			rc.left = rc.right - width;
			break;

		case ABE_TOP:
			rc.bottom = rc.top + width;
			break;

		case ABE_BOTTOM:
			rc.top = rc.bottom - width;
			break;
		}
	}

	void SetPos(HWND hWnd, int edge, RECT rc)
	{
		// Fill out the APPBARDATA struct and save the edge we're moving to
		// in the appbar OPTIONS struct.
		APPBARDATA abd{ sizeof(APPBARDATA) };
		abd.hWnd = hWnd;
		abd.rc = rc;
		abd.uEdge = edge;

		// Tell the system we're moving to this new approved position.
		SHAppBarMessage(ABM_SETPOS, &abd);

		// Save the new coordinates to be queried by the skin and meters
		Measure *measure = GetDesktopMeasure(hWnd);

		measure->m_Left = abd.rc.left;
		measure->m_Right = abd.rc.right;
		measure->m_Top = abd.rc.top;
		measure->m_Bottom = abd.rc.bottom;

		// Never actually update the window, as we just use it for the windows procedure
	}


}