#ifndef ARCHITECT_MATRIX_FUNCTION_REGISTRY_H
#define ARCHITECT_MATRIX_FUNCTION_REGISTRY_H

#include "base/contract_coding.h"
#include <functional>
#include <log.h>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace flow {

static std::string NormalizeNsName(const std::string& ns, const std::string& name);

template <typename Ret, typename... Args>
class MatrixFunctionRegistry final {
public:
    using Function = std::function<Ret(Args...)>;

    MatrixFunctionRegistry() = default;

    ~MatrixFunctionRegistry() = default;

    void Register(const std::string& ns, const std::string& name, Function func) {
        std::string norm_name = NormalizeNsName(ns, name);
        Register(norm_name);
    }

    void Register(const std::string& name, Function func) {
        if (!IsRegistered(name)) {
            std::unique_lock<std::mutex> lock(mutex_);
            functions_[name] = func;
        }
    }

    bool IsRegistered(const std::string& ns, const std::string& name) {
        std::string norm_name = NormalizeNsName(ns, name);
        return IsRegistered(norm_name);
    }

    bool IsRegistered(const std::string& name) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (functions_.count(name) > 0) {
            return true;
        } else {
            return false;
        }
    }

    Ret Invoke(const std::string& ns, const std::string& name, Args&&... args) {
        std::string norm_name = NormalizeNsName(ns, name);
        return Invoke(norm_name, std::forward<Args>(args)...);
    }

    Ret Invoke(const std::string& name, Args&&... args) {
        if (!IsRegistered(name)) {
            SIMPLE_LOG_ERROR("function: {} is not registered.", name);
            MATRIX_ASSERT(0);
        }
        Function f;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            f = functions_.at(name);
        }
        Ret ret = f(std::forward<Args>(args)...);
        return ret;
    }

    std::vector<std::string> GetRegisteredName() const {
        std::vector<std::string> result;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            for (auto& v : functions_) {
                result.push_back(v.first);
            }
        }
        return result;
    }



private:
    std::unordered_map<std::string, Function> functions_;
    // fixme: 读写锁
    std::mutex mutex_;
};

template <typename T, typename... Args>
class CreateFunction {
public:
    static std::shared_ptr<T> Create(Args&&... args) {
        return std::make_shared<T>(std::forward<args>...);
    }
};
} // namespace flow


#endif // ARCHITECT_MATRIX_FUNCTION_REGISTRY_H
