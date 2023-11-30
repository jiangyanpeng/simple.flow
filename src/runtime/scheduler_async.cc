
#include "runtime/scheduler_async.h"
#include "graph_view.h"
#include "node.h"
#include <chrono>


namespace flow {

void SchedulerAsync::AddPacketContext(const PacketContextPtr& ctx,
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

SchedulerAsync::SchedulerAsync(size_t executor_thread_num)
    : executor_thread_num_(executor_thread_num) {}

Status SchedulerAsync::Initialize() {
    SIMPLE_LOG_DEBUG("SchedulerAsync::Initialize Start");
    executor_         = std::make_shared<Executor>(executor_thread_num_);
    ready_task_queue_ = std::make_shared<ReadyTaskQueue>();
    topology_         = graph_view_->GetTopology();
    SIMPLE_LOG_DEBUG("SchedulerAsync::Initialize End");
    return ready_task_queue_->Initialize();
}

void SchedulerAsync::DoSchedule() {
    SIMPLE_LOG_DEBUG("SchedulerAsync::DoSchedule Start");
    pool_.Commit([&]() -> bool {
        while (!stop_) {
            auto t = ready_task_queue_->Top();
            if (t) {
                executor_->AddTask([=]() {
                    auto per_node_ctx = t->per_node_ctx_;
                    auto pkt_ctx      = per_node_ctx->GetPacketContext();

                    // check input ok
                    if (!per_node_ctx->IsInputReady()) {
                        return;
                    }
                    auto node = per_node_ctx->GetNode();
                    node->ProcessCtx(per_node_ctx);
                    if (node->GetId() == graph_view_->GetGraphNode()->GetId()) {
                        if (graph_call_back_) {
                            graph_call_back_(pkt_ctx);
                        }
                        // 处理完成
                        SIMPLE_LOG_INFO(
                            "SchedulerAsync, process finished, pkt_id: {}, source_id: {}.",
                            pkt_ctx->GetId(),
                            pkt_ctx->GetInputSourceContext()->GetId());
                        return;
                    }
                    // check output ok
                    if (!per_node_ctx->IsOutputReady()) {
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
                });
            } else {
                //                                  std::this_thread::sleep_for(std::chrono::milliseconds(1));
                std::this_thread::yield();
                //                                  std::this_thread::sleep_for(std::chrono::seconds
                //                                  (1)); SIMPLE_LOG_INFO("scheduler waiting....");
            }
        }
        stopped_ = true;
        stopped_cd_.notify_one();
        return true;
    });
    SIMPLE_LOG_DEBUG("SchedulerAsync::DoSchedule End");
}

std::set<size_t> SchedulerAsync::DoPropagate(const PacketPerNodeContextPtr& per_node_ctx,
                                             const std::shared_ptr<Node>& node) {

    SIMPLE_LOG_DEBUG("SchedulerAsync::DoPropagate Start, per_node_ctx: {}",
                     per_node_ctx->GetUniqueId());

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
    SIMPLE_LOG_DEBUG("SchedulerAsync::DoPropagate End");
    return dst_ids;
}

void SchedulerAsync::Start() {
    SIMPLE_LOG_DEBUG("SchedulerAsync::Start Start");
    DoSchedule();
    SIMPLE_LOG_DEBUG("SchedulerAsync::Start End");
}

void SchedulerAsync::Stop() {
    stop_.store(true);
    SIMPLE_LOG_DEBUG("stop_debug: SchedulerAsync::Stop 01..... ");
    std::unique_lock<std::mutex> lk(mtx_);
    SIMPLE_LOG_DEBUG("stop_debug: SchedulerAsync::Stop 02..... ");
    // stopped_cd_.wait(lk, [&] { return stopped_; });
    stopped_cd_.wait_for(lk, std::chrono::seconds(10), [&] { return stopped_; });
    SIMPLE_LOG_DEBUG("stop_debug: SchedulerAsync::Stop 03..... ");
    pool_.Stop();
    SIMPLE_LOG_INFO("SchedulerAsync stopped");
}


void ReadyTaskQueue::AddTask(const ReadyTaskPtr& t) {
    // get priority
    // FIXME: 映射到0~15
    int8_t p = t->pripority_;

    std::unique_lock<std::mutex> lk(p_queue_mtx_);
    p_queue_[p].push_back(t);
    SIMPLE_LOG_INFO("p_queue_[{}].size={}", p, p_queue_[p].size());
}

ReadyTaskQueue::ReadyTaskQueue() = default;

Status ReadyTaskQueue::Initialize() {
    std::unique_lock<std::mutex> lk(p_queue_mtx_);
    for (int8_t i = 0; i < 16; ++i) {
        p_queue_[i] = std::list<ReadyTaskPtr>();
    }
    return Status::OkStatus();
}

ReadyTaskPtr ReadyTaskQueue::Top() {
    std::unique_lock<std::mutex> lk(p_queue_mtx_);
    // update priority
    // 优先级：level越深优先级越高，pkt id越小优先级越高，时间越长优先级越高

    // get top
    for (int8_t i = 15; i >= 0; --i) {
        auto& l = p_queue_[i];
        if (!l.empty()) {
            ReadyTaskPtr ret = l.front();
            l.erase(l.begin());
            return ret;
        }
    }
    return nullptr;
}

int8_t ReadyTask::GetPriority(const std::shared_ptr<GraphTopology>& top) const {
    // 优先级：level越深优先级越高，pkt id越小优先级越高，时间越长优先级越高 FIX
    size_t pkt_id = per_node_ctx_->GetPacketContext()->GetId();
    auto node_id  = per_node_ctx_->GetNode()->GetId();
    auto level    = top->GetNodeLevel(node_id);

    // FIXME: 优先级计算
    int8_t p = level;
    return p;
}
} // namespace flow