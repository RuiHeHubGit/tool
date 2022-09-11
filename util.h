//
// Created by rui on 2022/9/12.
//

#ifndef TOOL_UTIL_H
#define TOOL_UTIL_H

#endif //TOOL_UTIL_H

#define MAX_LINE_SIZE 1000
#define MAX_URL_LENGTH 1024

int ReadTextFile(const char *path, int (*callback)(const char *filePath, void *p), void *p);

int GetSubText(const char *source, const char *key, const char *keyEnd, char *target, int maxTargetLen);

char Tolower(char c);