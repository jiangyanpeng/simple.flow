#ifndef SIMPLE_FLOW_GRAPH_NODE_H_
#define SIMPLE_FLOW_GRAPH_NODE_H_

#include "node.h"

namespace flow {
class GraphNode : public Node {
public:
    ~GraphNode() override = default;

    void Process() override;

    void ProcessCtx(const std::shared_ptr<PacketPerNodeContext>& ctx) override;

    size_t GetInputCount() const override;

    size_t GetOutputCount() const override;
};

} // namespace flow
#endif // SIMPLE_FLOW_GRAPH_NODE_H_
