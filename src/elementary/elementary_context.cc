#include "elementary/elementary_context.h"
#include "base/collection.h"
#include "inout_port.h"
#include "node.h"

#include <utility>

namespace flow {

ElementaryContext::~ElementaryContext() {}

ElementaryContext::ElementaryContext(std::vector<std::shared_ptr<Package>>& inputs_data,
                                     std::vector<std::shared_ptr<Package>>& outputs_data,
                                     std::shared_ptr<Node> node,
                                     const std::shared_ptr<Stream>& stream)
    : inputs_data_(inputs_data),
      outputs_data_(outputs_data),
      node_(std::move(node)),
      stream_(stream) {}

size_t ElementaryContext::GetInputIdWithTag(const std::string& tag) {
    return node_->GetInputPortIdWithTag(tag);
}

size_t ElementaryContext::GetOutputIdWithTag(const std::string& tag) {
    return node_->GetOutputPortIdWithTag(tag);
}

std::shared_ptr<Package> ElementaryContext::GetInputData(const std::string& tag) {
    size_t id = GetInputIdWithTag(tag);
    return GetInputData(id);
}

std::shared_ptr<Package> ElementaryContext::GetOutputData(const std::string& tag) {
    size_t id = GetOutputIdWithTag(tag);
    return GetOutputData(id);
}

std::shared_ptr<Package> ElementaryContext::GetInputData(size_t id) {
    MATRIX_ASSERT(id < inputs_data_.size());
    return inputs_data_[id];
}

std::shared_ptr<Package> ElementaryContext::GetOutputData(size_t id) {
    MATRIX_ASSERT(id < outputs_data_.size());
    return outputs_data_[id];
}

void ElementaryContext::AddOutputData(const std::string& tag, const std::shared_ptr<Package>& pkg) {
    size_t id = GetOutputIdWithTag(tag);

    AddOutputData(id, pkg);
}

void ElementaryContext::AddOutputData(size_t id, const std::shared_ptr<Package>& pkg) {
    MATRIX_ASSERT(id < outputs_data_.size());
    outputs_data_[id] = pkg;
}

void ElementaryContext::AddSkipNode() {
    std::vector<size_t> node_ids = GetPacketPerNodeContext()->GetNodeIdsByName();
    if (!node_ids.empty()) {
        uint64_t pkt_id = GetPacketPerNodeContext()->GetPacketContext()->GetPacketId();
        auto source_ctx = GetInputSourceContext();
        source_ctx->AddSkipPktId(node_ids, pkt_id);
    }
}

std::shared_ptr<Stream> ElementaryContext::GetStream() const {
    return stream_;
}

bool ElementaryContext::IsInputOptional(const std::string& tag) {
    auto id = GetInputIdWithTag(tag);
    return IsInputOptional(id);
}

bool ElementaryContext::IsInputOptional(size_t id) {
    auto& port = node_->GetInputPortWithId(id);
    return port->IsOptional();
}

void ElementaryContext::SetPacketPerNodeContext(const std::shared_ptr<PacketPerNodeContext>& ctx) {
    pernode_ctx_ = ctx;
}

void ElementaryContext::SetPacketPerNodeRefShape(const std::shared_ptr<PackageShape>& shape) {
    pernode_ctx_.lock()->SetRefPackageShape(shape);
}

std::shared_ptr<PacketPerNodeContext> ElementaryContext::GetPacketPerNodeContext() const {
    return pernode_ctx_.lock();
}

std::shared_ptr<InputSourceContext> ElementaryContext::GetInputSourceContext() const {
    return pernode_ctx_.lock()->GetInputSourceContext();
}

} // namespace flow