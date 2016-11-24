/**
 * Simple Task Queue implementation.
 * PopTask blocks until job availabl
 */

#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>

#include "Task.hpp"

class TaskQueue {
public:
  TaskQueue();
  TaskQueue(const TaskQueue&);
  TaskQueue& operator=(const TaskQueue&) = delete;
  virtual ~TaskQueue();

  virtual void PushTask(Task *task);
  virtual Task *PopTask();

private:
  // Internal Queue
  std::queue<Task*> tasks;
  // Queue thread safety
  mutable std::mutex task_mutex;
  std::condition_variable task_cond;
};
