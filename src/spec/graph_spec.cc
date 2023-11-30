#include "spec/graph_spec.h"

#include <log.h>
#include <sstream>
#include <utility>

namespace flow {

std::string InoutSpec::to_string() {
    std::stringstream ss;
    ss << "name[" << name << "], link_name[" << link_name << "], id[" << id << "], direction["
       << static_cast<int>(direction) << "], optional[" << optional << "], data_type_name["
       << data_type_name << "]";
    return ss.str();
}
CalculatorSpec& CalculatorSpec::SetDeviceName(std::string device_name) {
    device_name_ = std::move(device_name);
    return *this;
}


CalculatorSpec& CalculatorSpec::SetName(std::string name) {
    calculator_name_ = std::move(name);
    return *this;
}

CalculatorSpec& CalculatorSpec::SetId(size_t id) {
    calculator_id_ = id;
    return *this;
}

std::string CalculatorSpec::GetDeviceName() const {
    return device_name_;
}

size_t CalculatorSpec::GetId() const {
    return calculator_id_;
}

GraphSpec::GraphSpec() {
    is_graph = true;
    SIMPLE_LOG_DEBUG("Creat GraphSpec: is_grapg[{}]", is_graph);
}

GraphSpec::~GraphSpec() {}

void GraphSpec::LoadFromFile(std::string file_name) {
    NodeSpec::LoadFromFile(file_name);
}

void GraphSpec::AddNodeSpec(const std::shared_ptr<NodeSpec>& node) {
    SIMPLE_LOG_INFO("   AddNodeSpec: {}", node->to_string());
    nodes_.emplace_back(node);
}

std::vector<std::shared_ptr<NodeSpec>>& GraphSpec::GetNodeSpecs() {
    return nodes_;
}

void GraphSpec::AddDeviceSpec(const DeviceSpec& spec) {
    devices_.emplace_back(spec);
}

std::vector<DeviceSpec>& GraphSpec::GetDevices() {
    return devices_;
}

NodeSpec& GraphSpec::AddGraphInputSpec(InoutSpec spec) {
    SIMPLE_LOG_INFO("   AddGraphInputSpec: {}", spec.to_string());
    outputs.emplace_back(spec);
    return *this;
}

NodeSpec& GraphSpec::AddGraphOutputSpec(InoutSpec spec) {
    SIMPLE_LOG_INFO("   AddGraphOutputSpec: {}", spec.to_string());
    inputs.emplace_back(spec);
    return *this;
}

void NodeSpec::LoadFromFile(std::string file_name) {}

NodeSpec& NodeSpec::SetName(std::string name) {
    node_name = std::move(name);
    return *this;
}

NodeSpec& NodeSpec::SetId(size_t id) {
    node_id = id;
    return *this;
}

NodeSpec& NodeSpec::SetCalculatorType(std::string type) {
    calculator_type = std::move(type);
    return *this;
}

NodeSpec& NodeSpec::SetCalculatorOptionType(std::string type) {
    calculator_option_type = std::move(type);
    return *this;
}

NodeSpec& NodeSpec::SetCalculatorOptionJsonValue(std::string value) {
    calculator_option_json_value = std::move(value);
    return *this;
}

NodeSpec& NodeSpec::SetInputHandler(std::string h) {
    input_handler = std::move(h);
    return *this;
}

NodeSpec& NodeSpec::SetOutputHandler(std::string h) {
    output_handler = std::move(h);
    return *this;
}

NodeSpec& NodeSpec::SetCalculatorOption(std::shared_ptr<CalculatorOption> o) {
    option = std::move(o);
    return *this;
}

NodeSpec& NodeSpec::SetOrderPreserving(bool preserving) {
    order_preserving_ = preserving;
    return *this;
}

NodeSpec& NodeSpec::AddInputSpec(InoutSpec spec) {
    inputs.emplace_back(std::move(spec));
    return *this;
}

NodeSpec& NodeSpec::AddOutputSpec(InoutSpec spec) {
    outputs.emplace_back(std::move(spec));
    return *this;
}

NodeSpec& NodeSpec::SetElementaryNum(size_t n) {
    calculator_num = n;
    return *this;
}

NodeSpec& NodeSpec::AddElemSpec(const CalculatorSpec& spec) {
    calculators.emplace_back(spec);
    return *this;
}

NodeSpec& NodeSpec::AddSkipPacketNode(std::string name) {
    skip_pkt_node.emplace_back(name);
    return *this;
}

std::vector<std::string>& NodeSpec::GetSkipPacketNode() {
    return skip_pkt_node;
}

std::vector<InoutSpec>& NodeSpec::GetInputSpecs() {
    return inputs;
}

std::vector<InoutSpec>& NodeSpec::GetOutputSpecs() {
    return outputs;
}

std::string NodeSpec::GetName() const {
    return node_name;
}

std::vector<CalculatorSpec>& NodeSpec::GetElemSpecs() {
    return calculators;
}

std::string NodeSpec::GetElemType() const {
    return calculator_type;
}

size_t NodeSpec::GetId() const {
    return node_id;
}

std::string NodeSpec::to_string() const {
    std::stringstream ss;
    ss << "node_name[" << node_name << "], node_id[" << node_id << "], calculator_type["
       << calculator_type << "], calculator_option_type[" << calculator_option_type
       << "], calculator_option_json_value[" << calculator_option_json_value << "], input_handler["
       << input_handler << "], output_handler[" << output_handler << "], inputs[" << inputs.size()
       << "], outputs[" << outputs.size() << "]";
    return ss.str();
}

//    InoutSpec &NodeSpec::AddInputSpec() {
//        inputs.emplace_back();
//        return <#initializer#>;
//    }
DeviceSpec& DeviceSpec::SetType(std::string type) {
    type_ = std::move(type);
    return *this;
}

std::string DeviceSpec::Type() const {
    return type_;
}

std::string DeviceSpec::Name() const {
    return name_;
}

size_t DeviceSpec::Id() const {
    return id_;
}

DeviceSpec& DeviceSpec::SetId(size_t id) {
    id_ = id;
    return *this;
}

DeviceSpec& DeviceSpec::SetName(std::string name) {
    name_ = std::move(name);
    return *this;
}
} // namespace flow