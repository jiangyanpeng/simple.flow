#ifndef ARCHITECT_PACKAGE_H
#define ARCHITECT_PACKAGE_H

#include "base/contract_coding.h"
#include <bitset>
#include <functional>
#include <log.h>
#include <map>
#include <memory>
#include <vector>
namespace flow {
// 描述一组数据，可包括任意对象
// 类似于Tensor，但保存的不是基础数据单元. 支持非满Tensor
//

constexpr int16_t MAX_NUM_ELEMENTS = 1024;

class PackageShape {
public:
    PackageShape() {}

    ~PackageShape() {}

    void SetShape(std::vector<uint8_t> shape);

    bool IsFull() const;

    std::vector<uint8_t> GetShapeVec() const;

    /**
     * deprecated
     * @return
     */
    std::vector<uint8_t> GetShape() const;

    std::vector<uint8_t> GetSub1ShapeVec() const;

    uint8_t Dims() const;

    // 第N维的shape
    uint8_t NthDimShape(size_t n) const;

    uint8_t GetLastDimShape() const;

    // 指定Index，获取其Indicator的下标
    // 返回值为Id区间，[first, second)
    int16_t GetIdFromFullIndex(const std::vector<uint8_t>& index) const;

    std::pair<int16_t, int16_t> GetIdFromIndex(const std::vector<uint8_t>& index) const;

    std::vector<uint8_t> GetIndexFromId(int16_t id) const;

    int64_t NumElements() const;

    int64_t MaxNumElements() const;

    // 标识Index位置被占用.
    void Mark(const std::vector<uint8_t>& index);

    void Mark(int index);

    bool IsMarked(const std::vector<uint8_t>& index) const;

    // index size小于shape size
    // 不同维度下，特定index是否为空，如shape[2, 3, 4]
    // 可判定[0, 0] 是否为空（共判断四个indicator）
    // 可判定[0, 1] 是否为空（共判断四个indicator）
    // 可判定[0, 1, 1] 是否为空，与IsMarked等效
    bool IsIndexEmpty(const std::vector<uint8_t>& index) const;

    /**
     *
     * @param n [1, Dims()]
     * @return
     */
    std::shared_ptr<PackageShape> IsNthDimEmpty(size_t n) const;

    /**
     * 倒数第二维是否为空
     * @return
     */
    std::shared_ptr<PackageShape> IsLast2DimEmpty() const;

    bool IsMarked(int index) const;

    void Walk(const std::function<void(const std::vector<uint8_t>&)>& f);

    /**
     * 在维度N进行遍历
     * @param n 维度信息，取值范围[1, Dims()]
     * @param f 操作函数 std::function<void(int16_t start_id, int16_t end_id)>
     *  start_id，end_id为Nth维度index的id起点和终点。 [start_id, end_id)
     */
    void WalkNthDim(uint8_t n,
                    const std::function<void(const std::vector<uint8_t>&, int16_t, int16_t)>& f);

private:
    void Walk(const std::vector<uint8_t>& shape,
              std::vector<uint8_t>& index,
              int depth,
              const std::function<void(const std::vector<uint8_t>&)>& f) const;

    void Walk(const std::vector<uint8_t>& shape,
              std::vector<uint8_t>& index,
              int depth,
              uint8_t n,
              const std::function<void(const std::vector<uint8_t>&, int16_t, int16_t)>& f) const;

private:
    // 总的维shape[D0, D1, D2, ... Dn]
    std::vector<uint8_t> shape_;
    // 标识数据，false 标识在特定index上无数据。
    std::bitset<MAX_NUM_ELEMENTS> indicator_bitset_;
    int64_t max_num_elements_{0};
};

// FIXME: 数据类型校验
// Package存放的是数据的智能指针
// 对于Buffer类的，需设计Buffer的MetaData，使用Package管理，数据部分考虑使用连续内存，便于设备使用。
class Package {
public:
    Package() {}
    ~Package() {}

    void SetShape(std::vector<uint8_t> shape);

    void SetShape(const PackageShape& pkg_shape);

    const PackageShape& GetShape() const;

    template <typename T>
    bool AddData(const std::vector<uint8_t>& index, std::shared_ptr<T> data) {
        int16_t indicator = shape_.GetIdFromFullIndex(index);
        if (data_index_.find(indicator) != data_index_.end()) {
            return false;
        }
        if (shape_.IsMarked(indicator)) {
            return false;
        }
        data_.push_back(data);
        //        data_index_.push_back(indicator);
        data_index_[indicator] = data_.size() - 1;
        shape_.Mark(indicator);
        return true;
    }

    template <class T>
    bool AddData(int16_t index, std::shared_ptr<T> data) {
        MATRIX_ASSERT(index < shape_.MaxNumElements());
        if (data_index_.find(index) != data_index_.end()) {
            return false;
        }
        data_.push_back(data);
        //        data_index_.push_back(index);
        data_index_[index] = data_.size() - 1;
        shape_.Mark(index);
        return true;
    }

    int64_t NumElements() const;

    const std::vector<std::shared_ptr<void>>& GetData() const;

    template <typename T>
    std::shared_ptr<T> GetData(int16_t index) {
        MATRIX_ASSERT(index >= 0 && index < shape_.MaxNumElements());
        auto id     = data_index_[index];
        auto result = std::static_pointer_cast<T>(data_[id]);
        return std::move(result);
    }

    std::vector<int16_t> GetDataIndex() const;

private:
    PackageShape shape_;
    std::vector<std::shared_ptr<void>> data_;
    std::map<int16_t, int16_t> data_index_;
};

} // namespace flow


#endif // ARCHITECT_PACKAGE_H
