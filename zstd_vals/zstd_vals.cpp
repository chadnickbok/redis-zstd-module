/**
 * ZSTD redis module
 */

#include <memory>
#include <thread>
#include <iostream>
#include <fstream>
#include <sstream>

extern "C" {
#include "../redismodule.h"
#include "../rmutil/util.h"
#include "../rmutil/strings.h"
#include "../rmutil/test_util.h"

#include <zstd.h>
}

#include "ZSETTask.hpp"
#include "ZDICTSETTask.hpp"
#include "TaskScheduler.hpp"

#define COMPRESSION_LEVEL 1
std::shared_ptr<TaskScheduler> scheduler = nullptr;
ZSTD_CDict* cdict = NULL;
ZSTD_DDict* ddict = NULL;

int ZSET_Reply(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    REDISMODULE_NOT_USED(argv);
    REDISMODULE_NOT_USED(argc);

    ZSETTask *task = (ZSETTask*) RedisModule_GetBlockedClientPrivateData(ctx);

    if (ZSTD_isError(task->res))
    {
        const char *zstd_err = ZSTD_getErrorName(task->res);
        return RedisModule_ReplyWithError(ctx, zstd_err);
    }

    RedisModuleString *keyname = RedisModule_CreateString(ctx, (const char*) task->key, task->key_len);

    RedisModuleKey *key = (RedisModuleKey *) RedisModule_OpenKey(ctx, keyname, REDISMODULE_READ|REDISMODULE_WRITE);
    int keytype = RedisModule_KeyType(key);
    if ((keytype != REDISMODULE_KEYTYPE_STRING) && (keytype != REDISMODULE_KEYTYPE_EMPTY))
    {
        RedisModule_CloseKey(key);
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    // Update string with compressed data
    RedisModule_StringTruncate(key, task->res);
    size_t stringSize;
    char *stringDMA = (char *) RedisModule_StringDMA(key, &stringSize, REDISMODULE_READ | REDISMODULE_WRITE);
    memcpy(stringDMA, task->compressed, task->res);

    RedisModule_CloseKey(key);

    return RedisModule_ReplyWithSimpleString(ctx,"OK");
}

int ZSET_Timeout(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    REDISMODULE_NOT_USED(argv);
    REDISMODULE_NOT_USED(argc);
    return RedisModule_ReplyWithSimpleString(ctx, "Request timedout");
}

void ZSET_FreeData(void *privdata)
{
    ZSETTask *task = (ZSETTask*) privdata;
    delete task;
}

/*
 * zstd_vals.ZSET <key> <value>
 * Compress and store a key.
 */
int ZSETCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    // we need exactly 3 arguments
    if (argc != 3)
    {
        return RedisModule_WrongArity(ctx);
    }

    ZSETTask *task = new ZSETTask();

    const char *key_in = RedisModule_StringPtrLen(argv[1], &task->key_len);
    task->key = (char*) RedisModule_Alloc(task->key_len);
    memcpy(task->key, key_in, task->key_len);

    const char *value_in = RedisModule_StringPtrLen(argv[2], &task->value_len);
    task->value = (char*) RedisModule_Alloc(task->value_len);
    memcpy(task->value, value_in, task->value_len);

    task->bc = RedisModule_BlockClient(ctx, ZSET_Reply, ZSET_Timeout, ZSET_FreeData, 1000 * 10);

    scheduler->PushTask(task);

    return REDISMODULE_OK;
}

/*
 * zstd_vals.ZGET <key>
 * Get the raw value of a key and decompress it.
 */
int ZGETCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    // we need EXACTLY 2 arguments
    if (argc != 2)
    {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);

    // open the key and make sure it's indeed a string or empty
    RedisModuleKey *key = (RedisModuleKey*) RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ);
    if (RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_STRING)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    size_t len;
    char *compressed_buf = RedisModule_StringDMA(key, &len, REDISMODULE_READ);
    // XXX: Do I need to check for errors here?

    unsigned long long buf_size = ZSTD_getDecompressedSize(compressed_buf, len);

    void *decompressed_buf = RedisModule_Alloc(buf_size);
    // XXX: Errors?

    size_t actual_size = ZSTD_decompress(decompressed_buf, buf_size, compressed_buf, len);
    RedisModule_ReplyWithStringBuffer(ctx, (const char *) decompressed_buf, actual_size);

    RedisModule_Free(decompressed_buf);

    return REDISMODULE_OK;
}

