/**
 * Simple Task Runner.
 * Waits for tasks from a blocking TaskQueue.
 */

#pragma once

#include <atomic>
#include <memory>

#include "TaskQueue.hpp"

class TaskWorker {
public:
  TaskWorker(std::shared_ptr<TaskQueue> task_queue);
  virtual ~TaskWorker();

  void Run();
  void Stop();

private:
  std::shared_ptr<TaskQueue> task_queue;
  std::atomic<bool> should_stop;
};
