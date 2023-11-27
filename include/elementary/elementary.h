#ifndef ARCHITECT_ELEMENTARY_H
#define ARCHITECT_ELEMENTARY_H

#include "base/status.h"
#include "elementary/elementary_option.h"
#include "elementary_context.h"
#include "elementary_contract.h"
#include "inoutput_handler.h"

namespace flow {
class Device;
/**
 * Elementary设计为可批量进行数据处理，每次处理（T_IN, T_OUT).
 * 处理的数据为T_INn，输入为vector<shared_ptr<T_INn>>
 * 输出的数据为T_OUTn, 输出为vector<shared_ptr<T_OUTn>>
 * 输入输出size相同。
 * 输出部分可能有的size为0，比如检测类，如果在图像中找不到目标，则Rect输出为空，但可指定found
 */
class Elementary {
public:
    Elementary();
    virtual ~Elementary();

    virtual Status GetContract(ElementaryContract* contract);

    virtual Status Open(ElementaryContext* ctx);

    virtual Status Close(ElementaryContext* ctx);

    virtual Status Process(ElementaryContext* ctx);


    Status DoProcess(const std::function<Status()>& f);

    void SetDevice(std::shared_ptr<Device> d);

    std::shared_ptr<Device> GetDevice() const;

    void SetElementaryOption(std::shared_ptr<ElementaryOption> option);

    size_t GetId() const;

    void SetId(size_t id);

protected:
    std::shared_ptr<ElementaryOption> option_;
    std::shared_ptr<Device> device_;
    size_t id_;
};

//    using Node = Elementary;

} // namespace flow


#endif // ARCHITECT_ELEMENTARY_H
