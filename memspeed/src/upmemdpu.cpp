#include "memspeed.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <memory>
#include <random>
#include <sys/mman.h>

extern "C" {
#include "../include/dpuramcopy.h"
#include <dpu.h>
}

#include <benchmark/benchmark.h>

using namespace std::string_literals;
using namespace std::chrono;

constexpr size_t WRAM_TOTAL_BYTES = 64 * 1024;
constexpr size_t IRAM_TOTAL_BYTES = 24 * 1024;
constexpr size_t MRAM_TOTAL_BYTES = 64 * 1024 * 1024;

class UpmemDpuBenchmark : public MemspeedBenchmark {
public:
  uint32_t *cpu_buffer = nullptr;

  virtual ~UpmemDpuBenchmark() {
    if (this->cpu_buffer != nullptr) {
      munmap(this->cpu_buffer, this->buffer_size);
      this->cpu_buffer = nullptr;
    }
    if (this->dpu_count > 0) {
      DPU_ASSERT(dpu_free(this->dpuset));
    }
  }

  std::string name() final { return "upmem"s; }
  virtual void parse_arg(std::string_view arg) final {
    if (std::string_view pfx = "-cpusize="; starts_with(arg, pfx)) {
      std::string_view argval = arg.substr(pfx.size());
      this->buffer_size = parseSize(std::string(argval)) / 8 * 8;
    }
    if (std::string_view pfx = "-minsize="; starts_with(arg, pfx)) {
      std::string_view argval = arg.substr(pfx.size());
      this->min_size = parseSize(std::string(argval)) / 8 * 8;
    }
    if (std::string_view pfx = "-dpus="; starts_with(arg, pfx)) {
      std::string_view argval = arg.substr(pfx.size());
      this->dpu_count = static_cast<uint32_t>(parseSize(std::string(argval)));
    }
  }
  virtual void print_help() final {
    fprintf(stderr,
            " -cpusize=4M - specifies maximum CPU buffer size for speed tests\n"
            " -minsize=64 - specifies the first buffer size tested\n"
            " -dpus=0     - number of DPUs to allocate (0 for all)");
  }
  virtual void prepare(std::ostream &log) final {
    auto tstart = steady_clock::now();
    DPU_ASSERT(
        dpu_alloc((this->dpu_count == 0) ? DPU_ALLOCATE_ALL : this->dpu_count,
                  nullptr, &this->dpuset));
    DPU_ASSERT(dpu_get_nr_dpus(this->dpuset, &this->dpu_count));
    auto tend = steady_clock::now();
    log << "Allocated " << this->dpu_count << " DPUs in "
        << (duration_cast<microseconds>(tend - tstart)).count() << "us"
        << std::endl;
    tstart = steady_clock::now();
    DPU_ASSERT(dpu_load(this->dpuset, "../dpusrc/ramcopy.dpuelf", nullptr));
    tend = steady_clock::now();
    log << "Loaded DPU program in "
        << (duration_cast<microseconds>(tend - tstart)).count() << "us"
        << std::endl;

    log << "Allocating cpu buffer of size: " << formatSize(this->buffer_size)
        << " bytes" << std::endl;
    cpu_buffer = static_cast<uint32_t *>(
        mmap(nullptr,
             std::max(this->buffer_size,
                      this->dpu_count * MRAM_BUFFER_DWORDS * sizeof(uint32_t)),
             PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE,
             -1, 0));
    if (cpu_buffer == MAP_FAILED || cpu_buffer == nullptr) {
      perror("Couldn't allocate cpu_buffer");
      exit(1);
    }
    log << "Scrambling cpu buffer" << std::endl;
    this->scramble_buffers();
  }
  virtual void run(std::ostream &log) final {
    assert(cpu_buffer != nullptr);
    assert(dpu_count > 0);
    // CPU -> WRAM copy bench
    log << "CPU->WRAM copy of " << WRAM_BUFFER_DWORDS
        << " uint32_ts nanosecond times:" << std::endl;
    for (size_t run_size = 2; run_size <= WRAM_BUFFER_DWORDS; run_size *= 2) {
      log << run_size;
      for (int _try = 0; _try < 8; _try++) {
        uint64_t start_time = 0, end_time = 0, delta = 0;
        full_mem_fence();
        reset_cpu_tsc();
        full_mem_fence();
        start_time = get_cpu_tsc();
        full_mem_fence();
        DPU_ASSERT(dpu_broadcast_to(this->dpuset, "wram_buffer_a", 0,
                                    cpu_buffer, run_size * sizeof(uint32_t),
                                    DPU_XFER_DEFAULT));
        full_mem_fence();
        end_time = get_cpu_tsc();
        full_mem_fence();
        delta = end_time - start_time;
        log << "," << delta;
      }
      log << std::endl;
    }
    log << "\n\n";
    // CPU -> broadcast MRAM copy bench
    log << "CPU->broadcast MRAM copy of " << MRAM_BUFFER_DWORDS
        << " uint32_ts nanosecond times:" << std::endl;
    for (size_t run_size = 2; run_size <= MRAM_BUFFER_DWORDS; run_size *= 2) {
      log << run_size;
      for (int _try = 0; _try < 8; _try++) {
        uint64_t start_time = 0, end_time = 0, delta = 0;
        full_mem_fence();
        reset_cpu_tsc();
        full_mem_fence();
        start_time = get_cpu_tsc();
        full_mem_fence();
        DPU_ASSERT(dpu_broadcast_to(this->dpuset, "mram_buffer_a", 0,
                                    cpu_buffer, run_size * sizeof(uint32_t),
                                    DPU_XFER_DEFAULT));
        full_mem_fence();
        end_time = get_cpu_tsc();
        full_mem_fence();
        delta = end_time - start_time;
        log << "," << delta;
      }
      log << std::endl;
    }
    log << "\n\n";
    // CPU -> chunks MRAM copy bench
    log << "CPU->chunks MRAM copy of " << MRAM_BUFFER_DWORDS
        << " uint32_ts nanosecond times:" << std::endl;
    for (size_t run_size = 2; run_size <= MRAM_BUFFER_DWORDS; run_size *= 2) {
      log << run_size;
      for (int _try = 0; _try < 8; _try++) {
        uint64_t start_time = 0, end_time = 0, delta = 0;
        full_mem_fence();
        reset_cpu_tsc();
        full_mem_fence();
        start_time = get_cpu_tsc();
        full_mem_fence();
        {
          dpu_set_t dpu;
          uint32_t dpu_id;
          DPU_FOREACH(this->dpuset, dpu, dpu_id) {
            DPU_ASSERT(dpu_prepare_xfer(
                dpu, &this->cpu_buffer[dpu_id * MRAM_BUFFER_DWORDS]));
          }
          DPU_ASSERT(dpu_push_xfer(
              this->dpuset, DPU_XFER_TO_DPU, "mram_buffer_a", 0,
              MRAM_BUFFER_DWORDS * sizeof(uint32_t), DPU_XFER_DEFAULT));
        }
        full_mem_fence();
        end_time = get_cpu_tsc();
        full_mem_fence();
        delta = end_time - start_time;
        log << "," << delta;
      }
      log << std::endl;
    }
    // chunks MRAM -> CPU copy bench
    log << "Chunks MRAM->CPU copy of " << MRAM_BUFFER_DWORDS
        << " uint32_ts nanosecond times:" << std::endl;
    for (size_t run_size = 2; run_size <= MRAM_BUFFER_DWORDS; run_size *= 2) {
      log << run_size;
      for (int _try = 0; _try < 8; _try++) {
        uint64_t start_time = 0, end_time = 0, delta = 0;
        full_mem_fence();
        reset_cpu_tsc();
        full_mem_fence();
        start_time = get_cpu_tsc();
        full_mem_fence();
        {
          dpu_set_t dpu;
          uint32_t dpu_id;
          DPU_FOREACH(this->dpuset, dpu, dpu_id) {
            DPU_ASSERT(dpu_prepare_xfer(
                dpu, &this->cpu_buffer[dpu_id * MRAM_BUFFER_DWORDS]));
          }
          DPU_ASSERT(dpu_push_xfer(
              this->dpuset, DPU_XFER_FROM_DPU, "mram_buffer_a", 0,
              MRAM_BUFFER_DWORDS * sizeof(uint32_t), DPU_XFER_DEFAULT));
        }
        full_mem_fence();
        end_time = get_cpu_tsc();
        full_mem_fence();
        delta = end_time - start_time;
        log << "," << delta;
      }
      log << std::endl;
    }
  }

private:
  dpu_set_t dpuset;
  uint32_t dpu_count = 0;
  size_t buffer_size = 4 * 1024, min_size = 64;

