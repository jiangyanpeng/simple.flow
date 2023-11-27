#ifndef ARCHITECT_TENSOR_H
#define ARCHITECT_TENSOR_H

#include <memory>

namespace flow {

class Tensor {
public:
    Tensor()          = default;
    virtual ~Tensor() = default;

    std::shared_ptr<void> value_;
};

} // namespace flow


#endif // ARCHITECT_TENSOR_H
