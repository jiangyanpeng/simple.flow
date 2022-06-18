#include <iostream>

#include "Graph.hpp"

int main()
{
    coreflow::Node node0("A"), node1("B"), node2("C"), node3("D"), node4("E"), node5("F");
    node0.Next(&node1);
    node1.Next(&node2).Next(&node3);
    node2.Next(&node4);
    node3.Next(&node4);
    node4.Next(&node5);

    coreflow::Graph graph;
    std::cout << graph.Dump(&node0) << std::endl;

    std::cout << "=========================================" << std::endl;
    std::vector<coreflow::Node *> nodes;
    {
        nodes.emplace_back(&node0);
        nodes.emplace_back(&node1);
        nodes.emplace_back(&node2);
        nodes.emplace_back(&node3);
        nodes.emplace_back(&node4);
        nodes.emplace_back(&node5);
    }

    std::vector<coreflow::Node *> res = graph.Sort(nodes);
    for (const auto node : res)
    {
        std::cout << node->Name() << "(" << std::to_string(node->InDegree()) << ")"
                  << "->";
    }
    std::cout << std::endl;
    return 0;
}