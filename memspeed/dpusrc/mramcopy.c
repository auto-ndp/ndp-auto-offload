#include <defs.h>
#include <mram.h>
#include <stdint.h>

#include "../include/dpuramcopy.h"

#define MAX_BLOCKSIZE 64

__host uint32_t run_repetitions;
__host uint32_t copy_words_amount;

__dma_aligned __host uint32_t wram_cache[MAX_BLOCKSIZE * NR_TASKLETS];

__mram_noinit uint32_t buffer_a[MRAM_BUFFER_DWORDS];
__mram_noinit uint32_t buffer_b[MRAM_BUFFER_DWORDS];

int main() {
  uint32_t blocksize = MAX_BLOCKSIZE;
  uint32_t stride = NR_TASKLETS * blocksize;
  uint32_t __mram_ptr *read_end = buffer_a + copy_words_amount;
  for (uint32_t _rep = 0; _rep < run_repetitions; _rep++) {
    uint32_t *my_cache = wram_cache + me() * MAX_BLOCKSIZE;
    uint32_t __mram_ptr *read_begin = buffer_a + me() * blocksize;
    uint32_t __mram_ptr *write_begin = buffer_b + me() * blocksize;
    for (; read_begin < read_end; read_begin += stride, write_begin += stride) {
      mram_read(read_begin, wram_cache, blocksize);
      mram_write(wram_cache, write_begin, blocksize);
    }
  }
  return 0;
}
