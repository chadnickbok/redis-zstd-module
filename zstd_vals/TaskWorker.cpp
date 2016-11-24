/**
 * Simple Task Running.
 */

#include "TaskWorker.hpp"

TaskWorker::TaskWorker(std::shared_ptr<TaskQueue> task_queue) :
    task_queue(task_queue), should_stop(false)
{
}

TaskWorker::~TaskWorker() {
}

void TaskWorker::Stop() {
  this->should_stop = true;
}

void TaskWorker::Run() {
  while (!this->should_stop)
  {
    Task *task = task_queue->PopTask();
    if (!task)
    {
      continue;
    }

    task->Run();
  }
}
