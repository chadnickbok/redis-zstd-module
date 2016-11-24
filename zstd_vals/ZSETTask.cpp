/**
 * Abstract Task class.
 * Override Run in sub-classes.
 */

#include "ZSETTask.hpp"

extern "C" {
#include <zstd.h>
extern void (*RedisModule_Free)(void *ptr);
extern void *(*RedisModule_Alloc)(size_t bytes);
extern int (*RedisModule_UnblockClient)(RedisModuleBlockedClient *bc, void *privdata);
}

ZSETTask::ZSETTask() :
    key(NULL), value(NULL), compressed(NULL)
{
    ;
}

ZSETTask::~ZSETTask()
{
    RedisModule_Free(this->key);
    RedisModule_Free(this->value);
    RedisModule_Free(this->compressed);
}

void ZSETTask::Run()
{
    size_t bound = ZSTD_compressBound(this->value_len);
    this->compressed = RedisModule_Alloc(bound);
    this->res = ZSTD_compress(this->compressed, bound, this->value, this->value_len, 1);

    RedisModule_UnblockClient(this->bc, this);
}
