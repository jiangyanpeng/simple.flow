#ifndef SIMPLE_PIPE_SCHDULER_H_
#define SIMPLE_PIPE_SCHDULER_H_

#include <common.h>
#include <functional>
#include <memory>

namespace pipeline {
class GraphView;

class GraphTopology;

// using GraphPacketFinishedCallback = std::function<void(const PacketContextPtr& packet_ctx)>;
class Scheduler {
public:
    Scheduler();

    virtual ~Scheduler() = default;

    virtual MStatus Init() = 0;

    virtual void Start();

    virtual void Stop();

    // virtual void AddPacketContext(const PacketContextPtr& ctx,
    //                               const std::shared_ptr<Packet>& pkt) = 0;

    virtual void SetGraphView(const std::shared_ptr<GraphView>& view);

    // virtual void SetGraphPacketFinishedCallback(GraphPacketFinishedCallback& callback);


protected:
    // ThreadPool pool_{1};

    std::shared_ptr<GraphView> graph_view_{nullptr};
    std::shared_ptr<GraphTopology> topology_{nullptr};

    // GraphPacketFinishedCallback graph_call_back_{nullptr};
};
} // namespace pipeline
#endif // SIMPLE_PIPE_SCHDULER_H_