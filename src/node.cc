

#include "node.h"
#include "core/contract_coding.h"
#include "graph.h"
#include "inout_handler.h"

#include <log.h>
#include <stdlib.h>
#include <time.h>
#include <utility>

namespace flow {
void Node::Process() {
    do {
        SIMPLE_LOG_DEBUG("Node::Process: node_id [{}], node name [{}]", GetId(), GetName());
        PacketPerNodeContextPtr node_ctx = nullptr;
        node_ctx                         = nullptr;
        {
            std::unique_lock<std::mutex> lk(mtx_);
            if (inouts_queue_.empty()) {
                SIMPLE_LOG_DEBUG("node [{}], inout_queue empty", GetName());
                break;
            }
            SIMPLE_LOG_DEBUG("node [{}], inout_queue size: {}", GetName(), inouts_queue_.size());

            node_ctx = inouts_queue_.front();
            inouts_queue_.pop();
        }
        SIMPLE_LOG_DEBUG(
            "Node::Process: node_id [{}], node name [{}], ctx unique_id: {}, thread id: {}",
            GetId(),
            GetName(),
            node_ctx->GetUniqueId(),
            std::hash<std::thread::id>{}(std::this_thread::get_id()));


        auto package_cnt = 0;
        for (auto& in : node_ctx->inputs_) {
            if (package_cnt < in.size()) {
                package_cnt = in.size();
            }
        }

        for (int i = 0; i < package_cnt; ++i) {
            std::vector<std::shared_ptr<Package>> inputs_data;
            std::vector<std::shared_ptr<Package>> outputs_data;
            for (size_t j = 0; j < node_ctx->inputs_.size(); ++j) {
                auto port = input_ports_[j];
                if (!port->IsOptional()) {
                    SIMPLE_LOG_DEBUG("node [{}], node_ctx->inputs_[{}].size: {}",
                                     GetName(),
                                     j,
                                     node_ctx->inputs_[j].size());
                    // 一个Node的多个Port数据来源于多个Node，且IsReady有优先级时，可能存在某些Port有数据，某些Port没有数据的情况
                    // 此时，对于没有数据的Port，构造一个空的nullptr
                    if (node_ctx->inputs_[j].size() > i) {
                        inputs_data.push_back(node_ctx->inputs_[j][i]);
                    } else {
                        //                            inputs_data.push_back(std::make_shared<Package>());
                        inputs_data.push_back(nullptr);
                    }
                }
            }
            outputs_data.resize(node_ctx->outputs_.size());

            CalculatorContext ctx(
                inputs_data, outputs_data, shared_from_this(), node_ctx->GetStream());
            ctx.SetPacketPerNodeContext(node_ctx);
            calculators_[0]->GetDevice()->ComputeSync([&]() {
                auto s = calculators_[0]->Process(&ctx);
                if (!s.IsOk()) {
                    SIMPLE_LOG_ERROR("Calculator process failed. Node name: [{}], Node: [{}], "
                                     "ElemId: [{}]. ErrCode: {}, ErrMsg: {}",
                                     GetName(),
                                     GetId(),
                                     calculators_[0]->GetId(),
                                     GetStatusCodeValue(s.Code()),
                                     s.Msg());
                    exit(0);
                }
            });

            // 输出赋值
            for (size_t j = 0; j < node_ctx->outputs_.size(); ++j) {
                SIMPLE_ASSERT(outputs_data[j]);
                node_ctx->outputs_[j].push_back(outputs_data[j]);
            }
        }

        // 清理输入
        for (auto& v : node_ctx->inputs_) {
            v.clear();
        }

        node_ctx->Notify();
    } while (true);
}

void Node::ProcessCtx(const std::shared_ptr<PacketPerNodeContext>& ctx) {
    SIMPLE_LOG_DEBUG("Node::ProcessCtx Start: node_id [{}], node name [{}]", GetId(), GetName());
    PacketPerNodeContextPtr node_ctx = nullptr;
    node_ctx                         = ctx;

    SIMPLE_LOG_DEBUG(
        "Node::ProcessCtx: node_id [{}], node name [{}], ctx unique_id: {}, thread id: {}",
        GetId(),
        GetName(),
        node_ctx->GetUniqueId(),
        std::hash<std::thread::id>{}(std::this_thread::get_id()));
    if (GetId() == 8) {
        SIMPLE_LOG_DEBUG("Node::ProcessCtx: node_id [{}], node name [{}]", GetId(), GetName());
    }

    auto package_cnt = 0;
    for (auto& in : node_ctx->inputs_) {
        if (package_cnt < in.size()) {
            package_cnt = in.size();
        }
    }

    for (int i = 0; i < package_cnt; ++i) {
        std::vector<std::shared_ptr<Package>> inputs_data;
        std::vector<std::shared_ptr<Package>> outputs_data;
        for (size_t j = 0; j < node_ctx->inputs_.size(); ++j) {
            auto port = input_ports_[j];
            if (!port->IsOptional()) {
                SIMPLE_LOG_DEBUG("node [{}], node_ctx->inputs_[{}].size: {}",
                                 GetName(),
                                 j,
                                 node_ctx->inputs_[j].size());
                // 一个Node的多个Port数据来源于多个Node，且IsReady有优先级时，可能存在某些Port有数据，某些Port没有数据的情况
                // 此时，对于没有数据的Port，构造一个空的nullptr
                if (node_ctx->inputs_[j].size() > i) {
                    inputs_data.push_back(node_ctx->inputs_[j][i]);
                } else {
                    //                            inputs_data.push_back(std::make_shared<Package>());
                    inputs_data.push_back(nullptr);
                }
            }
        }
        outputs_data.resize(node_ctx->outputs_.size());

        CalculatorContext ctx(inputs_data, outputs_data, shared_from_this(), node_ctx->GetStream());
        ctx.SetPacketPerNodeContext(node_ctx);

        int index = 0;
        if (calculators_.size() > 1) {
            index = rand() % calculators_.size();
            SIMPLE_LOG_DEBUG("elementary index: {}", index);
        }
        calculators_[index]->GetDevice()->ComputeSync([&]() {
            auto s = calculators_[index]->Process(&ctx);
            if (!s.IsOk()) {
                SIMPLE_LOG_ERROR("Node::ProcessCtx failed. Node name: [{}], Node: [{}], ElemId: "
                                 "[{}]. ErrCode: {}, ErrMsg: {}",
                                 GetName(),
                                 GetId(),
                                 calculators_[index]->GetId(),
                                 GetStatusCodeValue(s.Code()),
                                 s.Msg());
                exit(0);
            }
        });

        // 输出赋值
        for (size_t j = 0; j < node_ctx->outputs_.size(); ++j) {

            if (outputs_data[j] == nullptr) {
                // outputs_data[j] 是一个nullptr
                // node_ctx->outputs_[j] 很有可能是一个empty的vector.
            } else {
                node_ctx->outputs_[j].push_back(outputs_data[j]);
            }
        }
    }

    // 清理输入
    for (auto& v : node_ctx->inputs_) {
        v.clear();
    }
}

Status Node::Initialize(std::shared_ptr<NodeSpec> spec,
                        const std::shared_ptr<CalculatorRegistry>& registry) {
    SIMPLE_LOG_DEBUG("Node::Initialize Start. NodeSpec: {}", spec->to_string());
    spec_ = std::move(spec);
    SIMPLE_ASSERT(weak_graph_.lock());
    id_               = spec_->node_id;
    order_preserving_ = spec_->order_preserving_;

    skip_pkt_node_ = spec_->GetSkipPacketNode();

    // todo
    std::string elem_type = "Calculator";
    if (!spec_->elementary_type.empty()) {
        elem_type = spec_->elementary_type;
    }

    SIMPLE_LOG_DEBUG("Node::Initialize elem_type: {}", elem_type.c_str());
    auto elem_objects = registry->InvokeCreate(elem_type, "CPU");

    // 创建ElementaryOption
    // FIXME: ElementaryOption的解析是在NodeSpec阶段做？还是如何？
    //        calculator_option_ = spec_->option;
    //        calculator_option_ = std::make_shared<CalculatorOption>();

    calculator_option_ = spec_->option;
    if (!calculator_option_) {
        calculator_option_ = std::get<1>(elem_objects);
        if (!calculator_option_->Parse(spec_->elementary_option_json_value)) {
            return Status(StatusCode::kInvalidArgument, "option Parse err");
        }
    }

    // 处理第1个elementary
    if (spec_->elemes.size() > 0) {
        auto& v = spec_->elemes[0];

        auto elem = std::get<0>(elem_objects);
        elem->SetId(v.GetId());
        // set device
        auto d = weak_graph_.lock()->GetDeviceByName(v.GetDeviceName());
        SIMPLE_ASSERT(d);
        elem->SetDevice(d);
        calculators_.emplace_back(elem);
    }

    if (spec_->elemes.size() > 1) {
        for (int i = 1; i < spec_->elemes.size(); i++) {
            auto elem_objects = registry->InvokeCreate(elem_type, "CPU");
            auto elem         = std::get<0>(elem_objects);
            auto& v           = spec_->elemes[i];
            elem->SetId(v.GetId());
            // set device
            auto d = weak_graph_.lock()->GetDeviceByName(v.GetDeviceName());
            SIMPLE_ASSERT(d);
            elem->SetDevice(d);
            calculators_.emplace_back(elem);
        }
    }

    // inputs
    input_ports_.resize(spec_->inputs.size());
    for (const auto& v : spec_->inputs) {
        auto input_port                   = std::make_shared<InoutPort>(shared_from_this(), v);
        input_ports_[input_port->GetId()] = input_port;
        input_name_to_id_map_[input_port->GetName()] = input_port->GetId();
    }

    // outputs
    output_ports_.resize(spec_->outputs.size());
    for (const auto& v : spec_->outputs) {
        auto output_port                    = std::make_shared<InoutPort>(shared_from_this(), v);
        output_ports_[output_port->GetId()] = output_port;
        output_name_to_id_map_[output_port->GetName()] = output_port->GetId();
    }

    // inout put handler
    inout_put_handler_ = std::get<2>(elem_objects);
    inout_put_handler_->SetNode(shared_from_this());

    SIMPLE_LOG_DEBUG("Node::Initialize End");
    return Status();
}

void Node::AddInoutContext(const PacketPerNodeContextPtr& ctx) {
    std::unique_lock<std::mutex> lk(mtx_);
    inouts_queue_.push(ctx);
    //        inouts_lists_.push_back(ctx);
}

void Node::SetGraphViewWeakPtr(std::weak_ptr<GraphView> g) {
    weak_graph_ = std::move(g);
}

std::vector<size_t> Node::GetNodeIdsByName() {
    return weak_graph_.lock()->GetNodeIdsByName(skip_pkt_node_);
}

std::shared_ptr<NodeSpec> Node::GetSpec() const {
    return spec_;
}

std::shared_ptr<PacketPerNodeContext> Node::CreatePerNodeContext() {
    auto ctx = std::make_shared<PacketPerNodeContext>();
    ctx->SetNode(shared_from_this());
    // inputs, outputs size
    ctx->inputs_.resize(input_ports_.size());
    ctx->outputs_.resize(output_ports_.size());

    for (auto& in : ctx->inputs_) {
        in = PackageGroup{};
    }
    for (auto& out : ctx->outputs_) {
        out = PackageGroup{};
    }

    ctx->SetInOutputHandler(inout_put_handler_);

    // 数据类型
    return ctx;
}

size_t Node::GetId() const {
    return id_;
}

bool Node::IsNodeContextReady(const PacketPerNodeContextPtr& ctx) {
    SIMPLE_ASSERT(inout_put_handler_);
    return inout_put_handler_->IsReady(ctx);
}

Status Node::Open() {
    SIMPLE_LOG_DEBUG("Node::Open Start");
    std::vector<std::shared_ptr<Package>> inputs_data;
    std::vector<std::shared_ptr<Package>> outputs_data;
    std::shared_ptr<Host> host(nullptr);
    CalculatorContext ctx(inputs_data, outputs_data, shared_from_this(), host);
    for (auto& e : calculators_) {
        e->SetCalculatorOption(calculator_option_);
        auto s = e->Open(&ctx);
        if (!s.IsOk()) {
            SIMPLE_LOG_ERROR("Calculator: {} Open failed. ", e->GetId());
            return s;
        }
    }
    SIMPLE_LOG_DEBUG("Node::Open End");
    return Status::OkStatus();
}

const std::shared_ptr<InoutPort>& Node::GetInputPortWithId(size_t id) const {
    SIMPLE_ASSERT(id < input_ports_.size());
    return input_ports_[id];
}

const std::shared_ptr<InoutPort>& Node::GetOutputPortWithId(size_t id) const {
    SIMPLE_ASSERT(id < output_ports_.size());
    return output_ports_[id];
}

size_t Node::GetInputCount() const {
    return input_ports_.size();
}

size_t Node::GetOutputCount() const {
    return output_ports_.size();
}

size_t Node::GetInputPortIdWithTag(const std::string& tag) const {
    auto it = input_name_to_id_map_.find(tag);
    SIMPLE_ASSERT(it != input_name_to_id_map_.end());
    return it->second;
}

size_t Node::GetOutputPortIdWithTag(const std::string& tag) const {
    auto it = output_name_to_id_map_.find(tag);
    SIMPLE_ASSERT(it != output_name_to_id_map_.end());
    return it->second;
}

std::string Node::GetName() const {
    return spec_->GetName();
}

bool Node::GetOrderPreserving() const {
    return order_preserving_;
}

std::vector<std::string>& Node::GetSkipPacketNode() {
    return skip_pkt_node_;
}

} // namespace flow