// PortionOfScreen.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "PortionOfScreen.h"
#include "WinReg.hpp"

#define MAX_LOADSTRING 100
#define IDT_REDRAW     101
#define IDC_OPTIONS    1100
#define POS_MIN_WIDTH  320
#define POS_MIN_HEIGHT 200

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
bool moveToDefaultWindowPos = false;

// Global settings
bool focusMode =  false;
RECT defaultWindowPos;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                LoadSettings();
void                SaveSettings();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Prevent automatic scaling
    SetProcessDPIAware();

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PORTIONOFSCREEN, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PORTIONOFSCREEN));

    WNDCLASSEXW wcex;
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = hIcon;
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = hIcon;

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   LoadSettings();

   HWND hWnd = CreateWindowExW(
       WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
       szWindowClass,
       szTitle,
       WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX,
       defaultWindowPos.left,
       defaultWindowPos.top,
       defaultWindowPos.right - defaultWindowPos.left,
       defaultWindowPos.bottom - defaultWindowPos.top,
       nullptr,
       nullptr,
       hInstance,
       nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   HMENU hSysMenu = GetSystemMenu(hWnd, FALSE);
   AppendMenu(hSysMenu, MF_SEPARATOR, 0, NULL);
   AppendMenu(hSysMenu, MF_STRING, IDC_OPTIONS, L"Options");

   ShowWindow(hWnd, nCmdShow);
   SetLayeredWindowAttributes(hWnd, RGB(255, 255, 255), 128, LWA_ALPHA);
   UpdateWindow(hWnd);

   SetTimer(hWnd, IDT_REDRAW, 200, (TIMERPROC)NULL);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_LBUTTONUP:
        SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
        break;

    case WM_SETFOCUS:
        if (focusMode)
        {
            // The window could be anywhere in Focus Mode, even over the taskbar. Quickly move window to it's original position.
            SetWindowPos(hWnd, HWND_TOPMOST, defaultWindowPos.left, defaultWindowPos.top, defaultWindowPos.right - defaultWindowPos.left, 0, 0);
        }
        else if (moveToDefaultWindowPos)
        {
            SetWindowPos(hWnd, HWND_TOPMOST, defaultWindowPos.left, defaultWindowPos.top, defaultWindowPos.right - defaultWindowPos.left, defaultWindowPos.bottom - defaultWindowPos.top, 0);
            moveToDefaultWindowPos = false;
        }

        SetLayeredWindowAttributes(hWnd, RGB(255, 255, 255), 128, LWA_ALPHA);
        SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOPMOST);
        return DefWindowProc(hWnd, message, wParam, lParam);

    case WM_KILLFOCUS:
        SetLayeredWindowAttributes(hWnd, RGB(255, 255, 255), 0, LWA_ALPHA);
        SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT);
        return DefWindowProc(hWnd, message, wParam, lParam);

    case WM_SIZE:
    case WM_MOVE:
    {
        LRESULT result = DefWindowProc(hWnd, message, wParam, lParam);

        if (GetForegroundWindow() == hWnd)
        {
            int prevBottom = defaultWindowPos.bottom;
            GetWindowRect(hWnd, &defaultWindowPos);
            if (focusMode) defaultWindowPos.bottom = prevBottom;
        }

        return result;
    }

    case WM_TIMER:
        switch (wParam)
        {
        case IDT_REDRAW:
            if (focusMode)
            {
                HWND hwndForeground = GetForegroundWindow();
                if (hwndForeground != hWnd)
                {
                    RECT rectForeground;
                    GetWindowRect(hwndForeground, &rectForeground);

                    RECT rectPoS;
                    GetWindowRect(hWnd, &rectPoS);

                    if (rectPoS.left != rectForeground.left ||
                        rectPoS.top != rectForeground.top ||
                        rectPoS.right != rectForeground.right ||
                        rectPoS.bottom != rectForeground.bottom)
                    {
                        SetWindowPos(hWnd, HWND_TOPMOST, rectForeground.left, rectForeground.top, rectForeground.right - rectForeground.left, rectForeground.bottom - rectForeground.top, SWP_NOACTIVATE);
                    }
                }
            }
            
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_ERASEBKGND:
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rect;
        GetWindowRect(hWnd, &rect);

        HWND hwndDesktop = GetDesktopWindow();
        HDC hdcDesktop = GetWindowDC(hwndDesktop);
        POINT clientPoint = { 0, 0 };
        ClientToScreen(hWnd, &clientPoint);

        BitBlt(hdc, 0, 0, rect.right - rect.left + 1, rect.bottom - rect.top + 1, hdcDesktop, clientPoint.x, clientPoint.y, SRCCOPY);

        ReleaseDC(hwndDesktop, hdcDesktop);
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* lpMMI = (MINMAXINFO*)lParam;
        lpMMI->ptMinTrackSize.x = POS_MIN_WIDTH;
        if (focusMode)
        {
            lpMMI->ptMinTrackSize.y = GetSystemMetrics(SM_CYCAPTION);
            // Prevent vertical sizing if the PoS window has the focus
            if (hWnd == GetForegroundWindow())
                lpMMI->ptMaxTrackSize.y = lpMMI->ptMinTrackSize.y;
        }
        else
            lpMMI->ptMinTrackSize.y = POS_MIN_HEIGHT;
    }
    break;

    case WM_DESTROY:
        SaveSettings();
        PostQuitMessage(0);
        break;

    case WM_SYSCOMMAND:
        if (wParam == IDC_OPTIONS)
        {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        }
        // fall through

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        SendMessage(GetDlgItem(hDlg, IDC_FOCUS_MODE), BM_SETCHECK, focusMode ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessage(GetDlgItem(hDlg, IDC_FIXED_MODE), BM_SETCHECK, !focusMode ? BST_CHECKED : BST_UNCHECKED, 0);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            if (!focusMode) moveToDefaultWindowPos = true;
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }

        if (LOWORD(wParam) == IDC_FOCUS_MODE || LOWORD(wParam) == IDC_FIXED_MODE)
        {
            UINT checkState = (UINT) SendMessage(GetDlgItem(hDlg, IDC_FOCUS_MODE), BM_GETCHECK, 0, 0);
            focusMode = checkState == BST_CHECKED;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void LoadSettings()
{
    try
    {
        winreg::RegKey key{ HKEY_CURRENT_USER, L"SOFTWARE\\PortionOfScreen" };
        defaultWindowPos.left = key.GetDwordValue(L"Left");
        defaultWindowPos.top = key.GetDwordValue(L"Top");
        defaultWindowPos.right = key.GetDwordValue(L"Right");
        defaultWindowPos.bottom = key.GetDwordValue(L"Bottom");
        winreg::RegExpected<DWORD> focusExpected = key.TryGetDwordValue(L"FocusMode");
        focusMode = focusExpected.IsValid() ? (bool) focusExpected.GetValue() : true;
    }
    catch(...)
    {
        defaultWindowPos.left = 100;
        defaultWindowPos.top = 100;
        defaultWindowPos.right = 900;
        defaultWindowPos.bottom = 700;
        focusMode = true;
    }
}

void SaveSettings()
{
    winreg::RegKey key{ HKEY_CURRENT_USER, L"SOFTWARE\\PortionOfScreen" };
    key.SetDwordValue(L"Left", defaultWindowPos.left);
    key.SetDwordValue(L"Top", defaultWindowPos.top);
    key.SetDwordValue(L"Right", defaultWindowPos.right);
    key.SetDwordValue(L"Bottom", defaultWindowPos.bottom);
    key.SetDwordValue(L"FocusMode", (DWORD) focusMode);
}
