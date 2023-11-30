#ifndef SIMPLE_FLOW_CALCULATOR_CONTRACT_H_
#define SIMPLE_FLOW_CALCULATOR_CONTRACT_H_

#include "core/collection.h"
#include "core/data_type.h"
#include "core/status.h"
#include "calculator/calculator_option.h"
#include "spec/graph_spec.h"

#include <memory>
#include <set>

namespace flow {
/**
 * 通过Node的描述中的options、inputs、outputs信息生成
 * 用于描述Elementary的对外承诺：输入、输出的tag名称、id、数据类型、提供哪些参数
 */
class CalculatorContract {
public:
    CalculatorContract();
    ~CalculatorContract();

    Status Initialize();

    CalculatorContract& AddInput(const std::string& port_name, const std::string& data_type_name);
    CalculatorContract& AddOutput(const std::string& port_name, const std::string& data_type_name);
    CalculatorContract& SetInputHandler(const std::string& input_handler_name);
    CalculatorContract& SetCalculatorName(const std::string& elem_name);
    CalculatorContract& SetCalculatorOptionName(const std::string& elem_option_name);
    CalculatorContract& AddSupportedDevice(const std::string& device_name);

    std::string DebugString();

private:
    // inputs
    Collection<DataType> inputs_;
    // outputs
    Collection<DataType> outputs_;

    std::string input_handler_name_{"matrix::InOutputHandler"};
    std::string elementary_name_{""};
    std::string elementary_option_name_{"matrix::CalculatorOption"};
    std::set<std::string> supported_device_;
};

} // namespace flow
#endif // SIMPLE_FLOW_CALCULATOR_CONTRACT_H_
