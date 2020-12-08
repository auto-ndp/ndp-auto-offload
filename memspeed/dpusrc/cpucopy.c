#include <defs.h>
#include <mram.h>
#include <stdint.h>

#include "../include/dpuramcopy.h"

__dma_aligned __host uint32_t wram_buffer_a[2 * WRAM_BUFFER_DWORDS];

__mram_noinit uint32_t mram_buffer_a[2 * MRAM_BUFFER_DWORDS];

int main() {
  // make sure the symbols are actually used so they aren't erased from the program
  wram_buffer_a[2] = wram_buffer_a[1];
  mram_buffer_a[2] = mram_buffer_a[1];
  return 0;
}
