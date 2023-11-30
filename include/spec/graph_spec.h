#ifndef SIMPLE_FLOW_GRAPH_SPEC_H_
#define SIMPLE_FLOW_GRAPH_SPEC_H_

#include <memory>
#include <string>
#include <vector>

namespace flow {
// The direction of stream flow
// IN is input stream
// OUT is output stream
enum Direction {
    IN  = 0,
    OUT = 1,
};

class NodeSpec;

// InoutPort Spec
//
// name is port name, can repeatable
// link_name is used to represent how multiple nodes are linked
// direction
// option
// data_type_name is port input/output data type
//
// id, Internally generated and unique ID, increasing from 0
// belonged_to_ is this port on where node
//
// id and belong_to_ is automatically generated
struct InoutSpec {
    std::string name{};
    std::string link_name{};
    Direction direction{IN};
    bool optional{false};
    std::string data_type_name{};

    size_t id{};
    std::weak_ptr<NodeSpec> belonged_to_;

    std::string to_string();
};

class CalculatorOption;


// Device spec information
//
// Name is globally unique in the Graph, Device in Node corresponds to it through name,
// ID is globally unique in the Graph, automatically generated,
// and not configured in the description file
//
// id is automatically generated
class DeviceSpec final {
    friend class GraphViewSpec;

public:
    DeviceSpec()  = default;
    ~DeviceSpec() = default;

    DeviceSpec& SetType(std::string type);
    DeviceSpec& SetName(std::string name);

    std::string Type() const;
    std::string Name() const;
    size_t Id() const;

protected:
    DeviceSpec& SetId(size_t id);

public:
    std::string type_{};
    size_t id_{0};
    std::string name_{};
};

// CalculatorSpec
// The most basic computing unit describes
//
// calculator_name_ and  calculator_id_ is automatically generated
class CalculatorSpec final {
    friend class NodeSpec;
    friend class GraphViewSpec;

public:
    CalculatorSpec()  = default;
    ~CalculatorSpec() = default;

    CalculatorSpec& SetDeviceName(std::string device_name);
    std::string GetDeviceName() const;
    size_t GetId() const;

protected:
    CalculatorSpec& SetName(std::string name);
    CalculatorSpec& SetId(size_t id);

private:
    std::string calculator_name_{};
    size_t calculator_id_{};
    std::string device_name_{};
};


// NodeSpec
// Node describes a set of units used to accomplish specific abilities. From a graph perspective,
// Node is the fundamental unit of graph scheduling. Data transmission is from node to node.
// Multiple calculator objects can be maintained within a node to improve performance. The
// difference between multiple elements lies in the device
//
// Contains three parts of information:
// (1) name, id, numbers of calculator...
// (2) calculator name, opeion name, input/output [InoutSpec]
// (3) device information
//
class NodeSpec {
public:
    NodeSpec()          = default;
    virtual ~NodeSpec() = default;

    virtual void LoadFromFile(std::string file_name);

    NodeSpec& SetName(std::string name);
    NodeSpec& SetId(size_t id);
    NodeSpec& SetCalculatorType(std::string type);
    NodeSpec& SetCalculatorOptionType(std::string type);
    NodeSpec& SetCalculatorOptionJsonValue(std::string type);
    NodeSpec& SetInputHandler(std::string h);
    NodeSpec& SetOutputHandler(std::string h);
    NodeSpec& SetCalculatorOption(std::shared_ptr<CalculatorOption> o);
    NodeSpec& SetOrderPreserving(bool preserving);
    NodeSpec& AddInputSpec(InoutSpec spec);
    NodeSpec& AddOutputSpec(InoutSpec spec);
    NodeSpec& SetElementaryNum(size_t n);
    NodeSpec& AddElemSpec(const CalculatorSpec& spec);

    NodeSpec& AddSkipPacketNode(std::string name);

    std::vector<InoutSpec>& GetInputSpecs();
    std::vector<InoutSpec>& GetOutputSpecs();
    std::vector<CalculatorSpec>& GetElemSpecs();

    std::vector<std::string>& GetSkipPacketNode();

    std::string GetName() const;
    size_t GetId() const;
    std::string GetElemType() const;

    std::string to_string() const;

public:
    std::string node_name;
    size_t node_id{0};
    std::string calculator_type{};
    std::string calculator_option_type{};
    std::string calculator_option_json_value{};
    std::shared_ptr<CalculatorOption> option;
    std::string input_handler{};
    std::string output_handler{};
    std::vector<InoutSpec> inputs{};
    std::vector<InoutSpec> outputs{};

    std::vector<std::string> skip_pkt_node{};

    bool order_preserving_{false};

    size_t calculator_num{1};
    std::vector<CalculatorSpec> calculators{};

protected:
    bool is_graph{false};
};


class GraphSpec : public NodeSpec {
public:
    GraphSpec();

    ~GraphSpec() override;

    void LoadFromFile(std::string file_name) override;

    void AddNodeSpec(const std::shared_ptr<NodeSpec>& node);

    void AddDeviceSpec(const DeviceSpec& spec);

    std::vector<std::shared_ptr<NodeSpec>>& GetNodeSpecs();

    std::vector<DeviceSpec>& GetDevices();

    NodeSpec& AddGraphInputSpec(InoutSpec spec);
    NodeSpec& AddGraphOutputSpec(InoutSpec spec);

private:
    std::vector<std::shared_ptr<NodeSpec>> nodes_;
    std::vector<DeviceSpec> devices_;
};

} // namespace flow
#endif // SIMPLE_FLOW_GRAPH_SPEC_H_
