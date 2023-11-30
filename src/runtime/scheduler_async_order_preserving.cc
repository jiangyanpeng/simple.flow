
#include "runtime/scheduler_async_order_preserving.h"
#include "graph_view.h"
#include <chrono> // std::chrono::seconds

namespace flow {

void SchedulerAsyncOrderPreserving::AddPacketContext(const PacketContextPtr& ctx,
                                                     const std::shared_ptr<Packet>& pkt) {
    auto dst_ids = DoPropagate(ctx->graph_node_ctx_, graph_view_->GetGraphNode());
    for (auto id : dst_ids) {
        auto p_ctx                = ctx->GetNodeContextById(id);
        auto ready_task           = std::make_shared<ReadyTask>();
        ready_task->per_node_ctx_ = p_ctx;
        ready_task->pripority_    = ready_task->GetPriority(topology_); //按照优先级添加
        ready_task_queue_->AddTask(ready_task);
    }
}

SchedulerAsyncOrderPreserving::SchedulerAsyncOrderPreserving(
    size_t executor_thread_num,
    std::shared_ptr<ExecutorOption> executor_option)
    : executor_thread_num_(executor_thread_num), executor_option_(executor_option) {}

Status SchedulerAsyncOrderPreserving::Initialize() {
    executor_         = std::make_shared<Executor>(executor_thread_num_, executor_option_);
    ready_task_queue_ = std::make_shared<ReadyTaskQueue>();
    topology_         = graph_view_->GetTopology();
    return ready_task_queue_->Initialize();
}

void SchedulerAsyncOrderPreserving::DoSchedule() {
    pool_.Commit([&]() -> bool {
        while (!stop_) {
            auto t = ready_task_queue_->Top();
            if (t) {
                executor_->AddTask([=]() {
                    auto per_node_ctx = t->per_node_ctx_;
                    auto pkt_ctx      = per_node_ctx->GetPacketContext();



                    auto node            = per_node_ctx->GetNode();
                    bool need_order      = node->GetOrderPreserving();
                    size_t node_id       = node->GetId();
                    uint64_t next_pkt_id = 0;
                    auto input_source    = per_node_ctx->GetInputSourceContext();
                    uint8_t source_id    = input_source->GetId();
                    uint64_t pkt_id      = per_node_ctx->GetPacketContext()->GetPacketId();

                    // check input ok
                    if (!per_node_ctx->IsInputReady()) {
                        SIMPLE_LOG_DEBUG("Input not ready, node_id {}, pkt_id {}, source_id {} ",
                                         node_id,
                                         pkt_id,
                                         source_id);
                        return;
                    }

                    if (need_order &&
                        input_source->order_information_.GetNextPktId(node_id, next_pkt_id)) {
                        SIMPLE_LOG_DEBUG(
                            "keep order, node id {}, pkt_id {}, next_pkt_id {}, source_id {} ",
                            node_id,
                            pkt_id,
                            next_pkt_id,
                            source_id);
                        if (next_pkt_id < pkt_id) {
                            //加到缓存，后面再处理
                            SIMPLE_LOG_DEBUG("add pkt to buffer, node id {}, pkt_id {}, "
                                             "next_pkt_id {}, source_id {}",
                                             node_id,
                                             pkt_id,
                                             next_pkt_id,
                                             source_id);
                            input_source->order_information_.AddUnprocessedPkt(
                                node_id, pkt_id, per_node_ctx);
                        } else if (next_pkt_id == pkt_id) {
                            // 继续处理
                            SIMPLE_LOG_DEBUG("prepare process pkt, node id {}, pkt_id {}, "
                                             "next_pkt_id {}, source_id {} ",
                                             node_id,
                                             pkt_id,
                                             next_pkt_id,
                                             source_id);
                            node->ProcessCtx(per_node_ctx);
                            if (node->GetId() == graph_view_->GetGraphNode()->GetId()) {
                                if (graph_call_back_) {
                                    graph_call_back_(pkt_ctx);
                                }
                                // 处理完成
                                SIMPLE_LOG_DEBUG("SchedulerAsyncOrderPreserving, process finished,"
                                                 "pkt_id: {}, source_id: {}",
                                                 pkt_ctx->GetPacketId(),
                                                 pkt_ctx->GetInputSourceContext()->GetId());

                                input_source->order_information_.SetNextPktId(node_id,
                                                                              (pkt_id + 1));
                                return;
                            }
                            // check output ok
                            if (!per_node_ctx->IsOutputReady()) {
                                SIMPLE_LOG_DEBUG(
                                    "Output not ready, node_id {}, pkt_id {}, source_id {} ",
                                    node_id,
                                    pkt_id,
                                    source_id);
                                return;
                            }
                            auto dst_ids = DoPropagate(per_node_ctx, node);

                            // dst_ids maybe ready
                            for (auto id : dst_ids) {
                                auto p_ctx                = pkt_ctx->GetNodeContextById(id);
                                auto ready_task           = std::make_shared<ReadyTask>();
                                ready_task->per_node_ctx_ = p_ctx;
                                ready_task->pripority_    = ready_task->GetPriority(topology_);
                                ready_task_queue_->AddTask(ready_task);
                            }

                            input_source->order_information_.SetNextPktId(node_id, (pkt_id + 1));
                        } else {
                            //
                        }


                        // 处理缓存里的数据，添加到readtask
                        while (true) {
                            uint64_t smallest_pkt_id                                = 0;
                            std::shared_ptr<PacketPerNodeContext> next_per_node_ctx = nullptr;
                            if (input_source->order_information_.GetSmallestUnprocessedPkt(
                                    node_id, smallest_pkt_id, next_per_node_ctx)) {
                                SIMPLE_LOG_DEBUG("process pkt from buffer, node id {}, pkt_id {},"
                                                 "samllest_pkt_id {}, source_id {} ",
                                                 node_id,
                                                 pkt_id,
                                                 smallest_pkt_id,
                                                 source_id);
                                auto ready_task           = std::make_shared<ReadyTask>();
                                ready_task->per_node_ctx_ = next_per_node_ctx;
                                ready_task->pripority_    = ready_task->GetPriority(topology_);
                                ready_task_queue_->AddTask(ready_task);
                            } else {
                                break;
                            }
                        }

                    } else {

                        node->ProcessCtx(per_node_ctx);
                        if (node->GetId() == graph_view_->GetGraphNode()->GetId()) {
                            if (graph_call_back_) {
                                graph_call_back_(pkt_ctx);
                            }
                            // 处理完成
                            SIMPLE_LOG_INFO("SchedulerAsyncOrderPreserving, process finished, "
                                            "pkt_id: {}, source_id: {}.",
                                            pkt_ctx->GetPacketId(),
                                            pkt_ctx->GetInputSourceContext()->GetId());
                            return;
                        }
                        // check output ok
                        if (!per_node_ctx->IsOutputReady()) {
                            SIMPLE_LOG_DEBUG(
                                "output not ready, node_id {}, pkt_id {}", node_id, pkt_id);
                            return;
                        }
                        auto dst_ids = DoPropagate(per_node_ctx, node);

                        // dst_ids maybe ready
                        for (auto id : dst_ids) {
                            auto p_ctx                = pkt_ctx->GetNodeContextById(id);
                            auto ready_task           = std::make_shared<ReadyTask>();
                            ready_task->per_node_ctx_ = p_ctx;
                            ready_task->pripority_    = ready_task->GetPriority(topology_);
                            ready_task_queue_->AddTask(ready_task);
                        }
                    }
                });
            } else {
                // std::this_thread::sleep_for(std::chrono::milliseconds(1));
                std::this_thread::yield();
                // std::this_thread::sleep_for(std::chrono::seconds (1));
                // SIMPLE_LOG_INFO("scheduler waiting....");
            }
        }
        stopped_ = true;
        stopped_cd_.notify_one();
        return true;
    });
}

std::set<size_t>
SchedulerAsyncOrderPreserving::DoPropagate(const PacketPerNodeContextPtr& per_node_ctx,
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

        SIMPLE_LOG_DEBUG("Propagate: src node {}, port {}, dst node {}, port {}",
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
    return dst_ids;
}

void SchedulerAsyncOrderPreserving::Start() {
    DoSchedule();
}

void SchedulerAsyncOrderPreserving::Stop() {
    stop_.store(true);
    SIMPLE_LOG_DEBUG("stop_debug: SchedulerAsyncOrderPreserving::Stop 01..... ");
    std::unique_lock<std::mutex> lk(mtx_);
    SIMPLE_LOG_DEBUG("stop_debug: SchedulerAsyncOrderPreserving::Stop 02..... ");
    // stopped_cd_.wait(lk, [&] { return stopped_; });
    stopped_cd_.wait_for(lk, std::chrono::seconds(10), [&] { return stopped_; });
    SIMPLE_LOG_DEBUG("stop_debug: SchedulerAsyncOrderPreserving::Stop 03..... ");
    pool_.Stop();
    SIMPLE_LOG_INFO("SchedulerAsyncOrderPreserving stopped");
}

} // namespace flow