  void scramble_buffers(ptrdiff_t upto = 0) {
    assert(cpu_buffer != nullptr);
    auto max_sz = static_cast<ptrdiff_t>(std::max(
        this->buffer_size / sizeof(uint32_t), size_t(MRAM_BUFFER_DWORDS)));
    if (upto <= 0 || upto > max_sz) {
      upto = max_sz;
    }
    std::minstd_rand rnd; // fast RNG
    rnd.seed(static_cast<unsigned long>(time(
        nullptr))); // seed doesn't really matter, so just use time to make sure
                    // there's no way the compiler can predict the values
    std::generate_n(cpu_buffer, upto, rnd);
  }
};

std::shared_ptr<MemspeedBenchmark> make_upmemdpu_benchmark() {
  return std::make_shared<UpmemDpuBenchmark>();
}

class DpuBenchFixture : public benchmark::Fixture {
public:
  dpu_set_t dpu_set;
  uint32_t dpu_count, dpu_rank_count;

  using benchmark::Fixture::SetUp;
  using benchmark::Fixture::TearDown;

  virtual void SetUp(const ::benchmark::State &state) override {
    dpu_counts();
    dpu_count = uint32_t(state.range(1));
    DPU_ASSERT(
        dpu_alloc((this->dpu_count == 0) ? DPU_ALLOCATE_ALL : this->dpu_count,
                  nullptr, &this->dpu_set));
    DPU_ASSERT(dpu_get_nr_dpus(this->dpu_set, &this->dpu_count));
    DPU_ASSERT(dpu_get_nr_ranks(this->dpu_set, &this->dpu_rank_count));
  }

