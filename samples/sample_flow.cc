#include <iostream>

#include "calculator/calculator.h"
#include "calculator/calculator_option.h"
#include "calculator/calculator_registry.h"
#include "device/device_registry.h"
#include "graph.h"
#include "inout.h"
#include "spec/graph_spec.h"

#include <log.h>

namespace flow {
class ElemFakeGraphComplierOption : public CalculatorOption {
public:
    ~ElemFakeGraphComplierOption() override = default;
    std::vector<std::string> models_path_;
    virtual bool Parse(std::string& json_str);
};

bool ElemFakeGraphComplierOption::Parse(std::string& json_str) {
    // do somthing
    return true;
}

class ElemFakeGraphComplier : public Calculator {
public:
    ElemFakeGraphComplier();
    static constexpr const char* kElemName    = "ElemFakeGraphComplier";
    static constexpr const char* kInput       = "IN_IMAGE_VECTOR";
    static constexpr const char* kOutput      = "OUT_ACTION_DETECTION_RESULT";
    virtual ~ElemFakeGraphComplier() override = default;
    Status GetContract(CalculatorContract* contract) override;
    Status Open(CalculatorContext* ctx) override;
    Status Close(CalculatorContext* ctx) override;
    Status Process(CalculatorContext* ctx) override;

private:
    size_t input_image_index_{0};
    size_t reconginze_result_index{0};
    std::shared_ptr<ElemFakeGraphComplierOption> nn_option_;
};



ElemFakeGraphComplier::ElemFakeGraphComplier() {}

Status ElemFakeGraphComplier::GetContract(CalculatorContract* contract) {
    if (!contract) {
        return Status::InvalidArgument("elementary contract is null");
    }

    contract->AddInput(kInput, "::STImage")
        .AddOutput(kOutput, "action_result_t")
        .SetCalculatorName(kElemName)
        .SetCalculatorOptionName("ElemFakeGraphComplierOption")
        .AddSupportedDevice("CPU")
        .AddSupportedDevice("GPU_CUDA");

    return Status::OkStatus();
}

Status ElemFakeGraphComplier::Open(CalculatorContext* ctx) {
    SIMPLE_LOG_INFO("ElemFakeGraphComplier Open Start");
    if (!ctx) {
        return Status::InvalidArgument("elementary context is null");
    }

    input_image_index_ = ctx->GetInputIdWithTag(kInput);
    if (input_image_index_ < 0) {
        return Status(StatusCode::kNotFound, "input image of nn base not found");
    }

    reconginze_result_index = ctx->GetOutputIdWithTag(kOutput);
    if (reconginze_result_index < 0) {
        return Status(StatusCode::kNotFound, "output detection result of nn base not found");
    }
    nn_option_ = std::dynamic_pointer_cast<ElemFakeGraphComplierOption>(option_);

    SIMPLE_LOG_INFO("ElemFakeGraphComplier Open End");
    return Status::OkStatus();
}

Status ElemFakeGraphComplier::Close(CalculatorContext* ctx) {
    return Calculator::Close(ctx);
}

Status ElemFakeGraphComplier::Process(CalculatorContext* ctx) {
    SIMPLE_LOG_DEBUG("ElemFakeGraphComplier::Process Start");
    auto input_images_pkg   = ctx->GetInputData(input_image_index_);
    auto input_images_index = input_images_pkg->GetDataIndex();
    if (!input_images_pkg) {
        return Status(StatusCode::kNotFound, "get input tensor failed");
    }

    auto output_pkg = std::make_shared<Package>();
    output_pkg->SetShape(input_images_pkg->GetShape());

    SIMPLE_LOG_INFO("ElemFakeGraphComplier::Process Success ...");
    for (auto index : input_images_index) {
        SIMPLE_LOG_INFO("************ Hello simple.flow *************");
        auto result = std::make_shared<bool>(true);
        output_pkg->AddData(index, std::move(result));
    }
    ctx->AddOutputData(reconginze_result_index, output_pkg);
    SIMPLE_LOG_DEBUG("ElemFakeGraphComplier::Process End");
    return Status::OkStatus();
}
} // namespace flow

using namespace flow;
class GraphComplier {
public:
    GraphComplier() {}
    ~GraphComplier() {}

