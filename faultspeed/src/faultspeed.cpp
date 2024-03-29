#include <benchmark/benchmark.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <mutex>
#include <ranges>
#include <shared_mutex>
#include <signal.h>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/mman.h>
#include <sys/poll.h>
#include <thread>
#include <utility>

// Userfaultfd
#include <fcntl.h>
#include <linux/userfaultfd.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>

struct UserfaultFd {
  int fd = -1;
  uffdio_api api = {};

  // Only allow moving, not copying - owns the fd
  UserfaultFd() {}
  ~UserfaultFd() { clear(); }
  UserfaultFd(const UserfaultFd &) = delete;
  UserfaultFd(UserfaultFd &&other) { this->operator=(std::move(other)); }
  UserfaultFd &operator=(const UserfaultFd &) = delete;
  UserfaultFd &operator=(UserfaultFd &&other) {
    fd = other.fd;
    api = other.api;
    other.fd = -1;
    other.api = {};
    return *this;
  }

  // Close fd if present
  void clear() {
    if (fd >= 0) {
      close(fd);
      fd = -1;
      api = {};
    }
  }

  // Release ownership and return the fd
  std::pair<int, uffdio_api> release() {
    int oldFd = fd;
    uffdio_api oldApi = api;
    fd = -1;
    api = {};
    return std::make_pair(oldFd, oldApi);
  }

  void create(int flags = 0, bool sigbus = false) {
    clear();
    int result = syscall(SYS_userfaultfd, flags);
    if (result < 0) {
      errno = -result;
      perror("Error creating userfaultfd");
      throw std::runtime_error("Couldn't create userfaultfd");
    }
    fd = result;
    api.api = UFFD_API;
    api.features = UFFD_FEATURE_THREAD_ID;
    if (sigbus) {
      api.features |= UFFD_FEATURE_SIGBUS;
    }
    api.ioctls = 0;
    result = ioctl(fd, UFFDIO_API, &api);
    if (result < 0) {
      throw std::runtime_error("Couldn't handshake userfaultfd api");
    }
  }

  // Thread-safe
  void checkFd() {
    if (fd < 0) {
      throw std::runtime_error("UFFD fd not initialized");
    }
  }

  // Thread-safe
  // Write-protect mode requires at least Linux 5.7 kernel
  void register_address_range(size_t startPtr, size_t length, bool modeMissing,
                              bool modeWriteProtect) {
    checkFd();
    uffdio_register r = {};
    if (!(modeMissing || modeWriteProtect)) {
      throw std::invalid_argument(
          "UFFD register call must have at least one mode enabled");
    }
    if (modeMissing) {
      r.mode |= UFFDIO_REGISTER_MODE_MISSING;
    }
    if (modeWriteProtect) {
      if ((api.features & UFFD_FEATURE_PAGEFAULT_FLAG_WP) == 0) {
        throw std::runtime_error("WriteProtect mode on UFFD not supported");
      }
      r.mode |= UFFDIO_REGISTER_MODE_WP;
    }
    r.range.start = startPtr;
    r.range.len = length;
    if (ioctl(fd, UFFDIO_REGISTER, &r) < 0) {
      perror("UFFDIO_REGISTER error");
      throw std::runtime_error("Couldn't register an address range with UFFD");
    }
  }

  // Thread-safe
  void unregister_address_range(size_t startPtr, size_t length) {
    checkFd();
    uffdio_range r = {};
    r.start = startPtr;
    r.len = length;
    if (ioctl(fd, UFFDIO_UNREGISTER, &r) < 0) {
      perror("UFFDIO_UNREGISTER error");
      throw std::runtime_error(
          "Couldn't unregister an address range from UFFD");
    }
  }

  // Thread-safe
  std::optional<uffd_msg> readEvent() {
    checkFd();
    uffd_msg ev;
  retry:
    int result = read(fd, (void *)&ev, sizeof(uffd_msg));
    if (result < 0) {
      if (errno == EAGAIN) {
        goto retry;
      } else if (errno == EWOULDBLOCK) {
        return std::nullopt;
      } else {
        perror("read from UFFD error");
        throw std::runtime_error("Error reading from the UFFD");
      }
    }
    return ev;
  }

