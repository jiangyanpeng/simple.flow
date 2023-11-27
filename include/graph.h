#ifndef ARCHITECT_GRAPH_H
#define ARCHITECT_GRAPH_H

#include "base/collection.h"
#include "device/matrix_device_registry.h"
#include "elementary/elementary.h"
#include "graph_view.h"
#include "input_source_manager.h"
#include "matrix_elementary_registry.h"
#include "node.h"
#include "packet.h"
#include "scheduler_async.h"
#include "scheduler_one_thread.h"
#include "specs/graph_spec.h"
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
using AsyncResultFullFunc = std::function<void(std::shared_ptr<InputSourceContext>& src_ctx,
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
    explicit GraphHelper(std::shared_ptr<GraphSpec> spec);

    ~GraphHelper();

    std::shared_ptr<InputSourceContext> CreateInputSourceContext();

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

    void Initialize(const std::shared_ptr<MatrixElementaryRegistry>& registry,
                    const std::shared_ptr<MatrixDeviceRegistry>& device_registry,
                    GRAPH_SCHEDULE_POLICY policy                    = GRAPH_SCH_ASYNC_ONE_THREAD,
                    std::shared_ptr<ExecutorOption> executor_option = nullptr);


    void AddSkipPacket(std::shared_ptr<InputSourceContext>& source,
                       const std::shared_ptr<Packet>& pkt);

private:
    std::shared_ptr<PacketContext> CreatePacketContext(const std::shared_ptr<Packet>& pkt);

private:
    std::shared_ptr<GraphSpec> graph_spec_;
    std::shared_ptr<Graph> graph_;
    static std::atomic_int_fast64_t packet_id_;
    std::shared_ptr<InputSourceManager> input_source_manager_;
    std::mutex mutex_;
};


} // namespace flow


#endif // ARCHITECT_GRAPH_H
