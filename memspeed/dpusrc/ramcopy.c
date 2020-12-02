#include <defs.h>
#include <mram.h>
#include <stdint.h>

#include "../include/dpuramcopy.h"

__host struct dpuramcopy_config_t program_config;

__dma_aligned __host uint32_t wram_buffer_a[WRAM_BUFFER_DWORDS];
__dma_aligned __host uint32_t wram_buffer_b[WRAM_BUFFER_DWORDS];

__mram_noinit uint32_t mram_buffer_a[MRAM_BUFFER_DWORDS];
__mram_noinit uint32_t mram_buffer_b[MRAM_BUFFER_DWORDS];

int main() {
  if (program_config.cfg_copy_mode == COPYMODE_WRAM) {
    uint32_t *read_end = wram_buffer_a + program_config.cfg_copy_size;
    for (uint32_t _rep = 0; _rep < program_config.cfg_copy_repetitions;
         _rep++) {
      uint32_t *read_begin = wram_buffer_a + me();
      uint32_t *write_begin = wram_buffer_b + me();
      for (; read_begin < read_end;
           read_begin += NR_TASKLETS, write_begin += NR_TASKLETS) {
        *write_begin = *read_begin;
      }
    }
  } else if (program_config.cfg_copy_mode == COPYMODE_MRAM) {
    mram_read(mram_buffer_a, wram_buffer_a, 8);
    mram_read(mram_buffer_b, wram_buffer_b, 8);
  } else {
    return 1;
  }
  return 0;
}
