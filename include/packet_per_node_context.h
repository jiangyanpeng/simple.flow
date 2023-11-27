#ifndef ARCHITECT_PACKET_PER_NODE_CONTEXT_H
#define ARCHITECT_PACKET_PER_NODE_CONTEXT_H

#include "device/stream.h"
#include "inout.h"
#include "inoutput_handler.h"
#include "input_source_context.h"
#include <vector>

namespace flow {
class InputSourceContext;
class PacketContext;

class Notifier;

class Node;

/**
 * Packet参与某个Node运算时，所持有的信息
 * CAUTION： input_ports_, output_ports_ 内存放的是PackageGroup。
 * 为了支持Node的Batch能力，Packet经历Node时，可产生多个结果。这一组Inout为Packet经历这个Node的所有数据。
 */

class PacketPerNodeContext : public std::enable_shared_from_this<PacketPerNodeContext> {
public:
    PacketPerNodeContext();

    ~PacketPerNodeContext();

    bool IsInputReady();

    bool IsOutputReady();

    void SetPacketContext(std::shared_ptr<PacketContext> pkt_ctx);

    void SetInOutputHandler(const std::shared_ptr<InOutputHandler>& input_handler);

    std::shared_ptr<PacketContext> GetPacketContext() const;

    void SetNode(std::weak_ptr<Node> node);

    int64_t GetUniqueId() const;

    void SetUniqueId(int64_t id);

    std::shared_ptr<Node> GetNode() const;

    std::vector<size_t> GetNodeIdsByName();

    void PropagateOutputPortTo(std::shared_ptr<PacketPerNodeContext>& dst,
                               size_t dst_port_id,
                               size_t src_port_id);

    void DoDebug();

    const std::vector<PackageGroup>& GetInputs() const;

    const std::vector<PackageGroup>& GetOutputs() const;

    void SetStream(std::shared_ptr<Stream> stream);

    const std::shared_ptr<Stream>& GetStream() const;

    void SetRefPackageShape(const std::shared_ptr<PackageShape>& shape);

    std::shared_ptr<InputSourceContext> GetInputSourceContext() const;

    void SetNotifier(std::shared_ptr<Notifier> notifier);

    std::shared_ptr<Notifier> GetNotifier() const;

    bool Notify() const;

public:
    //        struct NodeInout {
    //            std::vector<std::shared_ptr<Package>> input;
    //            std::vector<std::shared_ptr<Package>> output;
    //        };
    //        // 使用vector，主要希望解决batch能力，一个Packet可以产生多个Package，提前下行
    //        std::vector<NodeInout> inouts_;
    // input_ports_.size 由Node的描述决定，表示有多少个输入Port
    std::vector<PackageGroup> inputs_;
    // output_ports_.size 由Node的描述决定，表示有多少个输出Port
    std::vector<PackageGroup> outputs_;

    std::weak_ptr<PacketContext> packet_ctx_;
    std::weak_ptr<Node> node_;
    std::weak_ptr<InOutputHandler> inoutput_handler_;
    std::shared_ptr<Stream> stream_;

    std::shared_ptr<PackageShape> ref_shape_;
    int64_t unique_id_{0};

    std::shared_ptr<Notifier> notifier_;
};

using PacketPerNodeContextPtr = std::shared_ptr<PacketPerNodeContext>;

} // namespace flow


#endif // ARCHITECT_PACKET_PER_NODE_CONTEXT_H
