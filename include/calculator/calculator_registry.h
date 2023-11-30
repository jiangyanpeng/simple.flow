#ifndef SIMPLE_FLOW_CALCULATOR_REGISTRY_H_
#define SIMPLE_FLOW_CALCULATOR_REGISTRY_H_

#include "core/function_registry.h"
#include "calculator/calculator.h"
#include "inoutput_handler.h"

namespace flow {
template <typename ElemName, typename OptionName, typename InOutputHandlerName>
class CalculatorCreateFunctions {
public:
    CreateFunction<ElemName> elem_create{};
    CreateFunction<OptionName> option_create{};
    CreateFunction<InOutputHandlerName> handler_create{};
};

class CalculatorRegistry {
public:
    using ResultTuple         = std::tuple<std::shared_ptr<Calculator>,
                                   std::shared_ptr<CalculatorOption>,
                                   std::shared_ptr<InOutputHandler>>;
    using GetContractFunction = std::function<Status(CalculatorContract*)>;

    CalculatorRegistry() = default;

    ~CalculatorRegistry() = default;

    void RegisterGetContract(const std::string& name, GetContractFunction func);

    Status InvokeGetContract(const std::string& name, CalculatorContract* contract);

    template <typename ElemName, typename OptionName, typename InOutputHandlerName>
    void Register(const std::string& name, const std::string& device_type) {

        CalculatorCreateFunctions<ElemName, OptionName, InOutputHandlerName> f{};

        create_functions_.Register(name + "_elem", f.elem_create.Create);
        create_functions_.Register(name + "_option", f.option_create.Create);
        create_functions_.Register(name + "_inout_put", f.handler_create.Create);
    }

    ResultTuple InvokeCreate(const std::string& name, const std::string& device_type);

private:
    FunctionRegistry<Status, CalculatorContract*> get_contract_functions_;
    FunctionRegistry<std::shared_ptr<void>> create_functions_;
};

} // namespace flow
#endif // SIMPLE_FLOW_CALCULATOR_REGISTRY_H_
