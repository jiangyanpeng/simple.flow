#ifndef SIMPLE_FLOW_DEVICE_REGISTRY_H_
#define SIMPLE_FLOW_DEVICE_REGISTRY_H_

#include "core/function_registry.h"
#include "device/device.h"

#include <memory>

namespace flow {
class DeviceRegistry {
public:
    DeviceRegistry()  = default;
    ~DeviceRegistry() = default;

    template <typename T>
    void RegisterDevice(const std::string& name) {
        device_create_functions_.Register(name, []() { return std::make_shared<T>(); });
    }
    std::shared_ptr<Device> InvokeCreate(const std::string& name);

private:
    FunctionRegistry<std::shared_ptr<Device>> device_create_functions_;
};
} // namespace flow
#endif // SIMPLE_FLOW_DEVICE_REGISTRY_H_
