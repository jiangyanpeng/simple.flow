
#include "graph.h"
#include "packet_context.h"
#include "runtime/executor.h"
#include "runtime/scheduler_async_order_preserving.h"
#include "runtime/waiting_thread.h"

#include <iostream>
#include <log.h>
#include <set>
#include <utility>
#include <vector>

namespace flow {

std::atomic_int_fast64_t GraphHelper::packet_id_{0};

class Graph {
public:
    Graph();

    ~Graph();

    Status Initialize(std::shared_ptr<GraphSpec> graph_spec,
                      std::shared_ptr<CalculatorRegistry> registry,
                      std::shared_ptr<DeviceRegistry> device_registry,
                      GRAPH_SCHEDULE_POLICY policy,
                      std::shared_ptr<ExecutorOption> executor_option);

    size_t GetInputCount() const;

    size_t GetOutputCount() const;

    void Start();

    void Stop();

    void Schedule(const std::shared_ptr<PacketContext>& pkt_ctx,
                  const std::shared_ptr<Packet>& pkt);

    void AddCallFunct(const AsyncResultFunc& funct);

    void AddFullCallFunct(const AsyncResultFullFunc& funct);

    std::shared_ptr<GraphView> GetGraphView() const;

    std::vector<PackageGroup> GetResult(size_t packet_id);

    bool IsPacketProcessFinished(size_t packet_id);

    Status AddPacketContext(std::shared_ptr<PacketContext> ctx);

    Status RemovePacketContextIfFinished(const std::shared_ptr<PacketContext>& pkt_context);

    // TODO: add close all nodes
    Status OpenAllNodes();

    std::shared_ptr<Scheduler> scheduler_;

    void Propagate1(const PacketPerNodeContextPtr& packet_per_node_context,
                    const std::vector<std::shared_ptr<Edge>>& edges);

private:
    void AddGraphInput(const std::shared_ptr<PacketContext>& pkt_ctx,
                       const std::shared_ptr<Packet>& pkt);


    void GetGraphOutput(const std::shared_ptr<PacketContext>& ctx,
                        std::vector<PackageGroup>& result);


private:
    std::shared_ptr<GraphView> graph_view_;

    std::map<uint64_t, std::shared_ptr<PacketContext>> packet_contexts_;

    std::mutex mtx_;

    AsyncResultFunc get_result_async_;

    AsyncResultFullFunc get_result_async_full_;

public:
    std::vector<size_t> stay_order_node_ids_;
};

// @deprated
std::shared_ptr<Packet> GraphHelper::CreatePacket() {
    //        SIMPLE_ASSERT(0);
    std::cout << graph_->GetInputCount() << std::endl;
    return input_source_manager_->DefaultSourceContext()->CreatePacket();
}

Status GraphHelper::AddPacket(const std::shared_ptr<Packet>& pkt) {
    // FIXME: 触发流控时，PacketContext不需每次创建
    auto pkt_context = CreatePacketContext(pkt);
    auto s           = graph_->AddPacketContext(pkt_context);
    if (!s.IsOk()) {
        return s;
    }
    SIMPLE_LOG_WARN("ADD PACKET OK. Packet id: {}", pkt_context->id_);
    graph_->Schedule(pkt_context, pkt);
    int cnt = 0;
    while (!graph_->IsPacketProcessFinished(pkt->GetId())) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        cnt++;
        if (cnt == 1000) {
            SIMPLE_LOG_ERROR("Pkt with id [{}] is still not processed after 10s.", pkt->GetId());
            return Status(StatusCode::kInternal, "Pkt is not processed after 10s.");
        }
    }
    return Status::OkStatus();
}

void GraphHelper::AddCallFunct(const AsyncResultFunc& funct) {
    graph_->AddCallFunct(funct);
}

void GraphHelper::AddCallFunct(const AsyncResultFullFunc& funct) {
    graph_->AddFullCallFunct(funct);
}

