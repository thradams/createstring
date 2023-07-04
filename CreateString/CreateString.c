#define _CRT_SECURE_NO_WARNINGS
#include "framework.h"
#include "CreateString.h"

INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

BOOL CenterWindow(HWND hWnd, HWND hWndCenter);

#pragma comment(lib, "Comctl32.lib")


#if defined _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MSG msg;

    DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), NULL, About);

    while (GetMessage(&msg, 0, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, 0, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

BOOL CopyTextToClipboardW(HWND hwndOwner, const wchar_t* rstr)
{

    BOOL bOk = FALSE;
    if (OpenClipboard(hwndOwner))
    {
        if (EmptyClipboard())
        {
            HGLOBAL hClipbuffer = GlobalAlloc(GMEM_DDESHARE, (wcslen(rstr) + 1) * sizeof(WCHAR));
            LPTSTR lpszBuffer = (LPTSTR) GlobalLock(hClipbuffer);
            lstrcpy(lpszBuffer, (LPCTSTR) rstr);
            GlobalUnlock(hClipbuffer);

            HANDLE hclipData = SetClipboardData(CF_UNICODETEXT, hClipbuffer);
            if (hclipData != NULL)
                bOk = TRUE;
        }
        CloseClipboard();
    }

    return bOk;
}


wchar_t* MakeLiteral(const wchar_t* input, BOOL bMacro, BOOL bwchar)
{
    wchar_t* out = calloc(wcslen(input) * 2 + 100, sizeof(wchar_t));
    if (out == NULL) return NULL;

    wchar_t* p_out = out;

    if (bMacro)
    {
        if (bwchar)
            wcscat(out, L"#define STR \\\r\n L\"");
        else
            wcscat(out, L"#define STR \\\r\n \"");
    }
    else
    {
        if (bwchar)
        {
            wcscat(out, L"const wchar_t* str =\r\n L\"");
        }
        else
        {
            wcscat(out, L"const char* str\n =\r\n \"");
        }

    }

    while (*p_out) p_out++;

    const wchar_t* p = input;
    while (*p)
    {
        if (p[0] == '\r' && p[1] == '\n')
        {
            p++;
            p++;

            *p_out = L'\\';
            p_out++;
            *p_out = L'n';
            p_out++;


            *p_out = L'\"';
            p_out++;


            if (bMacro)
            {
                *p_out = L'\\';
                p_out++;
            }


            *p_out = L'\r';
            p_out++;

            *p_out = L'\n';
            p_out++;

            *p_out = L' ';
            p_out++;

            if (bwchar)
            {
                *p_out = L'L';
                p_out++;
            }


            *p_out = L'"';
            p_out++;



            continue;
        }
        if (p[0] == '"')
        {
            p++;

            *p_out = L'\\';
            p_out++;
            *p_out = L'\"';
            p_out++;
            continue;
        }

        *p_out = *p;
        p_out++;
        p++;
    }

    if (bMacro)
    {
        wcscat(out, L"\"");
    }
    else
        wcscat(out, L"\";");

    wcscat(out, L"\r\n");
    return out;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        case WM_INITDIALOG:

            CheckRadioButton(hDlg,
                IDC_RADIO1,
                IDC_RADIO2,
                IDC_RADIO1);

            CenterWindow(hDlg, NULL);
            return (INT_PTR) TRUE;

        case WM_NOTIFY:
            if (wParam)
                break;

        case WM_COMMAND:

            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                ExitProcess(0);
                return (INT_PTR) TRUE;
            }

            if (LOWORD(wParam) == IDC_COPYBTN)
            {

                UINT bMacro = IsDlgButtonChecked(hDlg, IDC_RADIO1);
                UINT bVariable = IsDlgButtonChecked(hDlg, IDC_RADIO2);
                UINT bWchar = IsDlgButtonChecked(hDlg, IDC_CHECK1);

                int size = GetWindowTextLength(GetDlgItem(hDlg, IDC_INPUT));
                wchar_t* s = malloc(sizeof(wchar_t) * (size + 1));
                if (s)
                {
                    GetDlgItemText(hDlg, IDC_INPUT, s, size + 1);
                    wchar_t* out = MakeLiteral(s, bMacro, bWchar);
                    CopyTextToClipboardW(hDlg, out);
                    free(out);
                    free(s);
                }

                return (INT_PTR) TRUE;
            }

            if (HIWORD(wParam) == BN_CLICKED ||
                HIWORD(wParam) == EN_CHANGE)
            {
                UINT bMacro = IsDlgButtonChecked(hDlg, IDC_RADIO1);
                UINT bVariable = IsDlgButtonChecked(hDlg, IDC_RADIO2);
                UINT bWchar = IsDlgButtonChecked(hDlg, IDC_CHECK1);

                int size = GetWindowTextLength(GetDlgItem(hDlg, IDC_INPUT));
                wchar_t* s = malloc(sizeof(wchar_t) * (size + 1));
                if (s)
                {
                    GetDlgItemText(hDlg, IDC_INPUT, s, size + 1);
                    wchar_t* out = MakeLiteral(s, bMacro, bWchar);
                    SetDlgItemText(hDlg, IDC_OUTPUT, out);
                    free(out);
                    free(s);
                }
                return (INT_PTR) TRUE;
            }

            break;
    }
    return (INT_PTR) FALSE;
}

