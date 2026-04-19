#include "demo.h"
#include "ipc/zmq/zmq_context.h"
#include <iostream>

int main() {
    // 创建服务端上下文（绑定地址）
    auto ctx = ZmqContext::createServer("ipc:///tmp/calculator.ipc");

    // 创建 RPC 服务端
    auto server = ctx.createRpcServer();

    // 注册一个加法服务
    server->on("add", [](const AddRequest& req) -> AddResponse {
        int a = req.a;
        int b = req.b;
        std::cout << "Received add request: " << a << " + " << b << std::endl;
        return {a + b};
    });

    // 注册一个减法服务
    server->on("sub", [](const SubRequest& req) -> SubResponse {
        int a = req.a;
        int b = req.b;
        std::cout << "Received sub request: " << a << " - " << b << std::endl;
        return {a - b};
    });
    
    #if 1 
    // 注册一个加法服务
    server->on("add", [](const Json& req) -> Json {
        int a = req["a"];
        int b = req["b"];
        std::cout << "Received add request json: " << a << " + " << b << std::endl;
        return {{"result", a + b}};
    });

    // 注册一个减法服务
    server->on("sub", [](const Json& req) -> Json {
        int a = req["a"];
        int b = req["b"];
        std::cout << "Received sub request json: " << a << " - " << b << std::endl;
        return {{"result", a - b}};
    });
    #endif
 
    
    
    std::cout << "RPC Server started on ipc:///tmp/calculator.ipc" << std::endl;
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    // ctx 析构时自动清理资源
    return 0;
}