  virtual void TearDown(const ::benchmark::State &_state) override {
    DPU_ASSERT(dpu_free(this->dpu_set));
  }

  static const std::vector<uint32_t>& dpu_counts() {
    static std::vector<uint32_t> v = []() {
      std::vector<uint32_t> vv;
      dpu_set_t dpuset;
      uint32_t dpucount;
      DPU_ASSERT(dpu_alloc(DPU_ALLOCATE_ALL, nullptr, &dpuset));
      DPU_ASSERT(dpu_get_nr_dpus(dpuset, &dpucount));
      DPU_ASSERT(dpu_free(dpuset));
      std::cerr << "DPUs available on the system: " << dpucount << std::endl;
      for (uint32_t n = 1; n <= dpucount; n *= 2) {
        vv.push_back(n);
      }
      return vv;
    }();
    return v;
  }
};

template<uint32_t MaxMemArg>
static void DpuArgsMemsizeDpucount(benchmark::internal::Benchmark* b) {
  b->ArgNames({"dwords", "dpus"});
  for(uint32_t mem_arg = 8; mem_arg <= MaxMemArg; mem_arg*=2) {
    for(const auto dpu_count: DpuBenchFixture::dpu_counts()) {
      b->Args({mem_arg, dpu_count});
    }
  }
}

BENCHMARK_DEFINE_F(DpuBenchFixture, BM_dpu_copy_cpu_to_wram_broadcast)(benchmark::State &state) {
  DPU_ASSERT(dpu_load(dpu_set, "../dpusrc/ramcopy.dpuelf", nullptr));
  size_t len_arg = size_t(state.range(0));
  MmapArray<uint32_t> cpubuf{len_arg};
  std::minstd_rand rnd; // fast RNG
  rnd.seed(2817398);
  std::generate_n(cpubuf.data(), cpubuf.size(), rnd);
  for (auto _ : state) {
    DPU_ASSERT(dpu_broadcast_to(dpu_set, "wram_buffer_a", 0,
                                    cpubuf.data(), cpubuf.size(),
                                    DPU_XFER_DEFAULT));
    benchmark::ClobberMemory();
  }
  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(cpubuf.size_in_bytes()));
  state.counters["memused"] = benchmark::Counter(
      double(cpubuf.size_in_bytes()),
      benchmark::Counter::Flags::kDefaults, benchmark::Counter::kIs1024);
  state.counters["dpu_ranks"] = benchmark::Counter(
      double(dpu_rank_count),
      benchmark::Counter::Flags::kDefaults, benchmark::Counter::kIs1024);
}
BENCHMARK_REGISTER_F(DpuBenchFixture, BM_dpu_copy_cpu_to_wram_broadcast)->Apply(DpuArgsMemsizeDpucount<WRAM_BUFFER_DWORDS>);

BENCHMARK_DEFINE_F(DpuBenchFixture, BM_dpu_copy_cpu_to_mram_broadcast)(benchmark::State &state) {
  DPU_ASSERT(dpu_load(dpu_set, "../dpusrc/ramcopy.dpuelf", nullptr));
  size_t len_arg = size_t(state.range(0));
  MmapArray<uint32_t> cpubuf{len_arg};
  std::minstd_rand rnd; // fast RNG
  rnd.seed(2817398);
  std::generate_n(cpubuf.data(), cpubuf.size(), rnd);
  for (auto _ : state) {
    DPU_ASSERT(dpu_broadcast_to(dpu_set, "mram_buffer_a", 0,
                                    cpubuf.data(), cpubuf.size(),
                                    DPU_XFER_DEFAULT));
    benchmark::ClobberMemory();
  }
  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(cpubuf.size_in_bytes()));
  state.counters["memused"] = benchmark::Counter(
      double(cpubuf.size_in_bytes()),
      benchmark::Counter::Flags::kDefaults, benchmark::Counter::kIs1024);
  state.counters["dpu_ranks"] = benchmark::Counter(
      double(dpu_rank_count),
      benchmark::Counter::Flags::kDefaults, benchmark::Counter::kIs1024);
}
BENCHMARK_REGISTER_F(DpuBenchFixture, BM_dpu_copy_cpu_to_mram_broadcast)->Apply(DpuArgsMemsizeDpucount<MRAM_BUFFER_DWORDS>);

