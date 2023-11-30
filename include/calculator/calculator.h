#ifndef SIMPLE_FLOW_CALCULATOR_H_
#define SIMPLE_FLOW_CALCULATOR_H_

#include "core/status.h"
#include "calculator/calculator_context.h"
#include "calculator/calculator_contract.h"
#include "calculator/calculator_option.h"
#include "inoutput_handler.h"

namespace flow {
class Device;

class Calculator {
public:
    Calculator() : id_(SIZE_MAX) {}
    virtual ~Calculator() {}

    virtual Status GetContract(CalculatorContract* contract);

    virtual Status Open(CalculatorContext* ctx);

    virtual Status Close(CalculatorContext* ctx);

    virtual Status Process(CalculatorContext* ctx);


    Status DoProcess(const std::function<Status()>& f);

    void SetDevice(std::shared_ptr<Device> d);

    std::shared_ptr<Device> GetDevice() const;

    void SetCalculatorOption(std::shared_ptr<CalculatorOption> option);

    size_t GetId() const;

    void SetId(size_t id);

protected:
    std::shared_ptr<CalculatorOption> option_;
    std::shared_ptr<Device> device_;
    size_t id_;
};

} // namespace flow
#endif // SIMPLE_FLOW_CALCULATOR_H_
