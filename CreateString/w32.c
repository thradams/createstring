
#include "w32.h"
#include <assert.h>
#define ASSERT assert

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

struct DLGTEMPLATEEX
{
    WORD dlgVer;
    WORD signature;
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;

    // Everything else in this structure is variable length,
    // and therefore must be determined dynamically

    // sz_Or_Ord menu;      // name or ordinal of a menu resource
    // sz_Or_Ord windowClass; // name or ordinal of a window class
    // WCHAR title[titleLen]; // title string of the dialog box
    // short pointsize;     // only if DS_SETFONT is set
    // short weight;      // only if DS_SETFONT is set
    // short bItalic;     // only if DS_SETFONT is set
    // WCHAR font[fontLen];   // typeface name, if DS_SETFONT is set
};

struct DLGITEMTEMPLATEEX
{
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    short x;
    short y;
    short cx;
    short cy;
    DWORD id;

    // Everything else in this structure is variable length,
    // and therefore must be determined dynamically

    // sz_Or_Ord windowClass; // name or ordinal of a window class
    // sz_Or_Ord title;     // title string or ordinal of a resource
    // WORD extraCount;     // bytes following creation data
};
//#pragma pack(pop)

struct DialogTemplate
{
    HGLOBAL m_hTemplate;
    DWORD m_dwTemplateSize;
    BOOL m_bSystemFont;
};

BOOL DialogTemplate_SetTemplate(struct DialogTemplate* pThis, const DLGTEMPLATE* pTemplate, UINT cb);

DWORD GetStyle(HWND hWnd)
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


BOOL IsDialogEx(const DLGTEMPLATE* pTemplate)
{
    return ((struct DLGTEMPLATEEX*)pTemplate)->signature == 0xFFFF;
}


int FontAttrSize(BOOL bDialogEx)
{
    return (int)sizeof(WORD) * (bDialogEx ? 3 : 1);
}


BYTE* GetFontSizeField(const DLGTEMPLATE* pTemplate)
{
    BOOL bDialogEx = IsDialogEx(pTemplate);
    WORD* pw;

    if (bDialogEx)
        pw = (WORD*)((struct DLGTEMPLATEEX*)pTemplate + 1);
    else
        pw = (WORD*)(pTemplate + 1);

    if (*pw == (WORD)-1)      // Skip menu name string or ordinal
        pw += 2; // WORDs
    else
        while (*pw++);

    if (*pw == (WORD)-1)      // Skip class name string or ordinal
        pw += 2; // WORDs
    else
        while (*pw++);

    while (*pw++);          // Skip caption string

    return (BYTE*)pw;
}

BOOL HasFont(const DLGTEMPLATE* pTemplate)
{
    return (DS_SETFONT &
            (IsDialogEx(pTemplate) ? ((struct DLGTEMPLATEEX*)pTemplate)->style :
            pTemplate->style));
}

BOOL DialogTemplate_SetFont(struct DialogTemplate* pThis, LPCWSTR lpFaceName, WORD nFontSize)
{
    ASSERT(pThis->m_hTemplate != NULL);

    if (pThis->m_dwTemplateSize == 0)
        return FALSE;

    DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)GlobalLock(pThis->m_hTemplate);
    if (pTemplate == NULL)
    {        
        return FALSE;
    }
    BOOL bDialogEx = IsDialogEx(pTemplate);
    BOOL bHasFont = HasFont(pTemplate);
    int cbFontAttr = FontAttrSize(bDialogEx);

    if (bDialogEx)
        ((struct DLGTEMPLATEEX*)pTemplate)->style |= DS_SETFONT;
    else
        pTemplate->style |= DS_SETFONT;

    size_t nFaceNameLen = wcslen(lpFaceName);

    if (nFaceNameLen >= LF_FACESIZE)
    {
        // Name too long
        return FALSE;
    }

#ifdef _UNICODE
    size_t cbNew = cbFontAttr + ((nFaceNameLen + 1) * sizeof(TCHAR));
    BYTE* pbNew = (BYTE*)lpFaceName;
#else
    WCHAR wszFaceName[LF_FACESIZE];
    int cbNew = cbFontAttr + 2 * MultiByteToWideChar(CP_ACP, 0, lpFaceName, -1, wszFaceName, LF_FACESIZE);
    BYTE* pbNew = (BYTE*)wszFaceName;
#endif

    if (cbNew < (size_t)cbFontAttr)
    {
        return FALSE;
    }

    BYTE* pb = GetFontSizeField(pTemplate);
    int cbOld = (int)(bHasFont ? cbFontAttr + 2 * (wcslen((WCHAR*)(pb + cbFontAttr)) + 1) : 0);

    BYTE* pOldControls = (BYTE*)(((DWORD_PTR)pb + cbOld + 3) & ~((DWORD_PTR)3));
    BYTE* pNewControls = (BYTE*)(((DWORD_PTR)pb + cbNew + 3) & ~((DWORD_PTR)3));

    WORD nCtrl = bDialogEx ? (WORD)((struct DLGTEMPLATEEX*)pTemplate)->cDlgItems :
        (WORD)pTemplate->cdit;

    if (cbNew != cbOld && nCtrl > 0)
    {
        size_t nBuffLeftSize = (size_t)(pThis->m_dwTemplateSize - (pOldControls - (BYTE*)pTemplate));

        if (nBuffLeftSize > pThis->m_dwTemplateSize)
        {
            return FALSE;
        }

        memmove_s(pNewControls, nBuffLeftSize, pOldControls, nBuffLeftSize);
    }

    *(WORD*)pb = nFontSize;
    memmove_s(pb + cbFontAttr, cbNew - cbFontAttr, pbNew, cbNew - cbFontAttr);

    pThis->m_dwTemplateSize += (ULONG)(pNewControls - pOldControls);

    GlobalUnlock(pThis->m_hTemplate);
    pThis->m_bSystemFont = FALSE;
    return TRUE;
}

