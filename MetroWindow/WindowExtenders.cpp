#include "stdafx.h"
#include "WindowExtenders.h"

namespace MetroWindow
{

namespace WindowExtenders
{
	bool IsDrawMaximizeBox(HWND hWnd)
	{
		bool ret = false;

		DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
		if ((dwStyle & WS_MAXIMIZEBOX) != 0)
		{
			ret = true;
			DWORD dwExStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
			if ((dwExStyle & WS_EX_TOOLWINDOW) == WS_EX_TOOLWINDOW)
			{
				ret = false;
			}
		}

		return ret;
	}

	bool IsDrawMinimizeBox(HWND hWnd)
	{
		bool ret = false;

		DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
		if ((dwStyle & WS_MINIMIZEBOX) != 0)
		{
			ret = true;
			DWORD dwExStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
			if ((dwExStyle & WS_EX_TOOLWINDOW) == WS_EX_TOOLWINDOW)
			{
				ret = false;
			}
		}

		return ret;
	}

	CSize GetBorderSize(HWND hWnd, bool dwmEnabled)
	{
        // Check for Caption
        DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
        bool caption = (dwStyle & WS_CAPTION) != 0;

		// Get BorderMultiplierFactor
		int bmfactor = 0;
		::SystemParametersInfo(SPI_GETBORDER, 0, &bmfactor, 0);
        int factor = bmfactor - 1;

		CSize border;
		if (((dwStyle & WS_THICKFRAME) != 0))
		{
			// Sizable or SizableToolWindow
			if (IsVista())
				border = GetFrameBorderSize();
            else
				border = GetFixedFrameBorderSize() +
							(caption ? GetBorderSize() + CSize(factor, factor)
								: CSize(factor, factor));
		}
		else if (((dwStyle & WS_EX_CLIENTEDGE) != 0))
		{
			// Fixed3D
			border = GetFixedFrameBorderSize() + GetBorder3DSize();
		}
		else
		{
			if (dwmEnabled)
				border = GetFrameBorderSize();
            else
				border = GetFixedFrameBorderSize();
		}

        return border;
	}

	CSize GetCaptionButtonSize(HWND hWnd)
	{
		DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
		CSize buttonSize = ((dwStyle & WS_EX_TOOLWINDOW) == WS_EX_TOOLWINDOW)
			? GetToolWindowCaptionButtonSize()
			: GetCaptionButtonSize();

        return buttonSize;
	}

	int GetCaptionHeight(HWND hWnd)
	{
		DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
		return ((dwStyle & WS_EX_TOOLWINDOW) == WS_EX_TOOLWINDOW)
			? ::GetSystemMetrics(SM_CYSMCAPTION)
			: ::GetSystemMetrics(SM_CYCAPTION);
	}
}

}; //namespace MetroWindow
