

#ifndef SIMPLE_FLOW_SCHEDULER_ASYNC_H_
#define SIMPLE_FLOW_SCHEDULER_ASYNC_H_

#include "core/status.h"
#include "graph_topology.h"
#include "packet.h"
#include "packet_context.h"
#include "packet_per_node_context.h"
#include "runtime/executor.h"
#include "runtime/scheduler.h"

#include <list>
#include <memory>
#include <set>
#include <vector>

namespace flow {
class ReadyTask : public std::enable_shared_from_this<ReadyTask> {
public:
    int8_t GetPriority(const std::shared_ptr<GraphTopology>& top) const;

    std::shared_ptr<PacketPerNodeContext> per_node_ctx_;
    int8_t pripority_{0};
};

using ReadyTaskPtr = std::shared_ptr<ReadyTask>;

/*
 * 16个优先级
 */
class ReadyTaskQueue : public std::enable_shared_from_this<ReadyTaskQueue> {
public:
    ReadyTaskQueue();

    virtual ~ReadyTaskQueue() = default;

    void AddTask(const ReadyTaskPtr& t);

    Status Initialize();

    ReadyTaskPtr Top();

private:
    std::map<int8_t, std::list<ReadyTaskPtr>> p_queue_;
    std::mutex p_queue_mtx_;
};

using ReadyTaskQueuePtr = std::shared_ptr<ReadyTaskQueue>;

/**
 * 本身维护一个ready 的task列表
 * 根据特定优先级从task中取出来，交给executor进行执行
 * task执行完成后，判断哪些ready，放回到task列表
 * 当前设计无法保证有序：对于一个inputsource的多个pkt，到达一个特定Node时，由于是构造异步任务交由executor执行，返回时机暂未控制，可能导致乱序。
 *
 * 简化的保序执行方法：
 * 1. scheduler 与 executor使用同一个线程（线程池共享，且仅一个线程）
 * 2. 单路输入，独立指定一个Scheduler(需确保：elementary是无状态的)（提升性能）
 */
class SchedulerAsync : public Scheduler {
public:
    SchedulerAsync() = default;

    explicit SchedulerAsync(size_t executor_thread_num);

    ~SchedulerAsync() override = default;

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
};

} // namespace flow
#endif // SIMPLE_FLOW_SCHEDULER_ASYNC_H_
