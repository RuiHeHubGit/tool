#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include <string.h>
#include <stdbool.h>
#include<commdlg.h>
#include<shlobj.h>

#include "openProjects.h"
#include "winFrame.h"
#include "levenshtein.h"

static Project *projects[MAX_PROJECT_COUNT];
static INT projectCount = 0;

static Property *properties[100];
static int propertyCount = 0;

static char ideaPath[MAX_PATH] = {0};
static char projectsRootPath[MAX_PATH] = {0};
static int setTopMostThreadRunning = 0;
static BOOL updateRows = FALSE;
static HINSTANCE mainAppInstance;

HWND InitTable(HWND pHwnd);

int GetSubText(const char *source, const char *key, const char *keyEnd, char *target, int maxTargetLen);

int TryAddProject(LPCSTR path, LPCSTR dirName, void *result);

int TryGetAppId(LPCSTR path, LPCSTR dirName, Project *project);

void EnumerateDirectory(LPCSTR path, int (*callBack)(LPCSTR, LPCSTR, void *), void *result);

bool AlreadyAdd(char path[260]);

void OnNotify(HWND hwnd, WPARAM wParam, LPARAM lParam);

void OpenProjectCommand(const char *appName, const char *cmd);

char Tolower(char c);

int ReadTextFile(const char *path, int (*callback)(const char *filePath, void *p), void *p);

int GetPomInfoCallback(const char *line, Project *project);

int GetGitUrl(const char *line, Project *project);

int GetGitBranch(const char *line, Project *project);

void ShowProjectDetail(HWND pHwnd, int index);

void OpenGitUrl(Project *pProject);

void ShowFileInExplorer(Project *pProject);

DWORD WINAPI LoadOpenProjectData(HWND pHwnd);

void SelectPath(HWND hwnd, char *path);

int SelectFile(HWND hwnd, char *fileName, int maxSize);

void SetProperty(char *key, char *value);

void loadProperties(const char *path, struct Property *property[100]);

void GetProperty(const char *key, char pvavalueath[260]);

void SaveProperties(const char *path, Property *properties[100]);

void CleanProjects();

void GetProjectDesc(Project *pProject);

HWND CreateFindTextEdit(HWND pHwnd);

void OnEditTextChange(HWND pHwnd, HWND pHwnd1);

int TryAddProject(LPCSTR path, LPCSTR dirName, void *result) {
    if (projectCount == MAX_PROJECT_COUNT) {
        return 0;
    }

    char filePath[MAX_PATH];
    sprintf(filePath, "%s\\%s\\pom.xml", path, dirName);

    if (AlreadyAdd(filePath)) {
        return 0;
    }

    Project *newProject = calloc(1, sizeof(Project));

    int find = ReadTextFile(filePath, (int (*)(const char *, void *)) GetPomInfoCallback, newProject);
    if (find) {
        strcpy(newProject->path, filePath);
        strcpy(newProject->name, dirName);

        sprintf(filePath, "%s\\%s", path, dirName);
        EnumerateDirectory(filePath, (int (*)(LPCSTR, LPCSTR, void *)) TryGetAppId, newProject);
        if (newProject->appId[0] == 0) {
            TryGetAppId(filePath, "", newProject);
        }

        if (newProject->appId[0] != 0) {
            GetProjectDesc(newProject);
        }

        // 获取git仓库地址
        sprintf(filePath, "%s\\%s\\.git\\config", path, dirName);
        ReadTextFile(filePath, (int (*)(const char *, void *)) GetGitUrl, newProject);

        // 获取git分支
        sprintf(filePath, "%s\\%s\\.git\\HEAD", path, dirName);
        ReadTextFile(filePath, (int (*)(const char *, void *)) GetGitBranch, newProject);

        projects[projectCount++] = newProject;
    }
    return 0;
}

