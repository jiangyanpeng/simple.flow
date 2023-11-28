#ifndef SIMPLE_FLOW_ELEMENTARY_CONTRACT_H_
#define SIMPLE_FLOW_ELEMENTARY_CONTRACT_H_

#include "base/collection.h"
#include "base/data_type.h"
#include "base/status.h"
#include "elementary_option.h"
#include "specs/graph_spec.h"

#include <memory>
#include <set>

namespace flow {
/**
 * 通过Node的描述中的options、inputs、outputs信息生成
 * 用于描述Elementary的对外承诺：输入、输出的tag名称、id、数据类型、提供哪些参数
 */
class ElementaryContract {
public:
    ElementaryContract();
    ~ElementaryContract();

    Status Initialize();

    ElementaryContract& AddInput(const std::string& port_name, const std::string& data_type_name);
    ElementaryContract& AddOutput(const std::string& port_name, const std::string& data_type_name);
    ElementaryContract& SetInputHandler(const std::string& input_handler_name);
    ElementaryContract& SetElementaryName(const std::string& elem_name);
    ElementaryContract& SetElemOptionName(const std::string& elem_option_name);
    ElementaryContract& AddSupportedDevice(const std::string& device_name);

    std::string DebugString();

private:
    // inputs
    Collection<DataType> inputs_;
    // outputs
    Collection<DataType> outputs_;

    std::string input_handler_name_{"matrix::InOutputHandler"};
    std::string elementary_name_{""};
    std::string elementary_option_name_{"matrix::ElementaryOption"};
    std::set<std::string> supported_device_;
};

} // namespace flow
#endif // SIMPLE_FLOW_ELEMENTARY_CONTRACT_H_
