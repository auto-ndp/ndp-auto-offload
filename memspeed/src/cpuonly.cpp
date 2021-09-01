#include "memspeed.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <memory>
#include <random>

#include <sys/mman.h>
#include <unistd.h>

#include <benchmark/benchmark.h>

static void BM_cpu_memcopy(benchmark::State &state) {
  size_t len_arg = size_t(state.range(0));
  MmapArray<uint32_t> src{len_arg}, dst{len_arg};
  std::minstd_rand rnd; // fast RNG
  rnd.seed(2817398);
  std::generate_n(src.data(), src.size(), rnd);
  std::generate_n(dst.data(), dst.size(), rnd);
  for (auto _ : state) {
    std::copy_n(src.data(), src.size(), dst.data());
    benchmark::DoNotOptimize(src.data());
    benchmark::DoNotOptimize(dst.data());
    benchmark::ClobberMemory();
  }
  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(src.size_in_bytes()));
  state.counters["memused"] = benchmark::Counter(
      2.0 * double(src.size_in_bytes()), benchmark::Counter::Flags::kDefaults,
      benchmark::Counter::kIs1024);
}
BENCHMARK(BM_cpu_memcopy)->RangeMultiplier(2)->Range(2, 1 * 1024 * 1024 * 1024);

static void BM_cpu_wasmmemloop(benchmark::State &state) {
  size_t g4 = 1024 * 1024 * 1024; // uint32_t takes 4 bytes -> 4GB
  int madv_arg = int(state.range(0));
  bool thp_arg = int(state.range(1)) == 1;
  bool mmapfixed_arg = int(state.range(2)) == 1;
  size_t len_arg = size_t(state.range(3));
  MmapArray<uint32_t> src{g4, false};
  if (thp_arg) {
    madvise(src.data(), src.size_in_bytes(), MADV_HUGEPAGE);
  }
  std::minstd_rand rnd; // fast RNG
  rnd.seed(2817398);
  std::generate_n(src.data(), len_arg, rnd);
  switch (madv_arg) {
  case 0:
    break;
  case 1:
    madv_arg = MADV_SEQUENTIAL;
    break;
  case 2:
    madv_arg = MADV_WILLNEED;
    break;
  default:
    assert(0);
  }
  for (auto _ : state) {
    {
      MmapArray<uint32_t> dst{g4, false};
      if (mmapfixed_arg) {
        void *mm =
            mmap(dst.data(), 4 * len_arg, PROT_READ | PROT_WRITE,
                 MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE, -1, 0);
        if (mm != dst.data()) {
          perror("Couldn't rewrite page mappings");
          exit(1);
        }
      }
      if (thp_arg) {
        madvise(dst.data(), dst.size_in_bytes(), MADV_HUGEPAGE);
      }
      if (madv_arg > 0) {
        madvise(dst.data(), 4 * len_arg,
                (madv_arg == 1) ? MADV_SEQUENTIAL : MADV_WILLNEED);
      }
      std::copy_n(src.data(), len_arg, dst.data());
      benchmark::DoNotOptimize(src.data());
      benchmark::DoNotOptimize(dst.data());
      benchmark::ClobberMemory();
    }
  }
  state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(4 * len_arg));
  state.counters["memused"] = benchmark::Counter(
      double(4 * len_arg), benchmark::Counter::Flags::kDefaults,
      benchmark::Counter::kIs1024);
}
BENCHMARK(BM_cpu_wasmmemloop)
    ->RangeMultiplier(2)
    ->Ranges({{0, 2}, {0, 1}, {0, 1}, {1024, 1 * 1024 * 1024 * 1024}});

static void BM_cpu_memread_linear(benchmark::State &state) {
  size_t len_arg = size_t(state.range(0));
  MmapArray<uint32_t> src{len_arg};
  std::minstd_rand rnd; // fast RNG
  rnd.seed(2817398);
  std::generate_n(src.data(), src.size(), rnd);
  for (auto _ : state) {
    for (size_t i = 0; i < src.size(); i++) {
      uint32_t val = src[i];
      benchmark::DoNotOptimize(val);
    }
    benchmark::ClobberMemory();
  }
  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(src.size_in_bytes()));
  state.counters["memused"] = benchmark::Counter(
      double(src.size_in_bytes()), benchmark::Counter::Flags::kDefaults,
      benchmark::Counter::kIs1024);
}
BENCHMARK(BM_cpu_memread_linear)
    ->RangeMultiplier(2)
    ->Range(2, 1 * 1024 * 1024 * 1024);

static void BM_cpu_memread_listwalk(benchmark::State &state) {
  size_t len_arg = size_t(state.range(0));
  MmapArray<uint32_t> src{len_arg};
  std::minstd_rand rnd; // fast RNG
  rnd.seed(2817398);
  size_t len_mask = len_arg - 1;
  std::generate_n(src.data(), src.size(), rnd);
  for (auto _ : state) {
    state.PauseTiming();
    size_t idx = rnd() & len_mask;
    state.ResumeTiming();
    for (size_t i = 0; i < src.size(); i++) {
      idx = src[i] & len_mask;
      benchmark::DoNotOptimize(idx);
    }
    benchmark::ClobberMemory();
  }
  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(src.size_in_bytes()));
  state.counters["memused"] = benchmark::Counter(
      double(src.size_in_bytes()), benchmark::Counter::Flags::kDefaults,
      benchmark::Counter::kIs1024);
}
BENCHMARK(BM_cpu_memread_listwalk)
    ->RangeMultiplier(2)
    ->Range(2, 1 * 1024 * 1024 * 1024);
