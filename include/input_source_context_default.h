#ifndef SIMPLE_FLOW_INPUT_SOURCE_CONTEXT_DEFAULT_H_
#define SIMPLE_FLOW_INPUT_SOURCE_CONTEXT_DEFAULT_H_

#include "input_source_context.h"

namespace flow {
class InputSourceContextDefault : public InputSourceContext {
public:
    InputSourceContextDefault() = default;

    ~InputSourceContextDefault() override = default;


    Status AddData(const std::string& tag, std::shared_ptr<void> d, size_t& id) override;

    std::shared_ptr<void> GetData(const std::string& tag) override;

    std::shared_ptr<void> GetData(size_t id) override;

    bool HasData(const std::string& tag) const override;

    bool HasData(size_t id) const override;

    Status DeleteData(const std::string& tag) override;

protected:
    Collection<std::shared_ptr<void>> collections_;
};

} // namespace flow
#endif // SIMPLE_FLOW_INPUT_SOURCE_CONTEXT_DEFAULT_H_
