/**
 * Compress value with dict and unblock client.
 */

#pragma once

#include "Task.hpp"
#include <cstddef>

extern "C" {
#include <zstd.h>
}

typedef struct RedisModuleBlockedClient RedisModuleBlockedClient;

class ZDICTSETTask : public Task {
public:
    ZDICTSETTask(ZSTD_CDict* cdict);
    ~ZDICTSETTask() override;

    void Run() override;

    RedisModuleBlockedClient *bc;
    ZSTD_CDict* cdict;
    std::size_t key_len;
    char *key;
    std::size_t value_len;
    char *value;
    std::size_t res;
    void *compressed;
};
