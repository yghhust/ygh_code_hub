#include "interface/ipc_context.h"
#include "zmq/zmq_context.h"

std::unique_ptr<IIpcContext> createIpcContext(IpcBackend backend) {
    switch (backend) {
        case IpcBackend::ZeroMQ: return std::make_unique<ZmqContext>();
        default: throw IpcException("Unsupported IPC backend");
    }
}
