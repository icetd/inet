#include "HttpServer.h"
#include "EventLoop.h"
#include <iostream>

using namespace inet;

void onRequest(const HttpRequest &req, HttpResponse *resp)
{
    std::cout << "REQ  ====================================================\n";
    std::cout << "Headers " << req.methodString() << " " << req.getPath() << std::endl;
    const auto &headers = req.getHeaders();
    for (const auto &header : headers) {
        std::cout << header.first << ": " << header.second << std::endl;
    }

    if (req.getPath() == "/") {
        resp->setStatusCode(HttpResponse::HttpStatusCode::k200OK);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "inet");
        std::string now = TimeStamp::now().toFormattedString();
        resp->setBody("<html><head><title>This is title</title></head>"
                      "<body><h1>Hello</h1>Now is "
                      + now + "</body></html>");
    }

    else if (req.getPath() == "/hello") {
        resp->setStatusCode(HttpResponse::HttpStatusCode::k200OK);
        resp->setStatusMessage("OK");
        resp->setContentType("text/html");
        resp->addHeader("Server", "inet");
        resp->setBody("hello, world!\n");
    }

    else {
        resp->setStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setCloseConnection(true);
    }

    std::cout << "RESP ====================================================\n";
    const auto &re_headers = resp->getHeaders();
    for (const auto &header : re_headers) {
        std::cout << header.first << ": " << header.second << std::endl;
    }
    std::cout << "\n";
}

int main(int argc, char *argv[])
{
    EventLoop loop;
    HttpServer server(&loop, InetAddress(9999));
    server.setHttpCallback(onRequest);
    server.start(4);
    loop.loop();

    return 0;
}