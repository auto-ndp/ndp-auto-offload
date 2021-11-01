#include <benchmark/benchmark.h>

#include <algorithm>
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

void on_errno(const char *msg) {
  perror(msg);
  throw std::runtime_error(msg);
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

  WMemory() {
    base = (std::byte *)mmap(0x0, WMEM_SIZE, PROT_NONE,
                             MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (base == MAP_FAILED || base == nullptr) {
      base = nullptr;
      on_errno("MMap failed");
    }
    madvise((void *)base, WMEM_SIZE, MADV_HUGEPAGE);
  }

  ~WMemory() {
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

  void fillWithData(size_t from = 0, size_t upTo = WMEM_SIZE) {
    upTo = (upTo > size) ? size : upTo;
    if (from >= upTo) {
      return;
    }
    std::ranges::fill(data().subspan(from, upTo - from),
                      std::byte(rand() % 0xFF));
  }

  // ---- No protection at all
  void setupNoProtect() { resizeWavm(WMEM_SIZE / 2); }

  void resizeNoProtect(size_t newSize) {
    const size_t oldSize = size;
    if (newSize < oldSize) {
      std::ranges::fill(data().subspan(newSize), std::byte(0x0));
    }
    size = newSize;
  }

  void restoreFromNoProtect(const WMemory &other) {
    resizeNoProtect(other.size);
    std::ranges::copy(other.data(), base);
  }

  // ---- WAVM-style implementation
  void resizeWavm(size_t newSize) {
    const size_t oldSize = size;
    newSize = ((newSize + 4095) / 4096) * 4096;
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
  }

  void restoreFromWavm(const WMemory &other) {
    resizeWavm(other.size);
    std::ranges::copy(other.data(), base);
  }

  // ---- Don't need-style implementation: DONT_NEED removes pages without
  // taking an exclusive MM lock Optional use of mprotect to set pages back to
  // prot_none - to check the cost of those calls

  void resizeWavmDontneed(size_t newSize, bool useMprotect) {
    const size_t oldSize = size;
    newSize = ((newSize + 4095) / 4096) * 4096;
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
  }

  void restoreFromDontneed(const WMemory &other, bool useMprotect) {
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
  for (auto _ : state) {
    func.restoreFromNoProtect(snapshot);
    benchmark::DoNotOptimize(func.base);
    func.fillWithData(WORK_FILL_START);
    benchmark::DoNotOptimize(func.base);
    func.resizeNoProtect(WORK_SIZE);
    benchmark::DoNotOptimize(func.base);
    func.fillWithData(WORK_FILL_START);
    benchmark::DoNotOptimize(func.base);
    benchmark::DoNotOptimize(func);
  }
}
BENCHMARK(BM_NoProtection)->UseRealTime()->ThreadRange(1, 64);

// Starting a function the slow way (no mapping reuse)
void BM_FullNewMapping(benchmark::State &state) {
  const WMemory &snapshot = getSnapshot();
  for (auto _ : state) {
    WMemory func;
    func.restoreFromWavm(snapshot);
    benchmark::DoNotOptimize(func.base);
    func.fillWithData(WORK_FILL_START);
    benchmark::DoNotOptimize(func.base);
    func.resizeWavm(WORK_SIZE);
    benchmark::DoNotOptimize(func.base);
    func.fillWithData(WORK_FILL_START);
    benchmark::DoNotOptimize(func.base);
    benchmark::DoNotOptimize(func);
  }
}
BENCHMARK(BM_FullNewMapping)->UseRealTime()->ThreadRange(1, 64);

// Starting a function the slow way, but reuse existing memory object
void BM_ReuseMapping(benchmark::State &state) {
  const WMemory &snapshot = getSnapshot();
  WMemory func;
  for (auto _ : state) {
    func.restoreFromWavm(snapshot);
    benchmark::DoNotOptimize(func.base);
    func.fillWithData(WORK_FILL_START);
    benchmark::DoNotOptimize(func.base);
    func.resizeWavm(WORK_SIZE);
    benchmark::DoNotOptimize(func.base);
    func.fillWithData(WORK_FILL_START);
    benchmark::DoNotOptimize(func.base);
    benchmark::DoNotOptimize(func);
  }
}
BENCHMARK(BM_ReuseMapping)->UseRealTime()->ThreadRange(1, 64);

// Starting a function the slow way, but reuse existing memory object
void BM_ReuseAndDontneed(benchmark::State &state) {
  const WMemory &snapshot = getSnapshot();
  const bool useMprotect = (state.range(0) > 0);
  WMemory func;
  for (auto _ : state) {
    func.restoreFromDontneed(snapshot, useMprotect);
    benchmark::DoNotOptimize(func.base);
    func.fillWithData(WORK_FILL_START);
    benchmark::DoNotOptimize(func.base);
    func.resizeWavmDontneed(WORK_SIZE, useMprotect);
    benchmark::DoNotOptimize(func.base);
    func.fillWithData(WORK_FILL_START);
    benchmark::DoNotOptimize(func.base);
    benchmark::DoNotOptimize(func);
  }
}
BENCHMARK(BM_ReuseAndDontneed)
    ->UseRealTime()
    ->ThreadRange(1, 64)
    ->DenseRange(0, 1)
    ->ArgName("use_mprotect");

BENCHMARK_MAIN();
