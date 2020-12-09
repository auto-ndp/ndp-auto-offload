#include "memspeed.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>

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
    (void)_state;
    DPU_ASSERT(dpu_free(this->dpu_set));
  }

  static const std::vector<uint32_t> &dpu_counts() {
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

template <uint32_t MaxMemArg>
static void DpuCopyCpuArgs(benchmark::internal::Benchmark *b) {
  b->ArgNames({"dwords", "dpus", "broadcast", "to_dpu", "mram"});
  for (uint32_t mem_arg = 2; mem_arg <= MaxMemArg; mem_arg *= 2)
    for (const auto dpu_count : DpuBenchFixture::dpu_counts())
      for (const auto broadcast : {0, 1})
        for (const auto mram : {0, 1}) {
          if (!mram && mem_arg > 2 * WRAM_BUFFER_DWORDS) {
            continue;
          }
          b->Args({mem_arg, dpu_count, broadcast, 1, mram});
          if (!broadcast) { // allow only non-broadcast reads
            b->Args({mem_arg, dpu_count, broadcast, 0, mram});
          }
        }
}

BENCHMARK_DEFINE_F(DpuBenchFixture, BM_dpu_copy_between_cpu_dpu)
(benchmark::State &state) {
  DPU_CHECK(dpu_load(dpu_set, "../dpubin/cpucopy.dpuelf", nullptr),
            throw std::runtime_error("DPU load error"));
  size_t len_arg = size_t(state.range(0));
  bool broadcast = bool(state.range(2));
  bool to_dpu = bool(state.range(3));
  bool use_mram = bool(state.range(4));
  const char *target_symbol_name = use_mram ? "mram_buffer_a" : "wram_buffer_a";
  size_t total_len = broadcast ? len_arg : len_arg * dpu_count;
  MmapArray<uint32_t> cpubuf{total_len};
  std::minstd_rand rnd; // fast RNG
  rnd.seed(2817398);
  std::generate_n(cpubuf.data(), cpubuf.size(), rnd);
  if (broadcast) {
    for (auto _ : state) {
      DPU_CHECK(dpu_broadcast_to(dpu_set, target_symbol_name, 0, cpubuf.data(),
                                 cpubuf.size_in_bytes(), DPU_XFER_DEFAULT),
                throw std::runtime_error("DPU broadcast error"));
      benchmark::ClobberMemory();
    }
  } else {
    for (auto _ : state) {
      dpu_set_t dpu;
      uint32_t dpu_id;
      DPU_FOREACH(this->dpu_set, dpu, dpu_id) {
        DPU_CHECK(dpu_prepare_xfer(dpu, &cpubuf[dpu_id * len_arg]),
                  throw std::runtime_error("DPU prepare xfer error"));
      }
      DPU_CHECK(dpu_push_xfer(dpu_set,
                              to_dpu ? DPU_XFER_TO_DPU : DPU_XFER_FROM_DPU,
                              target_symbol_name, 0, len_arg * sizeof(uint32_t),
                              DPU_XFER_DEFAULT),
                throw std::runtime_error("DPU push xfer error"));
      benchmark::ClobberMemory();
    }
  }
  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(cpubuf.size_in_bytes()));
  state.counters["memused"] = benchmark::Counter(
      double(cpubuf.size_in_bytes()), benchmark::Counter::Flags::kDefaults,
      benchmark::Counter::kIs1024);
  state.counters["dpu_ranks"] = benchmark::Counter(
      double(dpu_rank_count), benchmark::Counter::Flags::kDefaults,
      benchmark::Counter::kIs1024);
}
BENCHMARK_REGISTER_F(DpuBenchFixture, BM_dpu_copy_between_cpu_dpu)
    ->Apply(DpuCopyCpuArgs<2 * MRAM_BUFFER_DWORDS>)->UseRealTime();