void GraphHelper::AddSkipPacket(std::shared_ptr<InputSourceContext>& source,
                                const std::shared_ptr<Packet>& pkt) {
    source->AddSkipPktId(graph_->stay_order_node_ids_, pkt->GetId());
}

Status GraphHelper::AddPacketAsync(const std::shared_ptr<Packet>& pkt) {
    // FIXME: 触发流控时，PacketContext不需每次创建
    auto pkt_context = CreatePacketContext(pkt);
    auto s           = graph_->AddPacketContext(pkt_context);
    if (!s.IsOk()) {
        return s;
    }
    SIMPLE_LOG_DEBUG("ADD PACKET OK. Packet id: {}, Source id {} ",
                     pkt_context->GetPacketId(),
                     pkt->GetSourceId());
    graph_->Schedule(pkt_context, pkt);

    return Status::OkStatus();
}

void GraphHelper::Start() {}

void GraphHelper::Stop() {
    graph_->Stop();
}


void GraphHelper::Initialize(const std::shared_ptr<CalculatorRegistry>& registry,
                             const std::shared_ptr<DeviceRegistry>& device_registry,
                             GRAPH_SCHEDULE_POLICY policy,
                             std::shared_ptr<ExecutorOption> executor_option) {
    SIMPLE_LOG_DEBUG("GraphHelper::Initialize Start");
    graph_ = std::make_shared<Graph>();
    graph_->Initialize(graph_spec_, registry, device_registry, policy, executor_option);
    graph_->OpenAllNodes();
    input_source_manager_ = std::make_shared<InputSourceManager>();
    input_source_manager_->SetGraphInputCount(graph_->GetInputCount());
    SIMPLE_LOG_DEBUG("GraphHelper::Initialize End");
}

std::shared_ptr<PacketContext>
GraphHelper::CreatePacketContext(const std::shared_ptr<Packet>& pkt) {
    auto packet_context = std::make_shared<PacketContext>(pkt);
    packet_context->SetInputSourceContext(pkt->GetSourceContext());
    packet_context->SetGraphView(graph_->GetGraphView());
    packet_context->Initialize();
    return std::move(packet_context);
}

bool GraphHelper::IsPacketProcessed(size_t pkt_id) {
    return graph_->IsPacketProcessFinished(pkt_id);
}

std::vector<PackageGroup> GraphHelper::GetResult(size_t pkt_id) {
    return graph_->GetResult(pkt_id);
}

std::shared_ptr<InputSourceContext> GraphHelper::CreateInputSourceContext() {
    return input_source_manager_->GenerateSourceContext();
}

