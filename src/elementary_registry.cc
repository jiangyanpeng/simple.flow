#include "elementary_registry.h"

namespace flow {
void ElementaryRegistry::RegisterGetContract(
    const std::string& name,
    ElementaryRegistry::GetContractFunction func) {

    get_contract_functions_.Register(name, std::move(func));
}

Status ElementaryRegistry::InvokeGetContract(const std::string& name,
                                                   ElementaryContract* contract) {
    return get_contract_functions_.Invoke(name, std::forward<ElementaryContract*>(contract));
}

ElementaryRegistry::ResultTuple
ElementaryRegistry::InvokeCreate(const std::string& name, const std::string& device_type) {
    auto h_elem   = create_functions_.Invoke(name + "_elem");
    auto h_option = create_functions_.Invoke(name + "_option");
    auto h_input  = create_functions_.Invoke(name + "_inout_put");
    return std::make_tuple<std::shared_ptr<Elementary>,
                           std::shared_ptr<ElementaryOption>,
                           std::shared_ptr<InOutputHandler>>(
        std::static_pointer_cast<Elementary>(h_elem),
        std::static_pointer_cast<ElementaryOption>(h_option),
        std::static_pointer_cast<InOutputHandler>(h_input));
}
} // namespace flow