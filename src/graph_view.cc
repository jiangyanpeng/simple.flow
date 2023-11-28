#include "graph_view.h"
#include "graph_node.h"
#include "packet_context.h"

#include <iostream>
#include <log.h>
#include <set>
#include <stdlib.h>
#include <time.h>
#include <unordered_map>
#include <utility>

namespace flow {

GraphView::GraphView(std::shared_ptr<MatrixElementaryRegistry> registry,
                     std::shared_ptr<DeviceRegistry> device_registry)
    : registry_(std::move(registry)), device_registry_(std::move(device_registry)) {}

GraphView::~GraphView() = default;

std::vector<std::shared_ptr<Node>>
GraphView::GetSiblingNode(const std::vector<std::shared_ptr<Edge>>& edges) {

    std::set<std::shared_ptr<Node>> sibling_nodes_set;
    for (auto& v : edges) {
        sibling_nodes_set.insert(v->dst_node_);
    }

    std::vector<std::shared_ptr<Node>> sibling_nodes_v;
    sibling_nodes_v.reserve(sibling_nodes_set.size());
    for (auto& v : sibling_nodes_set) {
        sibling_nodes_v.emplace_back(v);
    }

    return std::move(sibling_nodes_v);
}

std::vector<std::shared_ptr<Node>> GraphView::GetSiblingNode(const std::shared_ptr<Node>& node) {
    std::vector<std::shared_ptr<Edge>> edges = GetOutputEdges(node);
    return GetSiblingNode(edges);
}

std::vector<size_t> GraphView::GetOutputEdgeId(const std::shared_ptr<Node>& node) {
    return spec_view_->GetOutputEdgeIdWithNodeId(node->GetId());
}

std::shared_ptr<Node> GraphView::GetNodeWithId(size_t id) {
    auto r = nodes_map_.find(id);
    if (r->first == 0) {
        SIMPLE_LOG_ERROR("no node with id {}", id);
        return nullptr;
    }
    return r->second;
}

void GraphView::InitPacketContext(const std::shared_ptr<PacketContext>& packet_context) {
    for (const auto& v : nodes_map_) {
        auto per_node_ctx = v.second->CreatePerNodeContext();
        //            int64_t id = unique_id_for_packet_per_node_ctx_.fetch_add(1);
        //            per_node_ctx->SetUniqueId(id);
        packet_context->AddPacketPerNodeContext(v.second->GetId(), per_node_ctx);
    }
    packet_context->SetGraphNodeContext(packet_context->node_ctxes_[0]);
}

std::vector<std::shared_ptr<Edge>> GraphView::GetOutputEdges(const std::shared_ptr<Node>& node) {
    std::vector<size_t> edges_id = GetOutputEdgeId(node);
    std::vector<std::shared_ptr<Edge>> edges;
    edges.reserve(edges_id.size());
    for (const auto id : edges_id) {
        edges.emplace_back(edges_map_.find(id)->second);
    }
    return std::move(edges);
}

Status GraphView::OpenAllNodes() {
    for (auto& n : nodes_map_) {
        auto s = n.second->Open();
        if (!s.IsOk()) {
            SIMPLE_LOG_ERROR("open node {} failed.", n.second->GetName());
            std::cerr << "open node " << n.second->GetName() << " failed" << std::endl;
        }
        SIMPLE_ASSERT(s.IsOk());
    }
    return Status::OkStatus();
}

Status GraphView::Initialize(std::shared_ptr<GraphSpecView> spec_view,
                             const std::shared_ptr<base::PipeManager>& global_thread_pool) {
    SIMPLE_LOG_DEBUG("GraphView::Initialize Start");
    srand((unsigned)time(NULL)); //初始化的时候产生随机数种子

    spec_view_ = std::move(spec_view);
    // Devices
    auto& devices = spec_view_->GetDeviceSpecs();
    for (auto& d : devices) {
        auto de = device_registry_->InvokeCreate(d.second->Type());
        //            std::shared_ptr<Device> de = std::make_shared<DeviceCPU>();
        de->Initialize();
        de->SetName(d.second->Name()).SetId(d.second->Id());
        de->SetThreadPool(global_thread_pool);
        auto status = devices_.AddTag(d.second->Name(), de);
        if (!status.first.IsOk()) {
            SIMPLE_LOG_ERROR("Add Device failed. error_code: {}, msg: {}",
                             (int)status.first.Code(),
                             status.first.Msg());
            return Status::InvalidArgument("Add Device failed");
        }
    }

    // GraphNode
    auto graph_spec = spec_view_->GetGraphSpe();
    auto node       = std::make_shared<GraphNode>();
    node->SetGraphViewWeakPtr(shared_from_this());
    node->Initialize(graph_spec, registry_);

    nodes_map_.insert({node->GetId(), node});
    node_name_to_node_id_ = spec_view_->GetNodeSpecNameAndId();

    // Nodes
    auto& node_specs = spec_view_->GetNodeSpecs();
    for (auto& ns : node_specs) {
        // GraphNode已经配置初始化
        if (ns.second->GetId() == 0) {
            continue;
        }
        auto node = std::make_shared<Node>();
        node->SetGraphViewWeakPtr(shared_from_this());
        auto s = node->Initialize(ns.second, registry_);

        if (!s.IsOk()) {
            SIMPLE_LOG_ERROR("elem: {}, err: {} ", node->GetName(), s.Msg());
            return s;
        }

        nodes_map_.insert({node->GetId(), node});
    }

    for (auto& node : nodes_map_) {
        SIMPLE_LOG_TRACE("node: id = {}, name = {}", node.second->GetId(), node.second->GetName());
    }

    //  Edges
    auto& edge_specs = spec_view_->GetEdgeSpecs();
    for (auto& e : edge_specs) {
        auto edge = GenerateEdgeWithSpec(e.second);
        edges_map_.insert({e.first, edge});
    }
    SIMPLE_LOG_DEBUG("GraphView::Initialize End");
    return Status::OkStatus();
}

std::shared_ptr<Device> GraphView::GetDeviceByName(const std::string& name) {
    std::shared_ptr<Device> d;
    auto status = devices_.Get(name, d);
    if (!status.IsOk()) {
        SIMPLE_LOG_ERROR("Get device with name {} failed. error_code: {}, msg: {}",
                         name,
                         int(status.Code()),
                         status.Msg());
        return nullptr;
    }
    return d;
}

std::shared_ptr<Edge> GraphView::GenerateEdgeWithSpec(const std::shared_ptr<EdgeSpec>& spec) {
    auto src_node_iter = nodes_map_.find(spec->src_node_spec_->GetId());
    auto dst_node_iter = nodes_map_.find(spec->dst_node_spec_->GetId());

    auto src_node = src_node_iter->second;
    auto dst_node = dst_node_iter->second;
    auto src_port = src_node->GetOutputPortWithId(spec->src_port_spec_->id);
    auto dst_port = dst_node->GetInputPortWithId(spec->dst_port_spec_->id);

    return std::make_shared<Edge>(src_node, src_port, dst_node, dst_port);
}

std::shared_ptr<Node> GraphView::GetGraphNode() const {
    return nodes_map_.at(0);
}

std::vector<size_t> GraphView::GetNodeIdsByName(const std::vector<std::string>& node_names) {
    std::vector<size_t> node_ids;
    for (auto name : node_names) {
        if (node_name_to_node_id_.find(name) != node_name_to_node_id_.end()) {
            node_ids.push_back(node_name_to_node_id_[name]);
        }
    }
    return node_ids;
}
std::shared_ptr<GraphTopology> GraphView::GetTopology() {
    auto topology = std::make_shared<GraphTopology>();
    topology->Initialize(shared_from_this());
    return topology;
}

Edge::Edge(std::shared_ptr<Node> src_node,
           std::shared_ptr<InoutPort> src_port,
           std::shared_ptr<Node> dst_node,
           std::shared_ptr<InoutPort> dst_port)
    : src_node_(std::move(src_node)),
      src_port_(std::move(src_port)),
      dst_node_(std::move(dst_node)),
      dst_port_(std::move(dst_port)) {}
} // namespace flow