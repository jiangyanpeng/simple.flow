#ifndef SIMPLE_FLOW_SCHEDULER_ONE_THREAD_H_
#define SIMPLE_FLOW_SCHEDULER_ONE_THREAD_H_

#include "graph_topology.h"
#include "packet.h"
#include "packet_context.h"
#include "packet_per_node_context.h"
#include "scheduler.h"

namespace flow {
class SchedulerOneThread : public Scheduler {
public:
    SchedulerOneThread() = default;

    ~SchedulerOneThread() override = default;

    Status Initialize() override;

    void DoSchedule(const PacketContextPtr& ctx, const std::shared_ptr<Packet>& pkt);

    void AddPacketContext(const PacketContextPtr& ctx, const std::shared_ptr<Packet>& pkt) override;

private:
    // FIXME：propagate位置要在确认下
    void DoPropagate(const PacketPerNodeContextPtr& ctx, const std::shared_ptr<Node>& node);
};

} // namespace flow
#endif // SIMPLE_FLOW_SCHEDULER_ONE_THREAD_H_
