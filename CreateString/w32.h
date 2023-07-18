#pragma once
#include <Windows.h>

DWORD GetStyle(HWND hWnd);
BOOL CenterWindow(HWND hWnd, HWND hWndCenter);

INT_PTR ShowDialog(HINSTANCE hInstance,
                   DWORD dlgID,
                   void* p,
                   HWND hParent,
                   DLGPROC lpDialogFunc);

BOOL IsWindowsXPOrGreater(void);
