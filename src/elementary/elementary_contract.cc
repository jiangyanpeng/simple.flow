#include "elementary/elementary_contract.h"

namespace flow {
ElementaryContract::ElementaryContract() {}

ElementaryContract::~ElementaryContract() {}

Status ElementaryContract::Initialize() {
    return Status::OkStatus();
}

ElementaryContract& ElementaryContract::AddInput(const std::string& port_name,
                                                 const std::string& data_type_name) {
    DataType type;
    type.SetTypeName(data_type_name);
    auto s = inputs_.AddTag(port_name, type);
    MATRIX_ASSERT(s.first.IsOk());
    return *this;
}

ElementaryContract& ElementaryContract::AddOutput(const std::string& port_name,
                                                  const std::string& data_type_name) {
    DataType type;
    type.SetTypeName(data_type_name);
    auto s = outputs_.AddTag(port_name, type);
    MATRIX_ASSERT(s.first.IsOk());
    return *this;
}

ElementaryContract& ElementaryContract::SetInputHandler(const std::string& input_handler_name) {
    input_handler_name_ = input_handler_name;
    return *this;
}

ElementaryContract& ElementaryContract::SetElementaryName(const std::string& elem_name) {
    elementary_name_ = elem_name;
    return *this;
}

ElementaryContract& ElementaryContract::SetElemOptionName(const std::string& elem_option_name) {
    elementary_option_name_ = elem_option_name;
    return *this;
}

ElementaryContract& ElementaryContract::AddSupportedDevice(const std::string& device_name) {
    supported_device_.insert(device_name);
    return *this;
}

std::string ElementaryContract::DebugString() {
    return elementary_name_;
}

} // namespace flow
