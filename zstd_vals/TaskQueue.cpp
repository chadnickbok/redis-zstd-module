/**
 * Simple Task Queue implementation.
 */

#include "TaskQueue.hpp"

#include <chrono>

TaskQueue::TaskQueue() {
};

TaskQueue::~TaskQueue()
{
    ;
}

void TaskQueue::PushTask(Task *task)
{
    std::lock_guard<std::mutex> lock(task_mutex);
    tasks.push(task);
    task_cond.notify_one();
}

Task *TaskQueue::PopTask()
{
    Task *task = nullptr;
    std::unique_lock<std::mutex> lock(task_mutex);

    task_cond.wait_for(lock, std::chrono::milliseconds(1000),
        [this]{return !tasks.empty();});

    if (!tasks.empty())
    {
        task = tasks.front();
        tasks.pop();
    }

    return task;
}