BOOL DialogTemplate_Load(struct DialogTemplate* pThis, HINSTANCE hInst, LPCTSTR lpDialogTemplateID);
void GetSystemIconFont(wchar_t* strFontNameOut, int* nPointSize);

INT_PTR ShowDialog(HINSTANCE hInstance,
                   DWORD dlgID,
                   void* p,
                   HWND hParent,
                   DLGPROC lpDialogFunc)
{
    struct DialogTemplate dlt = { 0 };
    
    DialogTemplate_Load(&dlt, hInstance, MAKEINTRESOURCEW(dlgID));

    wchar_t strFontName[200] = { 0 };
    int nPointSize = 0;
    GetSystemIconFont(strFontName, &nPointSize);
    DialogTemplate_SetFont(&dlt, strFontName, (WORD)nPointSize);

    // get pointer to the modified dialog template
    LPSTR pdata = (LPSTR)GlobalLock(dlt.m_hTemplate);

    if (pdata == 0)
    {
        return 0;
    }

    INT_PTR r = DialogBoxIndirectParam(
        hInstance,
        (LPCDLGTEMPLATEW)pdata,
        hParent,
        lpDialogFunc,
        (LPARAM)p);


    // unlock memory object
    GlobalUnlock(dlt.m_hTemplate);
    return r;
}


BOOL DialogTemplate_Load(struct DialogTemplate* pThis, HINSTANCE hInst, LPCTSTR lpDialogTemplateID)
{
    if (hInst == NULL)
    {
        return FALSE;
    }

    HRSRC hRsrc = FindResource(hInst, lpDialogTemplateID, RT_DIALOG);

    if (hRsrc == NULL)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    HGLOBAL hTemplate = LoadResource(hInst, hRsrc);
    if (hTemplate == NULL)
    {
        ASSERT(FALSE);
        return FALSE;
    }

    DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)LockResource(hTemplate);


    BOOL bSet = DialogTemplate_SetTemplate(pThis, pTemplate, (UINT)SizeofResource(hInst, hRsrc));
    UnlockResource(hTemplate);
    FreeResource(hTemplate);
    return bSet;
}



void GetSystemIconFont(wchar_t* strFontNameOut, int* nPointSize)
{
    wcscpy(strFontNameOut, L"MS Shell Dlg"); // out
    *nPointSize = 8; //out

    NONCLIENTMETRICS metrics;

    if (IsWindowsXPOrGreater())
    {
        metrics.cbSize = sizeof(NONCLIENTMETRICS);
    }
    else
    {
        metrics.cbSize = sizeof(NONCLIENTMETRICS) - sizeof(int);
    }

    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &metrics, 0) != 0)
    {
        HDC hDC = GetDC(NULL);
        int nLPixY = GetDeviceCaps(hDC, LOGPIXELSY);
        ReleaseDC(NULL, hDC);

        // copy font parameters ///nao sei bem pq 72 mas funciona?
        *nPointSize = -MulDiv(metrics.lfMessageFont.lfHeight, 72, nLPixY);
        wcscpy(strFontNameOut, metrics.lfMessageFont.lfFaceName);
    }
    else
    {
        ASSERT(0);//
    }
}


BOOL DialogTemplate_SetTemplate(struct DialogTemplate* pThis, const DLGTEMPLATE* pTemplate, UINT cb)
{
    pThis->m_dwTemplateSize = cb;
    SIZE_T nAllocSize = pThis->m_dwTemplateSize + LF_FACESIZE * 2;

    if (nAllocSize < pThis->m_dwTemplateSize)
    {
        return FALSE;
    }

    if ((pThis->m_hTemplate = GlobalAlloc(GPTR, nAllocSize)) == NULL)
    {
        return FALSE;
    }

    DLGTEMPLATE* pNew = (DLGTEMPLATE*)GlobalLock(pThis->m_hTemplate);
    memcpy_s((BYTE*)pNew, (size_t)pThis->m_dwTemplateSize, pTemplate, (size_t)pThis->m_dwTemplateSize);

    pThis->m_bSystemFont = (HasFont(pNew) == 0);

    GlobalUnlock(pThis->m_hTemplate);
    return TRUE;
}



BOOL IsWindowsXPOrGreater()
{
#pragma warning( push )
#pragma warning( disable : 4996 )
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    return osvi.dwMajorVersion >= 5;
#pragma warning( pop )
}

