#include <defs.h>
#include <mram.h>
#include <stdint.h>

#include "../include/dpuramcopy.h"

__host uint64_t cycles;

__host uint32_t run_repetitions;
__host uint32_t copy_words_amount;
__dma_aligned __host uint32_t buffer_a[WRAM_BUFFER_DWORDS];
__dma_aligned __host uint32_t buffer_b[WRAM_BUFFER_DWORDS];

int main() {
  uint32_t *read_end = buffer_a + copy_words_amount;
  for (uint32_t _rep = 0; _rep < run_repetitions;
        _rep++) {
    uint32_t *read_begin = buffer_a + me();
    uint32_t *write_begin = buffer_b + me();
    for (; read_begin < read_end;
          read_begin += NR_TASKLETS, write_begin += NR_TASKLETS) {
      *write_begin = *read_begin;
    }
  }
  return 0;
}
