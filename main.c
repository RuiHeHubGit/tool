#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include <string.h>
#include <wingdi.h>
#include <dir.h>


#include "openProjects.h"
#include "md5.h"
#include "uuid.h"
#include "unicode.h"
#include "jsonFormat.h"
#include "download.h"
#include "util.h"


#define IDI_ICON1 101
#define HMENU_BUTTON_TOPMOST 1
#define HMENU_BUTTON_NOT_TOPMOST 2

#define UPDATE_ALL 0
#define UPDATE_TS 1

#define WD_WIDTH 220
#define WD_HEIGHT 220
#define MAX_TITLE_LEN 120
#define GWL_HINSTANCE (-6)

#define TEXT_WINDOWS_TITLE "tool"

#define MAX_VERSION_SIZE 20

static HINSTANCE hAppInstance;
static HMODULE hInst;
static CHAR wsCharBuf[MAX_TITLE_LEN];
static CHAR tsTextCharBuf[MAX_TITLE_LEN];
static CHAR wsTitleText[MAX_TITLE_LEN];
static INT tsScrollPos;

static HWND fhWnd;
static HWND fhwTextHWnd;
static HWND tsTextHWnd;
static HWND lastHwnd;
static HWND hScrollBar;

static HWND switchForegroundWindow;
static HWND switchForegroundWindowNext;

static INT scw;
static INT sch;
static INT wy = -1;

DWORD WINAPI CheckVersion(LPVOID lpPara);

