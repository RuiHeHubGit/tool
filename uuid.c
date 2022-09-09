//
// Created by shuihe on 2022/9/7.
//
#include "uuid.h"
#include "winFrame.h"

LRESULT CALLBACK UuidGeneratorWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:

        case WM_PAINT:
            break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void UuidGenerator(HWND mainHwnd, HINSTANCE hAppInstance) {
    CreateSingleWinFrame("UuidGenerator", 400, 200, mainHwnd, hAppInstance, UuidGeneratorWndProc);
}