#ifndef ARCHITECT_DATA_TYPE_H
#define ARCHITECT_DATA_TYPE_H

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

#endif // ARCHITECT_DATA_TYPE_H
