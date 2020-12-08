#include "memspeed.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <memory>
#include <random>

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
