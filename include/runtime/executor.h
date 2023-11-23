#ifndef SIMPLE_PIPE_EXECUTOR_H_
#define SIMPLE_PIPE_EXECUTOR_H_

#include <functional>
#include <memory>
#include <string>

namespace pipe {

class ExecutorOption {
public:
    ExecutorOption() : use_gpu_(false), gpu_id_(0) {}
    ExecutorOption(bool use_gpu, int gpu_id) : use_gpu_(use_gpu), gpu_id_(gpu_id) {}
    virtual ~ExecutorOption() {}

    virtual void open();

protected:
    bool use_gpu_{false};
    int gpu_id_{0};
    int id_{0};
    std::string name_{};
};


class Executor {
public:
    explicit Executor(size_t thread_nums,
                      std::shared_ptr<ExecutorOption> executor_option = nullptr);
    ~Executor();

    void AddTask(const std::function<void()>& t);

    void Stop();

private:
    // std::shared_ptr<ThreadPool> thread_pool_;
    std::shared_ptr<ExecutorOption> executor_option_;
};

} // namespace pipe

#endif // SIMPLE_PIPE_EXECUTOR_H_