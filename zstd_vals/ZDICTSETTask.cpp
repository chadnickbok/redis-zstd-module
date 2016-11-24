/**
 * Abstract Task class.
 * Override Run in sub-classes.
 */

#include "ZDICTSETTask.hpp"

extern "C" {
#include <zstd.h>
extern void (*RedisModule_Free)(void *ptr);
extern void *(*RedisModule_Alloc)(size_t bytes);
extern int (*RedisModule_UnblockClient)(RedisModuleBlockedClient *bc, void *privdata);
}

ZDICTSETTask::ZDICTSETTask(ZSTD_CDict* cdict) :
    cdict(cdict), key(NULL), value(NULL), compressed(NULL)
{
    ;
}

ZDICTSETTask::~ZDICTSETTask()
{
    RedisModule_Free(this->key);
    RedisModule_Free(this->value);
    RedisModule_Free(this->compressed);
}

void ZDICTSETTask::Run()
{
    // TODO: Re-use one compression context per thread
    ZSTD_CCtx *cctx = ZSTD_createCCtx();
    size_t bound = ZSTD_compressBound(this->value_len);
    this->compressed = RedisModule_Alloc(bound);

    this->res = ZSTD_compress_usingCDict(cctx,
      this->compressed, bound, // compressed output
      this->value, this->value_len, // raw input
      cdict);

    ZSTD_freeCCtx(cctx);

    RedisModule_UnblockClient(this->bc, this);
}
