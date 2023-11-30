
#include "packet.h"
#include "core/contract_coding.h"
#include "package.h"

#include <utility>

namespace flow {
Packet::Packet(std::shared_ptr<InputPktId> id) : id_(id) {
    package_.reserve(1);
    package_.resize(1);
}

Packet::Packet(std::shared_ptr<InputPktId> id, size_t package_num) : id_(id) {
    package_.reserve(package_num);
    package_.resize(package_num);
}


Status Packet::AddPackage(size_t package_index, std::shared_ptr<Package> package) {
    SIMPLE_ASSERT(package_index < package_.size());
    SIMPLE_LOG_DEBUG("package_index: {}, size: {}", package_index, GetPackageNum());
    package_[package_index] = std::move(package);
    return Status();
}

std::shared_ptr<Package> Packet::GetPackage(size_t index) {
    SIMPLE_ASSERT(index < package_.size());
    return package_[index];
}

uint64_t Packet::GetId() const {
    return id_->PacketId();
}

size_t Packet::GetPackageNum() const {
    return package_.size();
}

void Packet::SetStreamContext(const std::shared_ptr<InputStreamContext>& ctx) {
    SIMPLE_ASSERT(ctx->GetId() == id_->InputStreamId());
    stream_ctx_ = ctx;
}

std::shared_ptr<InputStreamContext> Packet::GetStreamContext() const {
    return stream_ctx_.lock();
}

uint8_t Packet::GetStreamId() const {
    return id_->InputStreamId();
}

std::shared_ptr<InputPktId> Packet::GetInputPktId() const {
    return id_;
}
} // namespace flow