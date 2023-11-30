#include "calculator/calculator_context.h"
#include "core/collection.h"
#include "inout_port.h"
#include "node.h"

#include <utility>

namespace flow {

CalculatorContext::~CalculatorContext() {}

CalculatorContext::CalculatorContext(std::vector<std::shared_ptr<Package>>& inputs_data,
                                     std::vector<std::shared_ptr<Package>>& outputs_data,
                                     std::shared_ptr<Node> node,
                                     const std::shared_ptr<Host>& host)
    : inputs_data_(inputs_data),
      outputs_data_(outputs_data),
      node_(std::move(node)),
      host_(host) {}

size_t CalculatorContext::GetInputIdWithTag(const std::string& tag) {
    return node_->GetInputPortIdWithTag(tag);
}

size_t CalculatorContext::GetOutputIdWithTag(const std::string& tag) {
    return node_->GetOutputPortIdWithTag(tag);
}

std::shared_ptr<Package> CalculatorContext::GetInputData(const std::string& tag) {
    size_t id = GetInputIdWithTag(tag);
    return GetInputData(id);
}

std::shared_ptr<Package> CalculatorContext::GetOutputData(const std::string& tag) {
    size_t id = GetOutputIdWithTag(tag);
    return GetOutputData(id);
}

std::shared_ptr<Package> CalculatorContext::GetInputData(size_t id) {
    SIMPLE_ASSERT(id < inputs_data_.size());
    return inputs_data_[id];
}

std::shared_ptr<Package> CalculatorContext::GetOutputData(size_t id) {
    SIMPLE_ASSERT(id < outputs_data_.size());
    return outputs_data_[id];
}

void CalculatorContext::AddOutputData(const std::string& tag, const std::shared_ptr<Package>& pkg) {
    size_t id = GetOutputIdWithTag(tag);

    AddOutputData(id, pkg);
}

void CalculatorContext::AddOutputData(size_t id, const std::shared_ptr<Package>& pkg) {
    SIMPLE_ASSERT(id < outputs_data_.size());
    outputs_data_[id] = pkg;
}

void CalculatorContext::AddSkipNode() {
    std::vector<size_t> node_ids = GetPacketPerNodeContext()->GetNodeIdsByName();
    if (!node_ids.empty()) {
        uint64_t pkt_id = GetPacketPerNodeContext()->GetPacketContext()->GetPacketId();
        auto source_ctx = GetInputSourceContext();
        source_ctx->AddSkipPktId(node_ids, pkt_id);
    }
}

std::shared_ptr<Host> CalculatorContext::GetStream() const {
    return host_;
}

bool CalculatorContext::IsInputOptional(const std::string& tag) {
    auto id = GetInputIdWithTag(tag);
    return IsInputOptional(id);
}

bool CalculatorContext::IsInputOptional(size_t id) {
    auto& port = node_->GetInputPortWithId(id);
    return port->IsOptional();
}

void CalculatorContext::SetPacketPerNodeContext(const std::shared_ptr<PacketPerNodeContext>& ctx) {
    pernode_ctx_ = ctx;
}

void CalculatorContext::SetPacketPerNodeRefShape(const std::shared_ptr<PackageShape>& shape) {
    pernode_ctx_.lock()->SetRefPackageShape(shape);
}

std::shared_ptr<PacketPerNodeContext> CalculatorContext::GetPacketPerNodeContext() const {
    return pernode_ctx_.lock();
}

std::shared_ptr<InputStreamContext> CalculatorContext::GetInputSourceContext() const {
    return pernode_ctx_.lock()->GetInputSourceContext();
}

} // namespace flow