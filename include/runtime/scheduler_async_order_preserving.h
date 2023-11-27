
#ifndef GRAPHFLOW_FRAMEWORK_SCHEDULER_ASYNC_ORDER_PRESERVING_H
#define GRAPHFLOW_FRAMEWORK_SCHEDULER_ASYNC_ORDER_PRESERVING_H


#include "base/status.h"
#include "executor.h"
#include "graph_topology.h"
#include "packet.h"
#include "packet_context.h"
#include "packet_per_node_context.h"
#include "scheduler.h"
#include "scheduler_async.h"
#include <list>
#include <memory>
#include <set>
#include <vector>

namespace flow {

class SchedulerAsyncOrderPreserving : public Scheduler {
public:
    SchedulerAsyncOrderPreserving() = default;

    explicit SchedulerAsyncOrderPreserving(size_t executor_thread_num,
                                           std::shared_ptr<ExecutorOption> executor_option);

    ~SchedulerAsyncOrderPreserving() override = default;

    Status Initialize() override;

    void AddPacketContext(const PacketContextPtr& ctx, const std::shared_ptr<Packet>& pkt) override;

    void Start() override;

    void Stop() override;

    std::shared_ptr<Executor> executor_;

private:
    void DoSchedule();

protected:
    // FIXME：propagate位置要在确认下
    std::set<size_t> DoPropagate(const PacketPerNodeContextPtr& ctx,
                                 const std::shared_ptr<Node>& node);

    std::atomic_bool stop_{false};

    bool stopped_{false};

    std::condition_variable stopped_cd_;

    std::mutex mtx_;

    ReadyTaskQueuePtr ready_task_queue_;

    size_t executor_thread_num_{1};

    std::shared_ptr<ExecutorOption> executor_option_;
};
} // namespace flow


#endif // GRAPHFLOW_FRAMEWORK_SCHEDULER_ASYNC_ORDER_PRESERVING_H
