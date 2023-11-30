#include "calculator/calculator_registry.h"

namespace flow {
void CalculatorRegistry::RegisterGetContract(
    const std::string& name,
    CalculatorRegistry::GetContractFunction func) {

    get_contract_functions_.Register(name, std::move(func));
}

Status CalculatorRegistry::InvokeGetContract(const std::string& name,
                                                   CalculatorContract* contract) {
    return get_contract_functions_.Invoke(name, std::forward<CalculatorContract*>(contract));
}

CalculatorRegistry::ResultTuple
CalculatorRegistry::InvokeCreate(const std::string& name, const std::string& device_type) {
    auto h_elem   = create_functions_.Invoke(name + "_elem");
    auto h_option = create_functions_.Invoke(name + "_option");
    auto h_input  = create_functions_.Invoke(name + "_inout_put");
    return std::make_tuple<std::shared_ptr<Calculator>,
                           std::shared_ptr<CalculatorOption>,
                           std::shared_ptr<InOutHandler>>(
        std::static_pointer_cast<Calculator>(h_elem),
        std::static_pointer_cast<CalculatorOption>(h_option),
        std::static_pointer_cast<InOutHandler>(h_input));
}
} // namespace flow