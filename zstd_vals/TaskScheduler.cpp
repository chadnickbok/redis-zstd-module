/**
 * Simple Multi-Threaded Task Scheduler.
 */

#include "TaskScheduler.hpp"

TaskScheduler::TaskScheduler(int workers)
{
    this->task_queue = std::make_shared<TaskQueue>();

    for (int i = 0; i < workers; i++)
    {
        std::shared_ptr<TaskWorker> worker = std::make_shared<TaskWorker>(this->task_queue);
        this->workers.push_back(worker);
        this->worker_threads.push_back(std::thread(&TaskWorker::Run, worker.get()));
    }
}

TaskScheduler::~TaskScheduler()
{
    for (auto&& worker : this->workers)
    {
        worker->Stop();
    }

    for (auto&& worker_thread : this->worker_threads)
    {
        worker_thread.join();
    }
}

void TaskScheduler::PushTask(Task *task)
{
    this->task_queue->PushTask(task);
}
