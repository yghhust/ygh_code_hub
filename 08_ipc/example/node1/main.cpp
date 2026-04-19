#include "demo.h"
#include <iostream>
#include "ipc/interface/ipc_context.h"

int main() {
    auto ctx = createIpcContext(IpcBackend::ZeroMQ);

    // 服务端（绑定）
    std::cout << "RPC Server started on ipc:///tmp/node1.ipc" << std::endl;
    auto service = ctx->bind("ipc:///tmp/node1.ipc", "node1");
    service->on<AddResponse, AddRequest>("add", [](const AddRequest& req) {
        return AddResponse{req.a + req.b};
    });
    service->start();

    std::cout << "Please start node2..." << std::endl;
    std::cin.get();

    auto client = ctx->createClient("ipc:///tmp/node2.ipc", "node2");
    client->subscribe("alerts", [](const std::string& topic, const Json& data) {
        std::cout << "Received Node2 Alert: " << data << std::endl;
    });

    SubRequest subReq = {100, 20};
    SubResponse subResp = client->call<SubResponse>("sub", subReq, 1000);
    
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    std::cout << "\nShutting down..." << std::endl;
    client.reset(); // 先销毁客户端，发送 unsubscribe
    service->stop();

    return 0;
}