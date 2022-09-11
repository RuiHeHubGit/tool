//
// Created by shuihe on 2022/9/3.
//
#include <windows.h>

#ifndef UNTITLED_OPENPROJECTS_H
#define UNTITLED_OPENPROJECTS_H

#define IDI_ICON1 101
#define GWL_HINSTANCE (-6)
#define LVM_SETHOTITEM (LVM_FIRST+60)

#define IDC_LIST_VIEW 1
#define IDC_SEARCH_EDIT 2
#define IDC_BTN_SELECT 3

#define MAX_PROJECT_COUNT 100
#define MAX_PROJECT_NAME 100
#define MAX_PROJECT_ITEM_TEXT_SIZE 100
#define MAX_URL_LENGTH 1024
#define MAX_PROPERTY_KEY_SIZE 100
#define MAX_PROPERTY_VALUE_SIZE 300

/*
 * app
 */
typedef struct Project {
    char appId[15];
    char groupId[MAX_PROJECT_NAME];
    char artifactId[MAX_PROJECT_NAME];
    char name[MAX_PROJECT_NAME];
    char path[MAX_PATH];
    char gitUrl[MAX_URL_LENGTH];
    char gitBranch[MAX_URL_LENGTH];
    char desc[MAX_PROJECT_NAME];
    int sort;
} Project;

typedef struct Property {
    char key[MAX_PROPERTY_KEY_SIZE];
    char value[MAX_PROPERTY_VALUE_SIZE];
} Property;

#endif //UNTITLED_OPENPROJECTS_H

void OpenProjectsWindow(HWND mainHwnd, HINSTANCE hAppInstance);
