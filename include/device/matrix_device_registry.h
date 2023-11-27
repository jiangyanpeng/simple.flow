#ifndef GRAPHFLOW_FRAMEWORK_MATRIX_DEVICE_REGISTRY_H
#define GRAPHFLOW_FRAMEWORK_MATRIX_DEVICE_REGISTRY_H

#include "base/matrix_function_registry.h"
#include "device/device.h"
#include <memory>

namespace flow {
class MatrixDeviceRegistry {
public:
    MatrixDeviceRegistry()  = default;
    ~MatrixDeviceRegistry() = default;

    template <typename T>
    void RegisterDevice(const std::string& name) {
        device_create_functions_.Register(name, []() { return std::make_shared<T>(); });
    }
    std::shared_ptr<Device> InvokeCreate(const std::string& name);

private:
    MatrixFunctionRegistry<std::shared_ptr<Device>> device_create_functions_;
};
} // namespace flow


#endif // GRAPHFLOW_FRAMEWORK_MATRIX_DEVICE_REGISTRY_H
