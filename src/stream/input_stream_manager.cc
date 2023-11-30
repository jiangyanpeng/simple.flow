#include "stream/input_stream_manager.h"
#include "stream/input_stream_context_default.h"

namespace flow {
std::shared_ptr<InputStreamContext> InputStreamManager::GenerateSourceContext() {
    int64_t id = source_id_.fetch_add(1);
    auto ctx   = std::make_shared<InputStreamContextDefault>();
    ctx->SetId(id);
    ctx->SetInputCount(graph_input_count_);
    context_map_[id] = ctx;
    return std::move(ctx);
}

bool InputStreamManager::RemoveSourceContext(int64_t id) {
    context_map_.erase(id);
    return true;
}

void InputStreamManager::SetGraphInputCount(size_t count) {
    graph_input_count_ = count;
}

std::shared_ptr<InputStreamContext> InputStreamManager::DefaultSourceContext() {
    if (source_id_.load() == 0) {
        return GenerateSourceContext();
    } else {
        return context_map_[0];
    }
}

} // namespace flow