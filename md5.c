//
// Created by shuihe on 2022/9/7.
//

#include "md5.h"
#include "winFrame.h"

LRESULT CALLBACK Md5ConverterWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:

        case WM_PAINT:
            break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void Md5Generator(HWND mainHwnd, HINSTANCE hAppInstance) {
    CreateSingleWinFrame("Md5Generator", 600, 500, mainHwnd, hAppInstance, Md5ConverterWndProc);
}