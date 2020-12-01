#include <defs.h>
#include <mram.h>
#include <stdint.h>

#include "../include/dpuramcopy.h"

uint32_t cfg_copy_mode = COPYMODE_MRAM;
uint32_t cfg_copy_size = 1;
uint32_t cfg_copy_repetitions = 1;
uint32_t cfg_cache_size = 1; // size of wram_buffer_a in uint32_ts used when copying over blocks of mram memory

uint32_t wram_buffer_a[WRAM_BUFFER_DWORDS];
uint32_t wram_buffer_b[WRAM_BUFFER_DWORDS];

__mram_noinit uint32_t mram_buffer_a[MRAM_BUFFER_DWORDS];
__mram_noinit uint32_t mram_buffer_b[MRAM_BUFFER_DWORDS];

int main() {
  if (cfg_copy_mode == COPYMODE_WRAM) {
    uint32_t* read_end = wram_buffer_a + cfg_copy_size;
    for (uint32_t _rep = 0; _rep < cfg_copy_repetitions; _rep++) {
      uint32_t* read_begin = wram_buffer_a + me();
      uint32_t* write_begin = wram_buffer_b + me();
      for(; read_begin < read_end; read_begin += NR_TASKLETS, write_begin += NR_TASKLETS) {
        *write_begin = *read_begin;
      }
    }
  } else if (cfg_copy_mode == COPYMODE_MRAM) {
    //
  } else {
    return 1;
  }
  return 0;
}
