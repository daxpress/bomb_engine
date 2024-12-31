#include "task_graph.h"

#include <algorithm>
#include <ranges>

#include "log.h"

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
    const auto& promise = coro.handle.promise();
    if (promise.future.has_value() && promise.future->valid())
    {
        // manage future status
        if (const auto status = promise.future->wait_for(std::chrono::milliseconds(0));
            status == std::future_status::ready)
        {
            // future is ready!
            coro.handle.resume();
            if (!coro.handle.done())
            {
                std::lock_guard lock(m_graph.m_mutex);
                m_graph.m_task_queue.push(m_id);
            }
            return false;
        }
        // future is not ready, put the coroutine back in the queue
        std::lock_guard lock(m_graph.m_mutex);
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
    m_graph.m_cv.notify_all();
    return false;
}

auto TaskGraph::TaskVisitor::operator()(const std::function<void()>& func) const -> bool
{
    func();
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

auto TaskID::before(const TaskID& id) const -> void { m_graph.run_before(*this, id); }
auto TaskID::before(const std::vector<TaskID>& id) const -> void { m_graph.run_before(*this, id); }
auto TaskID::after(const TaskID& id) const -> void { m_graph.run_after(*this, id); }
auto TaskID::after(const std::vector<TaskID>& id) const -> void { m_graph.run_after(*this, id); }

#pragma endregion

#pragma region Worker Awaiter
TaskGraph::CoroutineWorkerAwaiter::CoroutineWorkerAwaiter(
    std::condition_variable& cv, std::mutex& mutex, std::function<bool()> predicate
)
    : m_cv(cv), m_mutex(mutex), m_predicate(std::move(predicate))
{
}
auto TaskGraph::CoroutineWorkerAwaiter::await_ready() const noexcept -> bool
{
    const std::unique_lock lock(m_mutex);
    return m_predicate();
}
void TaskGraph::CoroutineWorkerAwaiter::await_suspend(const std::coroutine_handle<> coro)
{
    std::unique_lock lock(m_mutex);
    m_handle = coro;
    m_cv.wait(lock, [&] { return m_predicate(); });
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
    if (before == after)
    {
        return;
    }
    m_adjacency_list[before.m_ID].insert(after.m_ID);
    m_indegree_list[after.m_ID]++;
}
auto TaskGraph::run_before(const TaskID& before, const std::span<const TaskID> after) -> void
{
    std::ranges::for_each(
        after.crbegin(), after.crend(), [&](const TaskID& taskID) { run_before(before, taskID); }
    );
}
auto TaskGraph::run_after(const TaskID& after, const TaskID& before) -> void
{
    if (before == after)
    {
        return;
    }
    m_adjacency_list[before.m_ID].insert(after.m_ID);
    m_indegree_list[after.m_ID]++;
}
auto TaskGraph::run_after(const TaskID& after, const std::span<const TaskID> before) -> void
{
    std::ranges::for_each(
        before.crbegin(), before.crend(), [&](const TaskID& taskID) { run_after(after, taskID); }
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
        for (const auto& dependent : m_adjacency_list[current_id])
        {
            if (--m_indegree_list[dependent] == 0)
            {
                m_task_queue.push(dependent);
            }
        }
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

    const auto thread_count = std::thread::hardware_concurrency();
    for (auto i = 0; i < thread_count; ++i)
    {
        threads.emplace_back(&TaskGraph::threads_worker, this);
    }

    {
        const std::lock_guard lock(m_mutex);
        m_stop = true;  // Set stop flag to true once all tasks are enqueued so that they will stop
        // once m_task_queue is empty
    }

    // Notify all workers that they can stop when no tasks left
    m_cv.notify_all();

    for (auto& thread : threads)
    {
        thread.join();
    }
}

auto TaskGraph::execute(const ExecutionPolicy policy) -> void
{
    switch (policy)
    {
        case ExecutionPolicy::SingleThreaded:
            execute_single_thread();
            return;
        case ExecutionPolicy::MultiThreaded:
            execute_with_threads();
            return;
    }
}

auto TaskGraph::threads_worker() -> void
{
    while (true)
    {
        task_id_t current_id;
        {
            std::unique_lock lock(m_mutex);
            m_cv.wait(lock, [&]() { return !m_task_queue.empty() || m_stop; });

            if (m_stop && m_task_queue.empty())
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
            for (const auto& dependent : m_adjacency_list[current_id])
            {
                if (--m_indegree_list[dependent] == 0)
                {
                    m_task_queue.push(dependent);
                }
            }
            m_cv.notify_all();
        }
    }
}

TaskGraph::~TaskGraph()
{
    stop();
    m_cv.notify_all();
}
#pragma endregion TaskGraph

}  // namespace BE_NAMESPACE