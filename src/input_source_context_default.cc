#include "input_source_context_default.h"


namespace flow {
Status
InputSourceContextDefault::AddData(const std::string& tag, std::shared_ptr<void> d, size_t& id) {
    if (collections_.HasTag(tag)) {
        return Status(StatusCode::kAlreadyExists, "Data with tag: " + tag + " already exist.");
    }
    auto r = collections_.AddTag(tag, d);
    id     = r.second;
    return r.first;
}

std::shared_ptr<void> InputSourceContextDefault::GetData(const std::string& tag) {
    return collections_.Get(tag);
}

std::shared_ptr<void> InputSourceContextDefault::GetData(size_t id) {
    std::shared_ptr<void> d;
    collections_.Get(id, d);
    return std::move(d);
}

bool InputSourceContextDefault::HasData(const std::string& tag) const {
    return collections_.HasTag(tag);
}

bool InputSourceContextDefault::HasData(size_t id) const {
    if (id >= collections_.GetNumEntries()) {
        return false;
    }
    return true;
}

Status InputSourceContextDefault::DeleteData(const std::string& tag) {
    return collections_.RemoveTag(tag);
}
} // namespace flow