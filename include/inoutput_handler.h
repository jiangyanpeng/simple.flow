#ifndef ARCHITECT_INPUT_OUTPUT_HANDLER_H
#define ARCHITECT_INPUT_OUTPUT_HANDLER_H

#include <memory>

namespace flow {


class PacketPerNodeContext;
class Node;
class InOutputHandler {
public:
    InOutputHandler()  = default;
    ~InOutputHandler() = default;

    // deprecated
    bool IsReady(const std::shared_ptr<PacketPerNodeContext>& ctx);
    bool IsInputReady(const std::shared_ptr<PacketPerNodeContext>& ctx);
    bool IsOutputReady(const std::shared_ptr<PacketPerNodeContext>& ctx);

    void SetNode(const std::shared_ptr<Node>& node);

protected:
    std::weak_ptr<Node> node_;
};

using InputHandler = InOutputHandler;

} // namespace flow

#endif // ARCHITECT_INPUT_OUTPUT_HANDLER_H
