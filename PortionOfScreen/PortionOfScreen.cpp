// PortionOfScreen.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "PortionOfScreen.h"
#include "WinReg.hpp"

#define MAX_LOADSTRING 100

#define IDT_REDRAW 101

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                LoadWindowPosition(RECT& rect);
void                SaveWindowPosition(RECT& rect);

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
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PORTIONOFSCREEN));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   RECT rect;
   LoadWindowPosition(rect);

   HWND hWnd = CreateWindowExW(
       WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
       szWindowClass,
       szTitle,
       WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX,
       rect.left,
       rect.top,
       rect.right - rect.left + 1,
       rect.bottom - rect.top + 1,
       nullptr,
       nullptr,
       hInstance,
       nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

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
        DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
        break;

    case WM_RBUTTONUP:
        SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
        break;

    case WM_SETFOCUS:
        SetLayeredWindowAttributes(hWnd, RGB(255, 255, 255), 128, LWA_ALPHA);
        SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOPMOST);
        return DefWindowProc(hWnd, message, wParam, lParam);

    case WM_KILLFOCUS:
        SetLayeredWindowAttributes(hWnd, RGB(255, 255, 255), 0, LWA_ALPHA);
        SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT);
        return DefWindowProc(hWnd, message, wParam, lParam);

    case WM_TIMER:
        switch (wParam)
        {
        case IDT_REDRAW:
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

    case WM_DESTROY:
    {
        RECT rect;
        GetWindowRect(hWnd, &rect);
        SaveWindowPosition(rect);
        PostQuitMessage(0);
    }
    break;

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
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void LoadWindowPosition(RECT& rect)
{
    try
    {
        winreg::RegKey  key{ HKEY_CURRENT_USER, L"SOFTWARE\\PortionOfScreen" };
        rect.left = key.GetDwordValue(L"Left");
        rect.top = key.GetDwordValue(L"Top");
        rect.right = key.GetDwordValue(L"Right");
        rect.bottom = key.GetDwordValue(L"Bottom");
    }
    catch(...)
    {
        rect.left = 100;
        rect.top = 100;
        rect.right = 900;
        rect.bottom = 700;
    }
}

void SaveWindowPosition(RECT& rect)
{
    winreg::RegKey  key{ HKEY_CURRENT_USER, L"SOFTWARE\\PortionOfScreen" };
    key.SetDwordValue(L"Left", rect.left);
    key.SetDwordValue(L"Top", rect.top);
    key.SetDwordValue(L"Right", rect.right);
    key.SetDwordValue(L"Bottom", rect.bottom);
}
