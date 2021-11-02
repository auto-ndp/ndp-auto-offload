#include <benchmark/benchmark.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/mman.h>
#include <sys/poll.h>
#include <thread>
#include <unistd.h>
#include <utility>

#define NOINLINE __attribute__((noinline))

void on_errno(const char *msg) {
  perror(msg);
  throw std::runtime_error(msg);
}

struct CpuTimes {
  uint64_t active, idle;
};

CpuTimes perf_cpu_times() {
  FILE *fp = fopen("/proc/stat", "rb");
  unsigned long long tUser{0}, tNice{0}, tSystem{0}, tIdle{0}, tIowait{0},
      tIrq{0}, tSoftIrq{0};
  fscanf(fp, "cpu %llu %llu %llu %llu %llu %llu %llu", &tUser, &tNice, &tSystem,
         &tIdle, &tIowait, &tIrq, &tSoftIrq);
  fclose(fp);
  uint64_t tActive = tUser + tNice + tSystem + tIrq + tSoftIrq;
  uint64_t tSumIdle = tIdle + tIowait;
  return (CpuTimes){.active = tActive, .idle = tSumIdle};
}

float cpu_utilization(CpuTimes pre, CpuTimes post) {
  uint64_t dActive = post.active - pre.active;
  uint64_t dIdle = post.idle - pre.idle;
  uint64_t dTotal = dActive + dIdle;
  double util = double(dActive) / double(dTotal);
  return float(util);
}

inline constexpr size_t WMEM_SIZE = 8ull * 1024ull * 1024ull * 1024ull;
// 4MiB initial memory size
inline constexpr size_t SNAP_SIZE = 4ull * 1024ull * 1024ull;
// Small function: allocates an extra megabyte during execution
inline constexpr size_t WORK_SIZE = 5ull * 1024ull * 1024ull;
// Doesn't write the first 3M of memory (e.g. unused thread stack)
inline constexpr size_t WORK_FILL_START = 3ull * 1024ull * 1024ull;

struct WMemory {
  std::byte *base = nullptr;
  size_t size = 0;

