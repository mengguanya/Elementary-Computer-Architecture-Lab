#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include <stdint.h>
#include <string.h>
#include "storage.h"

/*
define replacement algorithm kind
*/
#define LRU         0
#define BITPLRU     1
#define TREEPLRU    2
#define RAND        3

//cache hit latency
#define L1CACHEHITLATENCY 1.4794
#define L2CACHEHITLATENCY 1.9206

struct Block
{
  bool valid;
  int64_t tag;
  char *data;
  bool dirty;
  int64_t last_access;
  bool bit_lru;
};

struct Set
{
  Block *blocks;
};

struct NowAddress
{
  uint64_t addr;
  uint64_t block_addr; // block_addr = addr - block_offset
  int set;
  int tag;      // 
  int block_offset;    // offset
};

struct CacheConfig
{
    int size;
    int associativity;
    int block_size;
    int set_num;        // Number of cache sets
  
    int log_size;
    int log_associativity;
    int log_block_size;
    int log_set_sum;

    int write_back; 
    int write_allocate; 

    //cache预取，每次预取的块数，预取步长
    bool prefetch_enable;
    int prefetch_num;
    int prefetch_stride;

    //cache replacement algorithm
    int replacement_algorithm_kind;
};

class Cache : public Storage
{
public:
  Cache();
  ~Cache() {};

  // Sets & Gets
  void SetConfig(CacheConfig cc);
  CacheConfig GetConfig(void);
  void SetLower(Storage *ll) { lower_ = ll; }
  // Main access process
    void HandleRequest(uint64_t addr, int bytes, int read,
                     char *content, int &hit, int &time);

private:
  // Replacement
    Block* ReplaceAlgorithm(int &hit, int &time);
    int RLU();
    int BitPLRU();
    int TreePLRU();
    int Rand();
    // Prefetching
    void PrefetchAlgorithm(int& hit, int& time);

    void AnalysisNowAddress(uint64_t addr);
    Block* FindHitBlock();

    void LoadBlock(Block* target_block, int& hit, int& cycle);
    void StoreBlock(Block* source_block, int& hit, int& cycle);

    void ReadWriteCache(Block* hit_block, uint64_t addr, int bytes, int read, char* content, int& hit, int& time);

    int ChooseReplacementAlgorithm();

  CacheConfig config_;
  Set *sets_;
  NowAddress now_address_;
  Storage *lower_;
  DISALLOW_COPY_AND_ASSIGN(Cache);
};

#endif //CACHE_CACHE_H_
