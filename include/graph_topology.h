#ifndef SIMPLE_FLOW_GRAPH_TOPOLOGY_H_
#define SIMPLE_FLOW_GRAPH_TOPOLOGY_H_

#include "base/status.h"

#include <list>
#include <map>
#include <memory>
#include <vector>

namespace flow {
class Node;
class GraphView;
class GraphTopology {
public:
    GraphTopology();
    ~GraphTopology();

    Status Initialize(const std::shared_ptr<GraphView>& view);

    size_t GetTopologyDepth() const;

    std::list<std::shared_ptr<Node>> GetNodesOnLevelN(int8_t level);

    int8_t GetNodeLevel(size_t node_id);

private:
    void Topology(const std::shared_ptr<GraphView>& view,
                  const std::shared_ptr<Node>& node,
                  int8_t level);
    // key: level, value: list<Node>
    std::map<int8_t, std::list<std::shared_ptr<Node>>> top_;

    // key: node id, value: level
    std::map<size_t, int8_t> node_level_;
};

} // namespace flow
#endif // SIMPLE_FLOW_GRAPH_TOPOLOGY_H_
