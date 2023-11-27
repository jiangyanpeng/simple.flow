#include "elementary/elementary.h"
#include "base/contract_coding.h"
#include <log.h>

namespace flow {

Status Elementary::GetContract(ElementaryContract* contract) {
    return Status::OkStatus();
}

Status Elementary::Open(ElementaryContext* ctx) {
    MATRIX_ASSERT(option_);
    SIMPLE_LOG_TRACE("module-base-elem : [{}] opened.", GetId());
    return Status::OkStatus();
}

Status Elementary::Close(ElementaryContext* ctx) {
    SIMPLE_LOG_TRACE("module-base-elem : [{}] closed.", GetId());
    return Status::OkStatus();
}

Status Elementary::Process(ElementaryContext* ctx) {
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
        for (size_t i = 0; i < input0->NumElements(); ++i) {
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
        for (size_t i = 0; i < input0->NumElements(); ++i) {
            out_pkg0->AddData(input_data_indexes[i], input_datas[i]);
        }

        // 输出
        ctx->AddOutputData(1, out_pkg0);
    }
    return Status::OkStatus();
}

void Elementary::SetDevice(std::shared_ptr<Device> d) {
    device_ = std::move(d);
    MATRIX_ASSERT(device_);
}

void Elementary::SetElementaryOption(std::shared_ptr<ElementaryOption> option) {
    option_ = std::move(option);
}

size_t Elementary::GetId() const {
    return id_;
}

void Elementary::SetId(size_t id) {
    id_ = id;
}

std::shared_ptr<Device> Elementary::GetDevice() const {
    return device_;
}

Status Elementary::DoProcess(const std::function<Status()>& f) {
    return f();
}
} // namespace flow
