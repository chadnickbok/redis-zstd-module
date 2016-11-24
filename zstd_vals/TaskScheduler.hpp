/**
 * Simple Multi-Threaded Task Scheduler.
 */

#pragma once

#include <memory>
#include <vector>
#include <thread>

#include "Task.hpp"
#include "TaskQueue.hpp"
#include "TaskWorker.hpp"

class TaskScheduler {
public:
  TaskScheduler(int workers);
  virtual ~TaskScheduler();

  virtual void PushTask(Task *task);

private:
  std::shared_ptr<TaskQueue> task_queue;
  std::vector<std::shared_ptr<TaskWorker>> workers;
  std::vector<std::thread> worker_threads;
};
