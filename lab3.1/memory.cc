#include "memory.h"

void Memory::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content, int &hit, int &time) {
  time += latency_.hit_latency + latency_.bus_latency;
  stats_.access_time += latency_.hit_latency + latency_.bus_latency;;
}