Status Graph::Initialize(std::shared_ptr<GraphSpec> graph_spec,
                         std::shared_ptr<CalculatorRegistry> registry,
                         std::shared_ptr<DeviceRegistry> device_registry,
                         GRAPH_SCHEDULE_POLICY policy,
                         std::shared_ptr<ExecutorOption> executor_option) {
    SIMPLE_LOG_DEBUG("Graph::Initialize Start");
    auto spec_view = std::make_shared<GraphSpecView>(graph_spec);
    auto status    = spec_view->Initialize();
    if (!status.IsOk()) {
        SIMPLE_LOG_ERROR("Spec view init failed. Err msg: {}", status.Msg());
        return status;
    }

    status = spec_view->Optimize();
    if (!status.IsOk()) {
        SIMPLE_LOG_ERROR("Spec view optimize failed. Err msg: {}", status.Msg());
        return status;
    }

    auto global_thread_pool = std::make_shared<base::PipeManager>(4);
    SIMPLE_LOG_DEBUG("create global_thread_pool: {}", static_cast<void*>(global_thread_pool.get()));
    graph_view_ = std::make_shared<GraphView>(registry, device_registry);
    status      = graph_view_->Initialize(spec_view, global_thread_pool);
    if (!status.IsOk()) {
        SIMPLE_LOG_ERROR("Spec view init failed. Err msg: {}", status.Msg());
        return status;
    }

    for (auto it = graph_view_->node_name_to_node_id_.begin();
         it != graph_view_->node_name_to_node_id_.end();
         it++) {
        stay_order_node_ids_.push_back(it->second);
    }

    GraphPacketFinishedCallback callback = [=](const PacketContextPtr& ctx) {
        SIMPLE_LOG_DEBUG("GraphPacketFinishedCallback[RemovePacketContextIfFinished] Start");
        RemovePacketContextIfFinished(ctx);
        SIMPLE_LOG_DEBUG("GraphPacketFinishedCallback[RemovePacketContextIfFinished] End");
    };

    // FIXME: 根据配置选择使用的Scheduler
    switch (policy) {
        case GRAPH_SCH_ASYNC_ONE_THREAD:
            scheduler_ = std::make_shared<SchedulerOneThread>();
            SIMPLE_LOG_DEBUG("policy: GRAPH_SCH_ASYNC_ONE_THREAD");
            break;
        case GRAPH_SCH_ASYNC_ORDER_PRESERVING:
            scheduler_ = std::make_shared<SchedulerAsyncOrderPreserving>(3, executor_option);
            SIMPLE_LOG_DEBUG("policy: GRAPH_SCH_ASYNC_ORDER_PRESERVING");
            break;
        case GRAPH_SCH_ASYNC_NON_ORDER_PRESERVING:
            scheduler_ = std::make_shared<SchedulerAsync>(3);
            SIMPLE_LOG_DEBUG("policy: GRAPH_SCH_ASYNC_NON_ORDER_PRESERVING");
            break;
        case GRAPH_SCH_ASYNC_ONE_THREAD_PER_INPUT_SOURCE:
            SIMPLE_LOG_DEBUG("policy: GRAPH_SCH_ASYNC_ONE_THREAD_PER_INPUT_SOURCE");
            break;
    }

    scheduler_->SetGraphView(graph_view_);
    scheduler_->SetGraphPacketFinishedCallback(callback);
    scheduler_->Initialize();

    scheduler_->Start();
    SIMPLE_LOG_DEBUG("Graph::Initialize End");
    return Status::OkStatus();
}


void Graph::Start() {}

void Graph::AddCallFunct(const AsyncResultFunc& funct) {
    get_result_async_ = funct;
}


void Graph::Schedule(const std::shared_ptr<PacketContext>& pkt_ctx,
                     const std::shared_ptr<Packet>& pkt) {
    AddGraphInput(pkt_ctx, pkt);
    scheduler_->AddPacketContext(pkt_ctx, pkt);
    SIMPLE_LOG_DEBUG("Graph::Schedule return.");
}

void Graph::Propagate1(const PacketPerNodeContextPtr& packet_per_node_context,
                       const std::vector<std::shared_ptr<Edge>>& edges) {
    auto packet_context = packet_per_node_context->GetPacketContext();
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
        // TODO: 是否需使用reference？
        //            PacketPerNodeContextPtr src = packet_context->GetNodeContextById(src_node_id);
        // CAUTION: 此处不加锁，scheduler线程池线程数为1，则不会出现竞争的问题
        PacketPerNodeContextPtr dst = packet_context->GetNodeContextById(dst_node_id);

        // TODO: 抽象Propagate
        packet_per_node_context->PropagateOutputPortTo(dst, dst_port_id, src_port_id);
    }

    // clear output
    for (auto& v : packet_per_node_context->outputs_) {
        v.clear();
    }
}


Graph::Graph() : graph_view_(nullptr) {}

std::shared_ptr<GraphView> Graph::GetGraphView() const {
    return graph_view_;
}

void Graph::GetGraphOutput(const std::shared_ptr<PacketContext>& ctx,
                           std::vector<PackageGroup>& result) {
    auto& vec = ctx->graph_node_ctx_->inputs_;
    result.insert(result.begin(), vec.begin(), vec.end());
}

