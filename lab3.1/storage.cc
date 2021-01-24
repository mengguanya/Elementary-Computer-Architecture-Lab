#include"storage.h"

Storage::Storage() {
  /*
  initial the latency
  */
  latency_.bus_latency = 0;
  latency_.hit_latency = 0;
  /*
  initial the statistical data
  */
  stats_.access_counter = 0;
  stats_.access_time = 0;
  stats_.fetch_num = 0;
  stats_.miss_num = 0;
  stats_.prefetch_num = 0;
  stats_.replace_num = 0;
}