void GetProjectDesc(Project *pProject) {
    char key[30] = {};
    sprintf(key, "app.%s.desc", pProject->appId);
    for (int i = 0; i < propertyCount; ++i) {
        Property *property = properties[i];
        if (strcmp(key, property->key) == 0) {
            strcpy(pProject->desc, property->value);
            return;
        }
    }
}

int GetGitUrl(LPCSTR line, Project *project) {
    char strBuff[MAX_URL_LENGTH] = {0};
    int find = GetSubText(line, "url=git@", ".git", strBuff, MAX_URL_LENGTH);
    if (find) {
        for (int i = 0; i < strBuff[i] != 0; ++i) {
            if (strBuff[i] == ':') {
                strBuff[i] = '/';
            }
        }

        sprintf(project->gitUrl, "http://%s", strBuff);
        return 1;
    }

    find = GetSubText(line, "url=", "\n", strBuff, MAX_URL_LENGTH);
    if (find) {
        strcpy(project->gitUrl, strBuff);
        return 1;
    }

    return 0;
}

int GetGitBranch(LPCSTR line, Project *project) {
    char strBuff[MAX_PROJECT_ITEM_TEXT_SIZE] = {0};
    if (GetSubText(line, "ref:refs/heads/", "\0", strBuff, MAX_URL_LENGTH)) {
        strcpy(project->gitBranch, strBuff);
        return 1;
    }

    return 0;
}


int GetPomInfoCallback(const char *line, Project *project) {
    char strBuff[100] = {0};
    if (GetSubText(line, "<groupId>", "<", strBuff, MAX_PROJECT_ITEM_TEXT_SIZE)) {
        strcpy(project->groupId, strBuff);
        return 0;
    }

    if (GetSubText(line, "<artifactId>", "<", strBuff, MAX_PROJECT_ITEM_TEXT_SIZE)) {
        strcpy(project->artifactId, strBuff);
    }

    if (project->groupId[0] != 0 && project->artifactId[0] != 0) {
        return 1;
    }

    return 0;
}

void EnumerateDirectory(LPCSTR path, int (*callBack)(LPCSTR, LPCSTR, void *), void *result) {
    WIN32_FIND_DATA findFileData;
    HANDLE hListFile;

    CHAR szFilePath[MAX_PATH];

    // 构造代表子目录和文件夹路径的字符串，使用通配符"*"
    lstrcpy(szFilePath, path);
    lstrcat(szFilePath, "\\*");

    // 查找第一个文件/目录，获得查找句柄
    hListFile = FindFirstFile(szFilePath, &findFileData);
    // 判断句柄
    if (hListFile == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (findFileData.cFileName[0] == '.') {
            continue;
        }

        // 判断文件属性，是否为目录
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (callBack(path, findFileData.cFileName, result)) {
                return;
            }
        }
    } while (FindNextFile(hListFile, &findFileData));
}

bool AlreadyAdd(char *path) {
    for (int i = 0; i < projectCount; ++i) {
        if (strcmp(projects[i]->path, path) == 0) {
            return 1;
        }
    }
    return 0;
}

int GetAppIdCallback(const char *line, Project *project) {
    char appIdStr[15] = {0};
    if (GetSubText(line, "app.id=", "\n", appIdStr, 12)) {
        strcpy(project->appId, appIdStr);
        return 1;
    }

    return 0;
}

int TryGetAppId(LPCSTR path, LPCSTR dirName, Project *project) {
    char filePath[MAX_PATH];
    sprintf(filePath, "%s\\%s\\src\\main\\resources\\META-INF\\app.properties", path, dirName);
    ReadTextFile(filePath, (int (*)(const char *, void *)) GetAppIdCallback, project);
    return 0;
}

int ReadTextFile(const char *filePath, int (*callback)(const char *line, void *p), void *p) {
    int flag = 0;
    FILE *fp = fopen(filePath, "r");
    if (fp != NULL) {
        char line[MAX_LINE_SIZE];
        while (fgets(line, 100, fp)) {
            if (callback(line, p)) {
                flag = 1;
                break;
            }
        }
        fclose(fp);
    }
    return flag;
}

