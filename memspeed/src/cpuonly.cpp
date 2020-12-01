#include "memspeed.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <memory>
#include <random>
#include <sys/mman.h>

using namespace std::string_literals;

class CpuOnlyBenchmark : public MemspeedBenchmark {
public:
  size_t min_size = 2 * sizeof(uint32_t);
  size_t buffer_size = 4 * 1024 * 1024;
  volatile uint32_t *src_buffer = nullptr, *dst_buffer = nullptr;

  virtual ~CpuOnlyBenchmark() {
    if (src_buffer != nullptr) {
      munmap(const_cast<uint32_t *>(src_buffer), this->buffer_size);
      src_buffer = nullptr;
    }
    if (dst_buffer != nullptr) {
      munmap(const_cast<uint32_t *>(dst_buffer), this->buffer_size);
      dst_buffer = nullptr;
    }
  }

  std::string name() final { return "cpuonly"s; }
  virtual void parse_arg(std::string_view arg) final {
    if (std::string_view pfx = "-maxsize="; starts_with(arg, pfx)) {
      std::string_view argval = arg.substr(pfx.size());
      this->buffer_size = parseSize(std::string(argval)) / 8 * 8;
    }
    if (std::string_view pfx = "-minsize="; starts_with(arg, pfx)) {
      std::string_view argval = arg.substr(pfx.size());
      this->min_size = parseSize(std::string(argval)) / 8 * 8;
    }
  }
  virtual void print_help() final {
    fprintf(stderr,
            " -maxsize=4M - specifies maximum buffer size for speed tests\n"
            " -minsize=8  - specifies the first buffer size tested\n");
  }
  virtual void prepare(std::ostream &log) final {
    log << "Allocating buffers of size: " << formatSize(this->buffer_size)
        << " bytes" << std::endl;
    src_buffer = const_cast<volatile uint32_t *>(static_cast<uint32_t *>(
        mmap(nullptr, this->buffer_size, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0)));
    if (src_buffer == MAP_FAILED || src_buffer == nullptr) {
      perror("Couldn't allocate src_buffer");
      exit(1);
    }
    dst_buffer = const_cast<volatile uint32_t *>(static_cast<uint32_t *>(
        mmap(nullptr, this->buffer_size, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0)));
    if (dst_buffer == MAP_FAILED || dst_buffer == nullptr) {
      perror("Couldn't allocate dst_buffer");
      exit(1);
    }
    log << "Filling buffers with random data" << std::endl;
    scramble_buffers();
  }
  virtual void run(std::ostream &log) final {
    assert(src_buffer != nullptr);
    assert(dst_buffer != nullptr);
    for (size_t run_size = this->min_size; run_size <= this->buffer_size;
         run_size *= 2) {
      size_t run_size_ints = run_size / sizeof(uint32_t);
      log << " Size = " << formatSize(run_size) << std::endl;
      uint64_t start_time = 0, end_time = 0, delta = 0;
      double dbl_delta = 0.0;
      double dbl_run_size = static_cast<double>(run_size);
      size_t reps = 1;
      if (run_size < 32768) {
        reps = 32768 / run_size;
      }
      // Copy trials
      for (int _trial = 0; _trial < 5; _trial++) {
        reset_cpu_tsc();
        full_mem_fence();
        start_time = get_cpu_tsc();
        full_mem_fence();
        for (size_t _c = 0; _c < reps; _c++) {
          // std::copy_n(src_buffer, run_size_ints, dst_buffer);
          volatile uint32_t *src = src_buffer, *dst = dst_buffer;
          for (size_t _i = 0; _i < run_size_ints; _i++) {
            *dst = *src;
            src++;
            dst++;
          }
          full_mem_fence();
        }
        end_time = get_cpu_tsc();
        full_mem_fence();
        delta = (end_time - start_time) / reps;
        log << "  Copy trial time: " << delta << std::endl;
        dbl_delta = static_cast<double>(delta) * 1.0e-9; // in seconds
        log << "  Copy average per-byte time: "
            << 1.0e9 * dbl_delta / dbl_run_size << "ns" << std::endl;
        log << "  Copy throughput: "
            << formatSize(static_cast<size_t>(dbl_run_size / dbl_delta)) << "/s"
            << std::endl;
      }
      // Latency (list traversal) trials
      for (int _trial = 0; _trial < 5; _trial++) {
        reset_cpu_tsc();
        full_mem_fence();
        start_time = get_cpu_tsc();
        full_mem_fence();
        size_t pos = static_cast<size_t>(_trial);
        for (size_t _c = 0; _c < reps; _c++) {
          for (size_t els = 0; els < run_size_ints; els++) {
            pos = src_buffer[pos] & (run_size_ints - 1);
          }
        }
        full_mem_fence();
        end_time = get_cpu_tsc();
        full_mem_fence();
        delta = (end_time - start_time) / reps;
        log << "  Listwalk trial time: " << delta << "  # endpos = " << pos
            << std::endl;
        dbl_delta = static_cast<double>(delta) * 1.0e-9; // in seconds
        log << "  Listwalk average per-jump time: "
            << 1.0e9 * dbl_delta / static_cast<double>(run_size_ints) << "ns"
            << std::endl;
        log << "  Listwalk throughput: "
            << formatSize(static_cast<size_t>(dbl_run_size / dbl_delta) /
                          sizeof(uint32_t))
            << "/s" << std::endl;
      }
    }
  }

private:
  void scramble_buffers(ptrdiff_t upto = 0) {
    assert(src_buffer != nullptr);
    assert(dst_buffer != nullptr);
    auto max_sz = static_cast<ptrdiff_t>(this->buffer_size / sizeof(uint32_t));
    if (upto <= 0 || upto > max_sz) {
      upto = max_sz;
    }
    std::minstd_rand rnd; // fast RNG
    rnd.seed(static_cast<unsigned long>(time(
        nullptr))); // seed doesn't really matter, so just use time to make sure
                    // there's no way the compiler can predict the values
    std::generate_n(src_buffer, upto, rnd);
    std::generate_n(dst_buffer, upto, rnd);
  }
};

std::shared_ptr<MemspeedBenchmark> make_cpuonly_benchmark() {
  return std::make_shared<CpuOnlyBenchmark>();
}
