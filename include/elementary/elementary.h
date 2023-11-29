#ifndef SIMPLE_FLOW_ELEMENTARY_H_
#define SIMPLE_FLOW_ELEMENTARY_H_

#include "base/status.h"
#include "elementary/elementary_option.h"
#include "elementary_context.h"
#include "elementary_contract.h"
#include "inoutput_handler.h"

namespace flow {
class Device;

class Elementary {
public:
    Elementary() : id_(SIZE_MAX) {}
    virtual ~Elementary() {}
    
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

} // namespace flow
#endif // SIMPLE_FLOW_ELEMENTARY_H_