int GetSubText(const char *source, const char *key, const char *keyEnd, char *target, int maxTargetLen) {
    int keyIndex = 0;
    int findStartKey = 0;
    int findEndKey = 0;
    int tLen = 0;

    target[0] = 0;

    if (maxTargetLen < 2) {
        return 0;
    }

    for (int i = 0; i < MAX_LINE_SIZE; ++i) {
        char c = source[i];

        // 空格字符不会作为字符串开始
        if (target[0] == 0 && (c == ' ' || c == '\t')) {
            continue;
        }

        // 找到开始
        if (!findStartKey) {
            if (Tolower(c) == Tolower(key[keyIndex])) {
                ++keyIndex;
                if (key[keyIndex] == 0) {
                    findStartKey = 1;
                    continue;
                }
            } else {
                keyIndex = 0;
            }
        }

        if (findStartKey) {
            // 找到结束
            if (keyEnd != NULL) {
                for (int j = 0; i + j < MAX_LINE_SIZE; ++j) {
                    if (Tolower(keyEnd[j]) != Tolower(source[i + j])) {
                        break;
                    }

                    if (keyEnd[j + 1] == 0) {
                        findEndKey = 1;
                        c = 0;
                        break;
                    }
                }
            }

            if (tLen == maxTargetLen - 1) {
                break;
            }

            if (c == '\n' || c == '\r') {
                continue;
            }

            if (c == 0) {
                break;
            }

            target[tLen++] = c;
        }

        if (c == 0) {
            break;
        }
    }

    if (keyEnd != NULL && !findEndKey) {
        target[0] = 0;
        return 0;
    }

    target[tLen] = 0;

    // 去除末尾空格字符
    for (int i = tLen - 1; i >= 0; --i) {
        if (target[i] == ' ') {
            target[i] = 0;
        } else {
            break;
        }
    }

    return 1;
}

char Tolower(char c) {
    if (c >= 'A' && c <= 'Z')
        return c + 32;
    else
        return c;
}

HWND CreateListView(HWND hwndParent) {

    RECT rcClient;

    GetClientRect(hwndParent, &rcClient);

    HWND hWndListView = CreateWindow(WC_LISTVIEW,
                                     TEXT("Projects"),
                                     WS_CHILD | LVS_REPORT | WS_VISIBLE | LVS_EDITLABELS,
                                     0, 20,
                                     rcClient.right - rcClient.left,
                                     rcClient.bottom - rcClient.top - 20,
                                     hwndParent,
                                     (HMENU) IDC_LIST_VIEW,
                                     NULL,
                                     NULL);

    // 整行选中风格
    SendMessage(hWndListView, LVM_FIRST + 54, 0, 32);
    return hWndListView;
}

BOOL InitListViewColumns(HWND hListview) {
    // 设置ListView的列
    LVCOLUMN vcl;
    vcl.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    vcl.pszText = "appId"; //列标题
    vcl.cx = 92;           //列宽
    vcl.iSubItem = 0;      //子项索引，第一列无子项
    ListView_InsertColumn(hListview, 0, &vcl);

    vcl.pszText = "name";
    vcl.cx = 300;
    vcl.iSubItem = 1;
    ListView_InsertColumn(hListview, 1, &vcl);

    vcl.pszText = "desc";
    vcl.cx = 260;
    vcl.iSubItem = 2;
    ListView_InsertColumn(hListview, 2, &vcl);

    vcl.pszText = "branch";
    vcl.cx = 200;
    vcl.iSubItem = 3;
    ListView_InsertColumn(hListview, 3, &vcl);

    return TRUE;
}

