
#include "core/data_type.h"

namespace flow {

void DataType::SetTypeName(const std::string& name) {
    type_name_ = name;
}

const std::string DataType::GetTypeName() const {
    return type_name_;
}
} // namespace flow