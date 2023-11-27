#ifndef ARCHITECT_SCHEDULER_H
#define ARCHITECT_SCHEDULER_H

#include "packet.h"
#include "packet_context.h"
#include <functional>
#include <manager/pipe_manager.h>

namespace flow {
class GraphView;

class GraphTopology;

using GraphPacketFinishedCallback = std::function<void(const PacketContextPtr& packet_ctx)>;
class Scheduler {
public:
    Scheduler();

    virtual ~Scheduler() = default;

    virtual Status Initialize() = 0;

    virtual void Start();

    virtual void Stop();

    virtual void AddPacketContext(const PacketContextPtr& ctx,
                                  const std::shared_ptr<Packet>& pkt) = 0;

    virtual void SetGraphView(const std::shared_ptr<GraphView>& view);

    virtual void SetGraphPacketFinishedCallback(GraphPacketFinishedCallback& callback);


protected:
    base::PipeManager pool_{1};

    std::shared_ptr<GraphView> graph_view_;

    std::shared_ptr<GraphTopology> topology_;

    GraphPacketFinishedCallback graph_call_back_{nullptr};
};
} // namespace flow


#endif // ARCHITECT_SCHEDULER_H
