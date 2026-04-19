#include "demo.h"
#include "ipc/zmq/zmq_context.h"
#include <iostream>

int event() {
    auto ctx = ZmqContext::createClient("ipc:///tmp/myapp.ipc");

    auto rpcClient = ctx.createRpcClient();
    Json resp = rpcClient->call("add", {{"a", 3}, {"b", 5}});
    std::cout << "3+5=" << resp["result"] << std::endl;

    auto eventBus = ctx.createEventBus();
    eventBus->subscribe("status", [](const std::string& topic, const Json& data) {
        std::cout << "Event: " << topic << " -> " << data.dump() << std::endl;
    });

    while(1) {
        std::string topic;
        std::string data;
        std::cout << "Enter  (or 'exit' to quit): ";
        std::getline(std::cin, topic);
        if (topic == "exit") break;
    }

    return 0;
    //std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main() {
    // 创建客户端上下文（连接地址）
    auto ctx = ZmqContext::createClient("ipc:///tmp/calculator.ipc");

    // 创建 RPC 客户端
    auto client = ctx.createRpcClient();

    AddRequest addReq = {10, 20};
    AddResponse addResp = client->call<AddResponse>("add", addReq, 10);
    std::cout << "add req: 10 + 20 = " << addResp.result << std::endl;

    // 调用异常服务（演示错误处理）
    try {
        Json subReq = {{"a1", 100}, {"b", 7}};
        Json subResp = client->call("sub", subReq);
        //std::cout << "sub req json: 100 - 7 = " << subResp["result"].get<int>() << std::endl; 
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

    return 0;
}
