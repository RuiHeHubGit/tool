//
// Created by rui on 2022/9/12.
//
#include <stdio.h>
#include <string.h>
#include "util.h"

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
    int sourceLen = strlen(source);

    target[0] = 0;

    if (maxTargetLen < 2) {
        return 0;
    }

    for (int i = 0; i < sourceLen; ++i) {
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
                for (int j = 0; i + j < sourceLen; ++j) {
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