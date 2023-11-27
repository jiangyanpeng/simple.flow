
#include "package.h"
#include "base/contract_coding.h"
#include <iostream>

#include <utility>

namespace flow {

void PackageShape::SetShape(std::vector<uint8_t> shape) {
    shape_ = std::move(shape);
    indicator_bitset_.reset();
    max_num_elements_ = 1;
    for (auto v : shape_) {
        max_num_elements_ *= v;
    }
    MATRIX_ASSERT(max_num_elements_ <= 1024);
}

std::vector<uint8_t> PackageShape::GetShapeVec() const {
    return shape_;
}

uint8_t PackageShape::Dims() const {
    return shape_.size();
}

int16_t PackageShape::GetIdFromFullIndex(const std::vector<uint8_t>& index) const {
    MATRIX_ASSERT(index.size() == shape_.size());
    int id     = 0;
    int stride = 1;
    for (int i = int(shape_.size() - 1); i >= 0; --i) {
        id += stride * index[i];
        stride *= shape_[i];
    }
    return id;
}

std::pair<int16_t, int16_t> PackageShape::GetIdFromIndex(const std::vector<uint8_t>& index) const {
    MATRIX_ASSERT(index.size() <= shape_.size());
    auto index_t = index;
    index_t.resize(shape_.size());
    int16_t start = GetIdFromFullIndex(index_t);
    index_t       = index;
    ++index_t[index_t.size() - 1];
    index_t.resize(shape_.size());
    int16_t end = GetIdFromFullIndex(index_t);
    return std::make_pair(start, end);
}

PackageShape::~PackageShape() {}

int64_t PackageShape::NumElements() const {
    return indicator_bitset_.count();
}

int64_t PackageShape::MaxNumElements() const {
    return max_num_elements_;
}

bool PackageShape::IsFull() const {
    return indicator_bitset_.all();
}

void PackageShape::Mark(const std::vector<uint8_t>& index) {
    int indicator = GetIdFromFullIndex(index);
    Mark(indicator);
}

bool PackageShape::IsMarked(const std::vector<uint8_t>& index) const {
    int id = GetIdFromFullIndex(index);
    return IsMarked(id);
}

bool PackageShape::IsMarked(int index) const {
    return indicator_bitset_[index];
}

void PackageShape::Mark(int index) {
    indicator_bitset_.set(index);
}

std::vector<uint8_t> PackageShape::GetSub1ShapeVec() const {
    std::vector<uint8_t> result;
    if (Dims() <= 1) {
        return result;
    }
    for (size_t i = 0; i < Dims() - 1; ++i) {
        result.push_back(shape_[i]);
    }
    return std::move(result);
}

bool PackageShape::IsIndexEmpty(const std::vector<uint8_t>& index) const {
    MATRIX_ASSERT(index.size() <= shape_.size());
    size_t start_pos  = 0;
    size_t end_pos    = 0;
    auto index_append = index;

    index_append.resize(shape_.size(), 0);
    start_pos                      = GetIdFromFullIndex(index_append);
    index_append[index.size() - 1] = index[index.size() - 1] + 1;
    end_pos                        = GetIdFromFullIndex(index_append);

    for (int i = start_pos; i < end_pos; ++i) {
        if (IsMarked(i)) {
            return false;
        }
    }
    return true;
}

uint8_t PackageShape::NthDimShape(size_t n) const {
    MATRIX_ASSERT(n < shape_.size());
    return shape_[n];
}

std::vector<uint8_t> PackageShape::GetIndexFromId(int16_t id) const {
    MATRIX_ASSERT(id < MaxNumElements());
    std::vector<uint8_t> index(shape_.size(), 0);

    int16_t x      = 0;
    int16_t y      = 0;
    size_t depth   = index.size() - 1;
    int16_t stride = shape_[depth];
    do {
        x            = id % stride;
        id           = id / stride;
        index[depth] = x;
        if (0 == id || 0 == depth) {
            break;
        }
        --depth;
        stride = shape_[depth];
    } while (true);
    return std::move(index);
}

uint8_t PackageShape::GetLastDimShape() const {
    return *shape_.rbegin();
}

std::shared_ptr<PackageShape> PackageShape::IsNthDimEmpty(size_t n) const {
    MATRIX_ASSERT(n <= shape_.size() && n >= 1);
    if (n == shape_.size()) {
        return std::make_shared<PackageShape>(*this);
    }
    auto sub_shape = shape_;
    sub_shape.resize(n);

    auto result = std::make_shared<PackageShape>();
    result->SetShape(sub_shape);

    std::vector<uint8_t> index(sub_shape.size());
    Walk(sub_shape, index, 0, [&](const std::vector<uint8_t>& idx) {
        bool e = IsIndexEmpty(idx);
        if (e) {
            result->Mark(idx);
        }
    });
    return result;
}

void PackageShape::Walk(const std::vector<uint8_t>& shape,
                        std::vector<uint8_t>& index,
                        int depth,
                        const std::function<void(const std::vector<uint8_t>&)>& f) const {
    if (shape.size() == 1) {
        for (int i = 0; i < shape[0]; ++i) {
            index[depth] = i;
            f(index);
        }
        return;
    }
    for (int i = 0; i < shape[0]; ++i) {
        std::vector<uint8_t> right;
        for (int j = 1; j < shape.size(); ++j) {
            right.push_back(shape[j]);
        }
        index[depth] = i;
        Walk(right, index, depth + 1, f);
    }
}

void PackageShape::Walk(const std::function<void(const std::vector<uint8_t>&)>& f) {
    std::vector<uint8_t> index(shape_.size());
    Walk(shape_, index, 0, f);
}

std::shared_ptr<PackageShape> PackageShape::IsLast2DimEmpty() const {
    MATRIX_ASSERT(Dims() >= 1);
    return IsNthDimEmpty(Dims() - 1);
}

void PackageShape::WalkNthDim(
    uint8_t n,
    const std::function<void(const std::vector<uint8_t>& sub_shape, int16_t s, int16_t e)>& f) {
    MATRIX_ASSERT(n <= shape_.size() && n >= 0);
    std::vector<uint8_t> index(n);
    Walk(shape_, index, 0, Dims() - n, f);
}

void PackageShape::Walk(
    const std::vector<uint8_t>& shape,
    std::vector<uint8_t>& index,
    int depth,
    uint8_t dim_left,
    const std::function<void(const std::vector<uint8_t>&, int16_t, int16_t)>& f) const {
    if (shape.size() == dim_left) {
        auto se = GetIdFromIndex(index);
        f(index, se.first, se.second);
        return;
    }
    for (int i = 0; i < shape[0]; ++i) {
        std::vector<uint8_t> right;
        for (int j = 1; j < shape.size(); ++j) {
            right.push_back(shape[j]);
        }
        index[depth] = i;
        Walk(right, index, depth + 1, dim_left, f);
    }
}

std::vector<uint8_t> PackageShape::GetShape() const {
    return std::move(GetShapeVec());
}

void Package::SetShape(std::vector<uint8_t> shape) {
    shape_.SetShape(std::move(shape));
}

void Package::SetShape(const PackageShape& pkg_shape) {
    shape_.SetShape(pkg_shape.GetShapeVec());
}

const PackageShape& Package::GetShape() const {
    return shape_;
}

int64_t Package::NumElements() const {
    return shape_.NumElements();
}

std::vector<int16_t> Package::GetDataIndex() const {
    std::vector<int16_t> indexes;
    for (auto i : data_index_) {
        indexes.push_back(i.first);
    }
    return std::move(indexes);
}

const std::vector<std::shared_ptr<void>>& Package::GetData() const {
    return data_;
}
} // namespace flow
