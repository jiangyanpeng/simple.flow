#ifndef SIMPLE_FLOW_INPUT_STREAM_CONTEXT_H_
#define SIMPLE_FLOW_INPUT_STREAM_CONTEXT_H_

#include "core/collection.h"
#include "core/status.h"
#include "packet.h"
#include "packet_context.h"
#include "packet_per_node_context.h"

#include <algorithm>
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <vector>

namespace flow {
/**
 * 某个Input的Packet Id
 * 共64位
 * 其中高8位为InputSource Id，有效值为0 ~ 254
 * 其中低56位为Packet Id, 有效值0 ~ 2^56-2
 */
class InputPktId final {
private:
    static constexpr uint8_t kInvInputSourceId = 255;
    static constexpr uint64_t kInvPktId        = ((uint64_t)1 << 56) - 1;
    static constexpr uint8_t kShift            = 56;

public:
    explicit InputPktId(uint8_t src_id, uint64_t pkt_id);

    uint64_t Id() const { return id_; };

    bool IsValid() const;

    inline uint8_t InputSourceId() const { return id_ >> kShift; }

    inline uint64_t PacketId() const { return id_ & kInvPktId; }

private:
    uint64_t id_{UINT64_MAX};
};

class PacketPerNodeContext;
class OrderInformation final {
public:
    OrderInformation();

    ~OrderInformation();

    bool IsEmpty();

    bool GetNextPktId(size_t node_id, uint64_t& pkt_id);

    bool SetNextPktId(size_t node_id, uint64_t pkt_id);

    bool AddUnprocessedPkt(size_t node_id,
                           uint64_t pkt_id,
                           std::shared_ptr<PacketPerNodeContext> per_node_ctx);

    bool GetSmallestUnprocessedPkt(size_t node_id,
                                   uint64_t& pkt_id,
                                   std::shared_ptr<PacketPerNodeContext>& per_node_ctx);

    bool AddSkipPktId(std::vector<size_t> node_ids, uint64_t pkt_id);

private:
    std::mutex mtx_;

    //　每一个node对应一个最小堆
    std::map<size_t, std::priority_queue<uint64_t, std::vector<uint64_t>, std::greater<uint64_t>>>
        node_id_2_pkt_id_;

    // 每一个node对应一个nextPktId
    std::map<size_t, int> node_id_2_next_pkt_id_;

    // 每一个node对应一个map， map中每一个pktId对应一个per_node_ctx;
    std::map<size_t, std::map<uint64_t, std::shared_ptr<PacketPerNodeContext>>>
        node_id_2_pkt_id_2_ctx_;

    // 每一个node对应一个set，set中每一个id对应一个需要skip的id.
    std::map<size_t, std::set<uint64_t>> node_id_2_skip_pkt_id_;

    // 保护机制：防止node设置了保序，但没有调用保序的scheduler，导致skip_pkt数据量过大。
    uint64_t skip_pkt_max_size_{2048};
};

class Packet;

class InputStreamContext : public std::enable_shared_from_this<InputStreamContext> {
public:
    InputStreamContext();

    virtual ~InputStreamContext() = default;

    virtual std::shared_ptr<Packet> CreatePacket();

    virtual Status AddData(const std::string& tag, std::shared_ptr<void> d, size_t& id) = 0;

    virtual std::shared_ptr<void> GetData(const std::string& tag) = 0;

    virtual std::shared_ptr<void> GetData(size_t id) = 0;

    virtual bool HasData(const std::string& tag) const = 0;

    virtual bool HasData(size_t id) const = 0;

    virtual Status DeleteData(const std::string& tag) = 0;

    void SetId(uint8_t id);

    uint8_t GetId() const;

    void SetInputCount(size_t count);

    bool AddSkipPktId(std::vector<size_t> node_ids, uint64_t pkt_id);

public:
    OrderInformation order_information_;

protected:
    uint8_t source_id_{0};
    size_t input_count_{0};
    std::atomic_int_fast64_t packet_id_{0};
};

} // namespace flow
#endif // SIMPLE_FLOW_INPUT_STREAM_CONTEXT_H_