void SetWindowTopmost(HWND hwnd, BOOL topmost) {
    if (hwnd == NULL) {
        return;
    }

    if (topmost) {
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    } else {
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

BOOL IsWindowTopmost(HWND hwnd) {
    return GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST;
}

byte GetTransparency(HWND hWnd) {
    BYTE bAlpha = 0;
    if (hInst == NULL) {
        hInst = LoadLibrary("User32.DLL");
    }
    if (hInst) {
        typedef BOOL(WINAPI *LAYERFUNC)(HWND hwnd, COLORREF *pcrKey, BYTE *pbAlpha, DWORD *pdwFlags);
        LAYERFUNC GetLayer = NULL;
        // GetLayeredWindowAttributes
        GetLayer = (LAYERFUNC) GetProcAddress(hInst, "GetLayeredWindowAttributes");
        if (GetLayer) {
            BOOL success = GetLayer(hWnd, NULL, &bAlpha, NULL);
            if (!success) {
                bAlpha = 255;
            }
        }
    }
    return bAlpha;
}

void SetWindowTransparency(HWND hwnd, BYTE alpha) {
    typedef BOOL(PASCAL *LAYERFUNC)(HWND, COLORREF, BYTE, DWORD);
    LAYERFUNC SetLayer;
    if (hInst == NULL) {
        hInst = LoadLibrary("User32.DLL");
    }
    if (hInst) {
        SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        SetLayer = (LAYERFUNC) GetProcAddress(hInst, "SetLayeredWindowAttributes");
        SetLayer(hwnd, 0, alpha, LWA_ALPHA);
    }
}

void SwitchTwoWindow() {
    if (switchForegroundWindow == NULL) {
        return;
    }

    HWND temp = switchForegroundWindowNext;
    switchForegroundWindowNext = switchForegroundWindow;
    switchForegroundWindow = temp;
    if (!ShowWindow(switchForegroundWindowNext, SW_SHOW)) {
        return;
    }

    SetForegroundWindow(switchForegroundWindowNext);
}

void SaveSwitchTwoWindows(HWND hwnd) {
    if (switchForegroundWindow == NULL) {
        switchForegroundWindow = hwnd;
    } else {
        switchForegroundWindowNext = hwnd;
    }
}

INT OnhScroll(HWND hScrollBar, INT *lpPos, WPARAM wParam) {
    INT pos = *lpPos;
    switch (LOWORD(wParam)) {
        case SB_LINEUP:
            pos -= 1;
            break;
        case SB_LINEDOWN:
            pos += 1;
            break;
        case SB_PAGEUP:
            pos -= 10;
            break;
        case SB_PAGEDOWN:
            pos += 10;
            break;
        case SB_THUMBTRACK:
            pos = HIWORD(wParam);
            break;
    }

    *lpPos = pos;
    SetScrollPos(hScrollBar, SB_CTL, pos, TRUE);
    return pos;
}

void InitScroll(HWND hScrollBar, LPSCROLLINFO lpSi) {
    lpSi->cbSize = sizeof(SCROLLINFO);
    lpSi->fMask = SIF_ALL;
    lpSi->nMin = 10;
    lpSi->nMax = 255;
    lpSi->nPage = 10;
    lpSi->nPos = 0;
    SetScrollInfo(hScrollBar, SB_CTL, lpSi, TRUE);
}

void InitView(HWND hwnd, HINSTANCE hinstance) {
    HWND hWnds[10];
    INT hCount = 0;

    sprintf(tsTextCharBuf, "Transparency(0-255):");

    hWnds[hCount++] = tsTextHWnd = CreateWindow(WC_STATIC, TEXT(tsTextCharBuf), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                                20, 10, 170, 20, hwnd, NULL, hinstance, NULL);

    hWnds[hCount++] = hScrollBar = CreateWindow(
            WC_SCROLLBAR, NULL, WS_VISIBLE | WS_CHILD | WS_TABSTOP | SBS_HORZ,
            20, 30, 170, 30, hwnd, NULL, hinstance, NULL);
    SetScrollRange(hScrollBar, SB_CTL, 0, 255, FALSE);

    hWnds[hCount++] = CreateWindow(WC_BUTTON, TEXT("Topmost"), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                   20, 70, 80, 30, hwnd, (HMENU) HMENU_BUTTON_TOPMOST, hinstance, NULL);

    hWnds[hCount++] = CreateWindow(WC_BUTTON, TEXT("NotTopmost"), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                   110, 70, 80, 30, hwnd, (HMENU) HMENU_BUTTON_NOT_TOPMOST, hinstance, NULL);

    hWnds[hCount++] = fhwTextHWnd = CreateWindow(WC_STATIC, TEXT(""), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                                 20, 110, 170, 65, hwnd, NULL, hinstance, NULL);

    HFONT hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
    for (size_t i = 0; i < hCount; i++) {
        SendMessage(hWnds[i], WM_SETFONT, (WPARAM) hFont, 1);
    }
}

void UpdateUI(HWND hwnd, INT mode) {
    if (mode == UPDATE_TS || mode == UPDATE_ALL) {
        INT alph = GetTransparency(fhWnd);
        tsScrollPos = alph;
        SetScrollPos(hScrollBar, SB_CTL, tsScrollPos, TRUE);
        sprintf(tsTextCharBuf, "Transparency(0-255):%d", alph);
        SetWindowTextA(tsTextHWnd, tsTextCharBuf);
    }

    if (mode == UPDATE_ALL) {
        GetWindowTextA(fhWnd, wsTitleText, MAX_TITLE_LEN);
        sprintf(wsCharBuf, "[%d, %s]\n%s", fhWnd, IsWindowTopmost(fhWnd) ? "Topmost" : "NotTopmost", wsTitleText);
        SetWindowTextA(fhwTextHWnd, wsCharBuf);
        SetScrollRange(hScrollBar, SB_CTL, 0, 255, TRUE);
        UpdateWindow(hwnd);
    }
}

void PaintUI(HWND hwnd) {
    HWND currentHwnd = GetForegroundWindow();
    if (currentHwnd == lastHwnd || currentHwnd == NULL) {
        return;
    }

    lastHwnd = currentHwnd;

    if (currentHwnd != hwnd || fhWnd == NULL) {
        fhWnd = currentHwnd;
        if (hwnd != fhWnd) {
            SaveSwitchTwoWindows(fhWnd);
        }
        UpdateUI(hwnd, UPDATE_ALL);
    }
}

void SetWindowTopmostMode(HWND fhWnd, WPARAM wParam) {
    if (LOWORD(wParam) == HMENU_BUTTON_TOPMOST) {
        SetWindowTopmost(fhWnd, TRUE);
    } else if (LOWORD(wParam) == HMENU_BUTTON_NOT_TOPMOST) {
        SetWindowTopmost(fhWnd, FALSE);
    }
}

void SwitchToToolWindow(HWND hwnd) {
    fhWnd = hwnd;
    UpdateWindow(hwnd);
    UpdateUI(hwnd, UPDATE_ALL);
}

BOOL ToolWindowIsBorderRight(HWND hwnd) {
    RECT rect;
    GetWindowRect(hwnd, &rect);
    return rect.right <= 5;
}

void MoveToolWindowToRightOrBorderLeft(HWND hwnd, UINT message, LPARAM lParam) {
    if (message == WM_LBUTTONUP || message == WM_LBUTTONDBLCLK) {
        scw = GetSystemMetrics(SM_CXSCREEN);
        sch = GetSystemMetrics(SM_CYSCREEN);
        BOOL border = ToolWindowIsBorderRight(hwnd);

        INT cx = LOWORD(lParam);
        if (message == WM_LBUTTONUP && cx > 10 && !border) {
            return;
        }

        if (wy == -1) {
            wy = (int) (sch * 0.618 - WD_HEIGHT / 2);
        }

        if (border) {
            border = FALSE;
        } else {
            border = TRUE;
        }
        SetWindowPos(hwnd, HWND_TOPMOST, border ? -WD_WIDTH + 4 : -4, wy, 0, 0, SWP_NOSIZE);
        UpdateWindow(hwnd);
    }
}

void Quit() {
    exit(0);
}

void HandleMenuAction(HWND hwnd, INT code) {
    switch (code) {
        case 1099:
            Quit();
            break;
        case 1001:
            OpenProjectsWindow(hwnd, hAppInstance);
            break;
        case 1003:
            UuidGenerator(hwnd, hAppInstance);
            break;
        case 1004:
            Md5Generator(hwnd, hAppInstance);
            break;
        case 1005:
            UnicodeConverter(hwnd, hAppInstance);
            break;
        case 1007:
            JsonFormat(hwnd, hAppInstance);
            break;
    }
}

void PopToolMenu(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    static HMENU menu;
    if (menu == NULL) {
        menu = CreatePopupMenu();
        AppendMenu(menu, MF_STRING, 1001, TEXT("OpenProject"));
        AppendMenu(menu, MF_STRING, 1002, TEXT("StartTomcat"));
        AppendMenu(menu, MF_STRING, 1003, TEXT("UUID"));
        AppendMenu(menu, MF_STRING, 1004, TEXT("MD5"));
        AppendMenu(menu, MF_STRING, 1005, TEXT("Unicode"));
        AppendMenu(menu, MF_STRING, 1006, TEXT("TimeStamp"));
        AppendMenu(menu, MF_STRING, 1007, TEXT("JsonFormat"));
        AppendMenu(menu, MF_STRING, 1008, TEXT("TextComparator"));
        AppendMenu(menu, MF_STRING, 1099, TEXT("Exit"));
    }

    POINT point;
    GetCursorPos(&point);
    BOOL ret = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_HORIZONTAL, point.x, point.y,
                              0, hwnd, NULL);
    HandleMenuAction(hwnd, ret);
}

void GetFileVersion(char *asVer) {
    char FileName[MAX_PATH] = {0};
    GetModuleFileName(NULL, FileName, MAX_PATH - 1);

    VS_FIXEDFILEINFO *pVsInfo;
    unsigned int iFileInfoSize = sizeof(VS_FIXEDFILEINFO);

    int iVerInfoSize = GetFileVersionInfoSize(FileName, NULL);
    if (iVerInfoSize != 0) {
        char pBuf[MAX_LINE_SIZE];
        if (GetFileVersionInfo(FileName, 0, iVerInfoSize, pBuf)) {
            if (VerQueryValue(pBuf, "\\", (void **) &pVsInfo, &iFileInfoSize)) {
                sprintf(pBuf, "%d.%d.%d.%d", HIWORD(pVsInfo->dwProductVersionMS), LOWORD(pVsInfo->dwProductVersionMS),
                        HIWORD(pVsInfo->dwProductVersionLS), LOWORD(pVsInfo->dwProductVersionLS));
                strcpy(asVer, pBuf);
            }
        }
    }
}

int GetReleaseVersion(const char *line, char *releaseVersion) {
    if (GetSubText(line, "VALUE\"ProductVersion\",\"", "\"", releaseVersion, MAX_VERSION_SIZE)) {
        return 1;
    }
    return 0;
}

int GetReleaseVersionDesc(const char *line, char *versionDesc) {
    if (GetSubText(line, "VALUE\"VersionDesc\",\"", "\"", versionDesc, MAX_LINE_SIZE)) {
        return 1;
    }
    return 0;
}


DWORD WINAPI CheckVersion(LPVOID lpPara) {
    char path[MAX_PATH];
    _getcwd(path, MAX_PATH);
    strcat(path, "//version");
    int result = DownloadToFile("https://raw.githubusercontent.com/RuiHeHubGit/tool/main/resource/res.rc", path);
    switch (result) {
        case S_OK: {
            printf("The download version successfully.\n");
            char currentVersion[MAX_VERSION_SIZE] = {0};
            GetFileVersion(currentVersion);
            char releaseVersion[MAX_VERSION_SIZE] = {0};
            ReadTextFile(path, (int (*)(const char *, void *)) GetReleaseVersion, releaseVersion);
            printf("currentVersion:%s\nreleaseVersion:%s\n", currentVersion, releaseVersion);

            if (strcmp(releaseVersion, currentVersion) > 0) {
                char releaseVersionDesc[MAX_LINE_SIZE] = {0};
                char versionText[MAX_LINE_SIZE] = {0};
                ReadTextFile(path, (int (*)(const char *, void *)) GetReleaseVersionDesc, releaseVersionDesc);

                sprintf(versionText, "是否下载新版本？\n新版本号：%s\n 升级内容：%s", releaseVersion,
                        releaseVersionDesc);
                if (IDYES == MessageBoxA((HWND) lpPara, TEXT(versionText), TEXT("发现新版本"), MB_YESNO)) {
                    _getcwd(path, MAX_PATH);
                    strcat(path, "//tool.exe");
                    strcat(path, releaseVersion);
                    result = DownloadToFile("https://raw.githubusercontent.com/RuiHeHubGit/tool/tool.exe", path);
                    if (result == S_OK) {
                        sprintf(versionText, "需要立即重启使用新版本吗？\n版本号：%s", releaseVersion);
                        if (IDYES == MessageBoxA((HWND) lpPara, TEXT(versionText), TEXT("新版本下载成功"), MB_YESNO)) {
                            sprintf(path, "\"%s\"", path);
                            ShellExecute(NULL, NULL, "explorer.exe", path, NULL, SW_SHOW);
                            exit(0);
                        }
                    } else {
                        MessageBoxA((HWND) lpPara, TEXT("下载新版本失败，稍后再试"), TEXT("升级失败"), MB_OK);
                    }
                }
            }
        }
            break;
        default:
            printf("CheckVersion error:%d\n", result);
            break;
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

    switch (message) {
        case WM_CREATE:
            InitView(hwnd, hAppInstance);
            break;
        case WM_PAINT:
            PaintUI(hwnd);
            break;
        case WM_HSCROLL:
            SetWindowTransparency(fhWnd, OnhScroll(hScrollBar, &tsScrollPos, wParam));
            UpdateUI(hwnd, UPDATE_TS);
            break;
        case WM_COMMAND:
            SetWindowTopmostMode(fhWnd, wParam);
            UpdateUI(hwnd, UPDATE_ALL);
            break;
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONUP:
            MoveToolWindowToRightOrBorderLeft(hwnd, message, lParam);
            break;
        case WM_RBUTTONUP:
            PopToolMenu(hwnd, wParam, lParam);
            break;
        case WM_MOUSEWHEEL:
            SwitchTwoWindow();
            UpdateWindow(hwnd);
            break;
        case WM_MOVE:
            wy = (int) (short) HIWORD(lParam) - 25;
            break;
        case WM_DESTROY:
            FreeLibrary(hInst);
            PostQuitMessage(0);
            Quit();
            break;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   PSTR szCmdLine, INT iCmdShow) {
    system("chcp 936 > nul");
    setbuf(stdout, NULL);

    hAppInstance = hInstance;

    static CHAR szAppName[] = TEXT("WindowsTool");

    HWND preHwnd = FindWindowA(szAppName, NULL);
    if (preHwnd) {
        ShowWindow(preHwnd, SW_SHOWDEFAULT);
        SetForegroundWindow(preHwnd);
        exit(0);
    }

    HWND hwnd;
    MSG msg;
    WNDCLASS wndclass;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wndclass.hInstance = hInstance;
    wndclass.lpfnWndProc = WndProc;
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = szAppName;
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;

    if (!RegisterClass(&wndclass)) {
        MessageBox(NULL, "ERROR", "System tip", MB_CANCELTRYCONTINUE);
        return 0;
    }

    scw = GetSystemMetrics(SM_CXSCREEN);
    sch = GetSystemMetrics(SM_CYSCREEN);

    hwnd = CreateWindow(szAppName, TEXT(TEXT_WINDOWS_TITLE), WS_SYSMENU,
                        -4, (int) (sch * 0.618 - WD_HEIGHT / 2), WD_WIDTH, WD_HEIGHT,
                        NULL, NULL, hInstance, NULL);

    HMENU hmenu = GetSystemMenu(hwnd, FALSE);
    RemoveMenu(hmenu, SC_MAXIMIZE, MF_BYCOMMAND);
    RemoveMenu(hmenu, SC_SIZE, MF_BYCOMMAND);
    RemoveMenu(hmenu, SC_RESTORE, MF_BYCOMMAND);
    RemoveMenu(hmenu, SC_MINIMIZE, MF_BYCOMMAND);
    DestroyMenu(hmenu);

    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    // 隐藏任务栏图标
    SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

    // 在屏幕上显示窗口
    ShowWindow(hwnd, iCmdShow);
    // 指示窗口自我更新
    UpdateWindow(hwnd);

    // 检查版本
    CreateThread(NULL, 0, CheckVersion, hwnd, 0, NULL);

    // 从消息队列中取得消息
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}