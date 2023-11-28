#ifndef SIMPLE_FLOW_ELEMENTARY_OPTION_H_
#define SIMPLE_FLOW_ELEMENTARY_OPTION_H_

#include <string>

namespace flow {
/**
 * Elementary固定配置，每次执行过程中不会发生变化
 * 编写新的Elementary时，需继承该对象
 */
class ElementaryOption {
public:
    ElementaryOption() {}
    virtual ~ElementaryOption() {}
    bool Parse(std::string& json_str);
};

} // namespace flow
#endif // SIMPLE_FLOW_ELEMENTARY_OPTION_H_
