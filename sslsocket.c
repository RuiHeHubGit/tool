////
//// Created by rui on 2022/9/11.
////
//#include <stdio.h>
//#include <winsock2.h>
//#include <Ws2tcpip.h>
//
//#include <openssl/rsa.h>       /* SSLeay stuff */
//#include <openssl/crypto.h>
//#include <openssl/x509.h>
//#include <openssl/pem.h>
//#include <openssl/ssl.h>
//#include <openssl/err.h>
//
//#pragma comment( lib, "ws2_32.lib" )
//#pragma comment( lib, "libeay32.lib" )
//#pragma comment( lib, "ssleay32.lib" )
//
//#include <zlib.h>
//#include <wininet.h>
//
//#include "download.h"
//
//#define SERVER_PORT 443 //侦听端口
//
//typedef struct {
//    char *version;
//    char *code;   // 状态返回码
//    char *desc;   // 返回描述
//    char *body;
//    int bodySize;
//} HttpResponse;
//
//// 内存释放函数
//void releaseHttpResponse(HttpResponse *httpResponse);
//
//// HttpResponse解析函数
//HttpResponse *parseResponse(char *response);
//
//
//static char *httpRequest[] = {
//        "GET /RuiHeHubGit/tool/main/README.md HTTP/1.1\r\n",
//        "Host: localhost:8080\r\n",
//        "Connection: Close\r\n",
//        "Cache-Control: max-age=0\r\n",
//        "sec-ch-ua: \"Microsoft Edge\";v=\"105\", \" Not;A Brand\";v=\"99\", \"Chromium\";v=\"105\"\r\n",
//        "sec-ch-ua-mobile: ?0\r\n",
//        "sec-ch-ua-platform: \"Windows\"\r\n",
//        "Upgrade-Insecure-Requests: 1\r\n",
//        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/105.0.0.0 Safari/537.36 Edg/105.0.1343.33\r\n",
//        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n",
//        "Sec-Fetch-Site: none\r\n",
//        "Sec-Fetch-Mode: navigate\r\n",
//        "Sec-Fetch-Dest: document\r\n",
//        "Accept-Encoding: gzip, deflate, br\r\n",
//        "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6\r\n",
//        "\r\n"
//};
//
//int getFirstIpByDomain(const char *const domain, char *ip);
//
//int GzipDecompress(const char *src, int srcLen, const char *dst, int dstLen);
//
//void ShowCerts(SSL *ssl) {
//    X509 *cert;
//    char *line;
//    cert = SSL_get_peer_certificate(ssl);
//    if (cert != NULL) {
//        printf("数字证书信息:\n");
//        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
//        printf("证书: %s\n", line);
//        OPENSSL_free(line);
//        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
//        printf("颁发者: %s\n", line);
//        OPENSSL_free(line);
//        X509_free(cert);
//    } else
//        printf("无证书信息！\n");
//}
//
//int DownloadToFile(const char *url, const char *path) {
//    int domainEndIndex = 0;
//    int urlLen = strlen(url);
//    for (int i = 8; i < urlLen; ++i) {
//        char c = url[i];
//        if (c == '/') {
//            domainEndIndex = i;
//            break;
//        }
//    }
//    char baseUrl[MAX_PATH] = {0};
//    char urlPath[MAX_PATH] = {0};
//    strncpy(baseUrl, url, domainEndIndex);
//    strcpy(urlPath, url + domainEndIndex);
//
//    int code = 1;
//    WORD wVersionRequested;
//    WSADATA wsaData;
//    SOCKET sClient = INVALID_SOCKET; //连接套接字
//    struct sockaddr_in saServer; //地址信息
//    SSL_CTX *ctx;
//    SSL *ssl;
//
//    // 初始化SSL
//    SSL_library_init();
//    OpenSSL_add_all_algorithms();
//    SSL_load_error_strings();
//
//    ctx = SSL_CTX_new(SSLv23_client_method());
//    if (ctx == NULL) {
//        ERR_print_errors_fp(stdout);
//        return code;
//    }
//
//    //WinSock初始化
//    wVersionRequested = MAKEWORD(2, 2); //希望使用的WinSock DLL的版本
//    int ret = WSAStartup(wVersionRequested, &wsaData);
//    if (ret != 0) {
//        printf("WSAStartup() failed!\n");
//        return -1;
//    }
//
//    //确认WinSock DLL支持版本2.2
//    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
//        WSACleanup();
//        printf("Invalid WinSock version!\n");
//        return -1;
//    }
//
//
//    //创建Socket,使用TCP协议
//    sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//    if (sClient == INVALID_SOCKET) {
//        WSACleanup();
//        printf("socket() failed!\n");
//        goto clean;
//    }
//
//    char *domain = strchr(baseUrl, '/');
//    domain += 2;
//    char ipAddr[15] = {0};
//    getFirstIpByDomain(domain, ipAddr);
//    //构建服务器地址信息
//    saServer.sin_family = AF_INET; //地址家族
//    saServer.sin_port = htons(SERVER_PORT); //注意转化为网络节序
//    saServer.sin_addr.S_un.S_addr = inet_addr(ipAddr);
//
//
//    //连接服务器
//    ret = connect(sClient, (struct sockaddr *) &saServer, sizeof(saServer));
//    if (ret == SOCKET_ERROR) {
//        printf("connect() failed!\n");
//        closesocket(sClient); //关闭套接字
//        WSACleanup();
//        goto clean;
//    }
//
//    /* 基于 ctx 产生一个新的 SSL */
//    ssl = SSL_new(ctx);
//    SSL_set_fd(ssl, sClient);
//    /* 建立 SSL 连接 */
//    if (SSL_connect(ssl) == -1)
//        ERR_print_errors_fp(stderr);
//    else {
//        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
//        ShowCerts(ssl);
//    }
//
//
//    printf("\nrequest:\n");
//    int count = sizeof(httpRequest) / sizeof(char *);
//    char requestLine[MAX_PATH] = {0};
//    char httpHost[MAX_PATH] = {0};
//    sprintf(requestLine, "GET %s HTTP/1.1\r\n", urlPath);
//    sprintf(httpHost, "Host: %s\r\n", domain);
//    httpRequest[0] = requestLine;
//    httpRequest[1] = httpHost;
//    for (int i = 0; i < count; ++i) {
//        ret = SSL_write(ssl, (char *) httpRequest[i], strlen((char *) httpRequest[i]));
//        printf("%s", httpRequest[i]);
//        if (ret < 0) {
//            printf("send() failed!\n");
//        }
//    }
//
//    char buff[10240] = {0};
//    printf("\nresponse:\n");
//    ret = SSL_read(ssl, buff, 10240);
//    if (ret > 0) {
//        printf("%s", buff);
//        printf("rec count:%d\n", ret);
//
//        HttpResponse *response = parseResponse(buff);
//
//        releaseHttpResponse(response);
//    } else {
//        printf("recv failed: %d\n", WSAGetLastError());
//    }
//
//
//    clean:
//    if (sClient != INVALID_SOCKET) {
//        closesocket(sClient); //关闭套接字
//    }
//    SSL_shutdown(ssl);
//    SSL_free(ssl);
//    SSL_CTX_free(ctx);
//
//    WSACleanup();
//    return code;
//}
//
//int getFirstIpByDomain(const char *const hostname, char *ip) {
//    struct WSAData wsaData;
//    struct addrinfo *ptr = NULL;
//    struct addrinfo *res = NULL;
//    struct addrinfo hints;
//    int addr_ret;
//
//    printf("hostname:%s\n", hostname);
//
//    //hints必须初始化为0
//    memset(&hints, 0, sizeof(hints));
//    //指定要查询的类型
//    hints.ai_family = AF_INET;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_family = AF_UNSPEC;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_protocol = IPPROTO_TCP;
//
//    //初始化winsock
//    int wsaRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
//    if (wsaRes != 0) {
//        return -1;
//    }
//
//    //参数1既可以是ip也可以是域名 参数2是服务/端口类型如80/http 443/https
//    addr_ret = getaddrinfo(hostname, "443", &hints, &res);
//    if (addr_ret != 0) {
//        return -1;
//    }
//
//    //结果
//    struct sockaddr_in *ip_v4;
//    LPSOCKADDR ip_v6;
//    int retval;
//    DWORD ip_v6_strlen = 46;
//    char ip_v6_str[46];
//
//    //getaddrinfo查询成功返回的是链表
//    for (ptr = res; ptr != NULL; ptr = ptr->ai_next) {
//        switch (ptr->ai_family) {
//            case AF_UNSPEC:
//                printf("no ip type\n");
//                break;
//            case AF_INET:
//                printf("ip v4: \n");
//                ip_v4 = (struct sockaddr_in *) ptr->ai_addr;
//                printf("%s\n", inet_ntoa(ip_v4->sin_addr));
//                strcpy(ip, inet_ntoa(ip_v4->sin_addr));
//                return 0;
//            case AF_INET6:
//                printf("ip v6: \n");
//                ip_v6 = (LPSOCKADDR) ptr->ai_addr;
//                retval = WSAAddressToString(ip_v6, (DWORD) ptr->ai_addrlen, NULL, ip_v6_str, &ip_v6_strlen);
//                if (retval)
//                    printf("WSAAddressToString failed：%d\n", WSAGetLastError());
//                else
//                    printf("%s", ip_v6_str);
//                break;
//            default:
//                printf("other type：%d", ptr->ai_family);
//                break;
//        }
//    }
//
//    //清理
//    freeaddrinfo(res);
//    WSACleanup();
//    return 0;
//}
//
//// 内存释放函数
//void releaseHttpResponse(HttpResponse *httpResponse) {
//    if (httpResponse == NULL) {
//        return;
//    }
//
//    if (httpResponse->version != NULL) {
//        free(httpResponse->version);
//        httpResponse->version = NULL;
//    }
//
//    if (httpResponse->code != NULL) {
//        free(httpResponse->code);
//        httpResponse->code = NULL;
//    }
//
//    if (httpResponse->desc != NULL) {
//        free(httpResponse->desc);
//        httpResponse->desc = NULL;
//    }
//
//    if (httpResponse->body != NULL) {
//        free(httpResponse->body);
//        httpResponse->body = NULL;
//    }
//
//    free(httpResponse);
//    httpResponse = NULL;
//}
//
//// HttpResponse解析函数
//HttpResponse *parseResponse(char *response) {
//    HttpResponse *_httpResponseTemp = (HttpResponse *) malloc(sizeof(HttpResponse));
//    memset(_httpResponseTemp, 0, sizeof(HttpResponse));
//    HttpResponse *_httpResponse = (HttpResponse *) malloc(sizeof(HttpResponse));
//    memset(_httpResponse, 0, sizeof(HttpResponse));
//
//    // 解析第一行
//    char *_start = response;
//    for (; *_start && *_start != '\r'; _start++) {
//        if (_httpResponseTemp->version == NULL) {
//            _httpResponseTemp->version = _start;
//        }
//
//        if (*_start == ' ') {
//            if (_httpResponseTemp->code == NULL) {
//                _httpResponseTemp->code = _start + 1;
//            } else {
//                _httpResponseTemp->desc = _start + 1;
//            }
//            *_start = '\0';
//        }
//    }
//    *_start = '\0'; // \r -> \0
//    _start++;   // skip \n
//
//    // 解析header
//    _start++;
//    char *_line = _start;
//    while (*_line != '\r' && *_line != '\0') {
//        char *_key;
//        char *_value;
//        while (*(_start++) != ':');
//        *(_start - 1) = '\0';
//        _key = _line;
//        _value = _start + 1;
//        while (_start++, *_start != '\0' && *_start != '\r');
//        *_start = '\0'; // \r -> \0
//        _start++;   // skip \n
//
//        _start++;
//        _line = _start;
//
//        printf("parseResponse(): KEY= %s , VALUE= %s\n", _key, _value);
//        if (!strcmp(_key, "Content-Length")) {
//            _httpResponseTemp->bodySize = atoi(_value);
//        }
//    }
//
//    // 解析body 如果最后一行不是空行，说明有body数据
//    if (*_line == '\r') {
//        _line += 2;
//        _httpResponseTemp->body = _line;
//    }
//
//    if (_httpResponseTemp->version != NULL) {
//        _httpResponse->version = strdup(_httpResponseTemp->version);
//    }
//    if (_httpResponseTemp->code != NULL) {
//        _httpResponse->code = strdup(_httpResponseTemp->code);
//    }
//    if (_httpResponseTemp->desc != NULL) {
//        _httpResponse->desc = strdup(_httpResponseTemp->desc);
//    }
//    if (_httpResponseTemp->body != NULL) {
//        _httpResponse->body = strdup(_httpResponseTemp->body);
////        int gzSize = strlen(_httpResponse->body);
////        char *uncompressed = malloc(gzSize * 2);
////        memset(uncompressed, 0, gzSize * 2);
////        GzipDecompress(_httpResponse->body, gzSize, uncompressed, gzSize * 2);
////        _httpResponse->body = uncompressed;
//    }
//    _httpResponse->bodySize = _httpResponseTemp->bodySize;
//
//    free(_httpResponseTemp);
//    _httpResponseTemp = NULL;
//
//    return _httpResponse;
//}
//
//
//// GzipCompress: do the compressing
//int GzipCompress(const char *src, int srcLen, char *dest, int destLen) {
//    z_stream c_stream;
//    int err = 0;
//    int windowBits = 15;
//    int GZIP_ENCODING = 16;
//
//    if (src && srcLen > 0) {
//        c_stream.zalloc = (alloc_func) 0;
//        c_stream.zfree = (free_func) 0;
//        c_stream.opaque = (voidpf) 0;
//        if (deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
//                         windowBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY) != Z_OK)
//            return -1;
//        c_stream.next_in = (Bytef *) src;
//        c_stream.avail_in = srcLen;
//        c_stream.next_out = (Bytef *) dest;
//        c_stream.avail_out = destLen;
//        while (c_stream.avail_in != 0 && c_stream.total_out < destLen) {
//            if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK) return -1;
//        }
//        if (c_stream.avail_in != 0) return c_stream.avail_in;
//        for (;;) {
//            if ((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) break;
//            if (err != Z_OK) return -1;
//        }
//        if (deflateEnd(&c_stream) != Z_OK) return -1;
//        return c_stream.total_out;
//    }
//    return -1;
//}
//
//// GzipDecompress: do the decompressing
//int GzipDecompress(const char *src, int srcLen, const char *dst, int dstLen) {
//    z_stream strm;
//    strm.zalloc = NULL;
//    strm.zfree = NULL;
//    strm.opaque = NULL;
//
//    strm.avail_in = srcLen;
//    strm.avail_out = dstLen;
//    strm.next_in = (Bytef *) src;
//    strm.next_out = (Bytef *) dst;
//
//    int err = -1, ret = -1;
//    err = inflateInit2(&strm, MAX_WBITS + 16);
//    if (err == Z_OK) {
//        err = inflate(&strm, Z_FINISH);
//        if (err == Z_STREAM_END) {
//            ret = strm.total_out;
//        } else {
//            inflateEnd(&strm);
//            return err;
//        }
//    } else {
//        inflateEnd(&strm);
//        return err;
//    }
//    inflateEnd(&strm);
//    return err;
//}
