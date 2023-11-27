#include "input_source_manager.h"
#include "input_source_context_default.h"

namespace flow {
std::shared_ptr<InputSourceContext> InputSourceManager::GenerateSourceContext() {
    int64_t id = source_id_.fetch_add(1);
    auto ctx   = std::make_shared<InputSourceContextDefault>();
    ctx->SetId(id);
    ctx->SetInputCount(graph_input_count_);
    context_map_[id] = ctx;
    return std::move(ctx);
}

bool InputSourceManager::RemoveSourceContext(int64_t id) {
    context_map_.erase(id);
    return true;
}

void InputSourceManager::SetGraphInputCount(size_t count) {
    graph_input_count_ = count;
}

std::shared_ptr<InputSourceContext> InputSourceManager::DefaultSourceContext() {
    if (source_id_.load() == 0) {
        return GenerateSourceContext();
    } else {
        return context_map_[0];
    }
}

} // namespace flow