  // Thread-safe
  void writeProtectPages(size_t startPtr, size_t length,
                         bool preventWrites = true, bool dontWake = false) {
    checkFd();
    if ((api.features & UFFD_FEATURE_PAGEFAULT_FLAG_WP) == 0) {
      throw std::runtime_error(
          "Write-protect pages not supported by UFFD on this kernel version");
    }
    uffdio_writeprotect wp = {};
    if (preventWrites) {
      wp.mode |= UFFDIO_WRITEPROTECT_MODE_WP;
    }
    if (dontWake) {
      wp.mode |= UFFDIO_WRITEPROTECT_MODE_DONTWAKE;
    }
    wp.range.start = startPtr;
    wp.range.len = length;
  retry:
    if (ioctl(fd, UFFDIO_WRITEPROTECT, &wp) < 0) {
      if (errno == EAGAIN) {
        goto retry;
      }
      perror("UFFDIO_WRITEPROTECT error");
      throw std::runtime_error(
          "Couldn't write-protect-modify an address range through UFFD");
    }
  }

  // Thread-safe
  void zeroPages(size_t startPtr, size_t length, bool dontWake = false) {
    checkFd();
    uffdio_zeropage zp = {};
    if (dontWake) {
      zp.mode |= UFFDIO_ZEROPAGE_MODE_DONTWAKE;
    }
    zp.range.start = startPtr;
    zp.range.len = length;
  retry:
    if (ioctl(fd, UFFDIO_ZEROPAGE, &zp) < 0) {
      if (errno == EAGAIN) {
        goto retry;
      }
      perror("UFFDIO_ZEROPAGE error");
      throw std::runtime_error(
          "Couldn't zero-page an address range through UFFD");
    }
  }

  // Thread-safe
  void copyPages(size_t targetStartPtr, size_t length, size_t sourceStartPtr,
                 bool writeProtect = false, bool dontWake = false) {
    checkFd();
    uffdio_copy cp = {};
    if (dontWake) {
      cp.mode |= UFFDIO_COPY_MODE_DONTWAKE;
    }
    if (writeProtect) {
      cp.mode |= UFFDIO_COPY_MODE_WP;
    }
    cp.src = sourceStartPtr;
    cp.len = length;
    cp.dst = targetStartPtr;
  retry:
    if (ioctl(fd, UFFDIO_COPY, &cp) < 0) {
      if (errno == EAGAIN) {
        goto retry;
      }
      perror("UFFDIO_COPY error");
      throw std::runtime_error("Couldn't copy an address range through UFFD");
    }
  }

  // Thread-safe
  void wakePages(size_t startPtr, size_t length) {
    checkFd();
    uffdio_range wr = {};
    wr.start = startPtr;
    wr.len = length;
  retry:
    if (ioctl(fd, UFFDIO_WAKE, &wr) < 0) {
      if (errno == EAGAIN) {
        goto retry;
      }
      perror("UFFDIO_WAKE error");
      throw std::runtime_error("Couldn't wake an address range through UFFD");
    }
  }
};

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

  // ---- UFFD-style implementation
  void resizeWavmUffd(size_t newSize) NOINLINE {
    const size_t oldSize = size;
    if (newSize < oldSize) {
      int result =
          madvise((void *)(base + newSize), oldSize - newSize, MADV_DONTNEED);
      if (result < 0) {
        on_errno("Resize WAVM-Uffd failed");
      }
    }
    size = newSize;
  }

  void restoreFromUffd(const WMemory &other) NOINLINE {
    resizeWavmUffd(other.size);
    std::ranges::copy(other.data(), base);
  }
};

struct UffdHandler;
static UffdHandler *lastUffdH;

struct UffdHandler {
  UserfaultFd uffd;
  int threadCount;
  size_t pageSize;
  std::shared_mutex mappingsMx;
  WMemory const *snapshot = nullptr;
  std::vector<std::pair<size_t, WMemory *>> mappings;
  std::atomic_uint64_t faults = 0;

  explicit UffdHandler(int threads) : threadCount(threads) {
    fprintf(stderr, "Starting UFFD handler\n");
    mappings.reserve(128);
    uffd.create(O_CLOEXEC, threads < 1);
    pageSize = sysconf(_SC_PAGESIZE);
    for (int i = 0; i < threads; i++) {
      std::thread([&]() { this->workerThread(); }).detach();
    }
    lastUffdH = this;
  }

