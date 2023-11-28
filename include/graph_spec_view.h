#ifndef SIMPLE_FLOW_GRAPH_SPEC_VIEW_H_
#define SIMPLE_FLOW_GRAPH_SPEC_VIEW_H_

#include "base/status.h"
#include "specs/graph_spec.h"

#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace flow {
class EdgeSpec {
public:
    EdgeSpec(size_t id,
             std::shared_ptr<NodeSpec> src_node,
             std::shared_ptr<InoutSpec> src_port,
             std::shared_ptr<NodeSpec> dst_node,
             std::shared_ptr<InoutSpec> dst_port,
             std::string link_name)
        : edge_id(id),
          src_node_spec_(std::move(src_node)),
          src_port_spec_(std::move(src_port)),
          dst_node_spec_(std::move(dst_node)),
          dst_port_spec_(std::move(dst_port)),
          link_name(std::move(link_name)) {}
    ~EdgeSpec() = default;

    size_t edge_id;
    std::shared_ptr<NodeSpec> src_node_spec_;
    std::shared_ptr<InoutSpec> src_port_spec_;
    std::shared_ptr<NodeSpec> dst_node_spec_;
    std::shared_ptr<InoutSpec> dst_port_spec_;
    std::string link_name;
};

/**
 * 图的静态描述
 * 主要完成如下功能：
 *      1. 完成图的描述信息校验
 *      2. 完成图描述的信息补齐：包括各种资源的id，name，连接关系等
 *      3. 支持完成图的静态优化，如Node合并
 *      4. 生成连接有用信息，供执行时使用
 */
class GraphView;

class GraphSpecView {
public:
    using SpecName = std::string;
    explicit GraphSpecView(std::shared_ptr<GraphSpec> spec);

    ~GraphSpecView() = default;

    /**
     * 将外部配置生成的GraphSpec进行处理
     * 主要做如下动作：
     * 1. 解决Node的连接关系构建
     * 2. 每个Spec的ID信息生成，注意ID生成为全递增，且从0开始，便于vector保存信息
     * @return
     */
    Status Initialize();

    Status Optimize();

    /**
     * 添加子图
     * @param spec
     * @return
     */
    Status AddSubGraph(std::shared_ptr<GraphSpec> spec);

    std::shared_ptr<GraphSpec> GetGraphSpe() const;
    const std::map<SpecName, std::shared_ptr<NodeSpec>>& GetNodeSpecs() const;
    const std::map<SpecName, std::shared_ptr<DeviceSpec>>& GetDeviceSpecs() const;
    const std::map<size_t, std::shared_ptr<EdgeSpec>>& GetEdgeSpecs() const;
    const std::map<std::string, size_t> GetNodeSpecNameAndId() const;

    std::vector<size_t> GetOutputEdgeIdWithNodeId(size_t id) const;
    std::vector<size_t> GetInputEdgeIdWithNodeId(size_t id) const;


private:
    Status AddNodeSpec(std::shared_ptr<NodeSpec> spec);

    Status AttachNodeNameAndId(std::string name, size_t id);

    Status AddDeviceSpec(std::shared_ptr<DeviceSpec> spec);

    Status GenerateEdgeSpec();

    std::vector<std::shared_ptr<NodeSpec>> GetEdgeRelatedNode(size_t id) const;

    Status ParsingNodeSpec(const std::shared_ptr<NodeSpec>& spec,
                           size_t index,
                           const std::string& prefix_name);

private:
    std::shared_ptr<GraphSpec> graph_spec_;
    std::map<SpecName, std::shared_ptr<NodeSpec>> nodes_;
    std::map<SpecName, size_t> node_name_to_node_id_;
    std::map<SpecName, std::shared_ptr<DeviceSpec>> devices_;

    std::shared_ptr<GraphSpec> ori_graph_spec_;

    std::map<size_t, std::shared_ptr<EdgeSpec>> edges_;
    std::multimap<size_t, size_t> node_id_to_input_edge_id_map_;
    std::multimap<size_t, size_t> node_id_to_output_edge_id_map_;
};

} // namespace flow
#endif // SIMPLE_FLOW_GRAPH_SPEC_VIEW_H_
