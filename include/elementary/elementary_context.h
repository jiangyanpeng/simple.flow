#ifndef ARCHITECT_ELEMENTARY_CONTEXT_H
#define ARCHITECT_ELEMENTARY_CONTEXT_H

#include "base/collection.h"
#include "device/stream.h"
#include "inout.h"
#include <vector>

namespace flow {

class InoutPort;

class Node;

class PacketPerNodeContext;

class InputSourceContext;

class ElementaryContext {
public:
    ElementaryContext(std::vector<std::shared_ptr<Package>>& inputs_data,
                      std::vector<std::shared_ptr<Package>>& outputs_data,
                      std::shared_ptr<Node> node,
                      const std::shared_ptr<Stream>& stream);

    ~ElementaryContext();

    size_t GetInputIdWithTag(const std::string& tag);

    size_t GetOutputIdWithTag(const std::string& tag);

    std::shared_ptr<Package> GetInputData(const std::string& tag);

    std::shared_ptr<Package> GetOutputData(const std::string& tag);

    bool IsInputOptional(const std::string& tag);

    bool IsInputOptional(size_t id);

    std::shared_ptr<Package> GetInputData(size_t id);

    std::shared_ptr<Package> GetOutputData(size_t id);

    void AddOutputData(const std::string& tag, const std::shared_ptr<Package>& pkg);

    void AddOutputData(size_t id, const std::shared_ptr<Package>& pkg);

    std::shared_ptr<Stream> GetStream() const;

    void SetPacketPerNodeContext(const std::shared_ptr<PacketPerNodeContext>& ctx);

    std::shared_ptr<PacketPerNodeContext> GetPacketPerNodeContext() const;

    void SetPacketPerNodeRefShape(const std::shared_ptr<PackageShape>& shape);

    std::shared_ptr<InputSourceContext> GetInputSourceContext() const;

    void AddSkipNode();

public:
    // size 为输入输出数量
    std::vector<std::shared_ptr<Package>>& inputs_data_;
    std::vector<std::shared_ptr<Package>>& outputs_data_;

private:
    std::shared_ptr<Node> node_;
    const std::shared_ptr<Stream>& stream_;
    std::weak_ptr<PacketPerNodeContext> pernode_ctx_;
};

} // namespace flow


#endif // ARCHITECT_ELEMENTARY_CONTEXT_H
