#include "memspeed.h"

#include <cassert>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <memory>
#include <random>
#include <sys/mman.h>

using namespace std::string_literals;

class UpmemDpuBenchmark : public MemspeedBenchmark {
public:
  std::string name() final { return "upmem"s; }
  virtual void parse_arg(std::string_view arg) final {
    /*if (std::string_view pfx = "-maxsize="; starts_with(arg, pfx)) {
      std::string_view argval = arg.substr(pfx.size());
      this->buffer_size = parseSize(std::string(argval)) / 8 * 8;
    }
    if (std::string_view pfx = "-minsize="; starts_with(arg, pfx)) {
      std::string_view argval = arg.substr(pfx.size());
      this->min_size = parseSize(std::string(argval)) / 8 * 8;
    }*/
  }
  virtual void print_help() final {
    /*fprintf(stderr,
            " -maxsize=4M - specifies maximum buffer size for speed tests\n"
            " -minsize=8  - specifies the first buffer size tested\n");*/
  }
  virtual void prepare(std::ostream &log) final {
    //
  }
  virtual void run(std::ostream &log) final {
    //
  }

private:
  //
};

std::shared_ptr<MemspeedBenchmark> make_upmemdpu_benchmark() {
  return std::make_shared<UpmemDpuBenchmark>();
}