int ZDICTSET_Reply(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    REDISMODULE_NOT_USED(argv);
    REDISMODULE_NOT_USED(argc);

    ZDICTSETTask *task = (ZDICTSETTask*) RedisModule_GetBlockedClientPrivateData(ctx);

    if (ZSTD_isError(task->res))
    {
        const char *zstd_err = ZSTD_getErrorName(task->res);
        return RedisModule_ReplyWithError(ctx, zstd_err);
    }

    RedisModuleString *keyname = RedisModule_CreateString(ctx, (const char*) task->key, task->key_len);

    RedisModuleKey *key = (RedisModuleKey *) RedisModule_OpenKey(ctx, keyname, REDISMODULE_READ|REDISMODULE_WRITE);
    int keytype = RedisModule_KeyType(key);
    if ((keytype != REDISMODULE_KEYTYPE_STRING) && (keytype != REDISMODULE_KEYTYPE_EMPTY))
    {
        RedisModule_CloseKey(key);
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    // Update string with compressed data
    RedisModule_StringTruncate(key, task->res);
    size_t stringSize;
    char *stringDMA = (char *) RedisModule_StringDMA(key, &stringSize, REDISMODULE_READ | REDISMODULE_WRITE);
    memcpy(stringDMA, task->compressed, task->res);

    RedisModule_CloseKey(key);

    return RedisModule_ReplyWithSimpleString(ctx,"OK");
}

int ZDICTSET_Timeout(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    REDISMODULE_NOT_USED(argv);
    REDISMODULE_NOT_USED(argc);
    return RedisModule_ReplyWithSimpleString(ctx, "Request timedout");
}

void ZDICTSET_FreeData(void *privdata)
{
    ZSETTask *task = (ZSETTask*) privdata;
    delete task;
}

/**
 * zstd.ZDICTSET <key> <dictkey> <value>
 * Set a compressed value using the dictionary stored at <dictkey>
 */
int ZDICTSETCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    // we need exactly 3 arguments
    if (argc != 3)
    {
        return RedisModule_WrongArity(ctx);
    }

    ZDICTSETTask *task = new ZDICTSETTask(cdict);

    const char *key_in = RedisModule_StringPtrLen(argv[1], &task->key_len);
    task->key = (char*) RedisModule_Alloc(task->key_len);
    memcpy(task->key, key_in, task->key_len);

    const char *value_in = RedisModule_StringPtrLen(argv[2], &task->value_len);
    task->value = (char*) RedisModule_Alloc(task->value_len);
    memcpy(task->value, value_in, task->value_len);

    task->bc = RedisModule_BlockClient(ctx, ZDICTSET_Reply, ZDICTSET_Timeout, ZDICTSET_FreeData, 1000 * 10);

    scheduler->PushTask(task);

    return REDISMODULE_OK;
}

/**
 * zstd.ZDICTGET <key> <dictkey>
 * Get a compressed value stored at <key> and compressed with dictionary
 */
int ZDICTGETCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
    // we need EXACTLY 2 arguments
    if (argc != 2)
    {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);

    // ensure key is string
    RedisModuleKey *key = (RedisModuleKey*) RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ);
    if (RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_STRING)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    size_t compressed_len;
    char *compressed_buf = RedisModule_StringDMA(key, &compressed_len, REDISMODULE_READ);

    unsigned long long buf_size = ZSTD_getDecompressedSize(compressed_buf, compressed_len);
    void *decompressed_buf = RedisModule_Alloc(buf_size);

    ZSTD_DCtx* zdctx = ZSTD_createDCtx();
    size_t actual_size = ZSTD_decompress_usingDDict(
        zdctx,
        decompressed_buf, buf_size,
        compressed_buf, compressed_len,
        ddict);
    RedisModule_ReplyWithStringBuffer(ctx, (const char*) decompressed_buf, actual_size);

    RedisModule_Free(decompressed_buf);

    return REDISMODULE_OK;
}

void zstd_load_dictionary(const char *dictpath)
{
  if (!dictpath)
  {
      fprintf(stderr, "No dictionary provided, ignoring\n");
      return;
  }

  fprintf(stderr, "Loadng dictionary from: %s\n", dictpath);
  std::ifstream dictfile(dictpath, std::ifstream::binary);
  if (!dictfile.is_open())
  {
      fprintf(stderr, "Error loading input file '%s'\n", dictpath);
      return;
  }

  std::stringstream dictstream;
  dictstream << dictfile.rdbuf();
  std::string dictstring = dictstream.str();
  dictfile.close();

  cdict = ZSTD_createCDict(dictstring.c_str(), dictstring.size(), COMPRESSION_LEVEL);
  if (!cdict)
  {
      fprintf(stderr, "Invalid dictionary provided, could not load\n");
  }

  ddict = ZSTD_createDDict(dictstring.c_str(), dictstring.size());
  if (!ddict)
  {
      fprintf(stderr, "Invalid dictionary provided, could not load\n");
  }
}

extern "C" {
int RedisModule_OnLoad(RedisModuleCtx *ctx) {
    // Register the module itself
    if (RedisModule_Init(ctx, "example", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    RMUtil_RegisterWriteCmd(ctx, "zstd.SET", ZSETCommand);
    RMUtil_RegisterReadCmd(ctx, "zstd.GET", ZGETCommand);

    RMUtil_RegisterWriteCmd(ctx, "zstd.DICTSET", ZDICTSETCommand);
    RMUtil_RegisterReadCmd(ctx, "zstd.DICTGET", ZDICTGETCommand);

    // Task scheduler
    scheduler = std::make_shared<TaskScheduler>(4);

    // Detect Dictionary config and load
    zstd_load_dictionary(std::getenv("ZSTD_DICTPATH"));

    return REDISMODULE_OK;
}
};
