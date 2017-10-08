#include <Windows.h>
#include "../../API/RainmeterAPI.h"

void ToggleKeyState(int virtualKey);

// Overview: This is a blank canvas on which to build your plugin.

// Note: GetString and ExecuteBang have been commented out. If you need
// GetString and/or ExecuteBang and you have read what they are used for
// from the SDK docs, uncomment the function(s). Otherwise leave them
// commented out (or get rid of them)!

struct Measure
{
	Measure() : m_VirtualKey(VK_CAPITAL), m_KeyState(false) {};
	int m_VirtualKey;
	bool m_KeyState;
};

PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
	Measure* measure = new Measure;
	*data = measure;
}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue)
{
	Measure* measure = (Measure*)data;

	measure->m_VirtualKey = VK_CAPITAL;

	LPCWSTR value = RmReadString(rm, L"Key", L"Caps");
	if (_wcsicmp(value, L"Caps") == 0)
	{
		measure->m_VirtualKey = VK_CAPITAL;
	}
	else if (_wcsicmp(value, L"Num") == 0)
	{
		measure->m_VirtualKey = VK_NUMLOCK;
	}
	else if (_wcsicmp(value, L"Scroll") == 0)
	{
		measure->m_VirtualKey = VK_SCROLL;
	}
	else if (_wcsicmp(value, L"Insert") == 0)
	{
		measure->m_VirtualKey = VK_INSERT;
	}
	else
	{
		RmLog(LOG_ERROR, L"KeyboardState.dll: Invalid Key=");
	}

}

PLUGIN_EXPORT double Update(void* data)
{
	Measure* measure = (Measure*)data;
	SHORT state = GetKeyState(measure->m_VirtualKey);
	measure->m_KeyState = (state & 0x0001) == 1;  // Toggled state is in the low order bit
	return measure->m_KeyState;
}

//PLUGIN_EXPORT LPCWSTR GetString(void* data)
//{
//	Measure* measure = (Measure*)data;
//	return L"";
//}

PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args)
{

	if (_wcsicmp(args, L"ToggleCapsLock") == 0)
	{
		ToggleKeyState(VK_CAPITAL);
	}
	else if (_wcsicmp(args, L"ToggleNumLock") == 0)
	{
		ToggleKeyState(VK_NUMLOCK);
	}
	else if (_wcsicmp(args, L"ToggleScrollLock") == 0)
	{
		ToggleKeyState(VK_SCROLL);
	}
	else if (_wcsicmp(args, L"ToggleInsertState") == 0)
	{
		ToggleKeyState(VK_INSERT);
	}
	else
	{
		RmLog(LOG_ERROR, L"KeyboardState.dll: Invalid command");
	}


}

PLUGIN_EXPORT void Finalize(void* data)
{
	Measure* measure = (Measure*)data;
	delete measure;
}

void ToggleKeyState(int virtualKey)
{
	keybd_event(virtualKey, 0, 0, 0);
	keybd_event(virtualKey, 0, KEYEVENTF_KEYUP, 0);
}