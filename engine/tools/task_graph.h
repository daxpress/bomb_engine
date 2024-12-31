#pragma once

#include <functional>
#include <span>
#include <unordered_set>
#include <variant>

#include "../macros.h"
#include "coroutine.h"
#include "thread_safe_queue.h"

namespace BE_NAMESPACE
{
class TaskGraph;

using Task = std::variant<Coroutine, std::function<void()>>;

enum class ExecutionPolicy : uint8_t
{
    MultiThreaded,
    Coroutine,
};

struct TaskID
{
    // basically a wrapper of uint32_t with a reference to the graph to allow dependency declaration
    // through IDs as well

    friend class TaskGraph;

    auto operator++() -> TaskID&;
    auto operator++(int) -> TaskID;
    auto operator==(const TaskID& other) const;
    auto operator!=(const TaskID& other) const;
    auto operator<(const TaskID& other) const;
    auto operator<=(const TaskID& other) const;
    auto operator>(const TaskID& other) const;
    auto operator>=(const TaskID& other) const;
    auto operator<=>(const TaskID& other) const;

    auto before(const TaskID& id) const -> void;
    auto before(const std::vector<TaskID>& id) const -> void;
    auto after(const TaskID& id) const -> void;
    auto after(const std::vector<TaskID>& id) const -> void;

    TaskID(const TaskID& id) = default;

private:
    TaskID(const uint32_t id, TaskGraph& graph) : m_ID(id), m_graph(graph) {}

    uint32_t m_ID;
    TaskGraph& m_graph;
};

class TaskGraph
{
    // we don't care about the TaskID class, just the integer
    using task_id_t = uint32_t;

    friend struct TaskID;

public:
    [[nodiscard]] auto add_task(Coroutine&& task) -> TaskID;
    [[nodiscard]] auto add_task(std::function<void()>&& task) -> TaskID;
    auto execute(const ExecutionPolicy policy = ExecutionPolicy::MultiThreaded) -> void;
    void stop() { m_stop = true; }

    ~TaskGraph();

private:
    inline auto run_before(const TaskID& before, const TaskID& after) -> void;
    inline auto run_before(const TaskID& before, const std::span<const TaskID> after) -> void;
    inline auto run_after(const TaskID& after, const TaskID& before) -> void;
    inline auto run_after(const TaskID& after, const std::span<const TaskID> before) -> void;

    inline auto execute_with_coroutine() -> void;
    inline auto execute_with_threads() -> void;

    auto coroutine_worker() -> Coroutine;
    auto threads_worker() -> void;

    task_id_t m_current_taskID = 0;
    std::unordered_map<task_id_t, Task> m_tasks;
    std::unordered_map<task_id_t, std::unordered_set<task_id_t>> m_adjacency_list;
    std::unordered_map<task_id_t, uint32_t> m_indegree_list;

    std::queue<task_id_t> m_task_queue;
    std::atomic_bool m_stop{false};
    std::mutex m_mutex;
    std::condition_variable m_cv;

    struct TaskVisitor
    {
        TaskVisitor(TaskGraph& graph, const task_id_t id);
        inline auto operator()(const Coroutine& coro) const -> bool;
        inline auto operator()(const std::function<void()>& func) const -> bool;

    private:
        TaskGraph& m_graph;
        task_id_t m_id;
    };

    struct CoroutineWorkerAwaiter
    {
        CoroutineWorkerAwaiter(
            std::condition_variable& cv, std::mutex& mutex, std::function<bool()> predicate
        );

        auto await_ready() const noexcept -> bool;
        void await_suspend(std::coroutine_handle<> coro);
        void await_resume() const noexcept {}

    private:
        std::condition_variable& m_cv;
        std::mutex& m_mutex;
        std::function<bool()> m_predicate;
        std::coroutine_handle<> m_handle;
    };

};
}  // namespace BE_NAMESPACE