  void registerSigbusHandler() {
    struct sigaction action;
    action.sa_flags = SA_RESTART | SA_SIGINFO;
    action.sa_handler = nullptr;
    action.sa_sigaction = &sigbusHandler;
    sigemptyset(&action.sa_mask);
    lastUffdH = this;
    int result = sigaction(SIGBUS, &action, nullptr);
    if (result < 0) {
      perror("Couldn't register SIGBUS handler");
      throw std::runtime_error("Couldn't register SIGBUS handler");
    }
  }

  // The SIGBUS can only occur when accessing WASM memory, so async-signal
  // safety is not as important
  static void sigbusHandler(int code, siginfo_t *siginfo, void *contextR) {
    ucontext_t *context = (ucontext_t *)contextR;
    if (code == SIGBUS) {
      uffd_msg m = {};
      m.event = UFFD_EVENT_PAGEFAULT;
      m.arg.pagefault = {.flags = UFFD_PAGEFAULT_FLAG_WRITE,
                         .address = (size_t)siginfo->si_addr,
                         .feat = {.ptid = (unsigned)gettid()}};
      lastUffdH->handleOne(m);
    } else {
      std::terminate();
    }
  }

  void handleOne(uffd_msg m) {
    if (m.event != UFFD_EVENT_PAGEFAULT) {
      fprintf(stderr,
              "Caught event of type %d which is not UFFD_EVENT_PAGEFAULT\n",
              int(m.event));
      std::terminate();
    }
    const uint32_t tid = m.arg.pagefault.feat.ptid;
    const uint64_t flags = m.arg.pagefault.flags;
    // align to page boundary
    const uint64_t address = m.arg.pagefault.address & ~(pageSize - 1);
    // fprintf(stderr, "PF %zx\n", size_t(address));
    std::shared_lock<std::shared_mutex> lock(mappingsMx);
    const auto it = std::ranges::find_if(
        mappings,
        [address](size_t base) {
          return (address >= base) && (address < base + WMEM_SIZE);
        },
        [](const auto &p) { return p.first; });
    if (it == mappings.end()) {
      fprintf(stderr,
              "[1] Couldn't find mapping corresponding to address %llu\n",
              static_cast<unsigned long long>(address));
      std::terminate();
    }
    const WMemory *wmem = it->second;
    lock.unlock();
    const size_t wmem_base = size_t(wmem->base);
    if (address < wmem_base || address >= wmem_base + WMEM_SIZE) {
      fprintf(stderr,
              "[2] Couldn't find mapping corresponding to address %llu\n",
              static_cast<unsigned long long>(address));
      std::terminate();
    }
    // const bool writePf = flags & UFFD_PAGEFAULT_FLAG_WRITE;
    const bool writeProtectionChangePf = flags & UFFD_PAGEFAULT_FLAG_WP;
    const bool inBounds = m.arg.pagefault.address < wmem_base + wmem->size;
    if (!inBounds) {
      kill(tid, SIGSEGV);
      uffd.wakePages(address, pageSize);
      return;
    }
    faults.fetch_add(1, std::memory_order_acq_rel);
    if (writeProtectionChangePf) {
      // dirty page
      uffd.writeProtectPages(address, pageSize, false, false);
      return;
    }
    if (snapshot != nullptr && address < snapshot->size) {
      const size_t snapshot_base = size_t(snapshot->base);
      uffd.copyPages(address, pageSize, snapshot_base, false, false);
      // writePf -> dirty page
      return;
    } else {
      // dirty page
      uffd.zeroPages(address, pageSize, false);
      return;
    }
  }

  void workerThread() {
    try {
      while (true) {
        uffd_msg m = uffd.readEvent().value();
        handleOne(m);
      }
    } catch (std::exception &e) {
      fprintf(stderr, "Caught exception in UFFD worker thred: %s\n", e.what());
    }
  }

  void setSnapshot(WMemory const *wmem) {
    std::unique_lock<std::shared_mutex> _l(mappingsMx);
    snapshot = wmem;
  }

