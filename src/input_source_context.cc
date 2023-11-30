
#include "input_source_context.h"
#include "packet.h"

#include <log.h>

namespace flow {

InputSourceContext::InputSourceContext() = default;


uint8_t InputSourceContext::GetId() const {
    return source_id_;
}

void InputSourceContext::SetId(uint8_t id) {
    source_id_ = id;
}

bool InputSourceContext::AddSkipPktId(std::vector<size_t> node_ids, uint64_t pkt_id) {
    order_information_.AddSkipPktId(node_ids, pkt_id);
    return true;
}

std::shared_ptr<Packet> InputSourceContext::CreatePacket() {
    SIMPLE_ASSERT(input_count_ != 0);
    auto id = std::make_shared<InputPktId>(source_id_, packet_id_.fetch_add(1));
    std::shared_ptr<Packet> packet = std::make_shared<Packet>(id, input_count_);
    packet->SetSourceContext(shared_from_this());
    return packet;
}

void InputSourceContext::SetInputCount(size_t count) {
    input_count_ = count;
}

InputPktId::InputPktId(uint8_t src_id, uint64_t pkt_id) {
    SIMPLE_ASSERT(src_id != kInvInputSourceId);
    SIMPLE_ASSERT(pkt_id < kInvPktId);
    id_ = src_id;
    id_ <<= kShift;
    id_ += pkt_id;
}

bool InputPktId::IsValid() const {
    if (InputSourceId() == kInvInputSourceId) {
        return false;
    }
    if (PacketId() == kInvPktId) {
        return false;
    }
    return true;
}


OrderInformation::OrderInformation() {}

OrderInformation::~OrderInformation() {}

bool OrderInformation::IsEmpty() {
    std::unique_lock<std::mutex> lk(mtx_);
    return node_id_2_pkt_id_2_ctx_.empty();
}

bool OrderInformation::GetNextPktId(size_t node_id, uint64_t& pkt_id) {
    std::unique_lock<std::mutex> lk(mtx_);
    if (node_id_2_next_pkt_id_.find(node_id) != node_id_2_next_pkt_id_.end()) {
        pkt_id = node_id_2_next_pkt_id_[node_id];


        if (node_id_2_skip_pkt_id_.find(node_id) != node_id_2_skip_pkt_id_.end()) {
            // 如果pkt_id是4, node_id_2_skip_pkt_id_存在4,5,6,7，9，则pkt_id返回8
            while (node_id_2_skip_pkt_id_[node_id].count(pkt_id) > 0) {
                node_id_2_skip_pkt_id_[node_id].erase(pkt_id);
                pkt_id++;
            }
            // 更新内存里的next_pkt_id
            node_id_2_next_pkt_id_[node_id] = pkt_id;
        }
    } else {
        //假定从0开始，如果pkt为0就被过虑掉了，只能等下个packet来了再处理
        //只要有pkt到达这里，就能正常工作
        pkt_id                          = 0;
        node_id_2_next_pkt_id_[node_id] = pkt_id;
    }

    return true;
}

bool OrderInformation::SetNextPktId(size_t node_id, uint64_t pkt_id) {
    std::unique_lock<std::mutex> lk(mtx_);
    if (node_id_2_skip_pkt_id_.find(node_id) != node_id_2_skip_pkt_id_.end()) {
        // 如果pkt_id是4, node_id_2_skip_pkt_id_存在4,5,6,7，9，则设置pkt_id 为8
        while (node_id_2_skip_pkt_id_[node_id].count(pkt_id) > 0) {
            node_id_2_skip_pkt_id_[node_id].erase(pkt_id);
            pkt_id++;
        }
    }

    node_id_2_next_pkt_id_[node_id] = pkt_id;
    return true;
}

bool OrderInformation::AddUnprocessedPkt(size_t node_id,
                                         uint64_t pkt_id,
                                         std::shared_ptr<PacketPerNodeContext> per_node_ctx) {
    std::unique_lock<std::mutex> lk(mtx_);
    if (node_id_2_pkt_id_.find(node_id) != node_id_2_pkt_id_.end()) {
        node_id_2_pkt_id_[node_id].push(pkt_id);

        node_id_2_pkt_id_2_ctx_[node_id][pkt_id] = per_node_ctx;
    } else {
        std::priority_queue<uint64_t, std::vector<uint64_t>, std::greater<uint64_t>> q;
        q.push(pkt_id);
        node_id_2_pkt_id_[node_id] = q;

        std::map<uint64_t, std::shared_ptr<PacketPerNodeContext>> m;
        m[pkt_id]                        = per_node_ctx;
        node_id_2_pkt_id_2_ctx_[node_id] = m;
    }

    return true;
}

bool OrderInformation::GetSmallestUnprocessedPkt(
    size_t node_id,
    uint64_t& pkt_id,
    std::shared_ptr<PacketPerNodeContext>& per_node_ctx) {
    std::unique_lock<std::mutex> lk(mtx_);
    if (node_id_2_pkt_id_.find(node_id) != node_id_2_pkt_id_.end() &&
        node_id_2_next_pkt_id_.find(node_id) != node_id_2_next_pkt_id_.end()) {
        if (node_id_2_pkt_id_[node_id].empty()) {
            return false;
        }
        pkt_id          = node_id_2_pkt_id_[node_id].top();
        int next_pkt_id = node_id_2_next_pkt_id_[node_id];

        if (int(pkt_id) != next_pkt_id) {
            return false;
        }

        // 处理完这个per_node_ctx后需要更新node_id_2_next_pkt_id结构体　
        per_node_ctx = node_id_2_pkt_id_2_ctx_[node_id][pkt_id];


        // 更新
        node_id_2_pkt_id_[node_id].pop();
        node_id_2_pkt_id_2_ctx_[node_id].erase(pkt_id);

        return true;
    }
    return false;
}

bool OrderInformation::AddSkipPktId(std::vector<size_t> node_ids, uint64_t pkt_id) {
    std::unique_lock<std::mutex> lk(mtx_);
    for (auto& node_id : node_ids) {
        if (node_id_2_skip_pkt_id_.count(node_id) <= 0) {
            std::set<uint64_t> s;
            s.insert(pkt_id);
            node_id_2_skip_pkt_id_[node_id] = std::move(s);
        } else {
            if (node_id_2_skip_pkt_id_[node_id].count(pkt_id) <= 0 &&
                node_id_2_skip_pkt_id_[node_id].size() < skip_pkt_max_size_) {
                node_id_2_skip_pkt_id_[node_id].insert(pkt_id);
            }
        }
        SIMPLE_LOG_DEBUG(
            "OrderInformation::AddSkipPktId, node_id: {}, pkt_id {}, pkt_size_for_node_id {}",
            node_id,
            pkt_id,
            node_id_2_skip_pkt_id_[node_id].size());
    }
    return true;
}

} // namespace flow
