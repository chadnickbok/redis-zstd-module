/**
 * Abstract Task class.
 * Override Run in sub-classes.
 */

#pragma once

class Task {
public:
  Task() {};
  virtual ~Task() {};

  virtual void Run() = 0;
};
