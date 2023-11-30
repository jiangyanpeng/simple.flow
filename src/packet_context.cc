

#include "packet_context.h"
#include "graph.h"

#include <log.h>

namespace flow {
PacketContext::PacketContext(const std::shared_ptr<Packet>& pkt)
    : pkt_(pkt),
      id_(pkt->GetInputPktId()->Id()),
      pkt_id_(pkt->GetInputPktId()->PacketId()),
      source_id_(pkt->GetInputPktId()->InputSourceId()) {}

PacketContext::~PacketContext() = default;

void PacketContext::SetGraphView(std::shared_ptr<GraphView> graph) {
    graph_view_ = std::move(graph);
}

Status PacketContext::Initialize() {
    graph_view_->InitPacketContext(shared_from_this());
    return Status::OkStatus();
}

void PacketContext::AddPacketPerNodeContext(size_t id, const PacketPerNodeContextPtr& ctx) {
    ctx->SetPacketContext(shared_from_this());
    //        node_ctxes_.emplace_back(ctx);
    node_ctxes_[id] = ctx;
}

void PacketContext::SetGraphNodeContext(const PacketPerNodeContextPtr& ctx) {
    ctx->SetPacketContext(shared_from_this());
    graph_node_ctx_ = ctx;
}

void PacketContext::DoDebug() {
    SIMPLE_LOG_INFO("PacketContext: ");
    for (auto& ctx : node_ctxes_) {
        //            SIMPLE_LOG_INFO("")
        ctx.second->DoDebug();
    }
    SIMPLE_LOG_INFO("End of PacketContext: ");
}

std::shared_ptr<PacketPerNodeContext> PacketContext::GetNodeContextById(size_t id) {
    SIMPLE_ASSERT(id < node_ctxes_.size());
    return node_ctxes_[id];
}

void PacketContext::SetFinished() {
    finished_ = true;
}

bool PacketContext::IsFinished() const {
    // FIXME: 多线程
    return finished_;
}

std::shared_ptr<InputStreamContext> PacketContext::GetInputSourceContext() const {
    return input_source_ctx_.lock();
}

void PacketContext::SetInputSourceContext(const std::shared_ptr<InputStreamContext>& ctx) {
    input_source_ctx_ = ctx;
}

uint64_t PacketContext::GetId() const {
    return id_;
}

uint64_t PacketContext::GetPacketId() const {
    return pkt_id_;
}

uint8_t PacketContext::GetSourceId() const {
    return source_id_;
}


std::shared_ptr<Packet> PacketContext::GetPacket() const {
    return pkt_;
}
} // namespace flow
