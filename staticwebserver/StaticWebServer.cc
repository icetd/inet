#include "StaticWebServer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "Logger.h"

using namespace inet;

StaticWebServer::StaticWebServer(EventLoop* loop, uint16_t port)
    : m_server(loop, InetAddress(port)), m_loop(loop) {
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
        {".rar", "application/x-rar-compressed"},
        {".wasm", "application/wasm"},  // WebAssembly 文件类型
        {".webm", "video/webm"}  // WebM 视频格式
    };

    m_server.setHttpCallback([this](const HttpRequest& req, HttpResponse* resp) {
        this->onRequest(req, resp);
    });
}

void StaticWebServer::start(int numThreads) {
    m_server.start(numThreads);
}

void StaticWebServer::setRootDirectory(const std::string& rootDir) {
    m_rootDirectory = rootDir;
}

std::string StaticWebServer::readFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "";  // 文件无法打开，返回空字符串
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string StaticWebServer::getMimeType(const std::string& filePath) {
    std::string extension = filePath.substr(filePath.find_last_of("."));
    if (mimeTypes.find(extension) != mimeTypes.end()) {
        return mimeTypes[extension];
    }
    return "application/octet-stream";  // 默认二进制流类型
}

std::string StaticWebServer::urlDecode(const std::string& url) {
    std::string decoded = url;
    bool changed;
    do {
        changed = false;
        std::string temp;
        char ch;
        for (size_t i = 0; i < decoded.length(); i++) {
            if (decoded[i] == '%') {
                std::string hex = decoded.substr(i + 1, 2);
                ch = static_cast<char>(std::stoi(hex, nullptr, 16));
                temp += ch;
                i += 2;  // 跳过两个十六进制字符
            } else if (decoded[i] == '+') {
                temp += ' ';  // 处理 URL 编码中的加号（表示空格）
            } else {
                temp += decoded[i];
            }
        }
        if (temp != decoded) {
            changed = true;
            decoded = temp;
        }
    } while (changed);
    return decoded;
}

std::string StaticWebServer::getFileOrIndex(const std::string& path) {
    std::string filePath = m_rootDirectory + path;
    // 使用 stat 函数检查路径是否为目录
    struct stat statbuf;
    if (stat(filePath.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
        filePath += "/index.html";  // 如果是目录，则添加 index.html
    }
    return filePath;
}

void StaticWebServer::onRequest(const HttpRequest& req, HttpResponse* resp) {
    std::cout << "REQ  ====================================================\n";
    std::cout << "Headers " << req.methodString() << " " << req.getPath() << std::endl;
    const auto& headers = req.getHeaders();
    for (const auto& header : headers) {
        std::cout << header.first << ": " << header.second << std::endl;
    }

    // 解析请求路径，默认为 index.html
    std::string path = req.getPath();
    if (path == "/") {
        path = "/index.html";  // 默认返回首页
    }

    path = urlDecode(path);  // 解码 URL

    std::string filePath = getFileOrIndex(path);  // 获取文件路径，若为目录则尝试加载 index.html
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
        // 如果文件没有找到，加载 404 页面
        std::string notFoundPath = m_rootDirectory + "/404.html";
        fileContent = readFile(notFoundPath);

        if (!fileContent.empty()) {
            resp->setStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
            resp->setStatusMessage("Not Found");
            resp->setBody(fileContent);  // 设置自定义 404 页面内容
        } else {
            // 如果 404 页面也没有，返回一个简单的文本 404 错误
            resp->setStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
            resp->setStatusMessage("Not Found");
            resp->setBody("<html><body><h1>404 Not Found</h1></body></html>");
        }
    }

    std::cout << "RESP ====================================================\n";
    const auto& re_headers = resp->getHeaders();
    for (const auto& header : re_headers) {
        std::cout << header.first << ": " << header.second << std::endl;
    }
    std::cout << "\n";
}