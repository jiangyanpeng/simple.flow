#ifndef SIMPLE_FLOW_TAGGED_OBJECT_H_
#define SIMPLE_FLOW_TAGGED_OBJECT_H_

#include <climits>
#include <cstdint>
#include <string>

namespace flow {
class TaggedObject {
public:
    TaggedObject()          = default;
    virtual ~TaggedObject() = default;

    TaggedObject& SetName(std::string name);
    TaggedObject& SetId(size_t id);

    std::string Name() const;
    size_t Id() const;

private:
    std::string name_{};
    size_t id_{SIZE_MAX};
};

} // namespace flow
#endif // SIMPLE_FLOW_TAGGED_OBJECT_H_
