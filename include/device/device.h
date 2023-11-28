#ifndef ARCHITECT_DEVICE_H
#define ARCHITECT_DEVICE_H

#include "base/status.h"
#include "base/tagged_object.h"
#include "device/stream.h"
#include <functional>
#include <manager/pipe_manager.h>
#include <map>
#include <memory>
namespace flow {
using DoneCallback = std::function<void()>;
using Function     = std::function<void()>;

// FIXME: Device的一些参数如何指定，可以考虑在描述文件中增加部分配置信息
class Device : public TaggedObject {
public:
    Device() = default;

    ~Device() override = default;


public:
    virtual Status Initialize();

    virtual void SetThreadPool(const std::shared_ptr<base::PipeManager>& pool) = 0;

    virtual void ComputeSync(Function f) = 0;

    virtual void ComputeAsync(Function f, DoneCallback done) = 0;

    virtual void ComputeAsync(Function f) = 0;

    virtual void Sync(std::shared_ptr<Stream> stream) = 0;

    virtual void Sync() = 0;

    // virtual std::shared_ptr<DataMgrBase> GetAllocator() const;

    virtual std::shared_ptr<Stream> GetStream() const;

private:
    // device 包含了计算资源和内存资源
};

class DeviceCPU : public Device {
public:
    DeviceCPU() = default;

    ~DeviceCPU() override = default;

public:
    void SetThreadPool(const std::shared_ptr<base::PipeManager>& pool) override;

    void ComputeSync(Function f) override;

    void ComputeAsync(Function f, DoneCallback done) override;

    void ComputeAsync(Function f) override;

    void Sync(std::shared_ptr<Stream> stream) override;

    void Sync() override;

    Status Initialize() override;

    // F 为函数，参数为Args
    // F(Args...)
    //        template<class F, class... Args>
    //        void ComputeSync(F &&f, Args &&... args);
    //
    //        template<class F, class... Args>
    //        void ComputeAsync(F &&f, Args &&... args, const DoneCallback& done);


    // std::shared_ptr<DataMgrBase> GetAllocator() const override;


protected:
    // 计算资源：线程池
    std::shared_ptr<base::PipeManager> thread_pool_;

    // 内存资源：分配器
    //        std::shared_ptr<Allocator> allocator_;
};

//    template<class F, class... Args>
//    void DeviceCPU::ComputeSync(F &&f, Args &&... args) {
//        auto result = thread_pool_->enqueue(std::forward<F>(f), std::forward<Args>(args)...);
//        result.wait();
//    }
//
//    template<class F, class... Args>
//    void DeviceCPU::ComputeAsync(F &&f, Args &&... args, const DoneCallback& done) {
//        thread_pool_->enqueue([=](){
//            f(args...);
//            done();
//        });
//    }
} // namespace flow


#endif // ARCHITECT_DEVICE_H
