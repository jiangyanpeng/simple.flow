
#include "inout_handler.h"
#include "node.h"
#include "packet_context.h"
#include "packet_per_node_context.h"

#include <log.h>


namespace flow {
bool InOutHandler::IsReady(const std::shared_ptr<PacketPerNodeContext>& ctx) {
    return IsInputReady(ctx);
}

void InOutHandler::SetNode(const std::shared_ptr<Node>& node) {
    node_ = node;
}

bool InOutHandler::IsOutputReady(const std::shared_ptr<PacketPerNodeContext>& ctx) {
    auto outputs = ctx->GetOutputs();
    auto node    = node_.lock();
    SIMPLE_ASSERT(node);
    // if (node->GetId() == 3 && ctx->GetPacketContext()->id_) {
    //     std::cout << "wait" << std::endl;
    // }
    // for (auto& p : node->output_ports_) {
    //     if (outputs[p->GetId()].empty()) {
    //         return false;
    //     }
    // }
    // return true;


    /*
    // 原来的逻辑是：若有一个为empty，则返回false;
    // 现在的逻辑是：若有一个不为empry，则返回true;

    // 下面证明为何现在的逻辑是符合预期的
        // 问题：在一个elementary内存在多个输出端时
        // step1: elementary的所有的输出端的数据
    拷贝都是在elementary的process函数里完成的，即可以认为所有的输出端是同时输出的
        // step2: 在elementary的process函数处理完毕后，会转到node层次判断输出是否ready，
        // step3: 如果output ready了，把输出端口的数据拷贝到对应node的输入端口，
        // 对于step3，output不ready,一般发生在merge节点，一般是只有一个输入端口，数据维度匹配不上
        // step4: 如果某个输出端口的数据为空，则转移时实际什么都没有做，对应的node的输入端口仍然为空
        // step5: 添加packerpernodecontext执行队列
        // step6:
    检查这个packerpernodecontext的所有输入端是否ready，由前一个node的空的输出端转移过来的输入端，没有数据，不ready，
                  由前一个node的非空的输出端转移过来的输入端，是有数据的.
        // step7: 输入端ready则继续，不ready则退出
        // step8:
    输入ready后调用node的process函数，node的process函数调用对应elementary的process函数，重复step1
    */

    for (auto& p : node->output_ports_) {
        if (!outputs[p->GetId()].empty()) {
            return true;
        }
    }
    return false;
}

bool InOutHandler::IsInputReady(const std::shared_ptr<PacketPerNodeContext>& ctx) {
    auto inputs = ctx->GetInputs();
    bool ready  = true;
    auto node   = node_.lock();
    for (auto& p : node->input_ports_) {
        // 必选port，且为空
        if (!p->IsOptional() && inputs[p->GetId()].empty()) {
            ready = false;
            break;
        }
    }
    return ready;
}

} // namespace flow