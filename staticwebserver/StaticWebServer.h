#ifndef STATIC_WEBSERVER_H
#define STATIC_WEBSERVER_H

#include "HttpServer.h"
#include "EventLoop.h"
#include <string>
#include <unordered_map>

using namespace inet;

class StaticWebServer
{
public:
    StaticWebServer(EventLoop *loop, uint16_t port);

    void start(int numThreads = 1);

    void setRootDirectory(const std::string &rootDir);

private:
    HttpServer m_server;               // HTTP 服务器
    EventLoop *m_loop;                 // 事件循环
    std::string m_rootDirectory = "."; // 静态文件根目录，默认为当前目录

    // 文件扩展名到 MIME 类型的映射
    std::unordered_map<std::string, std::string> mimeTypes;

    // 读取文件内容
    std::string readFile(const std::string &filePath);

    // 获取文件的 MIME 类型
    std::string getMimeType(const std::string &filePath);

    // url decode
    std::string urlDecode(const std::string &url);
    std::string getFileOrIndex(const std::string &path);

    // 处理 HTTP 请求
    void onRequest(const HttpRequest &req, HttpResponse *resp);
};

#endif // STATIC_WEBSERVER_H
