#include "memspeed.h"

std::shared_ptr<MemspeedBenchmark> make_cpuonly_benchmark();

#ifdef ENABLE_UPMEM_DPU
std::shared_ptr<MemspeedBenchmark> make_upmemdpu_benchmark();
#endif

std::vector<std::shared_ptr<MemspeedBenchmark>> memspeed_benchmarks() {
  std::vector<std::shared_ptr<MemspeedBenchmark>> out_vec;
  out_vec.push_back(make_cpuonly_benchmark());
#ifdef ENABLE_UPMEM_DPU
  out_vec.push_back(make_upmemdpu_benchmark());
#endif
  return out_vec;
}
