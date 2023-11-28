#ifndef SIMPLE_FLOW_ELEMENTARY_REGISTRY_H_
#define SIMPLE_FLOW_ELEMENTARY_REGISTRY_H_

#include "base/function_registry.h"
#include "elementary/elementary.h"
#include "inoutput_handler.h"

namespace flow {
template <typename ElemName, typename OptionName, typename InOutputHandlerName>
class ElementaryCreateFunctions {
public:
    CreateFunction<ElemName> elem_create{};
    CreateFunction<OptionName> option_create{};
    CreateFunction<InOutputHandlerName> handler_create{};
};

class MatrixElementaryRegistry {
public:
    using ResultTuple         = std::tuple<std::shared_ptr<Elementary>,
                                   std::shared_ptr<ElementaryOption>,
                                   std::shared_ptr<InOutputHandler>>;
    using GetContractFunction = std::function<Status(ElementaryContract*)>;

    MatrixElementaryRegistry() = default;

    ~MatrixElementaryRegistry() = default;

    void RegisterGetContract(const std::string& name, GetContractFunction func);

    Status InvokeGetContract(const std::string& name, ElementaryContract* contract);

    template <typename ElemName, typename OptionName, typename InOutputHandlerName>
    void RegisterElem(const std::string& name, const std::string& device_type) {

        ElementaryCreateFunctions<ElemName, OptionName, InOutputHandlerName> f{};

        create_functions_.Register(name + "_elem", f.elem_create.Create);
        create_functions_.Register(name + "_option", f.option_create.Create);
        create_functions_.Register(name + "_inout_put", f.handler_create.Create);
    }

    ResultTuple InvokeCreate(const std::string& name, const std::string& device_type);

private:
    FunctionRegistry<Status, ElementaryContract*> get_contract_functions_;
    FunctionRegistry<std::shared_ptr<void>> create_functions_;
};

} // namespace flow
#endif // SIMPLE_FLOW_ELEMENTARY_REGISTRY_H_
