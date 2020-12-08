#pragma once

// DRAM
#define COPYMODE_MRAM 1
// SRAM (Data CCM)
#define COPYMODE_WRAM 2

#define MRAM_BUFFER_DWORDS 8388608
#define WRAM_BUFFER_DWORDS 6144

struct dpuramcopy_config_t {
  uint32_t cfg_copy_mode;
  uint32_t cfg_copy_size;
  uint32_t cfg_copy_repetitions;
  uint32_t cfg_cache_size; // size of wram_buffer_a in uint32_ts used when copying over blocks of mram memory
};
