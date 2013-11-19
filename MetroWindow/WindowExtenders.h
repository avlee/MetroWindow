#pragma once

#include "MiscWapppers.h"

namespace MetroWindow
{

namespace WindowExtenders
{
	inline bool IsVista()
	{
		OSVERSIONINFO ovi = { sizeof(OSVERSIONINFO) };
		BOOL bRet = ::GetVersionEx(&ovi);
		return ((bRet != FALSE) && (ovi.dwMajorVersion >= 6));
	}

	inline bool HasSysMenu(HWND hWnd)
	{
		DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
        return (dwStyle & WS_CAPTION) != 0 && (dwStyle & WS_SYSMENU) != 0;
	}

	inline bool IsWindowSizable(HWND hWnd)
	{
		DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
        DWORD dwExStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
        return ((dwStyle & WS_THICKFRAME) != 0) && !((dwExStyle & WS_EX_DLGMODALFRAME) != 0);
	}

	inline CSize GetBorderSize()
	{
		return CSize(::GetSystemMetrics(SM_CXBORDER), ::GetSystemMetrics(SM_CYBORDER));
	}

	inline CSize GetBorder3DSize()
	{
		return CSize(::GetSystemMetrics(SM_CXEDGE), ::GetSystemMetrics(SM_CYEDGE));
	}

	inline CSize GetFrameBorderSize()
	{
		return CSize(::GetSystemMetrics(SM_CXFRAME), ::GetSystemMetrics(SM_CYFRAME));
	}

	inline CSize GetFixedFrameBorderSize()
	{
		return CSize(::GetSystemMetrics(SM_CXFIXEDFRAME), ::GetSystemMetrics(SM_CYFIXEDFRAME));
	}

	inline CSize GetCaptionButtonSize()
	{
		return CSize(::GetSystemMetrics(SM_CXSIZE), ::GetSystemMetrics(SM_CYSIZE));
	}

	inline CSize GetToolWindowCaptionButtonSize()
	{
		return CSize(::GetSystemMetrics(SM_CXSMSIZE), ::GetSystemMetrics(SM_CYSMSIZE));
	}

	inline CSize GetSmallIconSize()
	{
		return CSize(::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	}

	bool IsDrawMaximizeBox(HWND hWnd);
	bool IsDrawMinimizeBox(HWND hWnd);

	CSize GetBorderSize(HWND hWnd, bool dwmEnabled);

	CSize GetCaptionButtonSize(HWND hWnd);

	int GetCaptionHeight(HWND hWnd);
};

}; //namespace MetroWindow
