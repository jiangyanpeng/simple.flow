#ifndef SIMPLE_FLOW_GRAPH_VIEW_H_
#define SIMPLE_FLOW_GRAPH_VIEW_H_

#include "device/device_registry.h"
#include "graph_spec_view.h"
#include "graph_topology.h"
#include "node.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace flow {
struct Edge {
    Edge(std::shared_ptr<Node> src_node,
         std::shared_ptr<InoutPort> src_port,
         std::shared_ptr<Node> dst_node,
         std::shared_ptr<InoutPort> dst_port);

    std::shared_ptr<Node> src_node_;
    std::shared_ptr<InoutPort> src_port_;
    std::shared_ptr<Node> dst_node_;
    std::shared_ptr<InoutPort> dst_port_;
};

/**
 * 通过GraphSpecView构造出具体的执行单元
 */
class GraphView : public std::enable_shared_from_this<GraphView> {
public:
    GraphView(std::shared_ptr<MatrixElementaryRegistry> registry,
              std::shared_ptr<DeviceRegistry> device_registry);

    ~GraphView();

    Status Initialize(std::shared_ptr<GraphSpecView> spec_view,
                      const std::shared_ptr<base::PipeManager>& global_thread_pool);


    std::shared_ptr<GraphTopology> GetTopology();

    //        std::shared_ptr<PacketPerNodeContext> CreatePacketPerNodeContext(size_t node_index);
    void InitPacketContext(const std::shared_ptr<PacketContext>& packet_context);

    std::vector<std::shared_ptr<Node>> GetSiblingNode(const std::shared_ptr<Node>& node);

    std::vector<size_t> GetOutputEdgeId(const std::shared_ptr<Node>& node);

    std::vector<std::shared_ptr<Edge>> GetOutputEdges(const std::shared_ptr<Node>& node);

    std::shared_ptr<Node> GetNodeWithId(size_t id);

    std::shared_ptr<Device> GetDeviceByName(const std::string& name);

    std::shared_ptr<Edge> GenerateEdgeWithSpec(const std::shared_ptr<EdgeSpec>& spec);

    Status OpenAllNodes();

    std::shared_ptr<Node> GetGraphNode() const;

    std::vector<size_t> GetNodeIdsByName(const std::vector<std::string>& node_names);

public:
    std::map<size_t, std::shared_ptr<Node>> nodes_map_;
    // 持有所有Device对象，其他里面只是引用
    // FIXME: 抽象成DeviceManager
    // key: Device Type, value: map<int, shared_ptr<Device>>
    //                   key: id, value: shared_ptr<Device>
    Collection<std::shared_ptr<Device>> devices_;

    std::map<size_t, std::shared_ptr<Edge>> edges_map_;

    std::shared_ptr<GraphSpecView> spec_view_;
    std::shared_ptr<MatrixElementaryRegistry> registry_;
    std::shared_ptr<DeviceRegistry> device_registry_;

    std::vector<std::shared_ptr<Node>>
    GetSiblingNode(const std::vector<std::shared_ptr<Edge>>& edges);

    std::atomic_int_fast64_t unique_id_for_packet_per_node_ctx_{0};

    // node的原始name与在graph中id的映射关系
    std::map<std::string, size_t> node_name_to_node_id_;
};

} // namespace flow
#endif // SIMPLE_FLOW_GRAPH_VIEW_H_
