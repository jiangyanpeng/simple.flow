

#include "packet_per_node_context.h"
#include "node.h"
#include "packet_context.h"
#include "runtime/waiting_thread.h"

#include <log.h>
#include <utility>

namespace flow {
PacketPerNodeContext::PacketPerNodeContext() {}

PacketPerNodeContext::~PacketPerNodeContext() {}

bool PacketPerNodeContext::IsInputReady() {
    return inoutput_handler_.lock()->IsInputReady(shared_from_this());
}

bool PacketPerNodeContext::IsOutputReady() {
    return inoutput_handler_.lock()->IsOutputReady(shared_from_this());
}

void PacketPerNodeContext::SetPacketContext(std::shared_ptr<PacketContext> pkt_ctx) {
    packet_ctx_ = std::move(pkt_ctx);
}

void PacketPerNodeContext::SetNode(std::weak_ptr<Node> node) {
    node_ = std::move(node);
}

std::shared_ptr<Node> PacketPerNodeContext::GetNode() const {
    return node_.lock();
}

std::shared_ptr<PacketContext> PacketPerNodeContext::GetPacketContext() const {
    if (!packet_ctx_.lock()) {
        SIMPLE_LOG_ERROR("lock packet context failed");
    }
    return packet_ctx_.lock();
}

void PacketPerNodeContext::PropagateOutputPortTo(std::shared_ptr<PacketPerNodeContext>& dst,
                                                 size_t dst_port_id,
                                                 size_t src_port_id) {
    auto& dst_vec = dst->inputs_[dst_port_id];
    auto& src_vec = outputs_[src_port_id];

    // src_vec为空，则说明该Node的src_port_id上没有输出，那么就不做传递
    dst_vec.insert(dst_vec.end(), src_vec.begin(), src_vec.end());
}

void PacketPerNodeContext::DoDebug() {
    SIMPLE_LOG_INFO("packetpernodecontext: node id: {} ", node_.lock()->GetId());
    int index = 0;
    for (auto& v : inputs_) {
        SIMPLE_LOG_INFO("   [input {}: size {}]", index++, v.size());
    }

    for (auto& v : outputs_) {
        SIMPLE_LOG_INFO("   [output {}: size {}]", index++, v.size());
    }
}

std::vector<size_t> PacketPerNodeContext::GetNodeIdsByName() {
    return GetNode()->GetNodeIdsByName();
}

const std::vector<PackageGroup>& PacketPerNodeContext::GetInputs() const {
    return inputs_;
}

void PacketPerNodeContext::SetInOutputHandler(
    const std::shared_ptr<InOutputHandler>& input_handler) {
    inoutput_handler_ = input_handler;
}

void PacketPerNodeContext::SetStream(std::shared_ptr<Stream> stream) {
    stream_ = std::move(stream);
}

const std::shared_ptr<Stream>& PacketPerNodeContext::GetStream() const {
    return stream_;
}

const std::vector<PackageGroup>& PacketPerNodeContext::GetOutputs() const {
    return outputs_;
}

void PacketPerNodeContext::SetRefPackageShape(const std::shared_ptr<PackageShape>& shape) {
    ref_shape_ = shape;
}

std::shared_ptr<InputSourceContext> PacketPerNodeContext::GetInputSourceContext() const {
    if (!packet_ctx_.lock()) {
        SIMPLE_LOG_ERROR("lock packet context failed");
    }
    return packet_ctx_.lock()->GetInputSourceContext();
}

int64_t PacketPerNodeContext::GetUniqueId() const {
    return unique_id_;
}

void PacketPerNodeContext::SetUniqueId(int64_t id) {
    unique_id_ = id;
}

void PacketPerNodeContext::SetNotifier(std::shared_ptr<Notifier> notifier) {
    notifier_ = std::move(notifier);
}

bool PacketPerNodeContext::Notify() const {
    return notifier_->Notify(GetUniqueId());
}

std::shared_ptr<Notifier> PacketPerNodeContext::GetNotifier() const {
    return notifier_;
}
} // namespace flow
