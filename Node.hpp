#pragma once

#include <vector>
#include <unordered_set>

namespace coreflow
{
    class Node
    {
    public:
        Node() = default;
        ~Node() = default;

        explicit Node(const std::string &name) : m_name(name), m_in_degree(0) {}
        const std::string Name() const { return m_name; }
        const size_t InDegree() const { return m_in_degree; }
        const size_t OutDegree() const { return m_next_node.size(); }

        Node &Next(Node *node)
        {

            m_next_node.emplace_back(node);
            node->m_in_degree++;
            node->m_prev_node.emplace_back(this);

            return *this;
        }

        Node *Next(size_t index)
        {
            if (index < m_next_node.size())
            {
                return nullptr;
            }
            return m_next_node[index];
        }

        Node &Prev(Node *node)
        {
            m_prev_node.emplace_back(node);
            this->m_in_degree++;
            node->m_next_node.emplace_back(this);

            return *this;
        }

    private:
        // node name
        std::string m_name;

        // current degree numbers
        size_t m_in_degree{0};

        // runtime join_nums == 0, push thread pool.
        size_t m_join_count{0};

        // used to know is the node visited or not.
        size_t m_current_traverse_seed;

        std::vector<Node *> m_next_node;
        std::vector<Node *> m_prev_node;
    };
}