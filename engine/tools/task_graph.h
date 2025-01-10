#pragma once

#include <functional>
#include <queue>
#include <span>
#include <unordered_set>
#include <variant>

#include "../macros.h"
#include "coroutine.h"

namespace BE_NAMESPACE
{
class TaskGraph;

using Task = std::variant<Coroutine, std::function<void()>>;

enum class ExecutionPolicy : uint8_t
{
    MultiThreaded,
    SingleThreaded,
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

    [[maybe_unused]] auto before(const TaskID& id) const -> const TaskID&;
    [[maybe_unused]] auto before(const std::initializer_list<const TaskID>& id) const
        -> const TaskID&;
    [[maybe_unused]] auto before(const std::span<const TaskID>& id) const -> const TaskID&;
    [[maybe_unused]] auto after(const TaskID& id) const -> const TaskID&;
    [[maybe_unused]] auto after(const std::initializer_list<const TaskID>& id) const
        -> const TaskID&;
    [[maybe_unused]] auto after(const std::span<const TaskID>& id) const -> const TaskID&;

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
    void set_thread_count(const uint8_t thread_count)
    {
        m_thread_count = std::clamp<uint8_t>(thread_count, 2, std::thread::hardware_concurrency());
    }

    ~TaskGraph();

private:
    // first one is also used for run_after
    inline auto run_before(const TaskID& before, const TaskID& after) -> void;
    inline auto run_before(const TaskID& before, const std::span<const TaskID>& after) -> void;
    inline auto run_after(const TaskID& after, const std::span<const TaskID>& before) -> void;

    inline auto execute_single_thread() -> void;
    inline auto execute_with_threads() -> void;

    auto thread_worker() -> void;
    inline void add_available_tasks(const task_id_t task_id);
    inline void increment_task_counter();

    task_id_t m_current_taskID = 0;
    uint8_t m_thread_count = std::thread::hardware_concurrency();
    std::unordered_map<task_id_t, Task> m_tasks;
    std::unordered_map<task_id_t, std::unordered_set<task_id_t>> m_adjacency_list;
    std::unordered_map<task_id_t, uint32_t> m_indegree_list;

    std::queue<task_id_t> m_task_queue;
    std::atomic_bool m_stop{false};
    bool m_running{false};
    std::atomic_uint32_t m_tasks_ended{0};
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
};
}  // namespace BE_NAMESPACE