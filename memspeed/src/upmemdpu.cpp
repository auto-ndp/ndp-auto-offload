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

using namespace std::string_literals;
using namespace std::chrono;

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
