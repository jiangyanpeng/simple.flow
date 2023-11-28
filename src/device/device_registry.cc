#include "device/device_registry.h"
namespace flow {

std::shared_ptr<Device> DeviceRegistry::InvokeCreate(const std::string& name) {
    return device_create_functions_.Invoke(name);
}
} // namespace flow
