#include <iostream>

#include "device/matrix_device_registry.h"
#include "graph.h"
#include "inout.h"
#include "matrix_elementary_registry.h"
#include "specs/graph_spec.h"
#include <log.h>

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
    std::shared_ptr<MatrixElementaryRegistry> registry;
    std::shared_ptr<MatrixDeviceRegistry> device_registry;
};



std::shared_ptr<GraphHelper> GraphComplier::GetGraph() const {
    return helper_;
}

void GraphComplier::Init(const std::string& cur_path) {

    GraphInit(cur_path);

    helper_ = std::make_shared<GraphHelper>(spec);

    registry        = std::make_shared<MatrixElementaryRegistry>();
    device_registry = std::make_shared<MatrixDeviceRegistry>();

    registry->RegisterElem<Elementary, ElementaryOption, InOutputHandler>("matrix.Elementary",
                                                                          "cpu");


    // registry->RegisterElem<ElemFakeGraphComplier, ElemFakeGraphComplierOption, InputHandler>(
    //     "matrix::ElemFakeGraphComplier", "cpu");



    // device_registry->RegisterDevice<DeviceCPU>("CPU");
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
    helper->Initialize(gc_->registry, gc_->device_registry, GRAPH_SCH_ASYNC_ORDER_PRESERVING);
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

        SIMPLE_LOG_INFO("add packet");
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
