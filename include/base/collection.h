#ifndef ARCHITECT_COLLECTION_H
#define ARCHITECT_COLLECTION_H

#include "base/contract_coding.h"
#include "base/status.h"
#include <map>
#include <string>
#include <vector>

namespace flow {

template <typename T>
class Collection {
public:
    Collection() = default;

    ~Collection() = default;

    Status Merge(const Collection<T>& collection) {
        for (auto iter = collection.tag_map_.begin(); iter != collection.tag_map_.end(); ++iter) {
            auto tag   = iter->first;
            auto index = iter->second;
            auto v     = collection.data_types_[index];

            if (HasTag(tag)) {
                return Status(StatusCode::kAlreadyExists, tag);
            }
            AddTag(tag, v);
        }
        return Status::OkStatus();
    }

    bool HasTag(const std::string& tag) const {
        if (tag_map_.find(tag) != tag_map_.end()) {
            return true;
        } else {
            return false;
        }
    }

    // 返回DataType&, 支持链式调用
    std::pair<Status, size_t> AddTag(const std::string tag, T& type) {
        if (HasTag(tag)) {
            return {Status::InvalidArgument("Tag {} already exists."), 0};
        }
        data_types_.push_back(type);
        size_t index  = data_types_.size() - 1;
        tag_map_[tag] = index;
        return std::make_pair(Status::OkStatus(), index);
    }

    // 如果存在，则覆盖
    // AddTag("TAG").Set<T>()
    T& AddTag(const std::string tag) {
        if (!HasTag(tag)) {
            T type;
            data_types_.push_back(type);
            tag_map_[tag] = data_types_.size() - 1;
            return data_types_[tag_map_[tag]];
        }
        return data_types_[tag_map_[tag]];
    }


    Status RemoveTag(const std::string& tag) {
        if (!HasTag(tag)) {
            // log for not exists.
        } else {
            auto index = tag_map_.at(tag);
            tag_map_.erase(tag);
            data_types_.erase(data_types_.begin() + index);
        }
        return Status::OkStatus();
    }

    //        Status RemoveTag(size_t id) {
    //            if (!HasTag(id)) {
    //                // log for not exists.
    //            } else {
    //                auto index = tag_map_.at(tag);
    //                tag_map_.erase(tag);
    //                data_types_.erase(data_types_.begin() + index);
    //            }
    //            return Status::OkStatus();
    //        }


    Status Get(const size_t index, T& type) {
        if (index >= data_types_.size()) {
            return Status::InvalidArgument("index out of range");
        }
        type = data_types_[index];
        return Status::OkStatus();
    }

    Status Get(const std::string& tag, T& type) const {
        if (!HasTag(tag)) {
            return Status::InvalidArgument("Tag does not exists.");
        }
        size_t index = tag_map_.at(tag);
        type         = data_types_[index];
        return Status::OkStatus();
    }

    // 支持链式调用，tag必须存在
    T& Get(const std::string& tag) {
        if (!HasTag(tag)) {
            // log fatal
            // fixme
            //                return T();
        }
        return data_types_[tag_map_.at(tag)];
    }

    size_t GetNumEntries() const {
        MATRIX_ASSERT(data_types_.size() == tag_map_.size());
        return data_types_.size();
    }

    const std::vector<T>& GetVec() const { return data_types_; }

private:
    std::map<std::string, size_t> tag_map_;
    std::vector<T> data_types_;
};

} // namespace flow

#endif // LUCY_COLLECTION_H
