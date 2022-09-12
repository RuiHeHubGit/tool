//
// Created by rui on 2022/9/12.
//
#include "download.h"

#include <Windows.h>
#include <wininet.h>
#include <tchar.h>
#include <stdio.h>

#pragma comment(lib, "wininet.lib")

int DownloadToFile(const char *url, const char *filePath) {
    /**
     *   解析网址为主机、端口和目标页面
     */

    TCHAR szHostName[128];
    TCHAR szUrlPath[256];
    URL_COMPONENTS crackedURL = { 0 };
    crackedURL.dwStructSize = sizeof (URL_COMPONENTS);
    crackedURL.lpszHostName = szHostName;
    crackedURL.dwHostNameLength = ARRAYSIZE(szHostName);
    crackedURL.lpszUrlPath = szUrlPath;
    crackedURL.dwUrlPathLength = ARRAYSIZE(szUrlPath);
    InternetCrackUrl(url, (DWORD)_tcslen(url), 0, &crackedURL);

    /**
     *   http请求相关初始化工作
     */
    HINTERNET hInternet = InternetOpen(TEXT("Microsoft InternetExplorer"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet == NULL)
        return -1;

    HINTERNET hHttpSession = InternetConnect(hInternet, crackedURL.lpszHostName, crackedURL.nPort, NULL, NULL,
                                             INTERNET_SERVICE_HTTP, 0, 0);
    if (hHttpSession == NULL)
    {
        InternetCloseHandle(hInternet);
        return -2;
    }

    HINTERNET hHttpRequest = HttpOpenRequest(hHttpSession, TEXT("GET"), crackedURL.lpszUrlPath,
                                             NULL, TEXT(""), NULL, INTERNET_FLAG_SECURE, 0);
    if (hHttpRequest == NULL)
    {
        InternetCloseHandle(hHttpSession);
        InternetCloseHandle(hInternet);
        return -3;
    }

    /**
     * 查询http状态码（这一步不是必须的）,但是HttpSendRequest()必须要调用
     */

    DWORD dwRetCode = 0;
    DWORD dwSizeOfRq = sizeof(DWORD);
    if (!HttpSendRequest(hHttpRequest, NULL, 0, NULL, 0) ||
        !HttpQueryInfo(hHttpRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwRetCode, &dwSizeOfRq, NULL)
        || dwRetCode >= 400)
    {
        InternetCloseHandle(hHttpRequest);
        InternetCloseHandle(hHttpSession);
        InternetCloseHandle(hInternet);

        return -4;
    }

    /**
    *  查询文件大小
    */
    DWORD dwContentLen;
    //这个地方有错误，参见后面分析！
    if (!InternetQueryDataAvailable(hHttpRequest, &dwContentLen, 0, 0) || dwContentLen == 0)
    {
        InternetCloseHandle(hHttpRequest);
        InternetCloseHandle(hHttpSession);
        InternetCloseHandle(hInternet);
        return -6;
    }

    FILE* file = fopen(filePath, "wb");
    if (file == NULL)
    {
        InternetCloseHandle(hHttpRequest);
        InternetCloseHandle(hHttpSession);
        InternetCloseHandle(hInternet);

        return -7;
    }



    DWORD dwError;
    DWORD dwBytesRead;
    DWORD nCurrentBytes = 0;
    char szBuffer[1024] = { 0 };
    while (TRUE)
    {
        //开始读取文件
        if (InternetReadFile(hHttpRequest, szBuffer, sizeof(szBuffer), &dwBytesRead))
        {
            if (dwBytesRead == 0)
            {
                break;
            }

            nCurrentBytes += dwBytesRead;
            fwrite(szBuffer, 1, dwBytesRead, file);
        }
        else
        {
            dwError = GetLastError();
            break;
        }
    }



    fclose(file);
    InternetCloseHandle(hInternet);
    InternetCloseHandle(hHttpSession);
    InternetCloseHandle(hHttpRequest);


    //这个地方有错误，参见后面分析！
    if (dwContentLen != nCurrentBytes)
        return -8;

    return 0;
}