BOOL AddListViewRows(HWND hListview) {

    LVITEM vitem;
    vitem.mask = LVIF_TEXT;

    for (int i = 0; i < projectCount; i++) {
        Project project = *projects[i];
        vitem.pszText = project.appId;
        vitem.iItem = i;
        vitem.iSubItem = 0;
        ListView_InsertItem(hListview, &vitem);

        vitem.iSubItem = 1;
        vitem.pszText = project.name;
        ListView_SetItem(hListview, &vitem);

        vitem.iSubItem = 2;
        vitem.pszText = project.desc;
        ListView_SetItem(hListview, &vitem);

        vitem.iSubItem = 3;
        vitem.pszText = project.gitBranch;
        ListView_SetItem(hListview, &vitem);
    }

    return TRUE;
}

HWND InitTable(HWND hwnd) {
    HWND hWndListView = CreateListView(hwnd);
    InitListViewColumns(hWndListView);
    return hWndListView;
}

DWORD WINAPI LoadOpenProjectData(HWND pHwnd) {

    int updateProperties = 0;

    char propertiesFilePath[MAX_PATH] = {0};
    GetCurrentDirectory(MAX_PATH, propertiesFilePath);
    strcat(propertiesFilePath, "\\config.properties");

    loadProperties(propertiesFilePath, properties);

    char path[MAX_PATH] = {0};
    GetProperty("PROJECT_ROOT_PATH", path);
    if (path[0] == 0) {
        projectCount = 0;
        SelectPath(pHwnd, path);
        SetProperty("PROJECT_ROOT_PATH", path);
        updateProperties = 1;
    }
    strcpy(projectsRootPath, path);
    if (projectsRootPath[0] != 0) {
        SetWindowText(pHwnd, path);
    }

    if (projectCount > 0 && strcpy(path, projectsRootPath) != 0) {
        CleanProjects();
    }

    GetProperty("IDEA_APP_PATH", path);
    if (path[0] == 0) {
        SelectFile(pHwnd, path, MAX_PATH);
        SetProperty("IDEA_APP_PATH", path);
        updateProperties = 1;
    }
    strcpy(ideaPath, path);

    if (updateProperties) {
        SaveProperties(propertiesFilePath, properties);
    }

    EnumerateDirectory(projectsRootPath, TryAddProject, NULL);
    updateRows = TRUE;
    SendMessageA(pHwnd, WM_PAINT, 0, 0);

    return 0;
}

void CleanProjects() {
    for (int i = 0; i < projectCount; ++i) {
        Project *p = projects[i];
        projects[i] = NULL;
        free(p);
    }
    projectCount = 0;
}

void SaveProperties(const char *path, struct Property *properties[100]) {
    FILE *fp = fopen(path, "w");
    if (fp != NULL) {
        for (int i = 0; i < propertyCount; i++) {
            Property *property = properties[i];
            fprintf(fp, "%s=%s\n", property->key, property->value);
        }
        fclose(fp);
    }
}

void GetProperty(const char *key, char *value) {
    value[0] = 0;
    for (int i = 0; i < propertyCount; ++i) {
        if (strcmp(properties[i]->key, key) == 0) {
            strcpy(value, properties[i]->value);
            break;
        }
    }
}

void SetProperty(char *key, char *value) {
    for (int i = 0; i < propertyCount; ++i) {
        Property *p = properties[i];
        if (strcmp(p->key, key) == 0) {
            strcpy(p->value, value);
            return;
        }
    }

    Property *p = calloc(1, sizeof(Property));
    strcpy(p->key, key);
    strcpy(p->value, value);
    properties[propertyCount++] = p;
}

int LoadPropertyCallback(const char *line, Property *properties[100]) {
    int preSpace = 0;
    for (; preSpace < MAX_PROPERTY_VALUE_SIZE; ++preSpace) {
        char c = line[preSpace];
        if (c == 0) {
            break;
        }

        if (c == '#') {
            return 0;
        }

        if (c != ' ' && c != '\t') {
            break;
        }
    }

    Property *property = calloc(1, sizeof(Property));

    char key[MAX_PROPERTY_KEY_SIZE + 1] = {0};
    int len = strlen(line);
    for (int i = preSpace, j = 0; i < len; ++i) {
        char c = line[i];
        if (c == '=') {
            key[j] = 0;
            break;
        }
        key[j++] = c;
    }

    strcpy(property->key, key);
    strcat(key, "=");
    if (GetSubText(line, key, "\0", property->value, MAX_PROPERTY_VALUE_SIZE)) {
        properties[propertyCount++] = property;
    } else {
        free(property);
    }

    return 0;
}

