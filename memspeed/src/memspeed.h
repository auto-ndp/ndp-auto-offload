#pragma once

#include <ios>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class MemspeedBenchmark {
public:
  inline virtual ~MemspeedBenchmark() {}
  virtual std::string name() = 0;
  virtual void parse_arg(std::string_view arg) = 0;
  virtual void print_help() = 0;
  virtual void prepare(std::ostream &log) = 0;
  virtual void run(std::ostream &log) = 0;
};

using benchmark_ptr_t = std::shared_ptr<MemspeedBenchmark>;

std::vector<benchmark_ptr_t> memspeed_benchmarks();
