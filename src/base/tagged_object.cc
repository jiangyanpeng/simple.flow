#include "base/tagged_object.h"

namespace flow {
TaggedObject& TaggedObject::SetName(std::string name) {
    name_ = std::move(name);
    return *this;
}

TaggedObject& TaggedObject::SetId(size_t id) {
    id_ = id;
    return *this;
}

std::string TaggedObject::Name() const {
    return name_;
}

size_t TaggedObject::Id() const {
    return id_;
}
} // namespace flow