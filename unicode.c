//
// Created by shuihe on 2022/9/7.
//

#include "unicode.h"

#include "winFrame.h"

LRESULT CALLBACK UnicodeConverterWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:

        case WM_PAINT:
            break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void UnicodeConverter(HWND mainHwnd, HINSTANCE hAppInstance) {
    CreateSingleWinFrame("UnicodeConverter", 600, 500, mainHwnd, hAppInstance, UnicodeConverterWndProc);
}
