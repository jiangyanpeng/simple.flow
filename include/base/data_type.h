#ifndef SIMPLE_FLOW_DATA_TYPE_H_
#define SIMPLE_FLOW_DATA_TYPE_H_

#include <string>

namespace flow {
class DataType {
public:
    DataType() = default;

    ~DataType() = default;

    void SetTypeName(const std::string& name);

    const std::string GetTypeName() const;

private:
    std::string type_name_{};
};
} // namespace flow

#endif // SIMPLE_FLOW_DATA_TYPE_H_
