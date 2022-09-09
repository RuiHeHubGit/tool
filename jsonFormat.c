//
// Created by shuihe on 2022/9/8.
//
#include "jsonFormat.h"
#include "winFrame.h"

LRESULT CALLBACK JsonFormatWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

    switch (message) {
        case WM_CREATE:
            break;
        case WM_PAINT:
            break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void JsonFormat(HWND mainHwnd, HINSTANCE hAppInstance) {
    CreateSingleWinFrame("JsonFormat", 800, 600, mainHwnd, hAppInstance, JsonFormatWndProc);
}
