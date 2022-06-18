#pragma once
#include "Node.hpp"

#include <unordered_map>
#include <queue>
#include <iostream>
#include <sstream>

namespace coreflow
{
    class Graph
    {
    private:
        struct TopoSortNode
        {
            size_t m_degree;
            Node *m_node;
            TopoSortNode()
            {
                m_degree = 0;
                m_node = nullptr;
            }

            TopoSortNode(size_t degree, Node *node)
            {
                m_degree = degree;
                m_node = node;
            }
        };

    public:
        Graph() = default;
        ~Graph() = default;

        std::vector<Node *> Sort(const std::vector<Node *> inputs)
        {
            if (inputs.empty())
                return {};
            std::queue<Node *> Q;

            std::vector<Node *> order_nodes(inputs.size());

            std::unordered_map<std::string, TopoSortNode> VecNode;
            for (size_t i = 0; i < inputs.size(); i++)
            {
                Node *node = inputs[i];
                if (node->InDegree() == 0)
                {
                    Q.push(node);
                }

                VecNode[node->Name()] = {node->InDegree(), node};
            }

            size_t order_index = 0;
            while (Q.size())
            {
                Node *front = Q.front();
                Q.pop();
                order_nodes[order_index++] = front;

                size_t index = 0;
                Node *next = front->Next(index);
                while (next)
                {
                    VecNode[next->Name()].m_degree--;
                    if (VecNode[next->Name()].m_degree == 0)
                    {
                        Q.push(next);
                    }

                    next = front->Next(++index);
                }
            }

            if (order_index < inputs.size())
            {
                order_nodes.clear();
                std::cout << "coreflow not support cycle" << std::endl;
            }
            return order_nodes;
        }

        std::string Dump(const Node *head)
        {
            std::stringstream ss;
            if (!head)
                return "";
            ss << head->Name() << "->";
            std::queue<Node *> Q;
            Node *root = const_cast<Node *>(head);
            Q.push(root);

            std::unordered_set<std::string> visited;
            while (Q.size())
            {
                Node *front = Q.front();
                Q.pop();

                size_t index = 0;
                Node *next = front->Next(index);
                while (next)
                {
                    if (next->InDegree() <= 1 || (next->InDegree() > 1 && visited.find(next->Name()) == visited.end()))
                    {
                        ss << next->Name() << "->";
                        Q.push(next);
                        if (next->InDegree() > 1)
                        {
                            visited.insert(next->Name());
                        }
                    }
                    next = front->Next(++index);
                }
            }
            std::string result = ss.str();
            return std::move(result.substr(0, result.length() - 2));
        }
    };
}