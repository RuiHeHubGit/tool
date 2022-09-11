//
// Created by shuihe on 2022/9/7.
//

#include <windows.h>
#include <stdio.h>

#ifndef CONFIG_PROPERTIES_WINFRAME_H
#define CONFIG_PROPERTIES_WINFRAME_H
#define IDI_ICON1 101

#endif //CONFIG_PROPERTIES_WINFRAME_H

HWND CreateSingleWinFrame(LPCSTR title, int width, int height, HWND parentHwnd, HINSTANCE hInstance, WNDPROC lpWndProc);

