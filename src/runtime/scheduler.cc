#include "runtime/scheduler.h"

namespace pipeline {
void Scheduler::Start() {}

Scheduler::Scheduler() {}

void Scheduler::Stop() {
    // pool_.Stop();
}

void Scheduler::SetGraphView(const std::shared_ptr<GraphView>& view) {
    graph_view_ = view;
}

// void Scheduler::SetGraphPacketFinishedCallback(GraphPacketFinishedCallback& callback) {
//     graph_call_back_ = callback;
// }
} // namespace pipeline