    void Init(const std::string& cur_path);
    std::shared_ptr<GraphHelper> GetGraph() const;

private:
    bool GraphInit(const std::string& cur_path);
    std::shared_ptr<GraphHelper> helper_;
    std::shared_ptr<GraphSpec> spec;

public:
    std::shared_ptr<CalculatorRegistry> registry;
    std::shared_ptr<DeviceRegistry> device_registry;
};



std::shared_ptr<GraphHelper> GraphComplier::GetGraph() const {
    return helper_;
}

void GraphComplier::Init(const std::string& cur_path) {

    GraphInit(cur_path);

    helper_ = std::make_shared<GraphHelper>(spec);

    registry        = std::make_shared<CalculatorRegistry>();
    device_registry = std::make_shared<DeviceRegistry>();

    registry->Register<Calculator, CalculatorOption, InOutHandler>("Calculator", "cpu");


    registry
        ->Register<flow::ElemFakeGraphComplier, flow::ElemFakeGraphComplierOption, InputHandler>(
            "ElemFakeGraphComplier", "cpu");

    device_registry->RegisterDevice<DeviceCPU>("CPU");
}

bool GraphComplier::GraphInit(const std::string& cur_path) {
    spec = std::make_shared<GraphSpec>();
    spec->SetName("Complier").SetElementaryType("").SetElementaryOptionType("");
    spec->option = nullptr;

    // graph input
    {
        InoutSpec in;
        in.name           = "graph_in_image";
        in.link_name      = "1";
        in.direction      = IN;
        in.data_type_name = "::STImage";
        spec->AddGraphInputSpec(in);
    }


    // graph output
    {
        InoutSpec o;
        o.name           = "graph_out_track_result";
        o.link_name      = "99";
        o.direction      = OUT;
        o.data_type_name = "action_result_t";
        spec->AddGraphOutputSpec(o);
    }



    // graph device
    {
        DeviceSpec device;
        device.SetType("CPU").SetName("CPU0");
        spec->AddDeviceSpec(device);
    }



    // graph nodes
    {
        auto node_spec1 = std::make_shared<NodeSpec>();
        node_spec1->SetName("fakeGraphComplier")
            .SetElementaryType("ElemFakeGraphComplier")
            .SetElementaryOptionType("ElemFakeGraphComplierOption");

        node_spec1->SetElementaryOptionJsonValue("");
        node_spec1->SetOrderPreserving(true);

        node_spec1->AddSkipPacketNode("crop");


        // input
        {
            InoutSpec in;
            in.name           = "IN_IMAGE_VECTOR";
            in.link_name      = "1";
            in.direction      = IN;
            in.data_type_name = "::STImage";
            node_spec1->AddInputSpec(in);
        }


        // output
        {
            InoutSpec out;
            out.name           = "OUT_ACTION_DETECTION_RESULT";
            out.link_name      = "99";
            out.direction      = OUT;
            out.data_type_name = "action_result_t";
            node_spec1->AddOutputSpec(out);
        }


        node_spec1->SetElementaryNum(1);
        ElemSpec elem_spec;
        elem_spec.SetDeviceName("CPU0");
        node_spec1->AddElemSpec(elem_spec);

        spec->AddNodeSpec(node_spec1);
    }


    return true;
}
int main() {
    auto gc_ = std::make_shared<GraphComplier>();
    gc_->Init("cur_path");

    auto helper = gc_->GetGraph();
    helper->Initialize(gc_->registry, gc_->device_registry, GRAPH_SCH_ASYNC_NON_ORDER_PRESERVING);
    helper->Start();

    {
        auto src_ctx = helper->CreateInputSourceContext();
        auto packet  = src_ctx->CreatePacket();

        auto cv_img = std::make_shared<std::vector<int>>();
        for (size_t i = 0; i < 10; i++) {
            cv_img->push_back(i);
        }

        auto img_pkg = std::make_shared<Package>();
        img_pkg->SetShape({1});
        img_pkg->AddData(0, cv_img);
        packet->AddPackage(0, img_pkg);

        helper->AddPacketAsync(packet);
    }

    static int wait = 0;
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        SIMPLE_LOG_INFO("waiting..... ");
        ++wait;
        if (wait > 3) {
            break;
        }
    } while (true);

    helper->Stop();
    return 0;
}
