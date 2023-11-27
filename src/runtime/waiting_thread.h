

#ifndef ACTION_DETECTION_WAITING_THREAD_H
#define ACTION_DETECTION_WAITING_THREAD_H

#include "graph_node.h"
#include <condition_variable>
#include <manager/pipe_manager.h>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace flow {

class Notifier {
public:
    Notifier() = default;

    virtual ~Notifier() = default;

    virtual bool Notify(int64_t id) = 0;
};

class WaitingThread : public Notifier {
public:
    WaitingThread() = default;

    ~WaitingThread() override = default;

    void Start();

    void Stop();

    void SetNotifiedFunction(std::function<void(std::shared_ptr<PacketPerNodeContext>)> f);

    bool AddWatch(const std::shared_ptr<PacketPerNodeContext>& ctx);

    bool Notify(int64_t id) override;

private:
    std::unordered_map<int64_t, std::shared_ptr<PacketPerNodeContext>> map_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<int64_t> notified_id_;
    std::function<void(std::shared_ptr<PacketPerNodeContext>)> f_;

    base::PipeManager pool_{1};
    bool stop_{false};
};
} // namespace flow


#endif // ACTION_DETECTION_WAITING_THREAD_H
