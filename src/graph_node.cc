#include "graph_node.h"
#include "packet_context.h"

namespace flow {

void GraphNode::Process() {
    do {
        PacketPerNodeContextPtr node_ctx = nullptr;
        {
            std::unique_lock<std::mutex> lk(mtx_);
            if (inouts_queue_.empty()) {
                break;
            }
            SIMPLE_LOG_DEBUG("inout_queue size: {}", inouts_queue_.size());
            node_ctx = inouts_queue_.front();
            inouts_queue_.pop();
        }
        SIMPLE_LOG_DEBUG("graph node process: node_id [{}], pkt_id: {}",
                         GetId(),
                         node_ctx->GetPacketContext()->GetId());
        // 已经处理完成
        node_ctx->GetPacketContext()->SetFinished();
        node_ctx->Notify();
    } while (true);
}

size_t GraphNode::GetInputCount() const {
    return Node::GetOutputCount();
}

size_t GraphNode::GetOutputCount() const {
    return Node::GetInputCount();
}

void GraphNode::ProcessCtx(const std::shared_ptr<PacketPerNodeContext>& ctx) {
    PacketPerNodeContextPtr node_ctx = ctx;

    SIMPLE_LOG_DEBUG("graph node processctx: node_id [{}], pkt_id: {}",
                     GetId(),
                     node_ctx->GetPacketContext()->GetId());
    // 已经处理完成
    node_ctx->GetPacketContext()->SetFinished();
}
} // namespace flow