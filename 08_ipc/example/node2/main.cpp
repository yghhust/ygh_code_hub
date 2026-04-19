
#include "demo.h"
#include <iostream>
#include "ipc/interface/ipc_context.h"

int main() {
    // 创建客户端上下文（连接地址）
    auto ctx = createIpcContext(IpcBackend::ZeroMQ);
   
    #if 1
    // 服务端（绑定）
    std::cout << "RPC Server started on ipc:///tmp/node2.ipc" << std::endl;
    auto service = ctx->bind("ipc:///tmp/node2.ipc", "node2");    
    service->on<SubResponse, SubRequest>("sub", [](const SubRequest& req) {
        return SubResponse{req.a - req.b};
    });
    service->start();
    #endif

    std::cout << "Press Enter to continue..." << std::endl;
    std::cin.get();
    service->publish("alerts", "Node2 is up!");
    
    // 创建 RPC 客户端
    auto client = ctx->createClient("ipc:///tmp/node1.ipc", "node1");
    AddRequest addReq = {10, 20};
    AddResponse addResp = client->call<AddResponse>("add", addReq, 1000);

#if 1
    // 调用异常服务（演示错误处理）
    try {
        Json subReq = {{"a", 100}, {"b", 7}};
        Json subResp = client->call("sub", subReq);
    } catch (const RpcException& e) {
        std::cerr << "RPC error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
    }

    // 调用不存在的服务（演示错误处理）
    try {
        Json badResp = client->call("multiply", {{"a", 2}, {"b", 3}});
    } catch (const std::exception& e) {
        std::cerr << "Expected error: " << e.what() << std::endl;
    }
#endif

    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    service->stop();

    return 0;
}
