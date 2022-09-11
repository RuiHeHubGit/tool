//
// Created by shuihe on 2022/9/7.
//

#include "winFrame.h"

HWND
CreateSingleWinFrame(LPCSTR title, int width, int height, HWND parentHwnd, HINSTANCE hInstance, WNDPROC lpWndProc) {
    static int id = 0;
    HWND hwnd = 0;
    WNDCLASS wndclass;
    char winClassName[30] = {0};
    char winCaption[200] = {0};

    sprintf(winClassName, "WindowsTool_%d", ++id);
    sprintf(winCaption, " %s\r", title);

    HWND preHwnd = FindWindow(NULL, title);
    if (preHwnd) {
        ShowWindow(preHwnd, SW_SHOWDEFAULT);
        SetForegroundWindow(preHwnd);
        return preHwnd;
    }

    wndclass.style = CS_BYTEALIGNCLIENT;
    wndclass.lpfnWndProc = lpWndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wndclass.hCursor = (HICON) LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = winClassName;

    if (hwnd == NULL && !RegisterClass(&wndclass)) {
        MessageBox(NULL, TEXT("This program requires Windows NT!"),
                   winCaption, MB_ICONERROR);
        return NULL;
    }


    int scw = GetSystemMetrics(SM_CXSCREEN);
    int sch = GetSystemMetrics(SM_CYSCREEN);

    hwnd = CreateWindow(winClassName, title,
                        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                        (scw - width) / 2, (sch - height) * (1 - 0.618),
                        width, height,
                        parentHwnd, NULL, hInstance, NULL);
    SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW);

    ShowWindow(hwnd, 1);
    UpdateWindow(hwnd);
    return hwnd;
}