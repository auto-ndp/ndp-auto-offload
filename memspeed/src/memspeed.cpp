#include <algorithm>
#include <cstdio>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "memspeed.h"

int main(int argc, char **argv) {
  fprintf(stderr, " --- Memspeed starting\n");
  auto benchmarks = memspeed_benchmarks();
  // parse args
  std::vector<std::string_view> passthru_args;
  passthru_args.reserve(8);
  std::unordered_set<benchmark_ptr_t> enabled_benchmarks;
  enabled_benchmarks.reserve(4);
  bool wants_help{false};
  bool arg_errors{false};
  for (int argi = 1; argi < argc; argi++) {
    std::string_view arg{argv[argi]};
    if (arg == "-help") {
      wants_help = true;
    } else if (std::string_view pfx = "-bench="; arg.starts_with(pfx)) {
      std::string_view argval = arg.substr(pfx.size());
      auto bench = std::find_if(
          benchmarks.cbegin(), benchmarks.cend(),
          [&argval](const benchmark_ptr_t &b) { return b->name() == argval; });
      if (bench == benchmarks.cend()) {
        arg_errors = true;
        fprintf(stderr, "ERROR: Could not find benchmark: %.*s\n",
                static_cast<int>(argval.size()), argval.data());
      }
      enabled_benchmarks.insert(*bench);
    } else {
      passthru_args.push_back(arg);
    }
  }
  if (enabled_benchmarks.size() == 0) {
    wants_help = true;
  }
  if (wants_help || arg_errors) {
    fprintf(stderr, "Usage: %s [-bench=a] [-bench=b] [-...]\n",
            (argc > 0) ? argv[0] : "memspeed");
    fprintf(stderr, "Options available:\n -help - shows this help text\n "
                    "-bench=BENCH - enables the benchmark BENCH\n");
    for (const auto &bench : benchmarks) {
      fprintf(stderr, "\nOptions for benchmark `%s`:\n", bench->name().c_str());
      bench->print_help();
    }
    return arg_errors ? 1 : 0;
  }
  // run benchmarks
  for (const auto &bench : enabled_benchmarks) {
    fprintf(stderr, " --- Preparing benchmark %s\n", bench->name().c_str());
    for (const auto &arg : passthru_args) {
      bench->parse_arg(arg);
    }
    bench->prepare(std::cout);
    fprintf(stderr, " --- Running benchmark %s\n", bench->name().c_str());
    bench->run(std::cout);
    fprintf(stderr, " --- Finished benchmark %s\n", bench->name().c_str());
  }
  return 0;
}
