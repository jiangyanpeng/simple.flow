
#include "runtime/executor.h"
#include <log.h>


namespace flow {

void ExecutorOption::open() {
    SIMPLE_LOG_INFO("use gpu: {}, gpu id: {}", use_gpu_, gpu_id_);
}

Executor::Executor(size_t thread_nums, std::shared_ptr<ExecutorOption> executor_option)
    : thread_pool_(std::make_shared<base::PipeManager>(thread_nums)) {
    if (executor_option != nullptr) {
        executor_option_ = executor_option;
        // for (auto& thread : thread_pool_->GetThreads()) {
        //     executor_option_->open();
        // }
    }
}

Executor::~Executor() {
    Stop();
}

void Executor::Stop() {
    thread_pool_->Stop();
}

void Executor::AddTask(const std::function<void()>& t) {
    thread_pool_->Commit(t);
}
} // namespace flow
