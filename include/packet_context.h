#ifndef SIMPLE_FLOW_PACKET_CONTEXT_H_
#define SIMPLE_FLOW_PACKET_CONTEXT_H_

#include "core/status.h"
#include "packet_per_node_context.h"
#include "stream/input_stream_context.h"

#include <memory>
#include <vector>

namespace flow {
class GraphView;

class Graph;

class Edge;

class Packet;

class PacketPerNodeContext;

class InputStreamContext;

using PacketPerNodeContextPtr = std::shared_ptr<PacketPerNodeContext>;

class PacketContext : public std::enable_shared_from_this<PacketContext> {
public:
    explicit PacketContext(const std::shared_ptr<Packet>& pkt);

    ~PacketContext();

    Status Initialize();

    void SetGraphView(std::shared_ptr<GraphView> graph);

    void AddPacketPerNodeContext(size_t id, const PacketPerNodeContextPtr& ctx);

    void SetGraphNodeContext(const PacketPerNodeContextPtr& ctx);

    std::shared_ptr<PacketPerNodeContext> GetNodeContextById(size_t id);

    std::shared_ptr<InputStreamContext> GetInputSourceContext() const;

    void SetInputStreamContext(const std::shared_ptr<InputStreamContext>& ctx);

    void DoDebug();

    bool IsFinished() const;

    void SetFinished();

    uint64_t GetId() const;

    uint64_t GetPacketId() const;

    uint8_t GetStreamId() const;

    std::shared_ptr<Packet> GetPacket() const;

public:
    // owned
    //        std::vector<PacketPerNodeContextPtr> node_ctxes_;
    // key: node_id, value: ctxptr
    std::map<size_t, PacketPerNodeContextPtr> node_ctxes_;
    // not owned
    std::weak_ptr<InputStreamContext> input_stream_ctx_;

    // not owned
    std::shared_ptr<GraphView> graph_view_;
    // 该Packet在图上的输入
    PacketPerNodeContextPtr graph_node_ctx_;

    bool finished_{false};

    uint64_t id_{UINT64_MAX};

    uint64_t pkt_id_{UINT64_MAX};

    uint8_t source_id_{UINT8_MAX};

    std::shared_ptr<Packet> pkt_;
};

using PacketContextPtr = std::shared_ptr<PacketContext>;

} // namespace flow
#endif // SIMPLE_FLOW_PACKET_CONTEXT_H_
