#ifndef SIMPLE_FLOW_PACKET_H_
#define SIMPLE_FLOW_PACKET_H_

#include "core/status.h"
#include "package.h"
#include "stream/input_stream_context.h"

#include <vector>

namespace flow {
class Package;

class InputPktId;

class InputStreamContext;

/**
 * 描述图处理的数据，是一组Package. 每个Package表示一个输入port的输入数据。
 * 主要是考虑图或子图支持多个输入
 */
class Packet final {
public:
    explicit Packet(std::shared_ptr<InputPktId> id);

    Packet(std::shared_ptr<InputPktId> id, size_t package_num);

    ~Packet() = default;

    Status AddPackage(size_t package_index, std::shared_ptr<Package> package);

    void SetStreamContext(const std::shared_ptr<InputStreamContext>& ctx);

    std::shared_ptr<InputStreamContext> GetStreamContext() const;

    std::shared_ptr<Package> GetPackage(size_t index);

    uint64_t GetId() const;

    std::shared_ptr<InputPktId> GetInputPktId() const;

    uint8_t GetStreamId() const;

    size_t GetPackageNum() const;

private:
    std::vector<std::shared_ptr<Package>> package_;
    std::shared_ptr<InputPktId> id_;
    std::weak_ptr<InputStreamContext> stream_ctx_;
};

} // namespace flow
#endif // SIMPLE_FLOW_PACKET_H_
