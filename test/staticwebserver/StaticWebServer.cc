#include "StaticWebServer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "Logger.h"

using namespace inet;

StaticWebServer::StaticWebServer(EventLoop *loop, uint16_t port)
    : m_server(loop, InetAddress(port)), m_loop(loop)
{
    // 文件扩展名到 MIME 类型的映射
    mimeTypes = {
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".bmp", "image/bmp"},
        {".svg", "image/svg+xml"},
        {".ico", "image/x-icon"},
        {".pdf", "application/pdf"},
        {".txt", "text/plain"},
        {".webp", "image/webp"},
        {".mp3", "audio/mpeg"},
        {".ogg", "audio/ogg"},
        {".xml", "application/xml"},
        {".csv", "text/csv"},
        {".woff", "font/woff"},
        {".woff2", "font/woff2"},
        {".ttf", "font/ttf"},
        {".mp4", "video/mp4"},
        {".zip", "application/zip"},
        {".rar", "application/x-rar-compressed"}};
    // 设置 HTTP 请求回调
    
    m_server.setHttpCallback([this](const HttpRequest& req, HttpResponse* resp) {
        this->onRequest(req, resp);
    });
}

void StaticWebServer::start(int numThreads) 
{
    m_server.start(numThreads);
}

void StaticWebServer::setRootDirectory(const std::string& rootDir) 
{
    m_rootDirectory = rootDir;
}

std::string StaticWebServer::readFile(const std::string& filePath) 
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        // LOG_WARN << filePath << " not exist.";
        return "";  // 文件无法打开，返回空字符串
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string StaticWebServer::getMimeType(const std::string& filePath) 
{
    std::string extension = filePath.substr(filePath.find_last_of("."));
    if (mimeTypes.find(extension) != mimeTypes.end()) {
        return mimeTypes[extension];
    }
    return "application/octet-stream";  // 默认二进制流类型
}

void StaticWebServer::onRequest(const HttpRequest& req, HttpResponse* resp) 
{
    std::cout << "REQ  ====================================================\n";
    std::cout << "Headers " << req.methodString() << " " << req.getPath() << std::endl;
    const auto &headers = req.getHeaders();
    for (const auto &header : headers) {
        std::cout << header.first << ": " << header.second << std::endl;
    }

    // 解析请求路径，默认为 index.html
    std::string path = req.getPath();
    if (path == "/") {
        path = "/index.html"; // 默认返回首页
    }

    std::string filePath = m_rootDirectory + path;  // 拼接文件路径
    std::string fileContent = readFile(filePath);

    if (!fileContent.empty()) {
        resp->setStatusCode(HttpResponse::HttpStatusCode::k200OK);
        resp->setStatusMessage("OK");

        // 设置 Content-Type
        std::string mimeType = getMimeType(filePath);
        resp->setContentType(mimeType);

        // 设置 Content-Length，告知浏览器响应体的长度
        resp->addHeader("Content-Length", std::to_string(fileContent.size()));
        resp->addHeader("Server", "inet");

        resp->setBody(fileContent);  // 设置文件内容为响应体


    } else {
        resp->setStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setBody("<html><body><h1>404 Not Found</h1></body></html>");
    }

    std::cout << "RESP ====================================================\n";
    const auto &re_headers = resp->getHeaders();
    for (const auto &header : re_headers) {
        std::cout << header.first << ": " << header.second << std::endl;
    }
    std::cout << "\n";
}