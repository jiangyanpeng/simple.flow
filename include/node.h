#ifndef ARCHITECT_NODE_H
#define ARCHITECT_NODE_H

#include "base/collection.h"
#include "base/status.h"
#include "device/device.h"
#include "elementary/elementary.h"
#include "inout_port.h"
#include "matrix_elementary_registry.h"
#include "packet_per_node_context.h"
#include "runtime/scheduler.h"
#include "specs/graph_spec.h"
#include <list>
#include <mutex>

namespace flow {

class Graph;

class GraphView;

/**
 * 根据Node描述，每个Node可能有多个Elementary，且可能放置在不通过Device
 * Process进行Elementary内的调度
 */
class Node : public std::enable_shared_from_this<Node> {
public:
    Node() = default;

    virtual ~Node() = default;

    virtual Status Open();

    virtual Status Initialize(std::shared_ptr<NodeSpec> spec,
                              const std::shared_ptr<MatrixElementaryRegistry>& registry);

    virtual void Process();

    virtual void ProcessCtx(const std::shared_ptr<PacketPerNodeContext>& ctx);

    bool IsNodeContextReady(const PacketPerNodeContextPtr& ctx);

    void AddInoutContext(const PacketPerNodeContextPtr& ctx);

    void SetGraphViewWeakPtr(std::weak_ptr<GraphView> g);

    const std::shared_ptr<InoutPort>& GetInputPortWithId(size_t id) const;

    size_t GetInputPortIdWithTag(const std::string& tag) const;

    const std::shared_ptr<InoutPort>& GetOutputPortWithId(size_t id) const;

    size_t GetOutputPortIdWithTag(const std::string& tag) const;

    std::shared_ptr<NodeSpec> GetSpec() const;

    size_t GetId() const;

    std::string GetName() const;

    bool GetOrderPreserving() const;

    virtual size_t GetInputCount() const;

    virtual size_t GetOutputCount() const;


    std::shared_ptr<PacketPerNodeContext> CreatePerNodeContext();

    std::vector<std::string>& GetSkipPacketNode();

    std::vector<size_t> GetNodeIdsByName();

    // FIXME
public:
    //    protected:
    std::shared_ptr<NodeSpec> spec_;
    std::shared_ptr<Scheduler> scheduler_;


    std::vector<std::shared_ptr<InoutPort>> input_ports_;
    std::vector<std::shared_ptr<InoutPort>> output_ports_;
    std::map<std::string, size_t> input_name_to_id_map_;
    std::map<std::string, size_t> output_name_to_id_map_;


    std::vector<std::shared_ptr<Elementary>> elementarys_;

    std::shared_ptr<ElementaryOption> elementary_option_;
    std::shared_ptr<InOutputHandler> inout_put_handler_;

    // 此处为队列，隔离Packet粒度的调度与Node的执行
    // FIXME: 此处应采用无锁队列
    std::queue<PacketPerNodeContextPtr> inouts_queue_;
    std::list<PacketPerNodeContextPtr> inouts_lists_;
    std::mutex mtx_;

    size_t id_{0};
    std::string name_{};
    bool order_preserving_{false};

    // 跳过packet节点集合
    std::vector<std::string> skip_pkt_node_{};

    std::weak_ptr<GraphView> weak_graph_;

    //        PacketPerNodeContextPtr node_ctx;
};

} // namespace flow


#endif // ARCHITECT_NODE_H