  void add(WMemory &wmem) {
    size_t addr = size_t(wmem.base);
    {
      std::unique_lock<std::shared_mutex> _l(mappingsMx);
      mappings.push_back(std::make_pair(addr, &wmem));
    }
    if (mprotect(wmem.base, WMEM_SIZE / 2, PROT_READ | PROT_WRITE) < 0) {
      perror("Mprotect error when adding UFFD range");
      throw std::runtime_error(
          "Couldn't unprotect memory range when enabling UFFD");
    }
    uffd.register_address_range(addr, WMEM_SIZE, true, false);
  }

  void remove(WMemory &wmem) {
    size_t addr = size_t(wmem.base);
    uffd.unregister_address_range(addr, WMEM_SIZE);
    {
      std::unique_lock<std::shared_mutex> _l(mappingsMx);
      auto it =
          std::ranges::find(mappings, addr, [](auto &p) { return p.first; });
      if (it == mappings.end()) {
        throw std::runtime_error("WMemory already missing from UFFD");
      }
      mappings.erase(it);
    }
  }
};

std::mutex uffdHandlerMutex;

std::unique_ptr<UffdHandler> &handlerPtrStorage() {
  static std::unique_ptr<UffdHandler> uffdHandler{nullptr};
  return uffdHandler;
}

UffdHandler &uffdHandler() {
  std::unique_lock<std::mutex> _l{uffdHandlerMutex};
  auto &hps = handlerPtrStorage();
  if (hps == nullptr || hps->threadCount < 1) {
    hps = nullptr;
    hps = std::make_unique<UffdHandler>(
        (int)std::thread::hardware_concurrency() / 2);
  }
  return *hps;
}

UffdHandler &uffdSigbusHandler() {
  std::unique_lock<std::mutex> _l{uffdHandlerMutex};
  auto &hps = handlerPtrStorage();
  if (hps == nullptr || hps->threadCount > 0) {
    hps = nullptr;
    hps = std::make_unique<UffdHandler>(-1);
  }
  return *hps;
}

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

#define FAULTSPEED_COMMON_CONFIG                                               \
  ->UseRealTime()->Threads(1)->Threads(8)->Threads(16)->Threads(24)->Threads(64)

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
FAULTSPEED_COMMON_CONFIG;

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
FAULTSPEED_COMMON_CONFIG;

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
FAULTSPEED_COMMON_CONFIG;

// Use madv_dontneed for lower kernel lock contention
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
FAULTSPEED_COMMON_CONFIG->DenseRange(0, 1)->ArgName("use_mprotect");

// UFFD handler, pre-filling the memory with data proactively
void BM_UFFD_Eager(benchmark::State &state) {
  const WMemory &snapshot = getSnapshot();
  WMemory func;
  CpuTimes ct_pre;
  if (state.thread_index() == 0) {
    uffdHandler().setSnapshot(nullptr); // use zero-page filling
    uffdHandler().faults.store(0);
    ct_pre = perf_cpu_times();
  }
  uffdHandler().add(func);
  for (auto _ : state) {
    func.restoreFromUffd(snapshot);
    func.sideEffect();
    func.fillWithData(WORK_FILL_START);
    func.sideEffect();
    func.resizeWavmUffd(WORK_SIZE);
    func.sideEffect();
    func.fillWithData(WORK_FILL_START);
    func.sideEffect();
    benchmark::DoNotOptimize(func);
  }
  if (state.thread_index() == 0) {
    const CpuTimes ct_post = perf_cpu_times();
    const float utilization = cpu_utilization(ct_pre, ct_post);
    state.counters["UFFD_Faults"] = (uffdHandler().faults.load());
    state.counters["CPU_Utilization"] = utilization;
    state.counters["CPU_Utilization_per_thread"] =
        utilization * std::thread::hardware_concurrency() / state.threads();
  }
  uffdHandler().remove(func);
}
BENCHMARK(BM_UFFD_Eager)
FAULTSPEED_COMMON_CONFIG;

