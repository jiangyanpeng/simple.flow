#include "device/matrix_device_registry.h"
namespace flow {

std::shared_ptr<Device> MatrixDeviceRegistry::InvokeCreate(const std::string& name) {
    return device_create_functions_.Invoke(name);
}
} // namespace flow