void loadProperties(const char *path, Property *properties[100]) {
    for (int i = 0; i < propertyCount; ++i) {
        Property *p = properties[i];
        properties[i] = NULL;
        free(p);
    }
    propertyCount = 0;
    ReadTextFile(path, (int (*)(const char *, void *)) LoadPropertyCallback, properties);
}

int SelectFile(HWND hwnd, char *fileName, int maxSize) {
    strcpy(fileName, "idea64.exe");

    OPENFILENAME opfn;
    //初始化
    ZeroMemory(&opfn, sizeof(OPENFILENAME));
    opfn.lStructSize = sizeof(OPENFILENAME);//结构体大小
    opfn.lpstrTitle = TEXT("Select Idea exe");
    //设置过滤
    opfn.lpstrFilter = TEXT("Idea\0*.exe\0");
    //默认过滤器索引设为1
    opfn.nFilterIndex = 1;
    //文件名的字段必须先把第一个字符设为 \0
    opfn.lpstrFile = fileName;
    opfn.lpstrFile[0] = '\0';
    opfn.nMaxFile = maxSize;
    //设置标志位，检查目录或文件是否存在
    opfn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    opfn.hwndOwner = hwnd;
    // 显示对话框让用户选择文件
    GetOpenFileName(&opfn);

    return 0;
}

void SelectPath(HWND pHwnd, char *path) {
    BROWSEINFO bi;
    ZeroMemory(&bi, sizeof(BROWSEINFO));
    bi.hwndOwner = pHwnd;
    bi.lpszTitle = TEXT("Select project root path");
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (NULL == pidl) {
        return;
    }

    if (SHGetPathFromIDList(pidl, path)) {
        SetWindowText(pHwnd, path);
    }
}

void PopListViewMenu(HWND hwnd, WPARAM wParam, LPARAM lParam, int index) {
    if (index < 0 || index >= projectCount) {
        return;
    }

    static HMENU menu;
    if (menu == NULL) {
        menu = CreatePopupMenu();

        AppendMenu(menu, MF_STRING, 1001, TEXT("open in Idea"));
        AppendMenu(menu, MF_STRING, 1002, TEXT("open git url"));
        AppendMenu(menu, MF_STRING, 1003, TEXT("open in Explorer"));
        AppendMenu(menu, MF_STRING, 1004, TEXT("detail info"));
    }

    POINT cursorPoint;
    GetCursorPos(&cursorPoint);
    BOOL ret = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_HORIZONTAL, cursorPoint.x, cursorPoint.y,
                              0, hwnd, NULL);

    Project *project = projects[index];
    switch (ret) {
        case 1001:
            OpenProjectCommand(ideaPath, project->path);
            break;
        case 1002:
            OpenGitUrl(project);
            break;
        case 1003:
            ShowFileInExplorer(project);
            break;
        case 1004:
            ShowProjectDetail(hwnd, index);
            return;
    }

    if (ret > 0) {
        SendMessageA(hwnd, WM_CLOSE, 0, 0);
    }
}

void ShowFileInExplorer(Project *pProject) {
    char params[MAX_PATH + 13] = {0};
    sprintf(params, "/e,/select, %s", pProject->path);
    ShellExecute(NULL, NULL, "explorer.exe", params, NULL, SW_SHOW);
}

static Project *currentShowProject;

