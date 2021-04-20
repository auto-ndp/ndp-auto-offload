#include <cstdio>
#include <vector>

#include "stublib.h"

int main() {
  pinnearmap_phase("main-start");
  std::vector<int> buffer(1024, 54321123);
  pinnearmap_phase("buffer-1024");
  buffer.insert(buffer.end(), 16384-1024, 1234567);
  pinnearmap_phase("buffer-16384");
  buffer.resize(512);
  buffer.shrink_to_fit();
  pinnearmap_phase("buffer-512");
  return 0;
}
