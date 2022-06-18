#pragma once
#include "Node.hpp"

#include <unordered_map>
#include <queue>
#include <iostream>
#include <sstream>
#include <algorithm>

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
        Graph(const std::vector<Node *> &inputs, bool sorted = false) : m_nodes(inputs), m_sorted(sorted)
        {
        }
        bool AddNode(Node *node)
        {
            if (!node)
                return false;
            auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [&](Node *n)
                                   { return n == node || n->Name() == node->Name(); });
            if (it != m_nodes.end())
            {
                return false;
            }
            m_nodes.emplace_back(node);
            return true;
        }
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

        std::string Dump()
        {
            // todo Graphviz
            return "";
        }

        std::string Dump(const Node *head)
        {
            if (!head)
            {
                return "";
            }
            std::stringstream ss;
            ss << head->Name() << "(" << std::to_string(head->InDegree()) << ")"
               << "->";
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
                        ss << next->Name() << "(" << std::to_string(next->InDegree()) << ")"
                           << "->";
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

    private:
        bool m_sorted;
        std::vector<Node *> m_nodes;
    };
}