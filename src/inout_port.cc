#include "inout_port.h"

namespace flow {

InoutPort::InoutPort(std::weak_ptr<Node> node, InoutSpec spec)
    : node_(std::move(node)), spec_(std::move(spec)) {}

void InoutPort::SetId(size_t id) {
    spec_.id = id;
}

size_t InoutPort::GetId() const {
    return spec_.id;
}

std::string InoutPort::GetLinkName() const {
    return spec_.link_name;
}

std::shared_ptr<Node> InoutPort::GetBelongedNode() const {
    return node_.lock();
}

std::string InoutPort::GetName() const {
    return spec_.name;
}

std::string InoutPort::GetDataType() const {
    return spec_.data_type_name;
}

bool InoutPort::IsOptional() const {
    return spec_.optional;
}
} // namespace flow
