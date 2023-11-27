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

} // namespace flow