HWND InitDetailListView(HWND hwnd) {
    RECT rcClient;

    GetClientRect(hwnd, &rcClient);

    HWND hWndListView = CreateWindow(WC_LISTVIEW,
                                     TEXT("Projects"),
                                     WS_CHILD | LVS_REPORT | WS_VISIBLE | LVS_EDITLABELS,
                                     0, 0,
                                     rcClient.right - rcClient.left,
                                     rcClient.bottom - rcClient.top,
                                     hwnd,
                                     NULL,
                                     NULL,
                                     NULL);

    // 整行选中风格
    SendMessage(hWndListView, LVM_FIRST + 54, 0, 32);


    // 设置ListView的列
    LVCOLUMN vcl;
    vcl.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    vcl.pszText = "item"; //列标题
    vcl.cx = 100;           //列宽
    vcl.iSubItem = 0;      //子项索引，第一列无子项
    ListView_InsertColumn(hWndListView, 0, &vcl);

    vcl.pszText = "object";
    vcl.cx = rcClient.right - rcClient.left - 100;
    vcl.iSubItem = 1;
    ListView_InsertColumn(hWndListView, 1, &vcl);

    return hWndListView;
}

void AddDetailListViewRows(HWND hWndListView) {
    Project p = *currentShowProject;
    char *items[] = {
            "appId", p.appId,
            "name", p.name,
            "groupId", p.groupId,
            "artifactId", p.artifactId,
            "path", p.path,
            "gitUrl", p.gitUrl,
            "gitBranch", p.gitBranch,
            "desc", p.desc
    };

    int size = sizeof(items) / sizeof(char *);

    // 设置行
    for (int i = 0; i < size; i += 2) {
        LVITEM vitem;
        vitem.mask = LVIF_TEXT;

        vitem.pszText = items[i];
        vitem.iItem = i / 2;
        vitem.iSubItem = 0;
        ListView_InsertItem(hWndListView, &vitem);
        vitem.iSubItem = 1;
        vitem.pszText = items[i + 1];
        ListView_SetItem(hWndListView, &vitem);
    }
}