inline DWORD GetStyle(HWND hWnd)
{
    return (DWORD) GetWindowLong(hWnd, GWL_STYLE);
}

BOOL CenterWindow(HWND hWnd, HWND hWndCenter)
{


    // determine owner window to center against
    DWORD dwStyle = GetStyle(hWnd);

    if (hWndCenter == NULL)
    {
        if (dwStyle & WS_CHILD)
        {
            hWndCenter = GetParent(hWnd);
        }
        else
        {
            hWndCenter = GetWindow(hWnd, GW_OWNER);
        }
    }

    // get coordinates of the window relative to its parent
    RECT rcDlg;
    GetWindowRect(hWnd, &rcDlg);
    RECT rcArea;
    RECT rcCenter;
    HWND hWndParent;

    if (!(dwStyle & WS_CHILD))
    {
        // don't center against invisible or minimized windows
        if (hWndCenter != NULL)
        {
            DWORD dwStyleCenter = GetWindowLong(hWndCenter, GWL_STYLE);

            if (!(dwStyleCenter & WS_VISIBLE) || (dwStyleCenter & WS_MINIMIZE))
            {
                hWndCenter = NULL;
            }
        }

        // center within screen coordinates
#if WINVER < 0x0500
        SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);
#else
        HMONITOR hMonitor = NULL;

        if (hWndCenter != NULL)
        {
            hMonitor = MonitorFromWindow(hWndCenter,
                MONITOR_DEFAULTTONEAREST);
        }
        else
        {
            hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
        }

        //ATLENSURE_RETURN_VAL(hMonitor != NULL, FALSE);

        MONITORINFO minfo;
        minfo.cbSize = sizeof(MONITORINFO);
        /*BOOL bResult = */
        GetMonitorInfo(hMonitor, &minfo);
        //ATLENSURE_RETURN_VAL(bResult, FALSE);

        rcArea = minfo.rcWork;
#endif

        if (hWndCenter == NULL)
        {
            rcCenter = rcArea;
        }
        else
        {
            GetWindowRect(hWndCenter, &rcCenter);
        }
    }
    else
    {
        // center within parent client coordinates
        hWndParent = GetParent(hWnd);


        GetClientRect(hWndParent, &rcArea);

        GetClientRect(hWndCenter, &rcCenter);
        MapWindowPoints(hWndCenter, hWndParent, (POINT*) &rcCenter, 2);
    }

    int DlgWidth = rcDlg.right - rcDlg.left;
    int DlgHeight = rcDlg.bottom - rcDlg.top;

    // find dialog's upper left based on rcCenter
    int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
    int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

    // if the dialog is outside the screen, move it inside
    if (xLeft + DlgWidth > rcArea.right)
    {
        xLeft = rcArea.right - DlgWidth;
    }

    if (xLeft < rcArea.left)
    {
        xLeft = rcArea.left;
    }

    if (yTop + DlgHeight > rcArea.bottom)
    {
        yTop = rcArea.bottom - DlgHeight;
    }

    if (yTop < rcArea.top)
    {
        yTop = rcArea.top;
    }

    // map screen coordinates to child coordinates
    return SetWindowPos(hWnd, NULL, xLeft, yTop, -1, -1,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}
