#include "memspeed.h"

std::shared_ptr<MemspeedBenchmark> make_cpuonly_benchmark();

std::vector<std::shared_ptr<MemspeedBenchmark>> memspeed_benchmarks() {
  std::vector<std::shared_ptr<MemspeedBenchmark>> out_vec;
  out_vec.push_back(make_cpuonly_benchmark());
  return out_vec;
}
