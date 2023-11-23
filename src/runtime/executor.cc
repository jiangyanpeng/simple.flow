#include "runtime/executor.h"

#include <log.h>

namespace pipeline {
void ExecutorOption::open() {
    SIMPLE_LOG_DEBUG("use gpu: {}, gpu id: {}", use_gpu_, gpu_id_);
}

Executor::Executor(size_t thread_nums, std::shared_ptr<ExecutorOption> executor_option)
    : pipe_manager_(std::make_shared<base::PipeManager>(thread_nums)) {
    if (executor_option != nullptr) {
        executor_option_ = executor_option;
        // for (auto& thread : pipe_manager_->GetThreads()) {
        //     executor_option_->open();
        // }
    }
}

Executor::~Executor() {
    Stop();
}

void Executor::Stop() {
    pipe_manager_->Stop();
}

void Executor::AddTask(const std::function<void()>& t) {
    pipe_manager_->Commit(t);
}
} // namespace pipeline
