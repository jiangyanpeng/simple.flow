#include "calculator/calculator.h"
#include "core/contract_coding.h"
#include <log.h>
#include <common.h>
namespace flow {

Status Calculator::GetContract(CalculatorContract* contract) {
    UNUSED_WARN(contract);
    return Status::OkStatus();
}

Status Calculator::Open(CalculatorContext* ctx) {
    UNUSED_WARN(ctx);
    SIMPLE_ASSERT(option_);
    return Status::OkStatus();
}

Status Calculator::Close(CalculatorContext* ctx) {
    UNUSED_WARN(ctx);
    return Status::OkStatus();
}

Status Calculator::Process(CalculatorContext* ctx) {
    SIMPLE_LOG_INFO("elem process. elem id: {}", GetId());

    {
        // 输入准备
        auto input0             = ctx->GetInputData(0);
        auto& input_datas       = input0->GetData();
        auto input_data_indexes = input0->GetDataIndex();

        // 输出数据准备
        auto out_pkg0 = std::make_shared<Package>();
        out_pkg0->SetShape(input0->GetShape().GetShapeVec());


        // 简单拷贝
        for (int64_t i = 0; i < input0->NumElements(); ++i) {
            out_pkg0->AddData(input_data_indexes[i], input_datas[i]);
        }

        // 输出
        ctx->AddOutputData(0, out_pkg0);
    }

    {
        // 输入准备
        auto input0             = ctx->GetInputData(1);
        auto& input_datas       = input0->GetData();
        auto input_data_indexes = input0->GetDataIndex();

        // 输出数据准备
        auto out_pkg0 = std::make_shared<Package>();
        out_pkg0->SetShape(input0->GetShape().GetShapeVec());


        auto s = DoProcess([&]() { return Status::OkStatus(); });

        // 简单拷贝
        for (int64_t i = 0; i < input0->NumElements(); ++i) {
            out_pkg0->AddData(input_data_indexes[i], input_datas[i]);
        }

        // 输出
        ctx->AddOutputData(1, out_pkg0);
    }
    return Status::OkStatus();
}

void Calculator::SetDevice(std::shared_ptr<Device> d) {
    device_ = std::move(d);
    SIMPLE_ASSERT(device_);
}

void Calculator::SetCalculatorOption(std::shared_ptr<CalculatorOption> option) {
    option_ = std::move(option);
}

size_t Calculator::GetId() const {
    return id_;
}

void Calculator::SetId(size_t id) {
    id_ = id;
}

std::shared_ptr<Device> Calculator::GetDevice() const {
    return device_;
}

Status Calculator::DoProcess(const std::function<Status()>& f) {
    return f();
}
} // namespace flow
