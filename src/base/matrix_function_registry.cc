#include "base/matrix_function_registry.h"

namespace flow {

constexpr const char kNsNameSep = '.';
std::string NormalizeNsName(const std::string& ns, const std::string& name) {
    if (std::string::npos != ns.find('.')) {
        SIMPLE_LOG_ERROR("namespace should not contain {}", kNsNameSep);
        MATRIX_ASSERT(0);
    }
    std::string norm_name(ns);
    norm_name.append(1, kNsNameSep);
    norm_name.append(name);
    return std::move(norm_name);
}
} // namespace flow