template <int32_t MaxMemArg>
static void DpuCopyDpuArgs(benchmark::internal::Benchmark *b) {
  b->ArgNames({"dwords", "dpus", "dpu_reps", "dpu_threads", "mram"});
  for (int32_t mem_arg = 2; mem_arg <= MaxMemArg; mem_arg *= 2)
    for (const auto dpu_count : DpuBenchFixture::dpu_counts())
      for (uint32_t dpu_reps = 1; dpu_reps < 256; dpu_reps *= 8)
        for (int32_t dpu_threads : {1, 2, 4, 8, 12, 16, 24})
          if (dpu_threads <= mem_arg) {
            b->Args({mem_arg, dpu_count, dpu_reps, dpu_threads, 1});
            if (mem_arg <= WRAM_BUFFER_DWORDS) {
              b->Args({mem_arg, dpu_count, dpu_reps, dpu_threads, 0});
            }
          }
}

BENCHMARK_DEFINE_F(DpuBenchFixture, BM_dpu_copy_within)
(benchmark::State &state) {
  bool use_mram = bool(state.range(4));
  uint32_t dpu_threads = uint32_t(state.range(3));
  std::stringstream fname;
  fname << (use_mram ? "../dpubin/mramcopy." : "../dpubin/wramcopy.")
        << dpu_threads << ".dpuelf";
  DPU_CHECK(dpu_load(dpu_set, fname.str().c_str(), nullptr),
            throw std::runtime_error("DPU load error"));
  size_t len_arg = size_t(state.range(0));
  uint32_t in_dpu_reps = uint32_t(state.range(2));
  MmapArray<uint32_t> cpubuf{len_arg * 2};
  std::minstd_rand rnd; // fast RNG
  rnd.seed(2817398);
  std::generate_n(cpubuf.data(), cpubuf.size(), rnd);
  DPU_CHECK(dpu_broadcast_to(dpu_set, "buffer_a", 0, cpubuf.data(),
                             cpubuf.size_in_bytes() / 2, DPU_XFER_DEFAULT),
            throw std::runtime_error("DPU broadcast error"));
  DPU_CHECK(dpu_broadcast_to(dpu_set, "buffer_b", 0,
                             cpubuf.data() + cpubuf.size_in_bytes() / 2,
                             cpubuf.size_in_bytes() / 2, DPU_XFER_DEFAULT),
            throw std::runtime_error("DPU broadcast error"));
  DPU_CHECK(dpu_broadcast_to(dpu_set, "run_repetitions", 0, &in_dpu_reps, 4,
                             DPU_XFER_DEFAULT),
            throw std::runtime_error("DPU broadcast error"));
  uint32_t len_arg_u32 = uint32_t(len_arg);
  DPU_CHECK(dpu_broadcast_to(dpu_set, "copy_words_amount", 0, &len_arg_u32, 4,
                             DPU_XFER_DEFAULT),
            throw std::runtime_error("DPU broadcast error"));
  for (auto _ : state) {
    DPU_CHECK(dpu_launch(dpu_set, DPU_SYNCHRONOUS),
              throw std::runtime_error("DPU launch error"));
    benchmark::ClobberMemory();
  }
  state.SetBytesProcessed(int64_t(state.iterations()) *
                          int64_t(cpubuf.size_in_bytes() / 2) *
                          int64_t(in_dpu_reps));
  state.counters["memused"] = benchmark::Counter(
      double(cpubuf.size_in_bytes()), benchmark::Counter::Flags::kDefaults,
      benchmark::Counter::kIs1024);
  state.counters["dpu_ranks"] = benchmark::Counter(
      double(dpu_rank_count), benchmark::Counter::Flags::kDefaults,
      benchmark::Counter::kIs1024);
}
BENCHMARK_REGISTER_F(DpuBenchFixture, BM_dpu_copy_within)
    ->Apply(DpuCopyDpuArgs<MRAM_BUFFER_DWORDS>)->UseRealTime();