LRESULT CALLBACK ShowDetailWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hWndDetailListView;
    static BOOL updateRows = FALSE;

    switch (message) {
        case WM_CREATE:
            hWndDetailListView = InitDetailListView(hwnd);
            updateRows = TRUE;
            break;
        case WM_PAINT:
            if (updateRows) {
                updateRows = FALSE;
                AddDetailListViewRows(hWndDetailListView);
            }
            break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void ShowProjectDetail(HWND pHwnd, int index) {
    currentShowProject = projects[index];
    HWND hwnd = CreateSingleWinFrame(currentShowProject->name, 600, 200, pHwnd, mainAppInstance, ShowDetailWndProc);

    int sch = GetSystemMetrics(SM_CYSCREEN);
    SetWindowPos(hwnd, HWND_TOPMOST, 20, (int) hwnd % (sch / 2), 0, 0, SWP_NOSIZE);
}

void OpenGitUrl(Project *project) {
    if (project->gitUrl[0] != 0) {
        ShellExecute(NULL, TEXT("open"), project->gitUrl, NULL, NULL, SW_SHOW);
    }
}

void OnNotify(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    NMHDR *pNmHdr = (NMHDR *) lParam;
    if (pNmHdr != NULL && pNmHdr->idFrom == 1) {
        LPNMLISTVIEW pNMLV = (LPNMLISTVIEW) lParam;
        if (pNmHdr->code == NM_DBLCLK) {
            if (pNMLV->iItem < 0 || pNMLV->iItem >= projectCount) {
                return;
            }
            // idea打开项目
            OpenProjectCommand(ideaPath, projects[pNMLV->iItem]->path);
        }

        if (pNmHdr->code == NM_RCLICK) {
            PopListViewMenu(hwnd, wParam, lParam, pNMLV->iItem);
        }
    }
}

DWORD WINAPI SetOpenProjectWindTopMost(LPVOID lpPara) {
    for (int i = 0; i < 5; ++i) {
        Sleep(1000);
        HWND hOpenWin = FindWindow(NULL, TEXT("Open Project"));
        if (hOpenWin) {
            SetWindowPos(hOpenWin, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
    }
    setTopMostThreadRunning = 0;
    return 0;
}

void OpenProjectCommand(const char *appName, const char *cmd) {
    PROCESS_INFORMATION info;
    STARTUPINFO si = {0};
    char openCmd[520] = {0};

    sprintf(openCmd, "\"%s\" \"%s\"", appName, cmd);

    if (!CreateProcess(NULL, (LPTSTR) openCmd,
                       NULL, NULL, FALSE,
                       CREATE_NO_WINDOW, NULL, NULL,
                       &si, &info)) {
        return;
    }

    CloseHandle(info.hProcess);
    CloseHandle(info.hThread);

    if (!setTopMostThreadRunning) {
        setTopMostThreadRunning = 1;
        CreateThread(NULL, 0, SetOpenProjectWindTopMost, NULL, 0, NULL);
    }
}

HWND CreateFindTextEdit(HWND pHwnd) {
    RECT rcClient;

    GetClientRect(pHwnd, &rcClient);

    HWND hwndEdit = CreateWindow(TEXT("EDIT"), TEXT(""), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                                 0, 0, (rcClient.right - rcClient.left) / 3, 20, pHwnd, (HMENU) IDC_FIND_EDIT, NULL,
                                 NULL);
    return hwndEdit;
}

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
    Project *project1 = (Project *)lParam1;
    Project *project2 = (Project *)lParam2;
    return project2->sort - project1->sort;
}

void OnEditTextChange(HWND pHwndEdit, HWND pHwndListView) {
    char text[21] = {0};
    GetWindowText(pHwndEdit, text, 20);

    RECT rcClient;
    GetClientRect(pHwndListView, &rcClient);
	
	int index = -1;

    for (int i = 0; i < projectCount; ++i) {
        Project *project = projects[i];
        int t1 = levenshteinTwoRows(text, strlen(text), project->appId, strlen(project->appId));
        int t2 = levenshteinTwoRows(text, strlen(text), project->name, strlen(project->name));
        project->sort = max(t1, t2);
    }

    ListView_SortItems(pHwndListView, CompareFunc, NULL);

/*    SNDMSG((pHwndListView), LVM_SETHOTITEM, (WPARAM) (i), 0);
    ListView_Scroll(pHwndListView, 0, (int) (1.0 * rcClient.bottom - rcClient.top) * i / projectCount - 20);*/
}


LRESULT CALLBACK OpenProjectsWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hWndListView;
    static HWND hWndFindEdit;

    switch (message) {
        case WM_CREATE:
            hWndListView = InitTable(hwnd);
            hWndFindEdit = CreateFindTextEdit(hwnd);
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) LoadOpenProjectData,
                         hwnd, 0, NULL);
            SetFocus(hWndFindEdit);
        case WM_PAINT:
            if (updateRows) {
                updateRows = false;
                AddListViewRows(hWndListView);
            }
            break;
        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_FIND_EDIT && HIWORD(wParam) == EN_CHANGE) {
                OnEditTextChange(hWndFindEdit, hWndListView);
            }
            /*BOOL bCtrl = GetKeyState(VK_CONTROL);
            if (bCtrl && (wParam == 'a' || wParam == 'A')) {
                SetFocus(hWndFindEdit);
                SendMessage(hWndFindEdit, EM_SETSEL, -1, 0);
            }*/
        }
        case WM_NOTIFY:
            if (LOWORD(wParam) == IDC_LIST_VIEW) {
                OnNotify(hwnd, wParam, lParam);
            }
            break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void OpenProjectsWindow(HWND hwndParent, HINSTANCE hAppInstance) {
    mainAppInstance = hAppInstance;
    CreateSingleWinFrame("OpenProject", 900, 600, hwndParent, hAppInstance, OpenProjectsWndProc);
}