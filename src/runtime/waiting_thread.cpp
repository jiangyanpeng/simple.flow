
// #include "waiting_thread.h"

// #include <utility>

// namespace flow {

// void WaitingThread::SetNotifiedFunction(
//     std::function<void(std::shared_ptr<PacketPerNodeContext>)> f) {
//     f_ = std::move(f);
// }

// bool WaitingThread::AddWatch(const std::shared_ptr<PacketPerNodeContext>& ctx) {
//     std::unique_lock<std::mutex> lk(mutex_);
//     int64_t id = ctx->GetUniqueId();
//     SIMPLE_LOG_INFO("add watch : {}", id);
//     if (map_.end() != map_.find(id)) {
//         SIMPLE_LOG_WARN("ctx {} is in the map", id);
//         return true;
//     }
//     map_[id] = ctx;
//     return true;
// }

// bool WaitingThread::Notify(int64_t id) {
//     std::unique_lock<std::mutex> lk(mutex_);
//     notified_id_.push_back(id);
//     cv_.notify_one();
//     return true;
// }

// void WaitingThread::Start() {
//     pool_.enqueue([&]() {
//         do {
//             std::unique_lock<std::mutex> lk(mutex_);
//             if (stop_) {
//                 return;
//             }
//             auto w = cv_.wait_for(lk, std::chrono::milliseconds(100));
//             if (w == std::cv_status::no_timeout) {
//                 // 每个激活的id，均执行f_
//                 for (auto id : notified_id_) {
//                     auto ctx = map_[id];
//                     map_.erase(id);
//                     MATRIX_ASSERT(ctx);
//                     // add to scheduler
//                     f_(ctx);
//                 }
//                 notified_id_.clear();
//             }
//         } while (true);
//     });
// }

// void WaitingThread::Stop() {
//     stop_ = true;
// }
// } // namespace flow