std::vector<PackageGroup> Graph::GetResult(size_t packet_id) {
    std::vector<PackageGroup> result;
    std::unique_lock<std::mutex> lk(mtx_);
    auto iter = packet_contexts_.find(packet_id);
    if (iter != packet_contexts_.end()) {
        if (iter->second->IsFinished()) {
            GetGraphOutput(iter->second, result);
            packet_contexts_.erase(iter);
        }
    }
    return result;
}

Status Graph::AddPacketContext(std::shared_ptr<PacketContext> ctx) {
    std::unique_lock<std::mutex> lk(mtx_);
    // FIXME: 流控参数需外部可配置
    if (packet_contexts_.size() >= 20) {
        SIMPLE_LOG_ERROR("flow control error, process buffer overflow");
        return Status(StatusCode::kFlowControl, "flow control");
    }
    packet_contexts_[ctx->id_] = std::move(ctx);
    return Status::OkStatus();
}

Status Graph::RemovePacketContextIfFinished(const std::shared_ptr<PacketContext>& pkt_context) {
    if (pkt_context->IsFinished()) {
        SIMPLE_LOG_DEBUG("Packet processing finished. source_id {}, pkt_id: {}",
                         pkt_context->GetSourceId(),
                         pkt_context->GetPacketId());
        // FIXME:
        // 获取结果的回调，理论上应该放到独立的线程池中执行，避免出现回调处理时间过长导致scheduler阻塞
        // CAUTION: 推荐使用full函数
        // do notify
        do {
            if (get_result_async_full_ != nullptr) {
                SIMPLE_LOG_INFO("Call function to process result");
                uint64_t pkt_id = pkt_context->GetId();
                auto src_ctx    = pkt_context->GetInputSourceContext();
                std::vector<PackageGroup> result;
                GetGraphOutput(pkt_context, result);
                get_result_async_full_(src_ctx, pkt_context->GetPacket()->GetInputPktId(), result);
                {
                    std::unique_lock<std::mutex> lk(mtx_);
                    packet_contexts_.erase(pkt_id);
                }
                break;
            }
            if (get_result_async_ != nullptr) {
                SIMPLE_LOG_INFO("Call function to process result");
                uint64_t pkt_id             = pkt_context->GetId();
                uint8_t input_source_ctx_id = pkt_context->GetInputSourceContext()->GetId();
                std::vector<PackageGroup> result;
                GetGraphOutput(pkt_context, result);
                get_result_async_(input_source_ctx_id, pkt_id, result);
                {
                    std::unique_lock<std::mutex> lk(mtx_);
                    packet_contexts_.erase(pkt_id);
                }
            }
        } while (false);

        return Status::OkStatus();
    }
    return Status(StatusCode::kOutOfRange, "not finished yet");
}

bool Graph::IsPacketProcessFinished(size_t packet_id) {
    std::unique_lock<std::mutex> lk(mtx_);
    auto iter = packet_contexts_.find(packet_id);
    if (iter != packet_contexts_.end()) {
        if (iter->second->IsFinished()) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

void Graph::AddGraphInput(const std::shared_ptr<PacketContext>& pkt_ctx,
                          const std::shared_ptr<Packet>& pkt) {
    auto& outputs = pkt_ctx->graph_node_ctx_->outputs_;
    SIMPLE_ASSERT(outputs.size() == pkt->GetPackageNum());
    for (size_t i = 0; i < outputs.size(); ++i) {
        outputs[i].push_back(pkt->GetPackage(i));
    }
}

Status Graph::OpenAllNodes() {
    return graph_view_->OpenAllNodes();
}

size_t Graph::GetInputCount() const {
    return graph_view_->GetGraphNode()->GetInputCount();
}

size_t Graph::GetOutputCount() const {
    return graph_view_->GetGraphNode()->GetOutputCount();
}

void Graph::Stop() {
    scheduler_->Stop();
}

Graph::~Graph() {
    Stop();
}

void Graph::AddFullCallFunct(const AsyncResultFullFunc& funct) {
    get_result_async_full_ = funct;
}

} // namespace flow