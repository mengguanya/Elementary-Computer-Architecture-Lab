#include<iostream>
#include<string>
#include<algorithm>
#include<string>
#include<fstream>
#include "stdio.h"
#include "cache.h"
#include "memory.h"

using namespace std;

void InitialMemorySystem(Memory &m, Cache &l1, Cache &l2) {
  StorageStats s;
  s.access_time = 0;
  s.access_counter = 0;
  s.fetch_num = 0;
  s.hit_num = 0;
  s.miss_num = 0;
  s.prefetch_num = 0;
  s.replace_num = 0;
  m.SetStats(s);
  l1.SetStats(s);
  l2.SetStats(s);

  CacheConfig l1_cache_config;
  l1_cache_config.size = 32;
  l1_cache_config.associativity = 8;
  l1_cache_config.block_size = 64;
  l1_cache_config.write_allocate = true;
  l1_cache_config.write_back = true;

  l1_cache_config.prefetch_enable = false;
  l1_cache_config.prefetch_num = 4;
  l1_cache_config.prefetch_stride = 1;

  l1_cache_config.replacement_algorithm_kind = LRU;
  l1.SetConfig(l1_cache_config);

  CacheConfig l2_cache_config;
  l2_cache_config.size = 256;
  l2_cache_config.associativity = 8;
  l2_cache_config.block_size = 64;
  l2_cache_config.write_allocate = true;
  l2_cache_config.write_back = true;

  l2_cache_config.prefetch_enable = false;
  l2_cache_config.prefetch_num = 4;
  l2_cache_config.prefetch_stride = 1;

  l2_cache_config.replacement_algorithm_kind = LRU;
  l2.SetConfig(l2_cache_config);

  StorageLatency memory_latency;
  memory_latency.bus_latency = 0;
  memory_latency.hit_latency = 100;
  m.SetLatency(memory_latency);

  StorageLatency l1cache_latency;
  l1cache_latency.bus_latency = 0;
  l1cache_latency.hit_latency = 3;
  l1.SetLatency(l1cache_latency);

  StorageLatency l2cache_latency;
  l2cache_latency.bus_latency = 6;
  l2cache_latency.hit_latency = 4;
  l2.SetLatency(l2cache_latency);
}

int main(int argc, char* argv[]) {
  //string traceName = "trace1.txt";
	
	//string traceName = "01-mcf-gem5-xcg.trace";
	string traceName = "02-stream-gem5-xaa.trace";
  //create the memory system
  Memory m;
  Cache l1;
  Cache l2;
  l1.SetLower(&l2);
  l2.SetLower(&m);
  InitialMemorySystem(m, l1, l2);

  int hit = 0, time = 0;
  char content[64];

  char operate_string;
  char addr_string[11];
  
	// Main access process
	// [in]  addr: access address
	// [in]  bytes: target number of bytes
	// [in]  read: 0|1 for write|read
	// [i|o] content: in|out data
	// [out] hit: 0|1 for miss|hit
	// [out] time: total access time

	//read the trace
	ifstream fin;
	for (int i = 0; i < 10; i++) {
		fin.open(traceName);
		cout << "running the " << i << "th cycles" << endl;
		while (fin >> operate_string >> addr_string)
		{
			bool read = (operate_string == 'r') ? true : false;
			long long addr;
			sscanf_s(addr_string, "0x%llx", &addr);
			l1.HandleRequest(addr, 4, read, content, hit, time);
		}
		fin.close();
	}
	
  printf("total cycle: %d\n", time);
  printf("total time: %0.2fms\n", time / 2000000.0);
  StorageStats stats_l1cache;
  StorageStats stats_l2cache;
  StorageStats stats_memory;
  l1.GetStats(stats_l1cache);
  l2.GetStats(stats_l2cache);
  m.GetStats(stats_memory);
  printf("memory cycle:%d\n", stats_memory.access_time);
  printf("l1cache cycle:%d\n", stats_l1cache.access_time);
  printf("l2cache cycle:%d\n", stats_l2cache.access_time);
  printf("access counter: %lld, l1_cache_hit_num: %d, l1_cache_miss_num: %d\n", stats_l1cache.access_counter, stats_l1cache.hit_num, stats_l1cache.miss_num);
  printf("access counter: %lld, l2_cache_hit_num: %d, l2_cache_miss_num: %d\n", stats_l2cache.access_counter, stats_l2cache.hit_num, stats_l2cache.miss_num);

  //double miss_rate = (1.0 * stats.miss_num) / stats.access_counter * 100;
  double l1_cache_miss_rate = 100 - (1.0 * stats_l1cache.hit_num) / stats_l1cache.access_counter * 100;
  double l2_cache_miss_rate = 100 - (1.0 * stats_l2cache.hit_num) / stats_l2cache.access_counter * 100;
  printf("l1 cache miss rate:%.3llf%%\n", l1_cache_miss_rate);
  printf("l2 cache miss rate:%.3llf%%\n", l2_cache_miss_rate);

  //printf("AMAT = %.3llfns\n", time / 2000000.0 / stats_l1cache.access_counter * 1000000);
  return 0;
}
