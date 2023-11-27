#ifndef GRAPHFLOW_FRAMEWORK_GRAPH_NODE_H
#define GRAPHFLOW_FRAMEWORK_GRAPH_NODE_H

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


#endif // GRAPHFLOW_FRAMEWORK_GRAPH_NODE_H
