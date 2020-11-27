#include "memspeed.h"

using namespace std::string_literals;

class CpuOnlyBenchmark : public MemspeedBenchmark {
  std::string name() final { return "cpuonly"s; }
  virtual void parse_arg(std::string_view arg) final {
    //
  }
  virtual void print_help() final {
    //
  }
  virtual void prepare(std::ostream &log) final {
    //
  }
  virtual void run(std::ostream &log) final {
    //
  }
};

std::shared_ptr<MemspeedBenchmark> make_cpuonly_benchmark() {
  return std::make_shared<CpuOnlyBenchmark>();
}
