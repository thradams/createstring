
#include "framework.h"
#include "CreateString.h"
#include "w32.h"

INT_PTR CALLBACK  About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MSG msg;

#define DESKTOP_FONT

#ifdef DESKTOP_FONT
  ShowDialog(hInstance, IDD_ABOUTBOX, NULL,NULL, About);
#else
 DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), NULL, About);
#endif

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

        if (p[0] == '\\')
        {
            p++;

            *p_out = L'\\';
            p_out++;
            *p_out = L'\\';
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
