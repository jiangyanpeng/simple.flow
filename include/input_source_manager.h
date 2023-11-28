#ifndef SIMPLE_FLOW_INPUT_SOURCE_MANAGER_H_
#define SIMPLE_FLOW_INPUT_SOURCE_MANAGER_H_

#include "input_source_context.h"

#include <atomic>
#include <map>
#include <memory>

namespace flow {
class InputSourceManager {
public:
    InputSourceManager() {}

    ~InputSourceManager() {}

    void SetGraphInputCount(size_t count);

    std::shared_ptr<InputSourceContext> GenerateSourceContext();

    std::shared_ptr<InputSourceContext> DefaultSourceContext();

    bool RemoveSourceContext(int64_t id);

private:
    std::atomic_int_fast64_t source_id_{0};
    size_t graph_input_count_{0};
    std::map<int64_t, std::shared_ptr<InputSourceContext>> context_map_;
};

} // namespace flow
#endif // SIMPLE_FLOW_INPUT_SOURCE_MANAGER_H_
