#include "graph_topology.h"
#include "graph_view.h"
#include "node.h"

namespace flow {
struct NodeElemTopology {
    NodeElemTopology(const std::shared_ptr<Node>& n, int8_t l) : node_(n), level_(l) {}

    std::shared_ptr<Node> node_;
    int8_t level_;
};

GraphTopology::GraphTopology() {}

GraphTopology::~GraphTopology() {}

void GraphTopology::Topology(const std::shared_ptr<GraphView>& view,
                             const std::shared_ptr<Node>& node,
                             int8_t level) {
    auto sibling_nodes = view->GetSiblingNode(node);
    for (auto& n : sibling_nodes) {
        size_t id = n->GetId();
        if (n->GetId() == view->GetGraphNode()->GetId()) {
            return;
        }

        int8_t l = 0;
        if (node_level_.end() != node_level_.find(id)) {
            l = node_level_[id];
        }

        node_level_[id] = std::max(l, level);
    }

    for (auto& n : sibling_nodes) {
        Topology(view, n, level + 1);
    }
}

Status GraphTopology::Initialize(const std::shared_ptr<GraphView>& view) {

    auto gn = view->GetGraphNode();
    std::vector<NodeElemTopology> elems;
    elems.emplace_back(gn, 0);

    int8_t level = 0;
    Topology(view, gn, 0);

    for (auto& nl : node_level_) {
        SIMPLE_LOG_INFO("node: {}, level {}", nl.first, nl.second);
        top_[nl.second].push_back(view->GetNodeWithId(nl.first));
    }

    for (auto& t : top_) {
        SIMPLE_LOG_INFO("topology, level {}:", t.first);
        for (auto& n : t.second) {
            SIMPLE_LOG_INFO("   node: id {}, name {}", n->GetId(), n->GetName());
        }
    }

    return Status::OkStatus();
}

size_t GraphTopology::GetTopologyDepth() const {
    return top_.size();
}

std::list<std::shared_ptr<Node>> GraphTopology::GetNodesOnLevelN(int8_t level) {
    MATRIX_ASSERT(level >= 0 && level <= GetTopologyDepth() - 1);
    return top_[level];
}

int8_t GraphTopology::GetNodeLevel(size_t node_id) {
    if (node_level_.end() != node_level_.find(node_id)) {
        return node_level_[node_id];
    }
    // graph node priority 最高
    return 15;
}


} // namespace flow
