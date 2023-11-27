

#include "runtime/scheduler_one_thread.h"
#include "graph_view.h"

namespace flow {

void SchedulerOneThread::DoSchedule(const PacketContextPtr& ctx,
                                    const std::shared_ptr<Packet>& pkt) {
    SIMPLE_LOG_INFO("Enter Do Schedule");
    auto gn_ctx = ctx->graph_node_ctx_;

    // do propagate
    DoPropagate(gn_ctx, graph_view_->GetGraphNode());

    // 从拓扑中，获取每层
    size_t depth = topology_->GetTopologyDepth();
    int level    = 0;
    do {
        auto l = topology_->GetNodesOnLevelN(level);
        for (auto& node : l) {
            size_t id = node->GetId();
            if (id == 8) {
                SIMPLE_LOG_TRACE("enter merge node");
            }
            auto per_node_ctx = ctx->GetNodeContextById(id);
            // check input ok
            if (!per_node_ctx->IsInputReady()) {
                continue;
            }

            // process
            node->ProcessCtx(per_node_ctx);

            // check output ok
            if (!per_node_ctx->IsOutputReady()) {
                continue;
            }
            DoPropagate(per_node_ctx, node);
        }
        ++level;
    } while (level < depth);

    // graph node 处理
    graph_view_->GetGraphNode()->ProcessCtx(gn_ctx);


    SIMPLE_LOG_INFO("Leave Do Schedule");
}

Status SchedulerOneThread::Initialize() {
    MATRIX_ASSERT(graph_view_);
    topology_ = graph_view_->GetTopology();
    return Status::OkStatus();
}

void SchedulerOneThread::DoPropagate(const PacketPerNodeContextPtr& per_node_ctx,
                                     const std::shared_ptr<Node>& node) {


    auto edges          = graph_view_->GetOutputEdges(node);
    auto packet_context = per_node_ctx->GetPacketContext();
    std::set<size_t> dst_ids;
    for (const auto& v : edges) {
        size_t dst_node_id = v->dst_node_->GetId();
        size_t dst_port_id = v->dst_port_->GetId();
        size_t src_node_id = v->src_node_->GetId();
        size_t src_port_id = v->src_port_->GetId();

        dst_ids.insert(dst_node_id);

        SIMPLE_LOG_TRACE("Propagate: src node {}, port {}, dst node {}, port {}",
                         src_node_id,
                         src_port_id,
                         dst_node_id,
                         dst_port_id);
        PacketPerNodeContextPtr dst = packet_context->GetNodeContextById(dst_node_id);

        // TODO: 抽象Propagate
        per_node_ctx->PropagateOutputPortTo(dst, dst_port_id, src_port_id);
    }

    // clear output
    for (auto& v : per_node_ctx->outputs_) {
        v.clear();
    }
}

void SchedulerOneThread::AddPacketContext(const PacketContextPtr& ctx,
                                          const std::shared_ptr<Packet>& pkt) {
    pool_.Commit([=]() {
        DoSchedule(ctx, pkt);
        if (graph_call_back_) {
            graph_call_back_(ctx);
            SIMPLE_LOG_TRACE("SchedulerOneThread process finished.");
        }
    });
}

} // namespace flow