#include "task_graph.h"

#include <algorithm>
#include <ranges>

#include "log.h"
#include "stopwatch.h"

MakeCategory(TaskGraph);

namespace BE_NAMESPACE
{

#pragma region Task Visitor
TaskGraph::TaskVisitor::TaskVisitor(TaskGraph& graph, const task_id_t id) : m_graph(graph), m_id(id)
{
}

auto TaskGraph::TaskVisitor::operator()(const Coroutine& coro) const -> bool
{
    // valid coroutine?
    if (!coro.handle || coro.handle.done())
    {
        return false;
    }

    // has a future to wait on or is it suspend_always{}?
    if (const auto& promise = coro.handle.promise();
        promise.future.has_value() && promise.future->valid())
    {
        // manage future status
        if (const auto status = promise.future->wait_for(std::chrono::milliseconds(0));
            status == std::future_status::ready)
        {
            // future is ready!
            coro.handle.resume();
            if (!coro.handle.done())
            {
                const std::lock_guard lock(m_graph.m_mutex);
                m_graph.m_task_queue.push(m_id);
            }
            else
            {
                m_graph.increment_task_counter();
            }
            m_graph.m_cv.notify_one();
            return false;
        }
        // future is not ready, put the coroutine back in the queue
        const std::lock_guard lock(m_graph.m_mutex);
        m_graph.m_task_queue.push(m_id);
        return true;
    }
    // no future used so if the coroutine is not done then simply re-queue it
    coro.handle.resume();
    if (coro.handle && !coro.handle.done())
    {
        {
            const std::lock_guard lock(m_graph.m_mutex);
            m_graph.m_task_queue.push(m_id);
        }
        return true;
    }

    m_graph.increment_task_counter();

    m_graph.m_cv.notify_one();
    return false;
}

auto TaskGraph::TaskVisitor::operator()(const std::function<void()>& func) const -> bool
{
    func();
    // increment ended tasks counter
    m_graph.increment_task_counter();
    return false;
}

#pragma endregion

#pragma region TaskID
auto TaskID::operator++() -> TaskID&
{
    ++m_ID;
    return *this;
}
auto TaskID::operator++(int) -> TaskID
{
    const TaskID tmp(*this);
    ++m_ID;
    return tmp;
}
auto TaskID::operator==(const TaskID& other) const { return m_ID == other.m_ID; }
auto TaskID::operator!=(const TaskID& other) const { return m_ID != other.m_ID; }
auto TaskID::operator<(const TaskID& other) const { return m_ID < other.m_ID; }
auto TaskID::operator<=(const TaskID& other) const { return m_ID <= other.m_ID; }
auto TaskID::operator>(const TaskID& other) const { return m_ID > other.m_ID; }
auto TaskID::operator>=(const TaskID& other) const { return m_ID >= other.m_ID; }
auto TaskID::operator<=>(const TaskID& other) const { return m_ID <=> other.m_ID; }

auto TaskID::before(const TaskID& id) const -> const TaskID&
{
    m_graph.run_before(*this, id);
    return *this;
}
auto TaskID::before(const std::initializer_list<const TaskID>& id) const -> const TaskID&
{
    m_graph.run_before(*this, id);
    return *this;
}
auto TaskID::before(const std::span<const TaskID>& id) const -> const TaskID&
{
    m_graph.run_before(*this, id);
    return *this;
}
auto TaskID::after(const TaskID& id) const -> const TaskID&
{
    m_graph.run_before(id, *this);
    return *this;
}
auto TaskID::after(const std::initializer_list<const TaskID>& id) const -> const TaskID&
{
    m_graph.run_after(*this, id);
    return *this;
}
auto TaskID::after(const std::span<const TaskID>& id) const -> const TaskID&
{
    m_graph.run_after(*this, id);
    return *this;
}

#pragma endregion

#pragma region TaskGraph

auto TaskGraph::add_task(Coroutine&& task) -> TaskID
{
    const auto taskID = m_current_taskID++;
    m_tasks[taskID] = Task(std::move(task));
    return {taskID, *this};
}
auto TaskGraph::add_task(std::function<void()>&& task) -> TaskID
{
    const auto taskID = m_current_taskID++;
    m_tasks[taskID] = Task(std::move(task));
    return {taskID, *this};
}

auto TaskGraph::run_before(const TaskID& before, const TaskID& after) -> void
{
    if (before == after || m_adjacency_list[before.m_ID].contains(after.m_ID))
    {
        return;
    }
    m_adjacency_list[before.m_ID].insert(after.m_ID);
    m_indegree_list[after.m_ID]++;
}
auto TaskGraph::run_before(const TaskID& before, const std::span<const TaskID>& after) -> void
{
    std::ranges::for_each(
        after.crbegin(), after.crend(), [&](const TaskID& taskID) { run_before(before, taskID); }
    );
}
auto TaskGraph::run_after(const TaskID& after, const std::span<const TaskID>& before) -> void
{
    std::ranges::for_each(
        before.crbegin(), before.crend(), [&](const TaskID& taskID) { run_before(taskID, after); }
    );
}
auto TaskGraph::execute_single_thread() -> void
{
    for (const auto& id : std::views::keys(m_tasks))
    {
        if (m_indegree_list[id] == 0)
        {
            m_task_queue.push(id);
        }
    }
    while (!m_task_queue.empty())
    {
        auto current_id = m_task_queue.front();
        m_task_queue.pop();

        auto& current_task = m_tasks[current_id];
        std::visit(TaskVisitor{*this, current_id}, current_task);
        add_available_tasks(current_id);
    }
}

auto TaskGraph::execute_with_threads() -> void
{
    std::vector<std::thread> threads;

    {
        const std::lock_guard lock(m_mutex);
        for (const auto& id : std::views::keys(m_tasks))
        {
            if (m_indegree_list[id] == 0)
            {
                m_task_queue.push(id);
            }
        }
    }

    for (auto i = 0; i < m_thread_count; ++i)
    {
        threads.emplace_back(&TaskGraph::thread_worker, this);
    }

    for (auto& thread : threads)
    {
        thread.join();
    }
}

auto TaskGraph::execute(const ExecutionPolicy policy) -> void
{
    if (m_running)
    {
        Log(TaskGraphCategory, LogSeverity::Error, "TaskGraph already running!");
        return;
    }
    m_running = true;
    m_tasks_ended = 0;
    m_stop = false;
    const auto stopwatch = Stopwatch();
    switch (policy)
    {
        case ExecutionPolicy::SingleThreaded:
            execute_single_thread();
            break;
        case ExecutionPolicy::MultiThreaded:
            execute_with_threads();
            break;
    }
    Log(TaskGraphCategory, LogSeverity::Display, "Execution time: {} seconds", stopwatch.elapsed());
    m_running = false;
}

auto TaskGraph::thread_worker() -> void
{
    while (true)
    {
        task_id_t current_id;
        {
            std::unique_lock lock(m_mutex);
            m_cv.wait(lock, [&] { return !m_task_queue.empty() || m_stop; });

            if (m_stop)
            {
                break;
            }
            current_id = m_task_queue.front();
            m_task_queue.pop();
        }

        // retrieve task and run it
        if (m_tasks.contains(current_id))
        {
            auto& current_task = m_tasks[current_id];
            std::visit(TaskVisitor{*this, current_id}, current_task);
        }

        {
            const std::lock_guard lock(m_mutex);
            add_available_tasks(current_id);
            m_cv.notify_all();
        }
    }
}
auto TaskGraph::add_available_tasks(const task_id_t task_id) -> void
{
    if (const auto& coro = m_tasks[task_id]; std::holds_alternative<Coroutine>(coro))
    {
        // if the coroutine is not done the adjacency list should not be touched!
        if (!std::get<Coroutine>(coro).handle.done())
        {
            return;
        }
    }
    for (const auto& dependent : m_adjacency_list[task_id])
    {
        if (--m_indegree_list[dependent] == 0)
        {
            m_task_queue.push(dependent);
        }
    }
}

void TaskGraph::increment_task_counter()
{
    ++m_tasks_ended;
    if (m_tasks_ended == m_tasks.size())
    {
        m_stop = true;
        m_cv.notify_all();
    }
}

TaskGraph::~TaskGraph()
{
    stop();
    m_cv.notify_all();
}
#pragma endregion TaskGraph

}  // namespace BE_NAMESPACE