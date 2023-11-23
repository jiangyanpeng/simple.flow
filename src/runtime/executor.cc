// #include <graphflow/base/matrix_log.h>
// #include <graphflow/runtime/executor.h>

// namespace pipe {

// void ExecutorOption::open() {
//     MATRIX_LOG_INFO("use gpu: {}, gpu id: {}", use_gpu_, gpu_id_);
// }

// matrix::Executor::Executor(size_t thread_nums, std::shared_ptr<ExecutorOption> executor_option)
//     : thread_pool_(std::make_shared<ThreadPool>(thread_nums)) {
//     if (executor_option != nullptr) {
//         executor_option_ = executor_option;
//         for (auto& thread : thread_pool_->GetThreads()) {
//             executor_option_->open();
//         }
//     }
// }

// matrix::Executor::~Executor() {
//     Stop();
// }

// void matrix::Executor::Stop() {
//     thread_pool_->Stop();
// }

// void Executor::AddTask(const std::function<void()>& t) {
//     thread_pool_->enqueue(t);
// }
// } // namespace pipe
