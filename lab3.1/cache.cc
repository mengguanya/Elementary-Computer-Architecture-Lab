#include <math.h>
#include <random>
#include "cache.h"
#include "def.h"

    Cache::Cache()
    {
        /*
        initial:        32KB = 8 * 64 * 64B
        block size      32KB
        associativity   8
        set number      64
        block size      64 Bytes
        */
        config_.size = 32;
        config_.log_size = 5;

        config_.associativity = 8;
        config_.log_associativity = 3;

        config_.set_num = 64;
        config_.log_set_sum = 6;

        config_.block_size = 64;
        config_.log_block_size = 6;
  
        sets_ = new Set[64];
        for (int i = 0; i < 64; i++) {
            sets_[i].blocks = new Block[8];
            for (int j = 0; j < 8; j++) {
                sets_[i].blocks[j].data = new char[64];
                sets_[i].blocks[j].valid = false;
                sets_[i].blocks[j].dirty = false;
                sets_[i].blocks[j].last_access = 0;
                sets_[i].blocks[j].tag = 0;
                sets_[i].blocks[j].bit_lru = false;
            }
        }

        config_.write_allocate = 1;
        config_.write_back = 1;

        latency_.bus_latency = 0;
        latency_.hit_latency = 10;

        now_address_.block_offset = 0;
        now_address_.set = 0;
        now_address_.tag = 0;
    
        config_.prefetch_enable = false;
        config_.prefetch_num = 0;
        config_.prefetch_stride = 0;

        config_.replacement_algorithm_kind = 0;// default = LRU
    }
    
    void Cache::SetConfig(CacheConfig cc){
        config_.size = cc.size;
        config_.log_size = log2(cc.size);

        int associativity = config_.associativity = cc.associativity;
        config_.log_associativity=log2(cc.associativity);
  
        int set_num = config_.set_num = cc.size * 1024/ (cc.block_size * cc.associativity);
        config_.log_set_sum=log2(config_.set_num);

        int block_size = config_.block_size = cc.block_size;
        config_.log_block_size = log2(cc.block_size);

        config_.write_allocate = cc.write_allocate;
        config_.write_back = cc.write_back;
  
        config_.prefetch_enable = cc.prefetch_enable;
        config_.prefetch_num = cc.prefetch_num;
        config_.prefetch_stride = cc.prefetch_stride;

        config_.replacement_algorithm_kind = cc.replacement_algorithm_kind;

        //删除之前分配的空间
        delete [] sets_;

        sets_ = new Set[set_num];
        for(int i = 0; i < set_num; i++){
            sets_[i].blocks = new Block[associativity];
            for(int j = 0; j < associativity; j++){
                sets_[i].blocks[j].data = new char [block_size];
                sets_[i].blocks[j].valid = false;
                sets_[i].blocks[j].dirty = false;
                sets_[i].blocks[j].last_access = 0;
                sets_[i].blocks[j].tag = 0;
                sets_[i].blocks[j].bit_lru = false;
            } 
        }
    }

    CacheConfig Cache::GetConfig()
    {
    return config_;
    }

    void Cache::AnalysisNowAddress(uint64_t addr){
  
        now_address_.addr = addr;

        now_address_.tag = addr >> (config_.log_set_sum + config_.log_block_size);

        now_address_.set = (addr >> config_.log_block_size) & ((1 << config_.log_set_sum) - 1);
  
        now_address_.block_offset = addr & ((1 << config_.log_block_size) - 1);

        now_address_.block_addr = addr - now_address_.block_offset;
    }

    Block* Cache::FindHitBlock() {
        int hit_block_index = -1;
        Block* hit_set_blocks = sets_[now_address_.set].blocks;
        for (int i = 0; i < config_.associativity; i++)
        {
            if ((hit_set_blocks[i].valid == true) && (hit_set_blocks[i].tag == now_address_.tag))
            {
                hit_block_index = i;
                break;
            }
        }
        if (hit_block_index != -1) {
            return &(sets_[now_address_.set].blocks[hit_block_index]);
        }
        else
        {
            return NULL;
        }
    }

    void Cache::HandleRequest(uint64_t addr, int bytes, int read, char *content, int &hit, int &cycle)
    {
        stats_.access_counter++;

        AnalysisNowAddress(addr);

        Block* access_block = FindHitBlock();

        if (access_block != NULL) {
            stats_.hit_num++;
            hit++;

            access_block->last_access = stats_.access_counter;

            ReadWriteCache(access_block, addr, bytes, read, content, hit, cycle);
        }
        else {
            stats_.replace_num++;
            stats_.miss_num++;
            if ((!read) && (!config_.write_allocate)) {
                StoreBlock(access_block, hit, cycle);
            }
            else {
                access_block = ReplaceAlgorithm(hit, cycle);
                ReadWriteCache(access_block, addr, bytes, read, content, hit, cycle);
                //预取
                if (config_.prefetch_enable) {
                    PrefetchAlgorithm(hit, cycle);
                }
            }
        }
        cycle += latency_.bus_latency + latency_.hit_latency;
        stats_.access_time += latency_.bus_latency + latency_.hit_latency;
        return;
    }

    int Cache::RLU(){
        int victim = -1;
        int recent_least_use = INF;    // INF = 0X3f3f3f3f
        Block* hit_set_blocks = sets_[now_address_.set].blocks;
  
        for(int i = 0;i < config_.associativity; i++){
            if(!(hit_set_blocks[i].valid)){
                victim = i;
                break;
            }
        }
  
        if(victim == -1){
            for(int i = 0;i < config_.associativity; i++){
                if(hit_set_blocks[i].last_access < recent_least_use){
                    recent_least_use = hit_set_blocks[i].last_access;
                    victim = i;
                }
            }
        }
        return victim;
    }

    int Cache::BitPLRU() {
        int victim = -1;
        Block* hit_set_blocks = sets_[now_address_.set].blocks;
        int false_count = 0;
        for (int i = 0; i < config_.associativity; i++) {
            if (hit_set_blocks[i].bit_lru == false) {
                victim = i;
                hit_set_blocks[i].bit_lru = true;
                break;
            }
        }

        for (int i = 0; i < config_.associativity; i++) {
            false_count += 1 - hit_set_blocks[i].bit_lru;
        }
        if (false_count == 0) {
            for (int i = 0; i < config_.associativity; i++) {
                hit_set_blocks[i].bit_lru = false;
            }
        }
        return victim;
    }

    int Cache::TreePLRU() {
        return RLU();
    }

    int Cache::Rand() {
        return rand() % config_.associativity;
    }

    /*
    replace_algorithm_kind:
                        LRU         = 0
                        bit_PLRU    = 1 pseudo-LRU
                        tree_PLRU   = 2 pseudo-LRU

    */
    int Cache::ChooseReplacementAlgorithm() {
        int victim = -1;
        //choose the replace algorithm
        switch (config_.replacement_algorithm_kind)
        {
            case LRU:
                victim = RLU();
                break;
            case BITPLRU:
                victim = BitPLRU();
                break;
            case TREEPLRU:
                victim = TreePLRU();
                break;
            case RAND:
                victim = Rand();
            default:
                break;
        }
        return victim;
    }

    Block* Cache::ReplaceAlgorithm(int &hit, int &time){

        int victim = -1;
        victim = ChooseReplacementAlgorithm();

        Block *victim_block = &(sets_[now_address_.set].blocks[victim]);
        
        if(victim_block -> dirty){
            StoreBlock(victim_block, hit, time);
        }

        LoadBlock(victim_block, hit, time);

        return victim_block;
    }

    void Cache::PrefetchAlgorithm(int& hit, int& time)
    {
        uint64_t addr = now_address_.addr;
        for (int i = 1; i < config_.prefetch_num; i++) {
            addr = addr + config_.prefetch_stride * config_.block_size;
            AnalysisNowAddress(addr);
            ReplaceAlgorithm(hit, time);
        }
    }

    void Cache::LoadBlock(Block* target_block, int& hit, int& cycle) {

        lower_->HandleRequest(now_address_.block_addr, config_.block_size, true, target_block->data, hit, cycle);

        target_block->valid = true;
        target_block->tag = now_address_.tag;
        target_block->last_access = stats_.access_counter;
        target_block->dirty = false;
    }

    void Cache::StoreBlock(Block* source_block, int& hit, int& time) {
        lower_->HandleRequest(now_address_.block_addr, config_.block_size, false, source_block->data, hit, time);
    }

    void Cache::ReadWriteCache(Block * access_block, uint64_t addr, int bytes, int read, char* content, int& hit, int& time) {
        if (read == true)
        {
            memcpy(content, access_block->data + now_address_.block_offset, bytes);
        }
        else
        {
            memcpy(access_block->data + now_address_.block_offset, content, bytes);
            if (config_.write_back == true)
            {
                access_block->dirty = true;
            }
            else {
                StoreBlock(access_block, hit, time);
            }
        }
    }