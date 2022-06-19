#pragma once

#include "ThreadPool.hpp"

#include "Graph.hpp"
namespace coreflow
{

    class Executor
    {
    public:
        Executor()
        {
            m_pool = std::make_shared<ThreadPool>(std::thread::hardware_concurrency());
        }
        ~Executor() = default;

        void run(Graph *graph);

    private:
        std::shared_ptr<ThreadPool> m_pool;
    };
}