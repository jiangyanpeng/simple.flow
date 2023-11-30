#ifndef SIMPLE_FLOW_GRAPH_SPEC_H_
#define SIMPLE_FLOW_GRAPH_SPEC_H_

#include <memory>
#include <string>
#include <vector>

namespace flow {
// Node或Elementary的端口方向，输入输出。站在Elementary角度来描述。
enum Direction { IN = 0, OUT = 1 };
class NodeSpec;
struct InoutSpec {
    // port名称，是Node中的特定Port的名称
    std::string name{};
    // link_name, 链接名称。用来表征多个Port如何链接
    std::string link_name{};
    // 图编排时不设置
    size_t id{};
    Direction direction{IN};
    bool optional{false};
    std::string data_type_name{};
    // 内部使用，不配置
    std::weak_ptr<NodeSpec> belonged_to_;

    std::string to_string();
};

class CalculatorOption;

/**
 * Device描述信息
 * Device管理采用平铺管理的方式
 * name在Graph中全局唯一，Node中的Device 通过name来进行对应
 * id在Graph中全局唯一，自动生成，描述文件中不配置
 */
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
/**
 * Node描述
 * Node描述的是一组用于完成特定能力的单元。从图的角度来看，Node是图调度的基本单元。数据传递都是从Node到Node。
 * 一个Node中可以维护多个Elementary对象，用于提升性能。多Elementary间的差异在于Device
 * 包含三部分信息：
 * 1. 基本信息：name, id，包含elem个数
 * 2. 计算相关：Elementary名称，Option名称，输入输出
 * 3. Elem单元相关：elem_name，elem_id，device信息
 */
class ElemSpec final {
    friend class NodeSpec;
    friend class GraphViewSpec;

public:
    ElemSpec()  = default;
    ~ElemSpec() = default;


    ElemSpec& SetDeviceName(std::string device_name);
    std::string GetDeviceName() const;
    size_t GetId() const;

protected:
    ElemSpec& SetName(std::string name);
    ElemSpec& SetId(size_t id);

private:
    // 自动生成
    std::string elem_name_{};
    // 自动生成
    size_t elem_id_{};
    std::string device_name_{};
};

class NodeSpec {
public:
    NodeSpec()          = default;
    virtual ~NodeSpec() = default;

    virtual void LoadFromFile(std::string file_name);

    NodeSpec& SetName(std::string name);
    NodeSpec& SetId(size_t id);
    NodeSpec& SetElementaryType(std::string type);
    NodeSpec& SetElementaryOptionType(std::string type);
    NodeSpec& SetElementaryOptionJsonValue(std::string type);
    NodeSpec& SetInputHandler(std::string h);
    NodeSpec& SetOutputHandler(std::string h);
    NodeSpec& SetCalculatorOption(std::shared_ptr<CalculatorOption> o);
    NodeSpec& SetOrderPreserving(bool preserving);
    NodeSpec& AddInputSpec(InoutSpec spec);
    //        InoutSpec& AddInputSpec();
    NodeSpec& AddOutputSpec(InoutSpec spec);
    NodeSpec& SetElementaryNum(size_t n);
    NodeSpec& AddElemSpec(const ElemSpec& spec);

    //添加需要跳过packet的节点的名字
    NodeSpec& AddSkipPacketNode(std::string name);

    std::vector<InoutSpec>& GetInputSpecs();
    std::vector<InoutSpec>& GetOutputSpecs();
    std::vector<ElemSpec>& GetElemSpecs();

    std::vector<std::string>& GetSkipPacketNode();

    std::string GetName() const;
    size_t GetId() const;
    std::string GetElemType() const;

    std::string to_string() const;

public:
    std::string node_name;
    size_t node_id{0};
    std::string elementary_type{};
    std::string elementary_option_type{};
    std::string elementary_option_json_value{};
    std::shared_ptr<CalculatorOption> option;
    std::string input_handler{};
    std::string output_handler{};
    std::vector<InoutSpec> inputs{};
    std::vector<InoutSpec> outputs{};

    // 跳过packet节点集合
    std::vector<std::string> skip_pkt_node{};

    // Node处理保序
    bool order_preserving_{false};

    size_t elem_num{1};
    std::vector<ElemSpec> elemes{};

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

    // 图的输入，体现到GraphNode为输出
    NodeSpec& AddGraphInputSpec(InoutSpec spec);
    //        InoutSpec& AddInputSpec();
    NodeSpec& AddGraphOutputSpec(InoutSpec spec);

private:
    std::vector<std::shared_ptr<NodeSpec>> nodes_;
    std::vector<DeviceSpec> devices_;
};

} // namespace flow
#endif // SIMPLE_FLOW_GRAPH_SPEC_H_