// UFFD handler, filling the memory with snapshot data as-needed
void BM_UFFD_Lazy(benchmark::State &state) {
  const WMemory &snapshot = getSnapshot();
  WMemory func;
  CpuTimes ct_pre;
  if (state.thread_index() == 0) {
    uffdHandler().setSnapshot(&snapshot);
    uffdHandler().faults.store(0);
    ct_pre = perf_cpu_times();
  }
  uffdHandler().add(func);
  for (auto _ : state) {
    func.resizeWavmUffd(0);
    func.sideEffect();
    func.resizeWavmUffd(SNAP_SIZE);
    func.sideEffect();
    func.fillWithData(WORK_FILL_START);
    func.sideEffect();
    func.resizeWavmUffd(WORK_SIZE);
    func.sideEffect();
    func.fillWithData(WORK_FILL_START);
    func.sideEffect();
    benchmark::DoNotOptimize(func);
  }
  if (state.thread_index() == 0) {
    const CpuTimes ct_post = perf_cpu_times();
    const float utilization = cpu_utilization(ct_pre, ct_post);
    state.counters["UFFD_Faults"] = (uffdHandler().faults.load());
    state.counters["CPU_Utilization"] = utilization;
    state.counters["CPU_Utilization_per_thread"] =
        utilization * std::thread::hardware_concurrency() / state.threads();
  }
  uffdHandler().remove(func);
}
BENCHMARK(BM_UFFD_Lazy)
FAULTSPEED_COMMON_CONFIG;

// UFFD handler, pre-filling the memory with data proactively
void BM_UFFD_SIGBUS_Eager(benchmark::State &state) {
  const WMemory &snapshot = getSnapshot();
  WMemory func;
  CpuTimes ct_pre;
  if (state.thread_index() == 0) {
    uffdSigbusHandler().setSnapshot(nullptr); // use zero-page filling
    uffdSigbusHandler().faults.store(0);
    ct_pre = perf_cpu_times();
  }
  uffdSigbusHandler().add(func);
  uffdSigbusHandler().registerSigbusHandler();
  for (auto _ : state) {
    func.restoreFromUffd(snapshot);
    func.sideEffect();
    func.fillWithData(WORK_FILL_START);
    func.sideEffect();
    func.resizeWavmUffd(WORK_SIZE);
    func.sideEffect();
    func.fillWithData(WORK_FILL_START);
    func.sideEffect();
    benchmark::DoNotOptimize(func);
  }
  if (state.thread_index() == 0) {
    const CpuTimes ct_post = perf_cpu_times();
    const float utilization = cpu_utilization(ct_pre, ct_post);
    state.counters["UFFD_Faults"] = (uffdSigbusHandler().faults.load());
    state.counters["CPU_Utilization"] = utilization;
    state.counters["CPU_Utilization_per_thread"] =
        utilization * std::thread::hardware_concurrency() / state.threads();
  }
  uffdSigbusHandler().remove(func);
}
BENCHMARK(BM_UFFD_SIGBUS_Eager)
FAULTSPEED_COMMON_CONFIG;

// UFFD handler, filling the memory with snapshot data as-needed
void BM_UFFD_SIGBUS_Lazy(benchmark::State &state) {
  const WMemory &snapshot = getSnapshot();
  WMemory func;
  CpuTimes ct_pre;
  if (state.thread_index() == 0) {
    uffdSigbusHandler().setSnapshot(&snapshot);
    uffdSigbusHandler().faults.store(0);
    ct_pre = perf_cpu_times();
  }
  uffdSigbusHandler().add(func);
  for (auto _ : state) {
    func.resizeWavmUffd(0);
    func.sideEffect();
    func.resizeWavmUffd(SNAP_SIZE);
    func.sideEffect();
    func.fillWithData(WORK_FILL_START);
    func.sideEffect();
    func.resizeWavmUffd(WORK_SIZE);
    func.sideEffect();
    func.fillWithData(WORK_FILL_START);
    func.sideEffect();
    benchmark::DoNotOptimize(func);
  }
  if (state.thread_index() == 0) {
    const CpuTimes ct_post = perf_cpu_times();
    const float utilization = cpu_utilization(ct_pre, ct_post);
    state.counters["UFFD_Faults"] = (uffdSigbusHandler().faults.load());
    state.counters["CPU_Utilization"] = utilization;
    state.counters["CPU_Utilization_per_thread"] =
        utilization * std::thread::hardware_concurrency() / state.threads();
  }
  uffdSigbusHandler().remove(func);
}
BENCHMARK(BM_UFFD_SIGBUS_Lazy)
FAULTSPEED_COMMON_CONFIG;

BENCHMARK_MAIN();
