#ifndef SIMPLE_FLOW_GRAPH_H_
#define SIMPLE_FLOW_GRAPH_H_

#include "core/collection.h"
#include "calculator/calculator.h"
#include "calculator/calculator_registry.h"
#include "device/device_registry.h"
#include "graph_view.h"
#include "stream/input_stream_manager.h"
#include "node.h"
#include "packet.h"
#include "runtime/scheduler_async.h"
#include "runtime/scheduler_one_thread.h"
#include "spec/graph_spec.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace flow {
class ExectorOption;
class Executor;

class WaitingThread;

using AsyncResultFunc =
    std::function<void(uint8_t source_ctx, uint64_t pkt_id, std::vector<PackageGroup>& result)>;
using AsyncResultFullFunc = std::function<void(std::shared_ptr<InputStreamContext>& src_ctx,
                                               std::shared_ptr<InputPktId> pkt_id,
                                               std::vector<PackageGroup>& result)>;

class Graph;

enum GRAPH_SCHEDULE_POLICY {
    // 非阻塞方式，业务将Packet添加至图后即返回，Graph自行执行，并回调处理完成函数。调度和executor使用单线程
    GRAPH_SCH_ASYNC_ONE_THREAD = 0,

    // TODO: 每路视频使用独立的scheduler，scheduler保序
    GRAPH_SCH_ASYNC_ONE_THREAD_PER_INPUT_SOURCE = 1,

    // 纯异步，不保序
    GRAPH_SCH_ASYNC_NON_ORDER_PRESERVING = 2,

    // 异步，保序
    GRAPH_SCH_ASYNC_ORDER_PRESERVING = 3,
};

class GraphHelper {
public:
    explicit GraphHelper(std::shared_ptr<GraphSpec> spec) : graph_spec_(std::move(spec)) {}

    ~GraphHelper() {}

    std::shared_ptr<InputStreamContext> CreateInputStreamContext();

    /**
     * @deprecated 废弃
     * @return
     */
    std::shared_ptr<Packet> CreatePacket();

    /**
     * 同步调用，处理一个pkt，阻塞返回一个执行结果。不推荐。
     * @param pkt
     * @return
     * @pre 1. 初始化图指定同步模式
     *      2. Elementary实现无状态
     */
    std::vector<PackageGroup> ProcessPacket(const std::shared_ptr<Packet>& pkt);

    /**
     * 向图中添加一个Packet
     * @deprecated
     */
    Status AddPacket(const std::shared_ptr<Packet>& pkt);


    /**
     * 向图添加回调函数，该回调函数在一个Packet执行完成时被回调
     * @deprecated
     * @param funct
     */
    void AddCallFunct(const AsyncResultFunc& funct);

    /**
     * 向图添加回调函数，该回调函数在一个Packet执行完成时被回调
     * @param funct
     */
    void AddCallFunct(const AsyncResultFullFunc& funct);

    /**
     * 异步添加Packet
     * @param pkt
     * @return
     */
    Status AddPacketAsync(const std::shared_ptr<Packet>& pkt);

    bool IsPacketProcessed(size_t pkt_id);

    // 查询pkt_id执行结果，如果执行完成则返回结果，否则返回空
    std::vector<PackageGroup> GetResult(size_t pkt_id);

    void Start();

    void Stop();

    void Initialize(const std::shared_ptr<CalculatorRegistry>& registry,
                    const std::shared_ptr<DeviceRegistry>& device_registry,
                    GRAPH_SCHEDULE_POLICY policy                    = GRAPH_SCH_ASYNC_ONE_THREAD,
                    std::shared_ptr<ExecutorOption> executor_option = nullptr);


    void AddSkipPacket(std::shared_ptr<InputStreamContext>& source,
                       const std::shared_ptr<Packet>& pkt);

private:
    std::shared_ptr<PacketContext> CreatePacketContext(const std::shared_ptr<Packet>& pkt);

private:
    std::shared_ptr<GraphSpec> graph_spec_;
    std::shared_ptr<Graph> graph_;
    static std::atomic_int_fast64_t packet_id_;
    std::shared_ptr<InputStreamManager> input_stream_manager_;
    std::mutex mutex_;
};

} // namespace flow
#endif // SIMPLE_FLOW_GRAPH_H_
