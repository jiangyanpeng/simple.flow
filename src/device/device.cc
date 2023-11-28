#include "device/device.h"
#include "base/status.h"

namespace flow {
// std::shared_ptr<DataMgrBase> Device::GetAllocator() const {
//     return nullptr;
// }

Status Device::Initialize() {
    return Status::OkStatus();
}

std::shared_ptr<Stream> Device::GetStream() const {
    return nullptr;
}

template <typename Ret, typename... Args>
class Func {
public:
    std::function<Ret(Args&&...)> func_;
};

// std::shared_ptr<DataMgrBase> DeviceCPU::GetAllocator() const {
//     return std::make_shared<DataMgrDefault>();
// }

void DeviceCPU::SetThreadPool(const std::shared_ptr<base::PipeManager>& pool) {
    thread_pool_ = pool;
}

void DeviceCPU::ComputeSync(Function f) {
    auto result = thread_pool_->Commit(f);
    result.wait();
}

void DeviceCPU::ComputeAsync(Function f, DoneCallback done) {
    thread_pool_->Commit([&]() {
        f();
        done();
    });
}

void DeviceCPU::ComputeAsync(Function f) {
    thread_pool_->Commit([&]() { f(); });
}

void DeviceCPU::Sync(std::shared_ptr<Stream> stream) {
    // do nothing
}

void DeviceCPU::Sync() {
    // do nothing
}

Status DeviceCPU::Initialize() {
    return Status::OkStatus();
}

} // namespace flow