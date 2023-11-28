#include "graph_spec_view.h"
#include "base/contract_coding.h"
#include "graph_view.h"

#include <log.h>
#include <map>
#include <set>
#include <utility>

namespace flow {

GraphSpecView::GraphSpecView(std::shared_ptr<GraphSpec> spec) : ori_graph_spec_(std::move(spec)) {}

Status GraphSpecView::Initialize() {
    SIMPLE_LOG_DEBUG("GraphSpecView::Initialize Start");
    SIMPLE_ASSERT(ori_graph_spec_);

    // Device信息补齐
    size_t device_id = 0;
    for (auto& v : ori_graph_spec_->GetDevices()) {
        v.SetId(device_id);
        auto s = AddDeviceSpec(std::make_shared<DeviceSpec>(v));
        if (!s.IsOk()) {
            return s;
        }
        ++device_id;
    }

    // GraphNodeSpec ??
    size_t node_index      = 0;
    std::string graph_name = ori_graph_spec_->GetName();
    {
        auto s = ParsingNodeSpec(ori_graph_spec_, node_index, graph_name);
        if (!s.IsOk()) {
            return s;
        }
        ++node_index;
        graph_spec_ = ori_graph_spec_;
    }

    // NodeSpec
    for (auto& n : ori_graph_spec_->GetNodeSpecs()) {
        auto s = ParsingNodeSpec(n, node_index, graph_name);
        if (!s.IsOk()) {
            return s;
        }
        ++node_index;
    }

    SIMPLE_LOG_INFO("need_keep_order_node size {}", node_name_to_node_id_.size());
    for (auto it = node_name_to_node_id_.begin(); it != node_name_to_node_id_.end(); it++) {
        SIMPLE_LOG_INFO(
            "need_keep_order_node, ori_node_name: {}, node id: {}", it->first, it->second);
    }
    SIMPLE_LOG_DEBUG("GraphSpecView::Initialize Start");
    return GenerateEdgeSpec();
}

Status GraphSpecView::AttachNodeNameAndId(std::string name, size_t id) {
    if (node_name_to_node_id_.end() == node_name_to_node_id_.find(name)) {
        node_name_to_node_id_[name] = id;
        return Status::OkStatus();
    } else {
        std::string msg("NodeSpec with name [");
        msg.append(name);
        msg.append("] conflict.");
        return Status::InvalidArgument(msg);
    }
}

Status GraphSpecView::AddNodeSpec(std::shared_ptr<NodeSpec> spec) {
    if (nodes_.end() == nodes_.find(spec->GetName())) {
        nodes_[spec->GetName()] = std::move(spec);
        return Status::OkStatus();
    } else {
        std::string msg("NodeSpec with name [");
        msg.append(spec->GetName());
        msg.append("] conflict.");
        return Status::InvalidArgument(msg);
    }
}

Status GraphSpecView::AddDeviceSpec(std::shared_ptr<DeviceSpec> spec) {
    if (devices_.end() == devices_.find(spec->Name())) {
        devices_[spec->Name()] = std::move(spec);
        return Status::OkStatus();
    } else {
        std::string msg("DeviceSpec with name [");
        msg.append(spec->Name());
        msg.append("] conflict.");
        return Status::InvalidArgument(msg);
    }
}

Status GraphSpecView::Optimize() {
    return Status();
}

Status GraphSpecView::AddSubGraph(std::shared_ptr<GraphSpec> spec) {
    UNUSED_WARN(spec);
    return Status();
}

/**
 * 生成边信息
 * 校验规则
 *  1. link连接的是两个不同的node（？？ 后面是否会有指向自己的link？）
 *  2. 无环路
 *  3. 所有的非optional input spec必须有link连接
 *  4. Node的至少一个OutputPort有link
 * @return
 */
Status GraphSpecView::GenerateEdgeSpec() {
    // 遍历获取所有的link(InoutSpec->link)
    // 保存所有input spec 和 output spec
    // FIXME: node_spec使用shared_ptr，减少不必要的拷贝
    std::multimap<std::string, std::shared_ptr<InoutSpec>> input_specs_;
    std::multimap<std::string, std::shared_ptr<InoutSpec>> output_specs_;
    std::set<std::string> all_link_names;
    for (auto& v : nodes_) {
        auto& node_spec = v.second;
        auto& in        = node_spec->GetInputSpecs();
        for (auto& n : in) {
            // 3. 所有的非optional input spec必须有link连接
            if (!n.optional && n.link_name.empty()) {
                std::string msg("Non optional input [");
                msg.append(n.name);
                msg.append("] should have link attribute.");
                return Status::InvalidArgument(msg);
            }
            if (n.link_name.empty()) {
                continue;
            }
            input_specs_.insert({n.link_name, std::make_shared<InoutSpec>(n)});
            all_link_names.insert(n.link_name);
        }
        auto& out        = node_spec->GetOutputSpecs();
        bool has_connect = false;
        for (auto& o : out) {
            if (!o.link_name.empty()) {
                output_specs_.insert({o.link_name, std::make_shared<InoutSpec>(o)});
                all_link_names.insert(o.link_name);
                has_connect = true;
            }
        }
        // 4. Node的至少一个OutputPort有link
        if (!has_connect) {
            std::string msg("Node [");
            msg.append(node_spec->GetName());
            msg.append("] should have at least one output link.");
            return Status::InvalidArgument(msg);
        }
    }

    // 根据link生成Edge
    size_t edge_id = 0;
    for (auto& link : all_link_names) {
        auto inputs  = input_specs_.equal_range(link);
        auto outputs = output_specs_.equal_range(link);
        for (auto in = inputs.first; in != inputs.second; ++in) {
            auto dst_node = in->second->belonged_to_.lock();
            auto dst_port = in->second;
            for (auto out = outputs.first; out != outputs.second; ++out) {
                auto src_node = out->second->belonged_to_.lock();
                auto src_port = out->second;

                // 1. link连接的是两个不同的node（？？ 后面是否会有指向自己的link？）
                if (dst_node->GetId() == src_node->GetId()) {
                    std::string msg("Link [");
                    msg.append(link);
                    msg.append("] should connect two different node.");
                    return Status::InvalidArgument(msg);
                }
                auto edge = std::make_shared<EdgeSpec>(
                    edge_id, src_node, src_port, dst_node, dst_port, link);
                edges_.insert({edge_id, edge});
                node_id_to_input_edge_id_map_.insert({dst_node->GetId(), edge_id});
                node_id_to_output_edge_id_map_.insert({src_node->GetId(), edge_id});
                ++edge_id;
            }
        }
    }
    // 环路校验

    for (auto& e : edges_) {
        SIMPLE_LOG_TRACE("edge [{}] : [{}, {}, {}, {}], link name: {}",
                         e.second->edge_id,
                         e.second->src_node_spec_->GetName(),
                         e.second->src_port_spec_->name,
                         e.second->dst_node_spec_->GetName(),
                         e.second->dst_port_spec_->name,
                         e.second->link_name);
    }

    return Status::OkStatus();
}

std::vector<std::shared_ptr<NodeSpec>> GraphSpecView::GetEdgeRelatedNode(size_t id) const {
    auto r = edges_.find(id);
    if (r == edges_.end()) {
        // ERROR
        SIMPLE_ASSERT("edge was not found");
        return std::vector<std::shared_ptr<NodeSpec>>();
    }
    std::vector<std::shared_ptr<NodeSpec>> result;
    result.push_back(r->second->src_node_spec_);
    result.push_back(r->second->dst_node_spec_);
    return std::move(result);
}

const std::map<GraphSpecView::SpecName, std::shared_ptr<NodeSpec>>&
GraphSpecView::GetNodeSpecs() const {
    return nodes_;
}

const std::map<std::string, size_t> GraphSpecView::GetNodeSpecNameAndId() const {
    return node_name_to_node_id_;
}

const std::map<GraphSpecView::SpecName, std::shared_ptr<DeviceSpec>>&
GraphSpecView::GetDeviceSpecs() const {
    return devices_;
}

const std::map<size_t, std::shared_ptr<EdgeSpec>>& GraphSpecView::GetEdgeSpecs() const {
    return edges_;
}

std::vector<size_t> GraphSpecView::GetOutputEdgeIdWithNodeId(size_t id) const {
    auto r = node_id_to_output_edge_id_map_.equal_range(id);
    SIMPLE_ASSERT(r.first != r.second);
    std::vector<size_t> result;
    for (auto it = r.first; it != r.second; ++it) {
        result.push_back(it->second);
    }
    return std::move(result);
}

std::vector<size_t> GraphSpecView::GetInputEdgeIdWithNodeId(size_t id) const {
    auto r = node_id_to_input_edge_id_map_.equal_range(id);
    SIMPLE_ASSERT(r.first != r.second);
    std::vector<size_t> result;
    for (auto it = r.first; it != r.second; ++it) {
        result.push_back(it->second);
    }
    return std::move(result);
}

Status GraphSpecView::ParsingNodeSpec(const std::shared_ptr<NodeSpec>& spec,
                                      size_t index,
                                      const std::string& prefix_name) {
    spec->SetId(index);

    if (spec->order_preserving_) {
        auto s = AttachNodeNameAndId(spec->GetName(), index);
        if (!s.IsOk()) {
            return s;
        }
    }

    spec->SetName(prefix_name + "-" + spec->GetName());
    size_t id = 0;
    for (auto& in : spec->GetInputSpecs()) {
        in.id           = id++;
        in.belonged_to_ = spec;
    }
    id = 0;
    for (auto& out : spec->GetOutputSpecs()) {
        out.id           = id++;
        out.belonged_to_ = spec;
    }
    id = 0;
    for (auto& e : spec->GetElemSpecs()) {
        e.SetId(id).SetName(spec->GetElemType().append("-").append(std::to_string(id)));
        ++id;
    }
    auto s = AddNodeSpec(spec);
    if (!s.IsOk()) {
        return s;
    }
    return Status::OkStatus();
}

std::shared_ptr<GraphSpec> GraphSpecView::GetGraphSpe() const {
    return graph_spec_;
}
} // namespace flow
