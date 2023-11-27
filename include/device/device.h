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
} // namespace flow


#endif // ARCHITECT_DEVICE_H
