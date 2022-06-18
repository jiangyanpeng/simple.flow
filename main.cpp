#include <iostream>

#include "Graph.hpp"

int main()
{
    std::cout << "hello coreflow" << std::endl;
    coreflow::Node node0("A"), node1("B"), node2("C"), node3("D"), node4("E"), node5("F");
    node0.Next(&node1);
    node1.Next(&node2).Next(&node3);
    node2.Next(&node4);
    node3.Next(&node4);
    node4.Next(&node5);

    coreflow::Graph graph;
    std::cout << graph.Dump(&node0) << std::endl;
    return 0;
}