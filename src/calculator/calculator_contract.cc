#include "calculator/calculator_contract.h"

namespace flow {
CalculatorContract::CalculatorContract() {}

CalculatorContract::~CalculatorContract() {}

Status CalculatorContract::Initialize() {
    return Status::OkStatus();
}

CalculatorContract& CalculatorContract::AddInput(const std::string& port_name,
                                                 const std::string& data_type_name) {
    DataType type;
    type.SetTypeName(data_type_name);
    auto s = inputs_.AddTag(port_name, type);
    SIMPLE_ASSERT(s.first.IsOk());
    return *this;
}

CalculatorContract& CalculatorContract::AddOutput(const std::string& port_name,
                                                  const std::string& data_type_name) {
    DataType type;
    type.SetTypeName(data_type_name);
    auto s = outputs_.AddTag(port_name, type);
    SIMPLE_ASSERT(s.first.IsOk());
    return *this;
}

CalculatorContract& CalculatorContract::SetInputHandler(const std::string& input_handler_name) {
    input_handler_name_ = input_handler_name;
    return *this;
}

CalculatorContract& CalculatorContract::SetCalculatorName(const std::string& elem_name) {
    elementary_name_ = elem_name;
    return *this;
}

CalculatorContract& CalculatorContract::SetCalculatorOptionName(const std::string& elem_option_name) {
    elementary_option_name_ = elem_option_name;
    return *this;
}

CalculatorContract& CalculatorContract::AddSupportedDevice(const std::string& device_name) {
    supported_device_.insert(device_name);
    return *this;
}

std::string CalculatorContract::DebugString() {
    return elementary_name_;
}

} // namespace flow
