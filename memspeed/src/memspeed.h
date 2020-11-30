#pragma once

#include <ios>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
#include <ctime>

class MemspeedBenchmark {
public:
  inline virtual ~MemspeedBenchmark() {}
  virtual std::string name() = 0;
  virtual void parse_arg(std::string_view arg) = 0;
  virtual void print_help() = 0;
  virtual void prepare(std::ostream &log) = 0;
  virtual void run(std::ostream &log) = 0;
};

using benchmark_ptr_t = std::shared_ptr<MemspeedBenchmark>;

std::vector<benchmark_ptr_t> memspeed_benchmarks();

inline size_t parseSize(std::string szstr) {
  size_t raw_size = static_cast<size_t>(strtoull(szstr.c_str(), nullptr, 0));
  // look for prefixes
  if (szstr.ends_with('k') || szstr.ends_with('K')) {
    raw_size *= size_t(1024);
  } else if (szstr.ends_with('m') || szstr.ends_with('M')) {
    raw_size *= size_t(1024) * 1024;
  } else if (szstr.ends_with('g') || szstr.ends_with('G')) {
    raw_size *= size_t(1024) * 1024 * 1024;
  }
  return raw_size;
}

inline std::string formatSize(size_t sz) {
  double szval = static_cast<double>(sz);
  char sfx = 'b';
  if (szval > 4096.0) {
    szval /= 1024.0;
    sfx = 'k';
  }
  if (szval > 4096.0) {
    szval /= 1024.0;
    sfx = 'M';
  }
  if (szval > 4096.0) {
    szval /= 1024.0;
    sfx = 'G';
  }
  char obuf[32];
  int len = snprintf(obuf, sizeof(obuf), "%.3lf%c", szval, sfx);
  return (len <= 0) ? std::string("error") : std::string(obuf, size_t(len));
}

class TimingAccumulator {
public:
  //
};

inline void full_mem_fence() {
  __sync_synchronize();
}

inline void reset_cpu_tsc() {
  timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 0;
  clock_settime(CLOCK_MONOTONIC, &ts);
}

inline uint64_t get_cpu_tsc() {
  //uint32_t aux;
  //return __rdtscp(&aux);
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<uint64_t>(ts.tv_nsec) + 1000000000ULL * static_cast<uint64_t>(ts.tv_sec);
}
