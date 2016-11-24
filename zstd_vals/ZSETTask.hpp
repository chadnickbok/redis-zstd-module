/**
 * Compress value and unblock client.
 */

#pragma once

#include "Task.hpp"
#include <cstddef>

typedef struct RedisModuleBlockedClient RedisModuleBlockedClient;

class ZSETTask : public Task {
public:
    ZSETTask();
    ~ZSETTask() override;

    void Run() override;

    RedisModuleBlockedClient *bc;
    std::size_t key_len;
    char *key;
    std::size_t value_len;
    char *value;
    std::size_t res;
    void *compressed;
};