  WMemory() NOINLINE {
    base = (std::byte *)mmap(0x0, WMEM_SIZE, PROT_NONE,
                             MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (base == MAP_FAILED || base == nullptr) {
      base = nullptr;
      on_errno("MMap failed");
    }
    madvise((void *)base, WMEM_SIZE, MADV_HUGEPAGE);
  }

  ~WMemory() NOINLINE {
    if (base != nullptr) {
      int result = munmap((void *)base, WMEM_SIZE);
      base = nullptr;
      if (result < 0) {
        on_errno("Munmap failed");
      }
    }
  }

  std::span<std::byte, std::dynamic_extent> data() {
    return std::span(base, size);
  }

  const std::span<std::byte, std::dynamic_extent> data() const {
    return std::span(base, size);
  }

  void sideEffect() const NOINLINE {
    asm("");
    uint8_t v = 0;
    for (const auto byte : data()) {
      v += uint8_t(byte);
    }
    benchmark::DoNotOptimize(v);
  }

  void fillWithData(size_t from = 0, size_t upTo = WMEM_SIZE) NOINLINE {
    from = std::min(from, size);
    size_t len = std::min(upTo - from, size - from);
    auto sp = data().subspan(from, len);
    // fprintf(stderr, "fill %lx bytes\n", (long)sp.size());
    static std::atomic_uint8_t xinit = 0;
    uint8_t x = xinit.fetch_add(1, std::memory_order_acq_rel);
    for (auto &byte : sp) {
      byte = std::byte(x++);
    }
  }

  // ---- No protection at all
  void setupNoProtect() NOINLINE {
    resizeWavm(WMEM_SIZE / 2);
    fillWithData(0, WORK_SIZE);
  }

  void resizeNoProtect(size_t newSize) NOINLINE { size = newSize; }

  void restoreFromNoProtect(const WMemory &other) NOINLINE {
    resizeNoProtect(other.size);
    std::ranges::copy(other.data(), base);
  }

  // ---- WAVM-style implementation
  void resizeWavm(size_t newSize) NOINLINE {
    const size_t oldSize = size;
    if (newSize < oldSize) {
      void *result = mmap(base + newSize, oldSize - newSize, PROT_NONE,
                          MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
      madvise((void *)(base + newSize), oldSize - newSize, MADV_HUGEPAGE);
      if (result != (void *)(base + newSize)) {
        on_errno("Resize WAVM failed");
      }
    } else if (newSize > oldSize) {
      int result =
          mprotect(base + oldSize, newSize - oldSize, PROT_READ | PROT_WRITE);
      if (result < 0) {
        on_errno("MProtect resize WAVM failed");
      }
    }
    size = newSize;
  }

  void restoreFromWavm(const WMemory &other) NOINLINE {
    resizeWavm(other.size);
    std::ranges::copy(other.data(), base);
  }

  // ---- Don't need-style implementation: DONT_NEED removes pages without
  // taking an exclusive MM lock Optional use of mprotect to set pages back to
  // prot_none - to check the cost of those calls

  void resizeWavmDontneed(size_t newSize, bool useMprotect) NOINLINE {
    const size_t oldSize = size;
    if (newSize < oldSize) {
      int result =
          madvise((void *)(base + newSize), oldSize - newSize, MADV_DONTNEED);
      if (result < 0) {
        on_errno("Resize WAVM-Dontneed failed");
      }
      if (useMprotect) {
        result = mprotect(base + newSize, oldSize - newSize, PROT_NONE);
        if (result < 0) {
          on_errno("MProtect resize WAVM-Dontneed failed");
        }
      }
    } else if (newSize > oldSize) {
      int result =
          mprotect(base + oldSize, newSize - oldSize, PROT_READ | PROT_WRITE);
      if (result < 0) {
        on_errno("MProtect resize WAVM-Dontneed failed");
      }
    }
    size = newSize;
  }

  void restoreFromDontneed(const WMemory &other, bool useMprotect) NOINLINE {
    resizeWavmDontneed(other.size, useMprotect);
    std::ranges::copy(other.data(), base);
  }
};

static std::once_flag snapshotInitFlag;

const WMemory &getSnapshot() {
  static WMemory snapshot;
  std::call_once(
      snapshotInitFlag,
      [](WMemory &snapshot) {
        snapshot.resizeWavm(SNAP_SIZE);
        snapshot.fillWithData(0, SNAP_SIZE);
      },
      snapshot);
  return snapshot;
}

// Fastest possible way, no memory protection or touching bindings at all
void BM_NoProtection(benchmark::State &state) {
  const WMemory &snapshot = getSnapshot();
  WMemory func;
  func.setupNoProtect();
  CpuTimes ct_pre;
  if (state.thread_index() == 0) {
    ct_pre = perf_cpu_times();
  }
  for (auto _ : state) {
    func.restoreFromNoProtect(snapshot);
    func.sideEffect();
    func.fillWithData(WORK_FILL_START);
    func.sideEffect();
    func.resizeNoProtect(WORK_SIZE);
    func.sideEffect();
    func.fillWithData(WORK_FILL_START);
    func.sideEffect();
    benchmark::DoNotOptimize(func);
  }
  if (state.thread_index() == 0) {
    const CpuTimes ct_post = perf_cpu_times();
    const float utilization = cpu_utilization(ct_pre, ct_post);
    state.counters["CPU_Utilization"] = utilization;
    state.counters["CPU_Utilization_per_thread"] =
        utilization * std::thread::hardware_concurrency() / state.threads();
  }
}
BENCHMARK(BM_NoProtection)
    ->UseRealTime()
    ->Threads(1)
    ->Threads(8)
    ->Threads(16)
    ->Threads(24)
    ->Threads(32)
    ->Threads(64);

// Starting a function the slow way (no mapping reuse)
void BM_FullNewMapping(benchmark::State &state) {
  const WMemory &snapshot = getSnapshot();
  CpuTimes ct_pre;
  if (state.thread_index() == 0) {
    ct_pre = perf_cpu_times();
  }
  for (auto _ : state) {
    WMemory func;
    func.restoreFromWavm(snapshot);
    func.sideEffect();
    func.fillWithData(WORK_FILL_START);
    func.sideEffect();
    func.resizeWavm(WORK_SIZE);
    func.sideEffect();
    func.fillWithData(WORK_FILL_START);
    func.sideEffect();
    benchmark::DoNotOptimize(func);
  }
  if (state.thread_index() == 0) {
    const CpuTimes ct_post = perf_cpu_times();
    const float utilization = cpu_utilization(ct_pre, ct_post);
    state.counters["CPU_Utilization"] = utilization;
    state.counters["CPU_Utilization_per_thread"] =
        utilization * std::thread::hardware_concurrency() / state.threads();
  }
}
BENCHMARK(BM_FullNewMapping)
    ->UseRealTime()
    ->Threads(1)
    ->Threads(8)
    ->Threads(16)
    ->Threads(24)
    ->Threads(32)
    ->Threads(64);

// Starting a function the slow way, but reuse existing memory object
void BM_ReuseMapping(benchmark::State &state) {
  const WMemory &snapshot = getSnapshot();
  WMemory func;
  CpuTimes ct_pre;
  if (state.thread_index() == 0) {
    ct_pre = perf_cpu_times();
  }
  for (auto _ : state) {
    func.restoreFromWavm(snapshot);
    func.sideEffect();
    func.fillWithData(WORK_FILL_START);
    func.sideEffect();
    func.resizeWavm(WORK_SIZE);
    func.sideEffect();
    func.fillWithData(WORK_FILL_START);
    func.sideEffect();
    benchmark::DoNotOptimize(func);
  }
  if (state.thread_index() == 0) {
    const CpuTimes ct_post = perf_cpu_times();
    const float utilization = cpu_utilization(ct_pre, ct_post);
    state.counters["CPU_Utilization"] = utilization;
    state.counters["CPU_Utilization_per_thread"] =
        utilization * std::thread::hardware_concurrency() / state.threads();
  }
}
BENCHMARK(BM_ReuseMapping)
    ->UseRealTime()
    ->Threads(1)
    ->Threads(8)
    ->Threads(16)
    ->Threads(24)
    ->Threads(32)
    ->Threads(64);

// Starting a function the slow way, but reuse existing memory object
void BM_ReuseAndDontneed(benchmark::State &state) {
  const WMemory &snapshot = getSnapshot();
  const bool useMprotect = (state.range(0) > 0);
  WMemory func;
  CpuTimes ct_pre;
  if (state.thread_index() == 0) {
    ct_pre = perf_cpu_times();
  }
  for (auto _ : state) {
    func.restoreFromDontneed(snapshot, useMprotect);
    func.sideEffect();
    func.fillWithData(WORK_FILL_START);
    func.sideEffect();
    func.resizeWavmDontneed(WORK_SIZE, useMprotect);
    func.sideEffect();
    func.fillWithData(WORK_FILL_START);
    func.sideEffect();
    benchmark::DoNotOptimize(func);
  }
  if (state.thread_index() == 0) {
    const CpuTimes ct_post = perf_cpu_times();
    const float utilization = cpu_utilization(ct_pre, ct_post);
    state.counters["CPU_Utilization"] = utilization;
    state.counters["CPU_Utilization_per_thread"] =
        utilization * std::thread::hardware_concurrency() / state.threads();
  }
}
BENCHMARK(BM_ReuseAndDontneed)
    ->UseRealTime()
    ->Threads(1)
    ->Threads(8)
    ->Threads(16)
    ->Threads(24)
    ->Threads(32)
    ->Threads(64)
    ->DenseRange(0, 1)
    ->ArgName("use_mprotect");

BENCHMARK_MAIN();
